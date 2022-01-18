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


void ContextOopDesc::initWithMethod(ObjectMemory &omem, Oop receiver,
    MethodOop aMethod)
{
	size_t heapVarsSize = aMethod->heapVarsSize().smi();

	m_size = ContextOopDesc::clsNstLength + aMethod->stackSize().smi() + 1;
	setIsa(ObjectMemory::clsContext);
	setBytecode(aMethod->bytecode());
	setReceiver(receiver);
	setMethodOrBlock(aMethod.as<OopOop>());
	setHeapVars(heapVarsSize ?
		      ArrayOopDesc::newWithSize(omem, heapVarsSize) :
		      Oop().as<ArrayOop>());
	setProgramCounter(SmiOop((intptr_t)0));

	reg0() = receiver;
}

void
ContextOopDesc::initWithBlock(ObjectMemory &omem, BlockOop aMethod)
{
	size_t heapVarsSize = aMethod->heapVarsSize().smi();

	m_size = ContextOopDesc::clsNstLength + aMethod->stackSize().smi() + 1;
	setIsa(ObjectMemory::clsContext);
	setBytecode(aMethod->bytecode());
	setReceiver(aMethod->receiver());
	setMethodOrBlock(aMethod.as<OopOop>());
	setHeapVars(heapVarsSize ?
		      ArrayOopDesc::newWithSize(omem, heapVarsSize) :
		      Oop().as<ArrayOop>());
	setParentHeapVars(aMethod->parentHeapVars());
	setProgramCounter(SmiOop((intptr_t)0));
	setHomeMethodContext(aMethod->homeMethodContext());
	reg0() = aMethod->receiver();

}

ContextOop
ContextOopDesc::newWithMethod(ObjectMemory &omem, Oop receiver,
    MethodOop aMethod)
{
	ContextOop ctx = omem.newOopObj<ContextOop>(clsNstLength + aMethod->stackSize().smi() + 1);
	ctx->initWithMethod(omem, receiver, aMethod);
	return ctx;
}

ContextOop
ContextOopDesc::newWithBlock(ObjectMemory &omem, BlockOop aMethod)
{
	ContextOop ctx = omem.newOopObj<ContextOop>(clsNstLength + aMethod->stackSize().smi() + 1);
	ctx->initWithBlock(omem, aMethod);
	return ctx;
}

bool ContextOopDesc::isBlockContext()
{
	return methodOrBlock().isa() == ObjectMemory::clsBlock;
}

static inline MethodOop
lookupMethod(ProcessOop proc, Oop receiver, ClassOop cls,
    SymbolOop selector)
{
	MethodOop meth;

	if (!cls->methods().isNil())
		meth = cls->methods()->symbolLookup(selector).as<MethodOop>();

	if (meth.isNil()) {
		ClassOop super = cls->superClass();
		if (super.isNil() || super == cls)
			return MethodOop();
		else
			return lookupMethod(proc, receiver, super, selector);
	} else
		return meth;
}

#define CTX proc->context()
#define HEAPVAR(x) proc->context()->heapVars()->basicAt(x)
#define PARENTHEAPVAR(x) proc->context()->parentHeapVars()->basicAt(x)
#define NSTVAR(x) proc->context()->receiver().as<OopOop>()->basicAt(x)
#define RECEIVER proc->context()->receiver()
#define PC proc->context()->programCounter()
#define METHODCLASS                  					       \
	((CTX->isBlockContext()) ?    					       \
	CTX->homeMethodContext()->methodOrBlock().as<MethodOop>()    	       \
	    ->methodClass() :    					       \
	CTX->methodOrBlock().as<MethodOop>()->methodClass())

#define FETCH (*pc++)
#define SPILL() {								\
	CTX->programCounter() = pc - &CTX->bytecode()->basicAt0(0);		\
	CTX->accumulator() = SmiOop(ac);					\
}
#define UNSPILL() {								\
	pc = &CTX->bytecode()->basicAt0(0)+ CTX->programCounter().smi();	\
	regs = &proc->context()->reg0();					\
	lits = &proc->context()->methodOrBlock().as<MethodOop>()->literals()->	\
	    basicAt0(0);							\
}

#define TESTCOUNTER() if (counter > 10000) return 0;

#define NEWCTX() size_t newSI = proc->stackIndex().smi() + CTX->size();		\
	ContextOop ctx = (void*)&proc->stack()->basicAt(newSI);			\
	proc->stackIndex() = SmiOop(newSI + 2);					\
	ctx->previousContext() = CTX;						\
	//printf("%d/NEW STACKINDEX IS %ld\n", in, proc->stackIndex().smi());

#define LOOKUPCACHED(RCVR, CLS, CACHE, METH_OUT) {				\
	if (!CACHE->method().isNil() && CACHE->cls() == CLS &&			\
	    CACHE->version().smi() == 5) {					\
		METH_OUT = CACHE->method();					\
	} else {								\
		METH_OUT = lookupMethod(proc, RCVR, CLS, CACHE->selector());	\
		CACHE->method() = METH_OUT;					\
		CACHE->cls() = CLS;						\
		CACHE->version() = 5;						\
	}									\
}

int
execute(ObjectMemory &omem, ProcessOop proc)
{
	Oop ac;
	uint8_t * pc = &CTX->bytecode()->basicAt0(0);
	Oop * regs;
	Oop * lits;
	volatile int counter = 0;

	disassemble(CTX->bytecode()->vns(), CTX->methodOrBlock().as<MethodOop>()->bytecode()->size());

	UNSPILL();

	loop:
	switch((Op::Opcode) FETCH) {
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
			block->parentHeapVars() = proc->context()->heapVars();
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
			throw std::runtime_error("UNIMPLEMENTED StaGlobal");
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
				MethodOop meth = lookupMethod(proc, arg1,
				    arg1.isa(), ObjectMemory::symBin[op]);

				assert(!meth.isNil());

				NEWCTX();
				ctx->initWithMethod(omem, arg1, meth);
				ctx->regAt0(1) = arg2;

				SPILL();
				CTX = ctx;
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
			MethodOop meth;

			LOOKUPCACHED(ac, cls, cache, meth);

			assert(!meth.isNil());

			NEWCTX();
			ctx->initWithMethod(omem, ac, meth);

			for (int i = 0; i < nArgs; i++)
				ctx->regAt0(i + 1) = regs[FETCH];

			nsends++;

			//std::cout << blanks(in) << "=> " <<
			//cls->name()->asCStr() << ">>" <<
			//    cache->selector()->asCStr() << "\n";
			SPILL();
			CTX = ctx;
			UNSPILL();
			IN;

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

			LOOKUPCACHED(ac, cls, cache, meth);
			assert(!meth.isNil());

			NEWCTX();
			ctx->initWithMethod(omem, ac, meth);

			for (int i = 0; i < nArgs; i++) {
				ctx->regAt0(i + 1) = regs[FETCH];
			}

			//std::cout << blanks(in) << "=> " <<
			//    cls->name()->asCStr() << ">>" <<
			//    lits[selIdx].as<SymbolOop>()->asCStr() << "\n";
			SPILL();
			CTX = ctx;
			UNSPILL();
			IN;
			nsends++;

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
			proc->stackIndex() = proc->stackIndex().smi() - (CTX->size() + 2);
			//printf("%lu/RET STACKINDEX IS %ld\n", in - 1, proc->stackIndex().smi());
			CTX = CTX->previousContext();

			//std::cout << blanks(in) << "Returning\n";
			if (CTX.isNil()) {
				std::cout << "Made " << nsends << " message sends.\n";
				std::cout << "Maximum context depth " << maxin <<"\n";
				std::cout << "Completed evaluation with result:\n";
				ac.print(5);
				return 0;
			}
			UNSPILL();
			OUT;
			break;
		}

		case Op::kReturnSelf:{
			TESTCOUNTER();
			ac = RECEIVER;
			proc->stackIndex() = proc->stackIndex().smi() - (CTX->size() + 2);
			//printf("%lu/RET STACKINDEX IS %ld\n", in - 1, proc->stackIndex().smi());
			CTX = CTX->previousContext();

			//std::cout << blanks(in) << "Returning\n";
			if (CTX.isNil()) {
				std::cout << "Did " << nsends << " message sends.\n";
				std::cout << "Maximum context depth " << maxin <<"\n";
				std::cout << "Completed evaluation with result:\n";
				ac.print(2);
				return 0;
			}
			UNSPILL();
			OUT;
			break;
		}

		case Op::kBlockReturn: {
			TESTCOUNTER();
			MethodOop meth = lookupMethod(proc,
			    CTX->methodOrBlock(), CTX->methodOrBlock().isa(),
			    SymbolOopDesc::fromString(omem, "nonLocalReturn:"));

#if 0
			assert(!meth.isNil());

			SPILL();
			ContextOop ctx = ContextOopDesc::newWithMethod(omem,
			    CTX->methodOrBlock(), meth);
			ctx->previousContext() = CTX;

			ctx->regAt0(1) = ac;

			//std::cout << blanks(in) <<"Beginning block return.\n";
			CTX = ctx;
			UNSPILL();
#else
			abort();
#endif
			break;
		}
	} /* switch (op) */
	goto loop;
}