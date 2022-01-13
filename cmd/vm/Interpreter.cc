#include <cassert>
#include <iostream>

#include "Interpreter.hh"
#include "Misc.hh"
#include "ObjectMemory.hh"
#include "Oops.hh"
#include "Generation.hh"

#define FETCH proc->context()->fetchByte()
#define CTX proc->context()
#define HEAPVAR(x) proc->context()->heapVars()->basicAt(x)
#define PARENTHEAPVAR(x) proc->context()->parentHeapVars()->basicAt(x)
#define LITERAL(x) \
	proc->context()->methodOrBlock().as<MethodOop>()->literals()->basicAt0(x)
#define NSTVAR(x) proc->context()->receiver().as<OopOop>()->basicAt(x)
#define REG(x) proc->context()->stack()->basicAt0(x)
#define RECEIVER proc->context()->receiver()

size_t in = 0;

ContextOop
ContextOopDesc::newWithMethod(ObjectMemory &omem, Oop receiver,
    MethodOop aMethod)
{
	ContextOop ctx = omem.newOopObj<ContextOop>(clsNstLength);
	size_t argCount, tempSize, heapVarsSize;

	argCount = aMethod->argumentCount().smi();
	tempSize = aMethod->temporarySize().smi();
	heapVarsSize = aMethod->heapVarsSize().smi();

	ctx.setIsa(ObjectMemory::clsContext);
	ctx->setBytecode(aMethod->bytecode());
	ctx->setReceiver(receiver);
	ctx->setMethodOrBlock(aMethod.as<OopOop>());
	ctx->setHeapVars(heapVarsSize ?
		      ArrayOopDesc::newWithSize(omem, heapVarsSize) :
		      Oop().as<ArrayOop>());
	ctx->setStack(ArrayOopDesc::newWithSize(omem,
	    aMethod->stackSize().smi() + 1 /* nil pushed */));
	ctx->setProgramCounter(SmiOop((intptr_t)0));
	ctx->setStackPointer(SmiOop((intptr_t)0));

	ctx->stack()->basicAt0(0) = receiver;

	return ctx;
}

ContextOop
ContextOopDesc::newWithBlock(ObjectMemory &omem, BlockOop aMethod)
{
	ContextOop ctx = omem.newOopObj<ContextOop>(clsNstLength);
	size_t argCount, tempSize, heapVarsSize;

	argCount = aMethod->argumentCount().smi();
	tempSize = aMethod->temporarySize().smi();
	heapVarsSize = aMethod->heapVarsSize().smi();

	ctx.setIsa(ObjectMemory::clsContext);
	ctx->setBytecode(aMethod->bytecode());
	ctx->setReceiver(aMethod->receiver());
	ctx->setMethodOrBlock(aMethod.as<OopOop>());
	ctx->setHeapVars(heapVarsSize ?
		      ArrayOopDesc::newWithSize(omem, heapVarsSize) :
		      Oop().as<ArrayOop>());
	ctx->setParentHeapVars(aMethod->parentHeapVars());
	ctx->setStack(ArrayOopDesc::newWithSize(omem,
	    aMethod->stackSize().smi() + 1 /* nil pushed */));
	ctx->setProgramCounter(SmiOop((intptr_t)0));
	ctx->setStackPointer(SmiOop((intptr_t)0));

	ctx->stack()->basicAt0(0) = aMethod->receiver();

	return ctx;
}

inline bool ContextOopDesc::isBlockContext()
{
	return methodOrBlock().isa() == ObjectMemory::clsBlock;
}

inline uint8_t ContextOopDesc::fetchByte ()
{
    int64_t pos = programCounter().smi() + 1;
    programCounter() = SmiOop(pos);
    std::cout << blanks(in) << "Fetching from pos " << pos <<"\n";
    return bytecode ()->basicAt (pos);
}

static inline MethodOop lookupMethodInClass (
    ProcessOop proc, Oop receiver, ClassOop cls, SymbolOop selector, bool super)
{
    ClassOop lookupClass = super ? cls->superClass () : cls;
    MethodOop meth;

    printf (" -> Begin search for %s in class %s\n", selector->asCStr(), lookupClass->name ()->asCStr ());

    if (!lookupClass->methods ().isNil ())
    {
        meth = lookupClass->methods ()->symbolLookup (selector).as<MethodOop> ();
    }
    else
        printf (" -> Class %s has blank methods table\n ",
                 lookupClass->name ()->asCStr ());

    if (meth.isNil ())
    {
        ClassOop super;
        if (((super = lookupClass->superClass ()) == Oop ()) ||
            (super == lookupClass))
        {
            ContextOop ctx = proc->context ();
            printf (" -> Failed to find method %s in class %s\n",
                     selector->asCStr (),
                     cls->name ()->asCStr ());
            printf ("          --> %s>>%s\n",
                     ctx->receiver ().isa ()->name ()->asCStr (),
                     ctx->isBlockContext () ? "<block>"
                                            : ctx->methodOrBlock ()
                                                  .as<MethodOop> ()
                                                  ->selector ()
                                                  ->asCStr ());
            while ((ctx = ctx->previousContext ()) != Oop ())
                printf ("          --> %s>>%s\n",
                         ctx->receiver ().isa ()->name ()->asCStr (),
                         ctx->isBlockContext () ? "<block>"
                                                : ctx->methodOrBlock ()
                                                      .as<MethodOop> ()
                                                      ->selector ()
                                                      ->asCStr ());
            abort ();
        }
        else
        {
            printf (
                " -> DID NOT find method %s in class %s, searching super\n",
                selector->asCStr (),
                cls->name ()->asCStr ());
            return lookupMethodInClass (proc, receiver, super, selector, false);
        }
    }

    return meth;
}

int
execute(ObjectMemory &omem, ProcessOop proc)
{
	uint8_t op;
	Oop ac;

	loop:
	op = FETCH;

	std::cout << blanks(in) << "Execute Op " << (unsigned)op <<"\n";

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
			ac = omem.objGlobals->symbolLookup(LITERAL(src).as<SymbolOop>());
			break;
		}

		case Op::kLdaNstVar: {
			unsigned src = FETCH;
			ac = NSTVAR(src);
			break;
		}

		case Op::kLdaLiteral: {
			unsigned src = FETCH;
			ac = LITERAL(src);
			break;
		}

		case Op::kLdaBlockCopy: {
			unsigned src = FETCH;
			BlockOop block = omem.copyObj <BlockOop> (LITERAL(src).
			    as<MemOop>());
			block->parentHeapVars() = proc->context()->heapVars();
			block->receiver() = RECEIVER;
			block->homeMethodContext() = CTX;
			ac = block;
			break;
		}

		case Op::kLdar: {
			unsigned src = FETCH;
			std::cout << blanks(in) << "LDAR " << src << "\n";
			ac = REG(src);
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
			REG(dst) = ac;
			break;
		}

		case Op::kMove: {
			unsigned dst = FETCH, src = FETCH;
			REG(dst) = REG(src);
			break;
		}

		/**
		 * u8 dest-reg, u8 receiver-reg, u8 selector-lit-idx,
		 * u8 num-args, (u8 arg-reg)+
		 */
		case Op::kSend: {
			unsigned selIdx = FETCH, nArgs = FETCH;
			MethodOop meth = lookupMethodInClass(proc, ac, ac.isa(),
			    LITERAL(selIdx).as<SymbolOop>(), false);

			assert(!meth.isNil());

			ContextOop ctx = ContextOopDesc::newWithMethod(omem, ac, meth);
			ctx->previousContext() = CTX;

			for (int i = 0; i < nArgs; i++) {
				ctx->stack()->basicAt0(i + 1) = REG(FETCH);
			}

			std::cout << blanks(in) <<"Beginning send.\n";
			CTX = ctx;
			in++;

			disassemble(meth->bytecode()->vns(), meth->bytecode()->size());

			break;
		}

		/**
		 * u8 dest-reg, u8 selector-literal-index, u8 num-args,
		 * (u8 arg-register)+
		 */
		case Op::kSendSuper: {
			unsigned selIdx = FETCH, nArgs = FETCH;
			throw std::runtime_error("Unimplemented kSendSuper");
			break;
		}

		/** u8 prim-num, u8 num-args, (u8 arg-reg)+ */
		case Op::kPrimitive: {
			unsigned prim = FETCH, nArgs = FETCH;
			ArrayOop args = ArrayOopDesc::newWithSize(omem, nArgs);

			std::cout << blanks(in) << "Invoke primitive " <<
			    prim << " with " << nArgs << " args\n";
			for (int i = 0; i < nArgs; i++) {
				args->basicAt0(i) = REG(FETCH);
			}

			ac = primVec[prim](omem, proc, args);

			//throw std::runtime_error("Unimplemented kPrimitive");
			break;
		}

		case Op::kReturnSelf: {
			Oop rval = RECEIVER;
			proc->context() = proc->context()->previousContext();
			ac = rval;
			break;
		}

		/* u8 source-register */
		case Op::kReturn: {
			CTX = CTX->previousContext();
			std::cout << blanks(in--) << "Returning\n";
			if (CTX.isNil()) {
				std::cout << "Completed evaluation with result:\n";
				ac.print(5);
				return 0;
			}
			break;
		}

		case Op::kBlockReturn: {
			throw std::runtime_error("Unimplemented kBlockReturn");
			break;
		}
	} /* switch (op) */

	goto loop;
}