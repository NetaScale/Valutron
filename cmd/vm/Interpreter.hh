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

/**
 * Layout of context registers:
 * | self [arguments] [locals] [temporaries] |
 */

class Op {
    public:
	enum Opcode {
		/* u8 index, u8 dest */
		kMoveParentHeapVarToMyHeapVars,
		kMoveMyHeapVarToParentHeapVars,

		/* ->a value */
		kLdaNil,
		kLdaTrue,
		kLdaFalse,
		kLdaThisContext,
		kLdaSmalltalk,

		/* u8 index/reg, -> a value*/
		kLdaParentHeapVar,
		kLdaMyHeapVar,
		kLdaGlobal,
		kLdaNstVar,
		kLdaLiteral,
		kLdaBlockCopy, /* u8 literal index */
		kLdar,

		/* a value, u8 index */
		kStaNstVar,
		kStaGlobal,
		kStaParentHeapVar,
		kStaMyHeapVar,

		/* a value, u8 dst-reg */
		kStar,

		/* u8 src-reg, u8 dst-reg */
		kMove,

		/* a value, u8 src-reg */
		kAnd,

		/* i16 pc-offset */
		kJump,
		/* a value, i16 pc-offset */
		kBranchIfFalse,
		/* a value, i16 pc-offset */
		kBranchIfTrue,

		/* a value, u8 op, u8 src-reg */
		kBinOp,

		/**
		 * a receiver, u8 selector-literal-index, u8 num-args,
		 *     (u8 arg-reg)+ -> a result
		 */
		kSend,
		/**
		 * a receiver, u8 selector-literal-index, u8 num-args,
		 *     (u8 arg-register)+, ->a result
		 */
		kSendSuper,
		/** u8 prim-num, u8 num-args, (u8 arg-reg)+, -> a result */
		kPrimitive,
		/** a arg, u8 prim-num, -> a result */
		kPrimitive1,
		/** a arg2, u8 prim-num, u8 arg1-reg */
		kPrimitive2,
		/** a arg3, u8 prim-num, u8 first-arg-reg */
		kPrimitive3,
		/* u8 prim-num, u8 nargs, u8 first-arg-reg */
		kPrimitiveV,

		kReturnSelf,

		/* u8 source-register */
		kReturn,
		kBlockReturn,
	};
};

extern "C" int execute(ObjectMemory &omem, ProcessOop proc) noexcept;

#endif /* INTERPRETER_HH_ */
