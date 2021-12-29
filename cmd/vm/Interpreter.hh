#ifndef INTERPRETER_HH_
#define INTERPRETER_HH_

#include <cstdint>

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

		kReturnSelf,

		/* u8 source-register */
		kReturn,
		kBlockReturn,
	};
};

class Interpreter {
};

#endif /* INTERPRETER_HH_ */
