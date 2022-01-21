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
	X(LdaSmalltalk)                  \
	X(LdaParentHeapVar)              \
	X(LdaMyHeapVar)                  \
	X(LdaGlobal)                     \
	X(LdaNstVar)           /* 10 */  \
	X(LdaLiteral)                    \
	X(LdaBlockCopy)                  \
	X(Ldar)                          \
	X(StaNstVar)                     \
	X(StaGlobal)           /* 15 */  \
	X(StaParentHeapVar)              \
	X(StaMyHeapVar)                  \
	X(Star)                          \
	X(Move)                          \
	X(And)                 /* 20 */  \
	X(Jump)                          \
	X(BranchIfFalse)                 \
	X(BranchIfTrue)                  \
	X(BinOp)                         \
	X(Send)                /* 25 */  \
	X(SendSuper)                     \
	X(Primitive)                     \
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
   uintptr_t timeslice) noexcept;

#endif /* INTERPRETER_HH_ */
