#include <cassert>
#include <cstddef>
#include <iostream>
#include <unistd.h>

#include "Interpreter.hh"
#include "Misc.hh"
#include "ObjectMemory.hh"
#include "Oops.hh"
#include "Generation.hh"

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
		if (cls.isNil() || cls == startCls)
			return MethodOop::nil();
		else
			return lookupMethod(receiver, cls, selector);
	} else
		return meth;
}

#define FETCH (*pc++)

#define CTX proc->context
#define HEAPVAR(x) proc->context->heapVars->basicAt(x)
#define PARENTHEAPVAR(x) proc->context->parentHeapVars->basicAt(x)
#define NSTVAR(x) proc->context->reg0.as<OopOop>()->basicAt(x)
#define RECEIVER proc->context->reg0

/** Fetch the class in which the current method scope was defined. */
#define METHODCLASS								\
	((CTX->isBlockContext()) ?						\
	    CTX->homeMethodContext->method()->methodClass() :			\
	    CTX->methodOrBlock.as<MethodOop>()->methodClass())
/**
 * Spills the contents of the changeable cached members of Context and Process.
 */
#define SPILL() {								\
	CTX->programCounter = pc - &CTX->bytecode->basicAt0(0);			\
	proc->accumulator = ac;							\
}
/**
 * Loads local variables caching frequently-accessed members of Process and
 * Context objects with the values from those objects.
 */
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

#define IN nsends++; in++; if (in > maxin) maxin = in
#define OUT in--

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

/**
 * Given a context and a stack (within which this context is allocated),
 * returns the appropriate stack index (1-based index) for the beginning of
 * that context.
*/
static inline size_t
stackIndexOfContext(ContextOop ctx, ArrayOop stack)
{
	return ((uintptr_t) &*(ctx) - (uintptr_t) & (stack)->basicAt0(0)) /
	    sizeof(Oop) + 1;
}

void blockReturn(ProcessOop proc)
{
	ContextOop home = CTX->methodOrBlock.as<BlockOop>()->
	    homeMethodContext();
	/* TODO: execute ifCurtailed: blocks - maybe fall back to Smalltalk-
	 * implemented block return there? */
	proc->context = home->previousContext;
	if (proc->context.isNil())
		return;
	else
		proc->stackIndex = stackIndexOfContext(home, proc->stack);
}

extern "C" int
execute(ObjectMemory &omem, ProcessOop proc, uintptr_t timeslice) noexcept
{
	uint64_t in = 0, maxin = 0;
	uint64_t nsends = 0;
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
	switch((Op::Opcode)FETCH) {
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
			SymbolOop name = lits[src].as<SymbolOop>();
			ac = omem.objGlobals->symbolLookup(name);
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
			MemOop constructor = lits[src].as<MemOop>();
			BlockOop block = omem.copyObj <BlockOop> (constructor);
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

		/* ac value, u8 src-reg */
		case Op::kAnd: {
			unsigned src = FETCH;
			if (ac != ObjectMemory::objTrue ||
			    regs[src] != ObjectMemory::objTrue)
				ac = ObjectMemory::objFalse;
			break;
		}

		/* ac value, i16 pc-offset */
		case Op::kJump: {
			uint8_t b1 = FETCH;
			uint8_t b2 = FETCH;
			int16_t offs = (b1 << 8) | b2;
			pc = pc + offs;
			break;
		}

		/* a value, i16 pc-offset */
		case Op::kBranchIfFalse: {
			TESTCOUNTER();
			uint8_t b1 = FETCH;
			uint8_t b2 = FETCH;
			int16_t offs = (b1 << 8) | b2;
			assert (ac == ObjectMemory::objFalse ||
			    ac == ObjectMemory::objTrue);
			if(ac == ObjectMemory::objFalse)
				pc = pc + offs;
			break;
		}

		/* a value, i16 pc-offset */
		case Op::kBranchIfTrue: {
			TESTCOUNTER();
			uint8_t b1 = FETCH;
			uint8_t b2 = FETCH;
			int16_t offs = (b1 << 8) | b2;
			assert (ac == ObjectMemory::objFalse ||
			    ac == ObjectMemory::objTrue);
			if(ac == ObjectMemory::objTrue)
				pc = pc + offs;
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
			for (int i = 0; i < nArgs; i++)
				newCtx->regAt0(i + 1) = regs[FETCH];

			SPILL();
			CTX = newCtx;
			UNSPILL();
			IN;
#ifdef TRACE_CALLS
			std::cout << blanks(in) << "=> " << cls->nameCStr() <<
			    ">>" << cache->selector()->asCStr() << "\n";
#endif
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

			SPILL();
			CTX = newCtx;
			UNSPILL();
			IN;
#ifdef TRACE_CALLS
			std::cout << blanks(in) << "=> " << cls->nameCStr() <<
			    ">>" << cache->selector()->asCStr() << "\n";
#endif
			break;
		}

		/** u8 prim-num, u8 num-args, (u8 arg-reg)+ */
		case Op::kPrimitive: {
			TESTCOUNTER();
			unsigned prim = FETCH, nArgs = FETCH;
			ArrayOop args = ArrayOopDesc::newWithSize(omem, nArgs);

			for (int i = 0; i < nArgs; i++)
				args->basicAt0(i) = regs[FETCH];

			SPILL();
			ac = Primitive::primitives[prim].fnp(omem, proc, args);
			UNSPILL();
			break;
		}

		/** ac arg, u8 prim-num */
		case Op::kPrimitive1: {
			TESTCOUNTER();
			unsigned prim = FETCH;
			SPILL();
			ac = Primitive::primitives[prim].fn1(omem, proc, ac);
			UNSPILL();
			break;
		}

		/** ac arg2, u8 prim-num, u8 arg1-reg */
		case Op::kPrimitive2: {
			TESTCOUNTER();
			unsigned prim = FETCH, arg1reg = FETCH;
			SPILL();
			ac = Primitive::primitives[prim].fn2(omem, proc,
			    regs[arg1reg], ac);
			UNSPILL();
			break;
		}

		/** ac arg3, u8 prim-num, u8 arg1-reg */
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
			size_t newSI = stackIndexOfContext(CTX->previousContext,
			    proc->stack);
			proc->stackIndex = newSI;
			CTX = CTX->previousContext;

			if (CTX.isNil()) {
				proc->accumulator = ac;
				return 0;
			} else {
#ifdef CALLTRACE
				std::cout << "Return to " << methodOf(CTX) <<
				    "\n";
#endif
			}
			UNSPILL();
			OUT;
			break;
		}

		case Op::kReturnSelf:{
			TESTCOUNTER();
			ac = RECEIVER;
			size_t newSI = stackIndexOfContext(CTX->previousContext,
			    proc->stack);
			proc->stackIndex = newSI;

			CTX = CTX->previousContext;

			if (CTX.isNil()) {
				proc->accumulator = ac;
				return 0;
			} else {
#ifdef CALLTRACE
				std::cout << "Return to " << methodOf(CTX) <<
				    "\n";
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

timeslice_done:
	proc->accumulator = ac;
	return 1;
}