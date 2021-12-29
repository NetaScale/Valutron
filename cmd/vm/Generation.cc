#include "Generation.hh"
#include "Interpreter.hh"

void
CodeGen::genCode(uint8_t code)
{
	m_bytecode.push_back(code);
}

void
CodeGen::gen(Op::Opcode code)
{
	genCode(code);
}

void
CodeGen::gen(Op::Opcode code, uint8_t arg1)
{
	genCode(code);
	genCode(arg1);
}

void
CodeGen::gen(Op::Opcode code, uint8_t arg1, uint8_t arg2)
{
	genCode(code);
	genCode(arg1);
	genCode(arg2);
}

void
CodeGen::gen(Op::Opcode code, uint8_t arg1, uint8_t arg2, uint8_t arg3)
{
	genCode(code);
	genCode(arg1);
	genCode(arg2);
	genCode(arg3);
}

int
CodeGen::addLit(Oop oop)
{
	m_literals.push_back(oop);
	return m_literals.size() - 1;
}

int
CodeGen::addSym(std::string str)
{
	return addLit(SymbolOopDesc::fromString(m_omem, str));
}

void
CodeGen::genMoveParentHeapVarToMyHeapVars(uint8_t index, uint8_t promotedIndex)
{
	gen(Op::kMoveParentHeapVarToMyHeapVars, index);
	genCode(promotedIndex);
}

void
CodeGen::genMoveArgumentToMyHeapVars(uint8_t index, uint8_t promotedIndex)
{
	gen(Op::kStoreToMyHeapVars, argIndex(index), promotedIndex);
}

void
CodeGen::genMoveLocalToMyHeapVars(uint8_t index, uint8_t promotedIndex)
{
	gen(Op::kStoreToMyHeapVars, localIndex(index), promotedIndex);
}

void
CodeGen::genMoveMyHeapVarToParentHeapVars(uint8_t myIndex, uint8_t parentIndex)
{
	gen(Op::kMoveMyHeapVarToParentHeapVars, myIndex, parentIndex);
}

RegisterID
CodeGen::genLoadArgument(uint8_t index)
{
	return argIndex(index);
}

RegisterID
CodeGen::genLoadGlobal(std::string name)
{
	RegisterID id = m_reg++;
	gen(Op::kLoadGlobal, addSym(name), id);
	return id;
}

RegisterID
CodeGen::genLoadInstanceVar(uint8_t index)
{
	RegisterID id = m_reg++;
	gen(Op::kLoadGlobal, index, id);
	return id;
}

RegisterID
CodeGen::genLoadLocal(uint8_t index)
{
	return localIndex(index);
}

RegisterID
CodeGen::genLoadParentHeapVar(uint8_t index)
{
	RegisterID id = m_reg++;
	gen(Op::kLoadParentHeapVar, index, id);
	return id;
}

RegisterID
CodeGen::genLoadMyHeapVar(uint8_t index)
{
	RegisterID id = m_reg++;
	gen(Op::kLoadMyHeapVar, index, id);
	return id;
}

RegisterID
CodeGen::genLoadSelf()
{
	return 0;
}

RegisterID
CodeGen::genLoadNil()
{
	RegisterID id = m_reg++;
	gen(Op::kLoadNil, id);
	return id;
}

RegisterID
CodeGen::genLoadTrue()
{
	RegisterID id = m_reg++;
	gen(Op::kLoadTrue, id);
	return id;
}

RegisterID
CodeGen::genLoadFalse()
{
	RegisterID id = m_reg++;
	gen(Op::kLoadFalse, id);
	return id;
}

RegisterID
CodeGen::genLoadSmalltalk()
{
	RegisterID id = m_reg++;
	gen(Op::kLoadSmalltalk, id);
	return id;
}

RegisterID
CodeGen::genLoadThisContext()
{
	RegisterID id = m_reg++;
	gen(Op::kLoadThisContext, id);
	return id;
}

RegisterID
CodeGen::genLoadLiteral(uint8_t num)
{
	RegisterID id = m_reg++;
	gen(Op::kLoadLiteral,num,  id);
	return id;
}

RegisterID
CodeGen::genLoadLiteralObject(Oop anObj)
{
	RegisterID id = m_reg++;
	gen(Op::kLoadLiteral, addLit(anObj), id);
	return id;
}

RegisterID
CodeGen::genLoadInteger(int val)
{
	RegisterID id = m_reg++;
	gen(Op::kLoadLiteral, addLit(val), id);
	return id;
}

RegisterID
CodeGen::genLoadBlockCopy(BlockOop block)
{
	RegisterID id = m_reg++;
	gen(Op::kLoadLiteral, addLit(block), id);
	return id;
}

RegisterID
CodeGen::genStoreInstanceVar(uint8_t index, RegisterID val)
{
	gen(Op::kStoreNstVar, index, val);
	return val;
}

RegisterID
CodeGen::genStoreGlobal(std::string name, RegisterID val)
{
	gen(Op::kStoreGlobal, addSym(name), val);
	return val;
}

RegisterID
CodeGen::genStoreLocal(uint8_t index, RegisterID val)
{
	gen(Op::kMove, localIndex(index), val);
	return localIndex(index);
}

RegisterID
CodeGen::genStoreParentHeapVar(uint8_t index, RegisterID val)
{
	gen(Op::kStoreParentHeapVar, index, val);
	return val;
}

RegisterID
CodeGen::genStoreMyHeapVar(uint8_t index, RegisterID val)
{
	gen(Op::kStoreMyHeapVar, index, val);
	return val;
}

RegisterID
CodeGen::genMessage(bool isSuper, RegisterID receiver, std::string selector,
    std::vector<RegisterID> args)
{
	RegisterID id = m_reg++;

	if (isSuper)
		gen(Op::kSendSuper, id, addSym(selector));
	else
	 	gen(Op::kSend, id, receiver, addSym(selector));

	genCode(args.size());

	for (auto arg: args)
		genCode(arg);

	return id;
}

RegisterID
CodeGen::genPrimitive(uint8_t primNum, std::vector<RegisterID> args)
{
	RegisterID id = m_reg++;

	gen(Op::kPrimitive, id, primNum, args.size());

	for (auto arg: args)
		genCode(arg);

	return id;
}

void
CodeGen::genReturn(RegisterID reg)
{
	gen(Op::kReturn, reg);
}

void
CodeGen::genReturnSelf()
{
	gen(Op::kReturnSelf);
}

void
CodeGen::genBlockReturn(RegisterID reg)
{
	gen(Op::kBlockReturn, reg);
}
