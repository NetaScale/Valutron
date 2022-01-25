#ifndef INTERPRETER_HH_
#define INTERPRETER_HH_

#include <cstdint>

#include "Oops.hh"

struct Primitive {
	static Primitive primitives[];

	static void initialise();
	static Primitive *named(std::string name);

	bool oldStyle;
	enum Kind {
		kNiladic,
		kMonadic,
		kDiadic,
		kTriadic,
		kVariadic,
	} kind; /* i.e. number of arguments */
	const char *name;
	union {
		Oop (*fnp)(ObjectMemory &omem, ProcessOop proc, ArrayOop args);
		Oop (*fn0)(ObjectMemory &omem, ProcessOop &proc);
		Oop (*fn1)(ObjectMemory &omem, ProcessOop &proc, Oop a);
		Oop (*fn2)(ObjectMemory &omem, ProcessOop &proc, Oop a, Oop b);
		Oop (*fn3)(ObjectMemory &omem, ProcessOop &proc, Oop a, Oop b,
		    Oop c);
		Oop (*fnv)(ObjectMemory &omem, ProcessOop &proc, size_t nArgs,
		    Oop args[]);
	};
	char index;
};

#define OPS                              \
	X(MoveParentHeapVarToMyHeapVars) \
	X(MoveMyHeapVarToParentHeapVars) \
	X(LdaNil)                        \
	X(LdaTrue)                       \
	X(LdaFalse)                      \
	X(LdaThisContext)      /* 5 */   \
	X(LdaThisProcess)                \
	X(LdaSmalltalk)                  \
	X(LdaParentHeapVar)              \
	X(LdaMyHeapVar)                  \
	X(LdaGlobal)           /* 10 */  \
	X(LdaNstVar)                     \
	X(LdaLiteral)                    \
	X(LdaBlockCopy)                  \
	X(Ldar)                          \
	X(StaNstVar)           /* 15 */  \
	X(StaGlobal)                     \
	X(StaParentHeapVar)              \
	X(StaMyHeapVar)                  \
	X(Star)                          \
	X(Move)                /* 20 */  \
	X(And)                           \
	X(Jump)                          \
	X(BranchIfFalse)                 \
	X(BranchIfTrue)        /* 25 */  \
	X(BinOp)                         \
	X(Send)                          \
	X(SendSuper)                     \
	X(Primitive)                     \
	X(Primitive0)                    \
	X(Primitive1)                    \
	X(Primitive2)                    \
	X(Primitive3)                    \
	X(PrimitiveV)                    \
	X(ReturnSelf)                    \
	X(Return)                        \
	X(BlockReturn)

class Op {
    public:
	enum Opcode {
#define X(OP) k##OP,
		OPS
#undef X
		kMax,
	};
};

extern "C" int execute(ObjectMemory &omem, ProcessOop proc,
   volatile bool &interruptFlag) noexcept;

/** FIXME should be a member of ClassOop? */
MethodOop lookupMethod(Oop receiver, ClassOop startCls, SymbolOop selector);

#endif /* INTERPRETER_HH_ */
