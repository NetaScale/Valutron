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
	gen(Op::kLdar, index);
	gen(Op::kStaMyHeapVar, promotedIndex);
}

void
CodeGen::genMoveLocalToMyHeapVars(uint8_t index, uint8_t promotedIndex)
{
	gen(Op::kLdar, localIndex(index));
	gen(Op::kStaMyHeapVar, promotedIndex);
}

void
CodeGen::genMoveMyHeapVarToParentHeapVars(uint8_t myIndex, uint8_t parentIndex)
{
	gen(Op::kMoveMyHeapVarToParentHeapVars, myIndex, parentIndex);
}

void
CodeGen::genLdar(RegisterID reg)
{
	gen(Op::kLdar, reg);
}

void
CodeGen::genLoadArgument(uint8_t index)
{
	genLdar(index); /* arg index is 1-based addressing */
}

void
CodeGen::genLoadGlobal(std::string name)
{
	gen(Op::kLdaGlobal, addSym(name));
}

void
CodeGen::genLoadInstanceVar(uint8_t index)
{
	gen(Op::kLdaGlobal, index);
}

void
CodeGen::genLoadLocal(uint8_t index)
{
	genLdar(localIndex(index));
}

void
CodeGen::genLoadParentHeapVar(uint8_t index)
{
	gen(Op::kLdaParentHeapVar, index);
}

void
CodeGen::genLoadMyHeapVar(uint8_t index)
{
	gen(Op::kLdaMyHeapVar, index);
}

void
CodeGen::genLoadSelf()
{
	gen(Op::kLdar, 0);
}

void
CodeGen::genLoadNil()
{
	gen(Op::kLdaNil);
}

void
CodeGen::genLoadTrue()
{
	gen(Op::kLdaTrue);
}

void
CodeGen::genLoadFalse()
{
	gen(Op::kLdaFalse);
}

void
CodeGen::genLoadSmalltalk()
{
	gen(Op::kLdaSmalltalk);
}

void
CodeGen::genLoadThisContext()
{
	gen(Op::kLdaThisContext);
}

void
CodeGen::genLoadLiteral(uint8_t num)
{
	gen(Op::kLdaLiteral,num);
}

void
CodeGen::genLoadLiteralObject(Oop anObj)
{
	gen(Op::kLdaLiteral, addLit(anObj));
}

void
CodeGen::genLoadInteger(int val)
{
	gen(Op::kLdaLiteral, addLit(val));
}

void
CodeGen::genLoadBlockCopy(BlockOop block)
{
	gen(Op::kLdaBlockCopy, addLit(block));
}

RegisterID
CodeGen::genStar()
{
	RegisterID reg = m_reg++;
	gen(Op::kStar, reg);
	return reg;
}

void
CodeGen::genStoreInstanceVar(uint8_t index)
{
	gen(Op::kStaNstVar, index);
}

void
CodeGen::genStoreGlobal(std::string name)
{
	gen(Op::kStaGlobal, addSym(name));
}

void
CodeGen::genStoreLocal(uint8_t index)
{
	gen(Op::kStar, localIndex(index));
}

void
CodeGen::genStoreParentHeapVar(uint8_t index)
{
	gen(Op::kStaParentHeapVar, index);
}

void
CodeGen::genStoreMyHeapVar(uint8_t index)
{
	gen(Op::kStaMyHeapVar, index);
}

void
CodeGen::genMessage(bool isSuper, std::string selector,
    std::vector<RegisterID> args)
{
	if (isSuper)
		gen(Op::kSendSuper, addSym(selector), args.size());
	else
	 	gen(Op::kSend, addSym(selector), args.size());

	for (auto arg: args)
		genCode(arg);
}

void
CodeGen::genPrimitive(uint8_t primNum, std::vector<RegisterID> args)
{
	gen(Op::kPrimitive, primNum, args.size());

	for (auto arg: args)
		genCode(arg);
}

void
CodeGen::genReturn()
{
	gen(Op::kReturn);
}

void
CodeGen::genReturnSelf()
{
	gen(Op::kReturnSelf);
}

void
CodeGen::genBlockReturn()
{
	gen(Op::kBlockReturn);
}
