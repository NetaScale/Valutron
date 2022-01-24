#include "Generation.hh"
#include "Interpreter.hh"
#include "Objects.hh"

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

RegisterID
CodeGen::allocReg()
{
	return m_reg++;
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
	gen(Op::kLdaNstVar, index);
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
CodeGen::genLoadThisProcess()
{
	gen(Op::kLdaThisProcess);
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

RegisterID
CodeGen::genStar(RegisterID into)
{
	gen(Op::kStar, into);
	return into;
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
i16tou8(int16_t i16, uint8_t out[2])
{
	out [0] = ((i16 & 0xFF00) >> 8);
	out [1] = (i16 & 0x00FF);
}
size_t
CodeGen::genJump()
{
	gen(Op::kJump, 0, 0);
	return m_bytecode.size();
}

size_t
CodeGen::genBranchIfFalse()
{
	gen(Op::kBranchIfFalse, 0, 0);
	return m_bytecode.size();
}

size_t
CodeGen::genBranchIfTrue()
{
	gen(Op::kBranchIfTrue, 0, 0);
	return m_bytecode.size();
}

void
CodeGen::patchJumpToHere(size_t jumpInstrLoc)
{
	uint8_t relative[2];
	i16tou8(m_bytecode.size() - jumpInstrLoc, relative);
	m_bytecode[jumpInstrLoc - 1] = relative[1];
	m_bytecode[jumpInstrLoc - 2] = relative[0];
}

void
CodeGen::patchJumpTo(size_t jumpInstrLoc, size_t loc)
{
	uint8_t relative[2];
	i16tou8(loc - jumpInstrLoc, relative);
	m_bytecode[jumpInstrLoc - 1] = relative[1];
	m_bytecode[jumpInstrLoc - 2] = relative[0];
}

void
CodeGen::genBinOp(uint8_t arg, uint8_t op)
{
	gen(Op::kBinOp, arg, op);
}

void
CodeGen::genMessage(bool isSuper, std::string selector,
    std::vector<RegisterID> args)
{
	CacheOop cache = CacheOopDesc::newWithSelector(m_omem,
	    SymbolOopDesc::fromString(m_omem, selector));

	if (isSuper)
		gen(Op::kSendSuper, addLit(cache), args.size());
	else
		gen(Op::kSend, addLit(cache), args.size());

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
CodeGen::genPrimitive0(uint8_t primNum)
{
	gen(Op::kPrimitive0, primNum);
}

void
CodeGen::genPrimitive1(uint8_t primNum)
{
	gen(Op::kPrimitive1, primNum);
}

void
CodeGen::genPrimitive2(uint8_t primNum, RegisterID arg1reg)
{
	gen(Op::kPrimitive2, primNum, arg1reg);
}

void
CodeGen::genPrimitive3(uint8_t primNum, RegisterID arg1reg)
{
	gen(Op::kPrimitive3, primNum, arg1reg);
}

void
CodeGen::genPrimitiveV(uint8_t primNum, uint8_t nArgs, RegisterID arg1reg)
{
	gen(Op::kPrimitiveV, primNum, nArgs);
	genCode(arg1reg);
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
