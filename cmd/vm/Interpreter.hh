#ifndef INTERPRETER_HH_
#define INTERPRETER_HH_

/**
 * Layout of context registers:
 * [arguments] [locals] [temporaries]
 */

class Op {
	enum Operation {
	/* u8 dest-register */
	kLoadNil,
	kLoadTrue,
	kLoadFalse,
	kLoadContext,
	kLoadSmalltalk,

	/* u8 dest-register, u8 index */
	kLoadNstVar,
	kLoadLiteral,

	/**
	 * u8 dest-register, u8 receiver-reg, u8 selector-literal-index,
	 *     u8 num-args, (u8 arg-register)+
	 */
	kSend,
	/**
	 * u8 dest-register, u8 selector-literal-index, u8 num-args,
	 *     (u8 arg-register)+
	 */
	kSendSuper,
	};
};



class Interpreter {

};

#endif /* INTERPRETER_HH_ */
