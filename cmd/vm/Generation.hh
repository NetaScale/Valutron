#ifndef GENERATION_HH_
#define GENERATION_HH_

#include <stack>

#include "AST.hh"
#include "Interpreter.hh"

/**
 * Some optimisations that may be made:
 * 1. Pre-allocate registers for each of a message send's arguments. By adding
 *    an optional "output register" parameter to the other calls here, thereby
 *    allow contiguous message send parameters.
 */

class CodeGen {
	ObjectMemory &m_omem;
	std::vector<uint8_t> m_bytecode;
	std::vector<Oop> m_literals;
	std::stack<Scope *> m_scope;

	bool m_isBlock;
	bool m_hasOwnBlockReturn;

	RegisterID m_reg;
	int m_nArgs;
	int m_nLocals;

	int argIndex(int idx) { return 1 + idx; }
	int localIndex(int idx) { return 1 + m_nArgs + idx; }

	void genCode(uint8_t code);
	void gen (Op::Opcode code);
	void gen (Op::Opcode code, uint8_t arg1);
	void gen (Op::Opcode code, uint8_t arg1, uint8_t arg2);
	void gen (Op::Opcode code, uint8_t arg1, uint8_t arg2, uint8_t arg3);

	int addLit(Oop oop);
	int addSym(std::string str);

    public:
	CodeGen(ObjectMemory &omem, int nArgs, int nLocals,
	    bool isBlock = false)
	    : m_omem(omem)
	    , m_isBlock(isBlock)
	    , m_nArgs(nArgs)
	    , m_nLocals(nLocals)
	    , m_reg (nArgs + nLocals + 1)
	{
	}

	ObjectMemory & omem() { return m_omem; }
	std::vector<uint8_t> bytecode() { return m_bytecode; }
	std::vector<Oop> literals() { return m_literals; }
	size_t nRegs() { return m_reg; }

	bool isBlock() { return m_isBlock; }
	bool hasOwnBlockReturn() { return m_hasOwnBlockReturn; }

	Scope *scope() { return m_scope.top(); }
	void pushCurrentScope(Scope *aScope) { m_scope.push(aScope); }
	void popCurrentScope() { m_scope.pop(); }

	void genMoveParentHeapVarToMyHeapVars(uint8_t index,
	    uint8_t promotedIndex);
	void genMoveArgumentToMyHeapVars(uint8_t index, uint8_t promotedIndex);
	void genMoveLocalToMyHeapVars(uint8_t index, uint8_t promotedIndex);
	void genMoveMyHeapVarToParentHeapVars(uint8_t myIndex,
	    uint8_t parentIndex);

	RegisterID genLoadArgument(uint8_t index);
	RegisterID genLoadGlobal(std::string name);
	RegisterID genLoadInstanceVar(uint8_t index);
	RegisterID genLoadLocal(uint8_t index);
	RegisterID genLoadParentHeapVar(uint8_t index);
	RegisterID genLoadMyHeapVar(uint8_t index);
	RegisterID genLoadSelf();
	RegisterID genLoadNil();
	RegisterID genLoadTrue();
	RegisterID genLoadFalse();
	RegisterID genLoadSmalltalk();
	RegisterID genLoadThisContext();
	RegisterID genLoadLiteral(uint8_t num);
	RegisterID genLoadLiteralObject(Oop anObj);
	RegisterID genLoadInteger(int val);
	RegisterID genLoadBlockCopy(BlockOop block);

	RegisterID genStoreInstanceVar (uint8_t index, RegisterID val);
	RegisterID genStoreGlobal (std::string name, RegisterID val);
	RegisterID genStoreLocal (uint8_t index, RegisterID val);
	RegisterID genStoreParentHeapVar (uint8_t index, RegisterID val);
	RegisterID genStoreMyHeapVar (uint8_t index, RegisterID val);

	RegisterID genMessage(bool isSuper, RegisterID receiver,
	    std::string selector, std::vector<RegisterID> args);
	RegisterID genPrimitive(uint8_t primNum, std::vector<RegisterID> args);

	void genReturn(RegisterID reg);
	void genReturnSelf();
	void genBlockReturn(RegisterID reg);

};

#endif /* GENERATION_HH_ */
