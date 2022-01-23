#include <sys/types.h>
#include <cassert>
#include <stdexcept>

#include "AST.hh"
#include "Generation.hh"
#include "Objects.hh"

ClassNode::ClassNode(std::string name, std::vector<VarDecl> tyParams,
    std::string superName, std::vector<Type*> superTyArgs,
    std::vector<VarDecl> cVars, std::vector<VarDecl> iVars)
    : name(name)
    , superName(superName)
    , m_tyParams(tyParams)
    , cVars(cVars)
    , iVars(iVars)
{
	superType = new Type(superName, superTyArgs);
}

void
ClassNode::addMethods(std::vector<MethodNode *> meths)
{
	for (auto m : meths)
		if (m->isClassMethod)
			cMethods.push_back(m);
		else
			iMethods.push_back(m);
}

void
PrimitiveExprNode::print(int in)
{
	std::cout << blanks(in) << "<prim:" << num << ">\n";
	for (auto a : args) {
		std::cout << blanks(in + 1) << "<arg>\n";
		a->print(in + 2);
		std::cout << blanks(in + 1) << "</arg>\n";
	}
	std::cout << blanks(in) << "</prim>\n";
}

void
IdentExprNode::print(int in)
{
	std::cout << blanks(in) << "<id:" << id << " />\n";
}

void
BlockExprNode::print(int in)
{
	std::cout << "<block>\n";

	std::cout << blanks(in + 1) << "<params>\n";
	for (auto & e : args)
		std::cout << blanks(in + 2) << "<param:" << e.first << " />\n";
	std::cout << blanks(in + 1) << "</params>\n";

	std::cout << blanks(in + 1) << "<statements>\n";
	for (auto e : stmts) {
		std::cout << blanks(in + 1) << "<statement>\n";
		e->print(in + 2);
		std::cout << blanks(in + 1) << "</statement>\n";
	}
	std::cout << blanks(in + 1) << "</statements>\n";

	std::cout << "</block>\n";
}

void
ExprStmtNode::print(int in)
{
	std::cout << blanks(in) << "<exprstmt>\n";
	expr->print(in + 1);
	std::cout << blanks(in) << "</exprstmt>\n";
}

void
ReturnStmtNode::print(int in)
{
	std::cout << blanks(in) << "<returnstmt> ";
	expr->print(in + 1);
	std::cout << blanks(in) << "</returnstmt>";
}

void
MethodNode::print(int in)
{
	std::cout << blanks(in) << "<method: " << sel << ">\n";

	for (auto & a : args)
		std::cout << blanks(in + 1) << "<param: " << a.first << "/>\n";

	for (auto & a : locals)
		std::cout << blanks(in + 1) << "<local: " << a.first << "/>\n";

	std::cout << blanks(in + 1) << "<statements>\n";
	for (auto s : stmts)
		s->print(in + 2);
	std::cout << blanks(in + 1) << "</ statements>\n";

	std::cout << blanks(in) << "</method>\n";
}

void
ClassNode::print(int in)
{
	std::cout << blanks(in) << "<class: " << name << " super: " << superName
		  << ">\n";

	for (auto a : cVars)
		std::cout << blanks(in + 1) << "<cVar: " << a.first << "/>\n";
	for (auto a : iVars)
		std::cout << blanks(in + 1) << "<iVar: " << a.first << "/>\n";

	std::cout << blanks(in + 1) << "<cMethods>\n";
	for (auto a : cMethods) {
		a->print(in + 2);
	}
	std::cout << blanks(in + 1) << "</cMethods>\n";

	std::cout << blanks(in + 1) << "<iMethods>\n";
	for (auto a : iMethods) {
		a->print(in + 2);
	}
	std::cout << blanks(in + 1) << "</iMethods>\n";

	std::cout << blanks(in) << "</class>\n";
}

void
ProgramNode::print(int in)
{
	std::cout << blanks(in) << "<program>\n";
	for (auto c : decls)
		c->print(in);
	std::cout << blanks(in) << "</program>\n";
}

/*
 * generation
 */
void
Var::generateOn(CodeGen &gen)
{
	switch (kind) {
	case kInstance:
		gen.genLoadInstanceVar(getIndex());
		return;

	case kArgument:
		if (!promoted)
			gen.genLoadArgument(getIndex());
		else
			gen.genLoadMyHeapVar(promotedIndex);
		return;

	case kLocal:
		if (!promoted)
			gen.genLoadLocal(getIndex());
		else
			gen.genLoadMyHeapVar(promotedIndex);
		return;

	case kParentsHeapVar: {
		std::map<int, int>::iterator it = gen.scope()->
		    parentHeapVarToMyHeapVar.find(getIndex());

		if (promoted || it != gen.scope()->parentHeapVarToMyHeapVar.
		    end())
			gen.genLoadMyHeapVar(it->second);
		else
			gen.genLoadParentHeapVar(getIndex());
		return;
	}

	case kHeapVar:
		gen.genLoadMyHeapVar(getIndex());
		return;

	default:
		gen.genLoadGlobal(name);
	}
}

RegisterID
Var::generateIntoReg(CodeGen &gen)
{
	if (kind == kArgument && !promoted)
		return getIndex();
	else if (kind == kLocal && !promoted)
		return gen.localIndex(getIndex());
	else {
		generateOn(gen);
		return gen.genStar();
	}
}

void
Var::generateAssignOn(CodeGen &gen, ExprNode *expr)
{
	expr->generateOn(gen);

	switch (kind) {
	case kInstance:
		gen.genStoreInstanceVar(getIndex());
		return;

	case kLocal:
		if (!promoted)
			gen.genStoreLocal(getIndex());
		else
			gen.genStoreMyHeapVar(promotedIndex);
		return;

	case kParentsHeapVar: {
		std::map<int, int>::iterator it = gen.scope()->
		    parentHeapVarToMyHeapVar.find(getIndex());

		if (promoted || it != gen.scope()->
		    parentHeapVarToMyHeapVar.end())
			gen.genStoreMyHeapVar(it->second);
		else
			gen.genStoreParentHeapVar(getIndex());
		return;
	}

	case kHeapVar:
		gen.genStoreMyHeapVar(getIndex());
		return;

	default:
		gen.genStoreGlobal(name);
		return;
	}
}

void
Var::generatePromoteOn(CodeGen &gen)
{
	switch (kind) {
	case kParentsHeapVar:
		gen.genMoveParentHeapVarToMyHeapVars(getIndex(), promotedIndex);
		gen.scope()->parentHeapVarToMyHeapVar[getIndex()] =
		    promotedIndex;
		return;

	case kLocal:
		gen.genMoveLocalToMyHeapVars(getIndex(), promotedIndex);
		return;

	case kArgument:
		gen.genMoveArgumentToMyHeapVars(getIndex(), promotedIndex);
		return;

	default:
		abort();
	}
}

/**
 * Restores the myheapvar value to the parents' heapvars. Implements the ability
 * to mutate parents' data.
 */
void
Var::generateRestoreOn(CodeGen &gen)
{
	switch (kind) {
	case kParentsHeapVar:
		gen.genMoveMyHeapVarToParentHeapVars(promotedIndex, getIndex());
		return;
	/* FIXME: i don't think we need to do any restoration in this case
	default:
		throw std::runtime_error("unreachable");
	*/
	}
}

void
IntExprNode::generateOn(CodeGen &gen)
{
	gen.genLoadInteger(num);
}

void
CharExprNode::generateOn(CodeGen &gen)
{
	gen.genLoadLiteralObject(CharOopDesc::newWith(gen.omem(),
	    khar[0]));
}

void
SymbolExprNode::generateOn(CodeGen &gen)
{
	gen.genLoadLiteralObject(SymbolOopDesc::fromString(gen.omem(),
	    sym));
}

void
StringExprNode::generateOn(CodeGen &gen)
{
	gen.genLoadLiteralObject(StringOopDesc::fromString(gen.omem(),
	    str));
}

void
FloatExprNode::generateOn(CodeGen &gen)
{
	gen.genLoadLiteralObject(Oop());
	/*int litNum = gen.genLiteral ((objRef)newFloat (num));
	if (!gen.inLiteralArray ())
	    gen.gen (PushLiteral, litNum);*/
}

void
ArrayExprNode::generateOn(CodeGen &gen)
{
	gen.genLoadLiteralObject(Oop());
	/*int litNum;

	gen.beginLiteralArray ();
	for (auto el : elements)
	    el->generateOn (gen);
	litNum = gen.endLiteralArray ();

	if (!gen.inLiteralArray ())
	    gen.gen (PushLiteral, litNum);*/
}

#pragma mark expressions

static void
tccErr(void *opaqueError, const char *msg)
{
#if 0
    fprintf (stderr, "%s\n", msg);
    fflush (NULL);
#endif
};

/*static const char jitPrelude[] = {
#include "JITPrelude.rh"
    0};*/

void
PrimitiveExprNode::generateOn(CodeGen &gen)
{
	std::string name;
	std::vector<RegisterID> argRegs;

	if (num->oldStyle) {
		for (auto arg : args) {
			arg->generateOn(gen);
			argRegs.push_back(gen.genStar());
		}
		return gen.genPrimitive(num->index, argRegs);
	}

	switch (num->kind) {
	case Primitive::kMonadic: {
		assert(args.size() == 1);
		args[0]->generateOn(gen);
		return gen.genPrimitive1(num->index);
	}

	case Primitive::kDiadic: {
		RegisterID arg1reg;
		assert(args.size() == 2);
		args[0]->generateOn(gen);
		arg1reg = gen.genStar();
		args[1]->generateOn(gen);
		return gen.genPrimitive2(num->index, arg1reg);
	}

	case Primitive::kTriadic: {
		RegisterID arg1reg, arg2reg;
		assert(args.size() == 3);

		args[0]->generateOn(gen);
		arg1reg = gen.genStar();
		arg2reg = gen.allocReg();
		args[1]->generateOn(gen);
		gen.genStar(arg2reg);
		args[2]->generateOn(gen);
		return gen.genPrimitive3(num->index, arg1reg);
	}

	case Primitive::kVariadic: {
		std::vector<RegisterID> argRegs(args.size());

		for (auto & reg: argRegs)
			reg = gen.allocReg();

		for (int i = 0; i < args.size(); i++) {
			args[i]->generateOn(gen);
			gen.genStar(argRegs[i]);
		}

		return gen.genPrimitiveV(num->index, args.size(), argRegs[0]);
	}
	default:
		abort();
	}
}

void
IdentExprNode::generateOn(CodeGen &gen)
{
	if (isSuper() || id == "self")
		return gen.genLoadSelf();
	else if (id == "nil")
		return gen.genLoadNil();
	else if (id == "true")
		return gen.genLoadTrue();
	else if (id == "false")
		return gen.genLoadFalse();
	else if (id == "Smalltalk")
		return gen.genLoadSmalltalk();
	else if (id == "thisContext")
		return gen.genLoadThisContext();
	else
		return var->generateOn(gen);
}

RegisterID
IdentExprNode::generateIntoReg(CodeGen & gen)
{
	if (isSuper() || id == "self") {
		return 0;
	}
	else if (id == "nil")
		gen.genLoadNil();
	else if (id == "true")
		gen.genLoadTrue();
	else if (id == "false")
		gen.genLoadFalse();
	else if (id == "Smalltalk")
		gen.genLoadSmalltalk();
	else if (id == "thisContext")
		gen.genLoadThisContext();
	else
		return var->generateIntoReg(gen);
	return gen.genStar();
}


void
IdentExprNode::generateAssignOn(CodeGen &gen, ExprNode *rValue)
{
	return var->generateAssignOn(gen, rValue);
}

void
AssignExprNode::generateOn(CodeGen &gen)
{
	return left->generateAssignOn(gen, right);
}

void
MessageExprNode::generateOn(CodeGen &gen)
{
	return generateOn(gen, -1, receiver->isSuper(), 0);
}

void
MessageExprNode::generateOn(CodeGen &gen, RegisterID recvReg, bool isSuper,
    ssize_t receiverBegin)
{
	if (m_specialKind != kNormal) {
		assert(!isSuper);
		return generateSpecialOn(gen, recvReg, receiverBegin);
	}
	std::vector<RegisterID> argRegs;

	if (args.size()) {
		if (recvReg == -1)
			recvReg = receiver->generateIntoReg(gen);
		for (auto a : args) {
			argRegs.push_back(a->generateIntoReg(gen));
		}
	}
	else if (recvReg == -1)
		receiver->generateOn(gen);

	if (!args.empty() && recvReg != -1) {
		gen.genLdar(recvReg);
	}

	gen.genMessage(isSuper, selector, argRegs);
}

void
MessageExprNode::generateSpecialOn(CodeGen &gen, RegisterID recvReg,
    ssize_t receiverBegin)
{
	assert(recvReg == -1);

	receiverBegin = gen.bytecode().size();

	switch (m_specialKind) {
		case kAnd: {
			receiver->generateOn(gen);
			auto skipSecondIfFirstFalse = gen.genBranchIfFalse();
			args[0]->generateOn(gen);
			gen.patchJumpToHere(skipSecondIfFirstFalse);
			break;
		}

		case kIfFalseIfTrue:
		case kIfTrueIfFalse: {
			receiver->generateOn(gen);
			size_t skipFirst = m_specialKind == kIfTrueIfFalse ?
			    gen.genBranchIfFalse() : gen.genBranchIfTrue();
			args[0]->generateOn(gen);
			size_t skipSecond = gen.genJump();
			gen.patchJumpToHere(skipFirst);
			args[1]->generateOn(gen);
			gen.patchJumpToHere(skipSecond);
			break;
		}

		case kIfFalse:
		case kIfTrue:{
			receiver->generateOn(gen);
			size_t skipFirst = m_specialKind == kIfTrue ?
			    gen.genBranchIfFalse() : gen.genBranchIfTrue();
			args[0]->generateOn(gen);
			gen.patchJumpToHere(skipFirst);
			break;
		}

		case kWhileTrue:
		case kWhileFalse: {
			size_t branch;

			assert(receiverBegin != -1);

			receiver->generateOn(gen);

			if(m_specialKind == kWhileTrue)
				branch = gen.genBranchIfFalse();
			else
				branch = gen.genBranchIfTrue();

			args[0]->generateOn(gen);
			size_t pos = gen.genJump();
			gen.patchJumpTo(pos, receiverBegin);
			gen.patchJumpToHere(branch);
			break;
		}

		case kWhileFalse0:
		case kWhileTrue0: {
			size_t branch;

			assert(receiverBegin != -1);

			receiver->generateOn(gen);

			if(m_specialKind == kWhileTrue0)
				branch = gen.genBranchIfFalse();
			else
				branch = gen.genBranchIfTrue();

			size_t pos = gen.genJump();
			gen.patchJumpTo(pos, receiverBegin);
			gen.patchJumpToHere(branch);
			break;
		}

		default:
			assert(m_specialKind >= kBinOp);
			recvReg = receiver->generateIntoReg(gen);
			args[0]->generateOn(gen);
			//RegisterID arg = gen.genStar();
			//gen.genLdar(receiver);
			gen.genBinOp(recvReg, m_specialKind - kBinOp);
			break;
	}
}


void
CascadeExprNode::generateOn(CodeGen &gen)
{
	print(10);

	if (messages.size() == 1) {
		receiver->generateOn(gen);
		 messages.front()->generateOn(gen, -1,
		   receiver->isSuper(), -1);
	} else {
		RegisterID rcvr;

		receiver->generateOn(gen);
		rcvr = gen.genStar();

		messages.front()->generateOn(gen, rcvr, receiver->isSuper(),
		    -1);

		for (auto it = ++std::begin(messages); it != std::end(messages);
		     it++)
			(*it)->generateOn(gen, rcvr, receiver->isSuper(), -1);
	}
}

void
BlockExprNode::generateReturnPreludeOn(CodeGen &gen)
{
    for (auto & v : scope->myHeapVars)
        v.second->generateRestoreOn (gen);
}

void
BlockExprNode::generateInlineOn(CodeGen &gen)
{
	for (auto & s: stmts)
		s->generateOn(gen);
}

void
BlockExprNode::generateOn(CodeGen &gen)
{
	if (m_inlined)
		return generateInlineOn(gen);

	BlockOop block = BlockOopDesc::new0(gen.omem());
	CodeGen blockGen(gen.omem(), args.size(), 0, true);

	blockGen.pushCurrentScope(scope);

	for (auto &v : scope->myHeapVars)
		v.second->generatePromoteOn(blockGen);

	for (auto &s : stmts) {
		s->generateOn(blockGen);

		if (s == stmts.back() && !dynamic_cast<ReturnStmtNode *>(s)) {
			generateReturnPreludeOn(blockGen);
			blockGen.genReturn();
		}
	}

	if (stmts.empty()) {
		generateReturnPreludeOn(blockGen);
		blockGen.genLoadNil();
		blockGen.genReturn();
	}

	block->setBytecode(ByteArrayOopDesc::fromVector(gen.omem(),
	    blockGen.bytecode()));
	block->setLiterals(ArrayOopDesc::fromVector(gen.omem(),
	    blockGen.literals()));
	block->setArgumentCount(Smi(args.size()));
	block->setTemporarySize(Smi((intptr_t)0));
	block->setHeapVarsSize(Smi(scope->myHeapVars.size()));
	block->setStackSize(Smi(blockGen.nRegs()));

	// gen.popCurrentScope ();
	// if (!blockGen._blockHasBlockReturn)
	//    gen.genPushLiteralObject (block);
	// else
	gen.genLoadBlockCopy(block);
}

#pragma mark statements

void
ExprStmtNode::generateOn(CodeGen &gen)
{
	expr->generateOn(gen);
}

void
ReturnStmtNode::generateOn(CodeGen &gen)
{
	if (gen.isBlock()) {
		/*RegisterID ret = gen.genStar();
		gen.genLoadSelf();
		gen.genMessage(false, "nonLocalReturn", {ret});*/
		expr->generateOn(gen);
		gen.genBlockReturn();
	} else if (expr->isSelf()) {
		gen.genReturnSelf();
	}
	else {
		expr->generateOn(gen);
		gen.genReturn();
	}
}

#pragma mark decls

MethodOop
MethodNode::generate(ObjectMemory &omem)
{
	bool finalIsReturn;
	MethodOop meth = MethodOopDesc::new0(omem);
	CodeGen gen(omem, args.size(), locals.size());

	gen.pushCurrentScope(scope);

	for (auto &v : scope->myHeapVars)
		v.second->generatePromoteOn(gen);

	for (auto s : stmts) {
		s->generateOn(gen);
		if (s == stmts.back() && dynamic_cast<ReturnStmtNode *>(s) ==
		    NULL)
			gen.genReturnSelf();
	}

	meth->setSelector(SymbolOopDesc::fromString(omem, sel));
	meth->setBytecode(ByteArrayOopDesc::fromVector(omem, gen.bytecode()));
	meth->setLiterals(ArrayOopDesc::fromVector(omem, gen.literals()));
	meth->setArgumentCount(args.size());
	meth->setTemporarySize(locals.size());
	meth->setHeapVarsSize(scope->myHeapVars.size());
	meth->setStackSize(gen.nRegs());

#if 0
	std::cout << "DISASSEMBLY OF METHOD " << sel << "\n";
	disassemble(gen.bytecode().data(), gen.bytecode().size());
	printf("Literals:\n");
	for (int i = 0; i < gen.literals().size(); i++) {
		std::cout << " " << i << "\t";
		gen.literals()[i].print(2);
	}
#endif

	gen.popCurrentScope();

	return meth;
}

void
ClassNode::generate(ObjectMemory &omem)
{
	printf("CLASS %s/%p/%p\n", name.c_str(), cls.m_ptr, cls.isa().m_ptr);
	for (auto m : cMethods)
		cls->addClassMethod(omem, m->generate(omem));

	for (auto m : cMethods)
		cls->addClassMethod(omem, m->generate(omem));
	for (auto m : iMethods)
		cls->addMethod(omem, m->generate(omem));
}

void
NamespaceNode::generate(ObjectMemory &omem)
{
	printf("NAMESPACE %s\n", name.c_str());
	for (auto cls : decls)
		cls->generate(omem);
}

void
ProgramNode::generate(ObjectMemory &omem)
{
	for (auto cls : decls)
		cls->generate(omem);
}