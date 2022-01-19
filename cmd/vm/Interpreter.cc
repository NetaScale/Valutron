#include <cassert>
#include <iostream>
#include <unistd.h>

#include "Interpreter.hh"
#include "Misc.hh"
#include "ObjectMemory.hh"
#include "Oops.hh"
#include "Generation.hh"

size_t in = 0, maxin = 0;
uint64_t nsends = 0;

#define IN in++; if (in > maxin) maxin = in
#define OUT in--


void ContextOopDesc::initWithMethod(ObjectMemory &omem, Oop aReceiver,
    MethodOop aMethod)
{
	size_t heapVarsSize = aMethod->heapVarsSize().smi();

	m_size = ContextOopDesc::clsNstLength + aMethod->stackSize().smi() +
	    sizeof(MemOopDesc) / sizeof(Oop);
	setIsa(ObjectMemory::clsContext);
	bytecode = aMethod->bytecode();
	methodOrBlock = aMethod.as<OopOop>();
	heapVars = heapVarsSize ?
		      ArrayOopDesc::newWithSize(omem, heapVarsSize) :
		      Oop().as<ArrayOop>();
	programCounter = (intptr_t)0;

	reg0 = aReceiver;
	for (int i = 1; i < aMethod->stackSize().smi(); i++)
		regAt0(i) = Oop::nil();
}

void
ContextOopDesc::initWithBlock(ObjectMemory &omem, BlockOop aMethod)
{
	size_t heapVarsSize = aMethod->heapVarsSize().smi();

	m_size = ContextOopDesc::clsNstLength + aMethod->stackSize().smi() +
	    sizeof(MemOopDesc) / sizeof(Oop);
	setIsa(ObjectMemory::clsContext);
	bytecode = aMethod->bytecode();
	methodOrBlock = aMethod.as<OopOop>();
	heapVars = heapVarsSize ?
		      ArrayOopDesc::newWithSize(omem, heapVarsSize) :
		      Oop().as<ArrayOop>();
	parentHeapVars = aMethod->parentHeapVars();
	programCounter = (intptr_t)0;
	homeMethodContext = aMethod->homeMethodContext();
	reg0 = aMethod->receiver();
	for (int i = 1; i < aMethod->stackSize().smi(); i++) 
		regAt0(i) = Oop::nil();
}

ContextOop
ContextOopDesc::newWithMethod(ObjectMemory &omem, Oop receiver,
    MethodOop aMethod)
{
	ContextOop ctx = omem.newOopObj<ContextOop>(clsNstLength + aMethod->stackSize().smi());
	ctx->initWithMethod(omem, receiver, aMethod);
	return ctx;
}

ContextOop
ContextOopDesc::newWithBlock(ObjectMemory &omem, BlockOop aMethod)
{
	ContextOop ctx = omem.newOopObj<ContextOop>(clsNstLength + aMethod->stackSize().smi());
	ctx->initWithBlock(omem, aMethod);
	return ctx;
}

//#define CALLTRACE
//#define STACKDEPTHTRACE
//#define TRACEDISPATCH

bool ContextOopDesc::isBlockContext()
{
	return methodOrBlock.isa() == ObjectMemory::clsBlock;
}

static inline MethodOop
lookupMethod(Oop receiver, ClassOop startCls, SymbolOop selector)
{
	ClassOop cls = startCls;
	MethodOop meth;

	if (!cls->methods().isNil())
		meth = cls->methods()->symbolLookup(selector).as<MethodOop>();

	if (meth.isNil()) {
		cls = cls->superClass();
		if (cls.isNil() || cls == startCls) {
#ifdef CALLTRACE
			std::cout << "Failed to find method " <<
			    selector->asCStr() << "in class " <<
			    receiver.isa()->name()->asCStr() << "\n";
#endif
			return MethodOop();
		}
		else
			return lookupMethod(receiver, cls, selector);
	} else {
#ifdef CALLTRACE
		std::cout << receiver.isa()->name()->asCStr() << "(" <<
		    cls->name()->asCStr() << ")>>" << selector->asCStr() <<
		    "\n";
		disassemble(meth->bytecode()->vns(), meth->bytecode()->size());
#endif
		return meth;
	}
}

#define CTX proc->context
#define HEAPVAR(x) proc->context->heapVars->basicAt(x)
#define PARENTHEAPVAR(x) proc->context->parentHeapVars->basicAt(x)
#define NSTVAR(x) proc->context->reg0.as<OopOop>()->basicAt(x)
#define RECEIVER proc->context->reg0
#define PC proc->context->programCounter
#define METHODCLASS                  					       \
	((CTX->isBlockContext()) ?    					       \
	CTX->homeMethodContext->methodOrBlock.as<MethodOop>()    	       \
	    ->methodClass() :    					       \
	CTX->methodOrBlock.as<MethodOop>()->methodClass())

#define FETCH (*pc++)
#define SPILL() {								\
	CTX->programCounter = pc - &CTX->bytecode->basicAt0(0);			\
	proc->accumulator = ac;							\
}
#define UNSPILL() {								\
	pc = &CTX->bytecode->basicAt0(0) + CTX->programCounter.smi();		\
	regs = &proc->context->reg0;						\
	lits = &proc->context->method()->literals()->basicAt0(0);		\
}

/**
 * Test if the major operations counter (sends, primitives, branches, etc)
 * exceeds the given timeslice; if so, spills registers and returns 1.
 *
 * n.b. extraneous setting of ac in proc object seems to enhance performance.
 */
#define TESTCOUNTER() if (counter++ > timeslice) {				\
	pc--; SPILL(); proc->accumulator = ac; return 1; 			\
}

/**
 * Returns a pointer to a new context within a process stack, and sets the
 * process' stackIndex to the appropriate new value.
 *
 * n.b. does NOT set process' context pointer to the new context!
 */
static inline ContextOop
newContext(ProcessOop &proc)
{
	size_t newSI = proc->stackIndex.smi() + proc->context->fullSize();
	ContextOop newCtx = (void *)&proc->stack->basicAt(newSI);
	proc->stackIndex = SmiOop(newSI);
	newCtx->previousContext = proc->context;
	return newCtx;
}

/**
 * Given an Oop receiver to look up within, a Class to begin lookup in, and a
 * Cache, looks up a method. Cache is checked and appropriately updated if
 * necessary.
 */
static inline MethodOop
lookupCached(Oop obj, ClassOop cls, CacheOop cache)
{
	if (!cache->method.isNil() && cache->cls == cls &&
	    cache->version.smi() == 5) {
		return cache->method;
	} else {
		MethodOop meth = lookupMethod(obj, cls, cache->selector);
		cache->method = meth;
		cache->cls = cls;
		cache->version = 5;
		return meth;
	}
}

void blockReturn(ProcessOop proc)
{
	ContextOop home = CTX->methodOrBlock.as<BlockOop>()->
	    homeMethodContext();
	/* TODO: execute ifCurtailed: blocks */
	proc->context = home->previousContext;
	if (proc->context.isNil()) {
		return;
	} else {

	proc->stackIndex = ((uintptr_t)&*home -
			    (uintptr_t)&proc->stack->basicAt0(0)) /
			    sizeof(Oop) + 1;
#ifdef STACKDEPTHTRACE
	printf("BLK SI %ld\n", proc->stackIndex.smi());
#endif
	}
}

extern "C" int
execute(ObjectMemory &omem, ProcessOop proc, uintptr_t timeslice) noexcept
{
	Oop ac;
	uint8_t * pc = &CTX->bytecode->basicAt0(0);
	Oop * regs;
	Oop * lits;
	volatile int counter = 0;

	std::cout <<"\n\nInterpreter will now run:\n";
	disassemble(CTX->bytecode->vns(), CTX->bytecode->size());
	std::cout << "\n\n";

	UNSPILL();
	ac = proc->accumulator;

	loop:
	Op::Opcode op = (Op::Opcode)FETCH;
#ifdef TRACEDISPATCH
	printf("DISPATCH %d\n", op);
#endif
	switch((Op::Opcode) op) {
		/* u8 index/reg, u8 dest */
		case Op::kMoveParentHeapVarToMyHeapVars: {
			unsigned src = FETCH, dst = FETCH;
			HEAPVAR(dst) = PARENTHEAPVAR(src);
			break;
		}

		case Op::kMoveMyHeapVarToParentHeapVars: {
			unsigned src = FETCH, dst = FETCH;
			PARENTHEAPVAR(dst) = HEAPVAR(src);
			break;
		}

		case Op::kLdaNil:
			ac = Oop();
			break;

		case Op::kLdaTrue:
			ac = ObjectMemory::objTrue;
			break;

		case Op::kLdaFalse:
			ac = ObjectMemory::objFalse;
			break;

		case Op::kLdaThisContext:
			ac = CTX;
			break;

		case Op::kLdaSmalltalk:
			ac = ObjectMemory::objsmalltalk;
			break;

		/* u8 index*/
		case Op::kLdaParentHeapVar: {
			unsigned src = FETCH;
			ac = PARENTHEAPVAR(src);
			break;
		}

		case Op::kLdaMyHeapVar: {
			unsigned src = FETCH;
			ac = HEAPVAR(src);
			break;
		}

		case Op::kLdaGlobal: {
			unsigned src = FETCH;
			ac = omem.objGlobals->symbolLookup(lits[src].as<SymbolOop>());
			break;
		}

		case Op::kLdaNstVar: {
			unsigned src = FETCH;
			ac = NSTVAR(src);
			break;
		}

		case Op::kLdaLiteral: {
			unsigned src = FETCH;
			ac = lits[src];
			break;
		}

		case Op::kLdaBlockCopy: {
			unsigned src = FETCH;
			BlockOop block = omem.copyObj <BlockOop> (lits[src].
			    as<MemOop>());
			block->parentHeapVars() = proc->context->heapVars;
			block->receiver() = RECEIVER;
			block->homeMethodContext() = CTX;
			ac = block;
			break;
		}

		case Op::kLdar: {
			unsigned src = FETCH;
			ac = regs[src];
			break;
		}

		/* u8 index */
		case Op::kStaNstVar: {
			unsigned dst = FETCH;
			NSTVAR(dst) = ac;
			break;
		}

		case Op::kStaGlobal: {
			unsigned dst = FETCH;
			printf("UNIMPLEMENTED StaGlobal\n");
			abort();
			break;
		}

		case Op::kStaParentHeapVar: {
			unsigned dst = FETCH;
			PARENTHEAPVAR(dst) = ac;
			break;
		}

		case Op::kStaMyHeapVar: {
			unsigned dst = FETCH;
			HEAPVAR(dst) = ac;
			break;
		}

		case Op::kStar: {
			unsigned dst = FETCH;
			regs[dst] = ac;
			break;
		}

		case Op::kMove: {
			unsigned dst = FETCH, src = FETCH;
			regs[dst] = regs[src];
			break;
		}

		/* a value, u8 src-reg */
		case Op::kAnd: {
			unsigned src = FETCH;
			if (ac == ObjectMemory::objTrue && regs[src] == ObjectMemory::objTrue)
			{
				//printf("TRUE\n");
			}
			break;
		}

		/* a value, i16 pc-offset */
		case Op::kJump: {
			uint8_t b1 = FETCH;
			uint8_t b2 = FETCH;
			int16_t offs = (b1 << 8) | b2;
			//std::cout << "Jumping " << offs << "\n";
			pc = pc + offs;
			break;
		}

		/* a value, i16 pc-offset */
		case Op::kBranchIfFalse: {
			TESTCOUNTER();
			uint8_t b1 = FETCH;
			uint8_t b2 = FETCH;
			int16_t offs = (b1 << 8) | b2;
			assert (ac == ObjectMemory::objFalse || ac == ObjectMemory::objTrue);
			if(ac == ObjectMemory::objFalse) {
				//std::cout << "Branching " << offs << "for false\n";
				pc = pc + offs;
			}
			break;
		}

		/* a value, i16 pc-offset */
		case Op::kBranchIfTrue: {
			TESTCOUNTER();
			uint8_t b1 = FETCH;
			uint8_t b2 = FETCH;
			int16_t offs = (b1 << 8) | b2;
			assert (ac == ObjectMemory::objFalse || ac == ObjectMemory::objTrue);
			if(ac == ObjectMemory::objTrue) {
				//std::cout << "Branching " << offs << "for true\n";
				pc = pc + offs;
			}
			break;
		}

		case Op::kBinOp: {
			TESTCOUNTER();
			uint8_t src = FETCH;
			uint8_t op = FETCH;
			Oop arg1 = regs[src];
			Oop arg2 = ac;

			ac = Primitive::primitives[op].fn2(omem, proc,arg1,
			    arg2);
			if (ac.isNil()) {
				MethodOop meth = lookupMethod(arg1, arg1.isa(),
				    ObjectMemory::symBin[op]);
				ContextOop newCtx;

				assert(!meth.isNil());

				newCtx = newContext(proc);
				newCtx->initWithMethod(omem, arg1, meth);
				newCtx->regAt0(1) = arg2;

				SPILL();
				CTX = newCtx;
				UNSPILL();
				IN;
			}
			break;
		}

		/**
		 * a receiver, u8 selector-literal-index, u8 num-args,
		 *     (u8 arg-register)+, ->a result
		 */
		case Op::kSend: {
			TESTCOUNTER();
			unsigned selIdx = FETCH, nArgs = FETCH;
			CacheOop cache = lits[selIdx].as<CacheOop>();
			ClassOop cls = ac.isa();
			MethodOop meth = lookupCached(ac, cls, cache);
			ContextOop newCtx;

			assert(!meth.isNil());

			newCtx = newContext(proc);
			newCtx->initWithMethod(omem, ac, meth);

			for (int i = 0; i < nArgs; i++) {
				Oop obj = regs[FETCH];
				//obj.print(16); FIXME:
				newCtx->regAt0(i + 1) = obj;
			}

			nsends++;

			//std::cout << blanks(in) << "=> " <<
			//cls->name()->asCStr() << ">>" <<
			//    cache->selector()->asCStr() << "\n";
			SPILL();
			CTX = newCtx;
			UNSPILL();
			IN;
#ifdef STACKDEPTHTRACE
			std::cout << "SI=" << newSI << " after send\n";
#endif
			//disassemble(meth->bytecode()->vns(), meth->bytecode()->size());
			//sleep(1);

			break;
		}

		/**
		 * a receiver, u8 selector-literal-index, u8 num-args,
		 *     (u8 arg-register)+, ->a result
		 */
		case Op::kSendSuper: {
			TESTCOUNTER();
			unsigned selIdx = FETCH, nArgs = FETCH;
			ClassOop cls = METHODCLASS->superClass();
			CacheOop cache = lits[selIdx].as<CacheOop>();
			MethodOop meth;
			ContextOop newCtx;

			meth = lookupCached(ac, cls, cache);
			assert(!meth.isNil());

			newCtx = newContext(proc);
			newCtx->initWithMethod(omem, ac, meth);

			for (int i = 0; i < nArgs; i++) {
				newCtx->regAt0(i + 1) = regs[FETCH];
			}

			//std::cout << blanks(in) << "=> " <<
			//    cls->name()->asCStr() << ">>" <<
			//    lits[selIdx].as<SymbolOop>()->asCStr() << "\n";
			SPILL();
			CTX = newCtx;
			UNSPILL();
			IN;
			nsends++;
#ifdef STACKDEPTHTRACE
			std::cout << "SI=" << newSI << " after send\n";
#endif

			break;
		}

		/** u8 prim-num, u8 num-args, (u8 arg-reg)+ */
		case Op::kPrimitive: {
			TESTCOUNTER();
			unsigned prim = FETCH, nArgs = FETCH;
			ArrayOop args = ArrayOopDesc::newWithSize(omem, nArgs);

			//printf("PRIMITIVE %s\n", Primitive::primitives[prim].name);

			for (int i = 0; i < nArgs; i++) 
				args->basicAt0(i) = regs[FETCH];

			SPILL();
			ac = Primitive::primitives[prim].fnp(omem, proc, args);
			UNSPILL();
			break;
		}

		/** a arg, u8 prim-num */
		case Op::kPrimitive1: {
			TESTCOUNTER();
			unsigned prim = FETCH;
			SPILL();
			ac = Primitive::primitives[prim].fn1(omem, proc, ac);
			UNSPILL();
			break;
		}

		/** a arg2, u8 prim-num, u8 arg1-reg */
		case Op::kPrimitive2: {
			TESTCOUNTER();
			unsigned prim = FETCH, arg1reg = FETCH;
			SPILL();
			ac = Primitive::primitives[prim].fn2(omem, proc,
			    regs[arg1reg], ac);
			UNSPILL();
			break;
		}

		/** a arg3, u8 prim-num, u8 arg1-reg */
		case Op::kPrimitive3: {
			TESTCOUNTER();
			unsigned prim = FETCH, arg1reg = FETCH;
			SPILL();
			ac = Primitive::primitives[prim].fn3(omem, proc,
			    regs[arg1reg], regs[arg1reg + 1], ac);
			UNSPILL();
			break;
		}

		case Op::kPrimitiveV: {
			TESTCOUNTER();
			unsigned prim = FETCH, nArgs = FETCH, arg1reg = FETCH;
			SPILL();
			ac = Primitive::primitives[prim].fnv(omem, proc, nArgs,
			    &regs[arg1reg]);
			UNSPILL();
			break;
		}

		case Op::kReturn: {
			TESTCOUNTER();
			size_t newSI = ((uintptr_t)&*CTX->previousContext -
			    (uintptr_t)&proc->stack->basicAt0(0)) /
			    sizeof(Oop) + 1;
			proc->stackIndex = newSI;
			CTX = CTX->previousContext;

			if (CTX.isNil()) {
				std::cout << "Made " << nsends << " message sends.\n";
				std::cout << "Maximum context depth " << maxin <<"\n";
				std::cout << "Completed evaluation with result:\n";
				ac.print(5);
				return 0;
			} else {
#ifdef CALLTRACE
				std::cout <<"RETURN TO: ";
				std::cout << RECEIVER.isa()->name()->asCStr() << ">>";
				if (proc->context->isBlockContext())
		 		std::cout << proc->context->homeMethodContext->methodOrBlock.as<MethodOop>()->selector()->asCStr() << "(Block)\n";
				else
		 		std::cout << proc->context->methodOrBlock.as<MethodOop>()->selector()->asCStr() << "\n";

#endif
#ifdef STACKDEPTHTRACE
				std::cout << "SI=" << newSI << " after ret \n";
#endif
			}
			UNSPILL();
			OUT;
			break;
		}

		case Op::kReturnSelf:{
			TESTCOUNTER();
			ac = RECEIVER;
			size_t newSI = ((uintptr_t)&*CTX->previousContext -
			    (uintptr_t)&proc->stack->basicAt0(0)) /
			    sizeof(Oop) + 1;
			proc->stackIndex = newSI;
			
			CTX = CTX->previousContext;

			//std::cout << blanks(in) << "Returning\n";
			if (CTX.isNil()) {
				std::cout << "Did " << nsends << " message sends.\n";
				std::cout << "Maximum context depth " << maxin <<"\n";
				std::cout << "Completed evaluation with result:\n";
				ac.print(2);
				return 0;
			} else {
#ifdef CALLTRACE
				std::cout <<"RETURN TO: " << proc->context->methodOrBlock.as<MethodOop>()->selector()->asCStr() << "\n";
#endif
#ifdef STACKDEPTHTRACE
				std::cout << "SI=" << newSI << "after ret \n";
#endif
			}
			UNSPILL();
			OUT;
			break;
		}

		case Op::kBlockReturn: {
			TESTCOUNTER();
			SPILL();
			blockReturn(proc);
			if (CTX.isNil()) {
				ac.print(2);
				return 0;
			}
			UNSPILL();
			break;
		}
	} /* switch (op) */
	goto loop;

}