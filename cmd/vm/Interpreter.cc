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

	m_size = ContextOopDesc::clsNstLength + aMethod->stackSize().smi() - 1;
	m_hash = ObjectMemory::getHashCode();
	m_kind = kStackAllocatedContext;
	setIsa(ObjectMemory::clsContext);
	bytecode = aMethod->bytecode();
	methodOrBlock = aMethod.as<OopOop>();
	homeMethodContext = ContextOop::nil();
	heapVars = heapVarsSize ?
		      ArrayOopDesc::newWithSize(omem, heapVarsSize) :
		      ArrayOop::nil();
	parentHeapVars = ArrayOop::nil();
	programCounter = (intptr_t)0;

	reg0 = aReceiver;
	for (int i = 1; i < aMethod->stackSize().smi(); i++)
		regAt0(i) = Oop::nil();
}

void
ContextOopDesc::initWithBlock(ObjectMemory &omem, BlockOop aMethod)
{
	size_t heapVarsSize = aMethod->heapVarsSize().smi();

	m_size = ContextOopDesc::clsNstLength + aMethod->stackSize().smi() - 1;
	m_hash = ObjectMemory::getHashCode();
	m_kind = kStackAllocatedContext;
	setIsa(ObjectMemory::clsContext);
	bytecode = aMethod->bytecode();
	methodOrBlock = aMethod.as<OopOop>();
	heapVars = heapVarsSize ?
		      ArrayOopDesc::newWithSize(omem, heapVarsSize) :
		      ArrayOop::nil();
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

	if (!cls->methods.isNil())
		meth = cls->methods->symbolLookup(selector).as<MethodOop>();

	if (meth.isNil()) {
		cls = cls->superClass;
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
#define TESTCOUNTER() if (counter++ > timeslice) goto timesliceDone

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
	proc->context->setIsa(ClassOop::nil());
	proc->context = home->previousContext;
	if (proc->context.isNil())
		return;
	else
		proc->stackIndex = stackIndexOfContext(home, proc->stack);
}


extern "C" int
execute(ObjectMemory &omem, ProcessOop proc, uintptr_t timeslice) noexcept
{
	static void* opTable[] = {
#define X(OP) &&op##OP,
		OPS
#undef X
	};
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

	#define DISPATCH goto *opTable[FETCH]
	loop:
	DISPATCH;
	/* u8 index/reg, u8 dest */
	opMoveParentHeapVarToMyHeapVars : {
		unsigned src = FETCH, dst = FETCH;
		HEAPVAR(dst) = PARENTHEAPVAR(src);
		DISPATCH;
	}

	opMoveMyHeapVarToParentHeapVars : {
		unsigned src = FETCH, dst = FETCH;
		PARENTHEAPVAR(dst) = HEAPVAR(src);
		DISPATCH;
	}

	opLdaNil:
		ac = Oop();
		DISPATCH;

	opLdaTrue:
		ac = ObjectMemory::objTrue;
		DISPATCH;

	opLdaFalse:
		ac = ObjectMemory::objFalse;
		DISPATCH;

	opLdaThisContext:
		ac = CTX;
		DISPATCH;

	opLdaSmalltalk:
		ac = ObjectMemory::objsmalltalk;
		DISPATCH;

	/* u8 index*/
	opLdaParentHeapVar : {
		unsigned src = FETCH;
		ac = PARENTHEAPVAR(src);
		DISPATCH;
	}

	opLdaMyHeapVar : {
		unsigned src = FETCH;
		ac = HEAPVAR(src);
		DISPATCH;
	}

	opLdaGlobal : {
		unsigned src = FETCH;
		SymbolOop name = lits[src].as<SymbolOop>();
		ac = omem.objGlobals->symbolLookup(name);
		DISPATCH;
	}

	opLdaNstVar : {
		unsigned src = FETCH;
		ac = NSTVAR(src);
		DISPATCH;
	}

	opLdaLiteral : {
		unsigned src = FETCH;
		ac = lits[src];
		DISPATCH;
	}

	opLdaBlockCopy : {
		unsigned src = FETCH;
		MemOop constructor = lits[src].as<MemOop>();
		BlockOop block = omem.copyObj<BlockOop>(constructor);
		block->m_kind = MemOopDesc::kBlock;
		block->parentHeapVars() = proc->context->heapVars;
		block->receiver() = RECEIVER;
		block->homeMethodContext() = CTX;
		ac = block;
		DISPATCH;
	}

	opLdar : {
		unsigned src = FETCH;
		ac = regs[src];
		DISPATCH;
	}

	/* u8 index */
	opStaNstVar : {
		unsigned dst = FETCH;
		NSTVAR(dst) = ac;
		DISPATCH;
	}

	opStaGlobal : {
		unsigned dst = FETCH;
		printf("UNIMPLEMENTED StaGlobal\n");
		abort();
		DISPATCH;
	}

	opStaParentHeapVar : {
		unsigned dst = FETCH;
		PARENTHEAPVAR(dst) = ac;
		DISPATCH;
	}

	opStaMyHeapVar : {
		unsigned dst = FETCH;
		HEAPVAR(dst) = ac;
		DISPATCH;
	}

	opStar : {
		unsigned dst = FETCH;
		regs[dst] = ac;
		DISPATCH;
	}

	opMove : {
		unsigned dst = FETCH, src = FETCH;
		regs[dst] = regs[src];
		DISPATCH;
	}

	/* ac value, u8 src-reg */
	opAnd : {
		unsigned src = FETCH;
		if (ac != ObjectMemory::objTrue ||
		    regs[src] != ObjectMemory::objTrue)
			ac = ObjectMemory::objFalse;
		DISPATCH;
	}

	/* ac value, i16 pc-offset */
	opJump : {
		uint8_t b1 = FETCH;
		uint8_t b2 = FETCH;
		int16_t offs = (b1 << 8) | b2;
		pc = pc + offs;
		DISPATCH;
	}

	/* a value, i16 pc-offset */
	opBranchIfFalse : {
		TESTCOUNTER();
		uint8_t b1 = FETCH;
		uint8_t b2 = FETCH;
		int16_t offs = (b1 << 8) | b2;
		assert(ac == ObjectMemory::objFalse ||
		    ac == ObjectMemory::objTrue);
		if (ac == ObjectMemory::objFalse)
			pc = pc + offs;
		DISPATCH;
	}

	/* a value, i16 pc-offset */
	opBranchIfTrue : {
		TESTCOUNTER();
		uint8_t b1 = FETCH;
		uint8_t b2 = FETCH;
		int16_t offs = (b1 << 8) | b2;
		assert(ac == ObjectMemory::objFalse ||
		    ac == ObjectMemory::objTrue);
		if (ac == ObjectMemory::objTrue)
			pc = pc + offs;
		DISPATCH;
	}

	opBinOp : {
		TESTCOUNTER();
		uint8_t src = FETCH;
		uint8_t op = FETCH;
		Oop arg1 = regs[src];
		Oop arg2 = ac;

		ac = Primitive::primitives[op].fn2(omem, proc, arg1, arg2);
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
		DISPATCH;
	}

	/**
	 * a receiver, u8 selector-literal-index, u8 num-args,
	 *     (u8 arg-register)+, ->a result
	 */
	opSend : {
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
		std::cout << blanks(in) << "=> " << cls->nameCStr() << ">>"
			  << cache->selector()->asCStr() << "\n";
#endif
		DISPATCH;
	}

	/**
	 * a receiver, u8 selector-literal-index, u8 num-args,
	 *     (u8 arg-register)+, ->a result
	 */
	opSendSuper : {
		TESTCOUNTER();
		unsigned selIdx = FETCH, nArgs = FETCH;
		ClassOop cls = METHODCLASS->superClass;
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
		std::cout << blanks(in) << "=> " << cls->nameCStr() << ">>"
			  << cache->selector()->asCStr() << "\n";
#endif
		DISPATCH;
	}

	/** u8 prim-num, u8 num-args, (u8 arg-reg)+ */
	opPrimitive : {
		TESTCOUNTER();
		unsigned prim = FETCH, nArgs = FETCH;
		ArrayOop args = ArrayOopDesc::newWithSize(omem, nArgs);

		for (int i = 0; i < nArgs; i++)
			args->basicAt0(i) = regs[FETCH];

		SPILL();
		ac = Primitive::primitives[prim].fnp(omem, proc, args);
		UNSPILL();
		DISPATCH;
	}

	/** ac arg, u8 prim-num */
	opPrimitive1 : {
		TESTCOUNTER();
		unsigned prim = FETCH;
		SPILL();
		ac = Primitive::primitives[prim].fn1(omem, proc, ac);
		UNSPILL();
		DISPATCH;
	}

	/** ac arg2, u8 prim-num, u8 arg1-reg */
	opPrimitive2 : {
		TESTCOUNTER();
		unsigned prim = FETCH, arg1reg = FETCH;
		SPILL();
		ac = Primitive::primitives[prim].fn2(omem, proc, regs[arg1reg],
		    ac);
		UNSPILL();
		DISPATCH;
	}

	/** ac arg3, u8 prim-num, u8 arg1-reg */
	opPrimitive3 : {
		TESTCOUNTER();
		unsigned prim = FETCH, arg1reg = FETCH;
		SPILL();
		ac = Primitive::primitives[prim].fn3(omem, proc, regs[arg1reg],
		    regs[arg1reg + 1], ac);
		UNSPILL();
		DISPATCH;
	}

	opPrimitiveV : {
		TESTCOUNTER();
		unsigned prim = FETCH, nArgs = FETCH, arg1reg = FETCH;
		SPILL();
		ac = Primitive::primitives[prim].fnv(omem, proc, nArgs,
		    &regs[arg1reg]);
		UNSPILL();
		DISPATCH;
	}

	opReturn : {
		TESTCOUNTER();
		size_t newSI = stackIndexOfContext(CTX->previousContext,
		    proc->stack);

		proc->stackIndex = newSI;
		CTX->setIsa(ClassOop::nil());
		CTX = CTX->previousContext;

		if (CTX.isNil()) {
			proc->accumulator = ac;
			return 0;
		} else {
#ifdef CALLTRACE
			std::cout << "Return to " << methodOf(CTX) << "\n";
#endif
		}
		UNSPILL();
		OUT;
		DISPATCH;
	}

	opReturnSelf : {
		TESTCOUNTER();
		ac = RECEIVER;
		size_t newSI = stackIndexOfContext(CTX->previousContext,
		    proc->stack);

		proc->stackIndex = newSI;
		CTX->setIsa(ClassOop::nil());
		CTX = CTX->previousContext;

		if (CTX.isNil()) {
			proc->accumulator = ac;
			return 0;
		} else {
#ifdef CALLTRACE
			std::cout << "Return to " << methodOf(CTX) << "\n";
#endif
		}
		UNSPILL();
		OUT;
		DISPATCH;
	}

	opBlockReturn : {
		TESTCOUNTER();
		SPILL();
		blockReturn(proc);
		if (CTX.isNil()) {
			ac.print(2);
			return 0;
		}
		UNSPILL();
		DISPATCH;
	}

timesliceDone:
	pc--;
	SPILL();
	proc->accumulator = ac;
	return 1;
}