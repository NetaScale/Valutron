#include <cassert>
#include <iostream>
#include <unistd.h>

#include "Interpreter.hh"
#include "Misc.hh"
#include "ObjectMemory.hh"
#include "Oops.hh"
#include "Generation.hh"

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

	ctx->reg0()->basicAt0(0) = receiver;

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
	ctx->setHomeMethodContext(aMethod->homeMethodContext());

	ctx->reg0()->basicAt0(0) = aMethod->receiver();

	return ctx;
}

bool ContextOopDesc::isBlockContext()
{
	return methodOrBlock().isa() == ObjectMemory::clsBlock;
}

/* The PROBLEM: super is only looking up super wrt to receiver
 * need to lookup super wrt to the current class
 */
static inline MethodOop lookupMethodInClass (
    ProcessOop proc, Oop receiver, ClassOop cls, SymbolOop selector)
{
    ClassOop lookupClass = cls;
    MethodOop meth;

    //printf (" -> Begin search for %s in class %s\n", selector->asCStr(), lookupClass->name ()->asCStr ());

    if (!lookupClass->methods ().isNil ())
    {
        meth = lookupClass->methods ()->symbolLookup (selector).as<MethodOop> ();
    }
   // else
   //     printf (" -> Class %s has blank methods table\n ",
   //              lookupClass->name ()->asCStr ());

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
            /*printf (
                " -> DID NOT find method %s in class %s, searching super\n",
                selector->asCStr (),
                cls->name ()->asCStr ());*/
            return lookupMethodInClass (proc, receiver, super, selector);
	}
    }

    //std::cout << blanks(in) << "=> " << receiver.isa()->name()->asCStr() <<
    //"(" << lookupClass->name()->asString() << ")>>"
    //	      << selector->asCStr() << "\n";
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
	regs = &proc->context()->reg0()->basicAt0(0);				\
	lits = &proc->context()->methodOrBlock().as<MethodOop>()->literals()->	\
	    basicAt0(0);							\
}

int
execute(ObjectMemory &omem, ProcessOop proc)
{
	Oop ac;
	uint8_t * pc = &CTX->bytecode()->basicAt0(0);
	Oop * regs;
	Oop * lits;

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
			uint8_t src = FETCH;
			uint8_t op = FETCH;
			Oop arg1 = regs[src];
			Oop arg2 = ac;

			ac = Primitive::primitives[op].fn2(omem, proc,arg1,
			    arg2);
			if (ac.isNil()) {
				MethodOop meth = lookupMethodInClass(proc,
				    arg1, arg1.isa(), ObjectMemory::symBin[op]);

				assert(!meth.isNil());

				ContextOop ctx = ContextOopDesc::newWithMethod(
				    omem, arg1, meth);

				ctx->previousContext() = CTX;
				ctx->reg0()->basicAt0(1) = arg2;

				SPILL();
				CTX = ctx;
				UNSPILL();
				in++;
			}
			break;
		}

		/**
		 * a receiver, u8 selector-literal-index, u8 num-args,
		 *     (u8 arg-register)+, ->a result
		 */
		case Op::kSend: {
			unsigned selIdx = FETCH, nArgs = FETCH;
			MethodOop meth = lookupMethodInClass(proc, ac, ac.isa(),
			    lits[selIdx].as<SymbolOop>());

			assert(!meth.isNil());

			ContextOop ctx = ContextOopDesc::newWithMethod(omem, ac, meth);
			ctx->previousContext() = CTX;

			for (int i = 0; i < nArgs; i++) {
				ctx->reg0()->basicAt0(i + 1) = regs[FETCH];
			}

			SPILL();
			CTX = ctx;
			in++;
			UNSPILL();

			//disassemble(meth->bytecode()->vns(), meth->bytecode()->size());

			break;
		}

		/**
		 * a receiver, u8 selector-literal-index, u8 num-args,
		 *     (u8 arg-register)+, ->a result
		 */
		case Op::kSendSuper: {
			unsigned selIdx = FETCH, nArgs = FETCH;
			ClassOop cls = METHODCLASS->superClass();
			MethodOop meth = lookupMethodInClass(proc, ac, cls,
			    lits[selIdx].as<SymbolOop>());

			assert(!meth.isNil());

			ContextOop ctx = ContextOopDesc::newWithMethod(omem, ac, meth);
			ctx->previousContext() = CTX;

			for (int i = 0; i < nArgs; i++) {
				ctx->reg0()->basicAt0(i + 1) = regs[FETCH];
			}

			//std::cout << blanks(in) << "=> " <<
			//    ac.isa()->name()->asCStr() << ">>" <<
			//    lits[selIdx].as<SymbolOop>()->asCStr() << "\n";
			SPILL();
			CTX = ctx;
			in++;
			UNSPILL();

			break;
		}

		/** u8 prim-num, u8 num-args, (u8 arg-reg)+ */
		case Op::kPrimitive: {
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
			unsigned prim = FETCH;

			SPILL();
			ac = Primitive::primitives[prim].fn1(omem, proc, ac);
			UNSPILL();

			break;
		}

		/** a arg2, u8 prim-num, u8 arg1-reg */
		case Op::kPrimitive2: {
			unsigned prim = FETCH, arg1reg = FETCH;

			SPILL();
			ac = Primitive::primitives[prim].fn2(omem, proc,
			    regs[arg1reg], ac);
			UNSPILL();

			break;
		}

		/** a arg3, u8 prim-num, u8 arg1-reg */
		case Op::kPrimitive3: {
			unsigned prim = FETCH, arg1reg = FETCH;

			SPILL();
			ac = Primitive::primitives[prim].fn3(omem, proc,
			    regs[arg1reg], regs[arg1reg + 1], ac);
			UNSPILL();

			break;
		}

		case Op::kPrimitiveV: {
			unsigned prim = FETCH, nArgs = FETCH, arg1reg = FETCH;

			SPILL();
			ac = Primitive::primitives[prim].fnv(omem, proc, nArgs,
			    &regs[arg1reg]);
			UNSPILL();

			break;
		}

		case Op::kReturnSelf: {
			Oop rval = RECEIVER;
			proc->context() = proc->context()->previousContext();
			UNSPILL();
			ac = rval;
			break;
		}

		/* u8 source-register */
		case Op::kReturn: {
			CTX = CTX->previousContext();
			//std::cout << blanks(in--) << "Returning\n";
			if (CTX.isNil()) {
				std::cout << "Completed evaluation with result:\n";
				ac.print(5);
				return 0;
			}
			UNSPILL();
			break;
		}

		case Op::kBlockReturn: {
			MethodOop meth = lookupMethodInClass(proc,
			    CTX->methodOrBlock(), CTX->methodOrBlock().isa(),
			    SymbolOopDesc::fromString(omem, "nonLocalReturn:"));

			assert(!meth.isNil());

			SPILL();
			ContextOop ctx = ContextOopDesc::newWithMethod(omem,
			    CTX->methodOrBlock(), meth);
			ctx->previousContext() = CTX;

			ctx->reg0()->basicAt0(1) = ac;

			//std::cout << blanks(in) <<"Beginning block return.\n";
			CTX = ctx;
			UNSPILL();
			break;
		}
	} /* switch (op) */

	goto loop;
}