#ifndef GENERATION_HH_
#define GENERATION_HH_

#include <stack>

#include "Interpreter.hh"

struct Scope;

typedef int RegisterID;

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

	int localIndex(int idx) { return m_nArgs + idx; }

	ObjectMemory & omem() { return m_omem; }
	std::vector<uint8_t> bytecode() { return m_bytecode; }
	std::vector<Oop> literals() { return m_literals; }
	size_t nRegs() { return m_reg; }

	bool isBlock() { return m_isBlock; }
	bool hasOwnBlockReturn() { return m_hasOwnBlockReturn; }

	Scope *scope() { return m_scope.top(); }
	void pushCurrentScope(Scope *aScope) { m_scope.push(aScope); }
	void popCurrentScope() { m_scope.pop(); }

	RegisterID allocReg();

	void genMoveParentHeapVarToMyHeapVars(uint8_t index,
	    uint8_t promotedIndex);
	void genMoveArgumentToMyHeapVars(uint8_t index, uint8_t promotedIndex);
	void genMoveLocalToMyHeapVars(uint8_t index, uint8_t promotedIndex);
	void genMoveMyHeapVarToParentHeapVars(uint8_t myIndex,
	    uint8_t parentIndex);

	void genLdar(RegisterID reg);
	void genLoadArgument(uint8_t index);
	void genLoadGlobal(std::string name);
	void genLoadInstanceVar(uint8_t index);
	void genLoadLocal(uint8_t index);
	void genLoadParentHeapVar(uint8_t index);
	void genLoadMyHeapVar(uint8_t index);
	void genLoadSelf();
	void genLoadNil();
	void genLoadTrue();
	void genLoadFalse();
	void genLoadSmalltalk();
	void genLoadThisContext();
	void genLoadLiteral(uint8_t num);
	void genLoadLiteralObject(Oop anObj);
	void genLoadInteger(int val);
	void genLoadBlockCopy(BlockOop block);

	RegisterID genStar();
	RegisterID genStar(RegisterID into);
	void genStoreInstanceVar (uint8_t index);
	void genStoreGlobal (std::string name);
	void genStoreLocal (uint8_t index);
	void genStoreParentHeapVar (uint8_t index);
	void genStoreMyHeapVar (uint8_t index);

	size_t genJump();
	size_t genBranchIfFalse();
	size_t genBranchIfTrue();
	void patchJumpToHere(size_t jumpInstrLoc);
	void patchJumpTo(size_t jumpInstrLoc, size_t loc);

	void genBinOp(uint8_t arg, uint8_t op);

	void genMessage(bool isSuper,std::string selector,
	    std::vector<RegisterID> args);
	void genPrimitive(uint8_t primNum, std::vector<RegisterID> args);
	void genPrimitive1(uint8_t primNum);
	void genPrimitive2(uint8_t primNum, RegisterID arg1reg);
	void genPrimitive3(uint8_t primNum, RegisterID arg1reg);
	void genPrimitiveV(uint8_t primNum, uint8_t nArgs, RegisterID arg1reg);

	void genReturn();
	void genReturnSelf();
	void genBlockReturn();
};

void disassemble(uint8_t * code, int length) noexcept;

#endif /* GENERATION_HH_ */
