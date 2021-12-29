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
		/* u8 index/reg, u8 dest */
		kMoveParentHeapVarToMyHeapVars,
		kStoreToMyHeapVars,
		kMoveMyHeapVarToParentHeapVars,

		/* u8 dest-reg */
		kLoadNil,
		kLoadTrue,
		kLoadFalse,
		kLoadThisContext,
		kLoadSmalltalk,

		/* u8 index, u8 dest-reg */
		kLoadParentHeapVar,
		kLoadMyHeapVar,
		kLoadGlobal,
		kLoadNstVar,
		kLoadLiteral,

		/* u8 index, u8 source-reg */
		kStoreNstVar,
		kStoreGlobal,
		kStoreParentHeapVar,
		kStoreMyHeapVar,
		kMove,

		/**
		 * u8 dest-reg, u8 receiver-reg, u8 selector-literal-index,
		 *     u8 num-args, (u8 arg-reg)+
		 */
		kSend,
		/**
		 * u8 dest-reg, u8 selector-literal-index, u8 num-args,
		 *     (u8 arg-register)+
		 */
		kSendSuper,
		/** u8 dest-register, u8 prim-num, u8 num-args, (u8 arg-reg)+ */
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
