#include <stdexcept>
#include "AST.hh"
#include "Generation.hh"

ClassNode::ClassNode(std::string name, std::string superName,
    std::vector<std::string> cVars, std::vector<std::string> iVars)
    : name(name)
    , superName(superName)
    , cVars(cVars)
    , iVars(iVars)
{
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
	for (auto e : args)
		std::cout << blanks(in + 2) << "<param:" << e << " />\n";
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

	for (auto a : args)
		std::cout << blanks(in + 1) << "<param: " << a << "/>\n";

	for (auto a : locals)
		std::cout << blanks(in + 1) << "<local: " << a << "/>\n";

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
		std::cout << blanks(in + 1) << "<cVar: " << a << "/>\n";
	for (auto a : iVars)
		std::cout << blanks(in + 1) << "<iVar: " << a << "/>\n";

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
RegisterID
Var::generateOn(CodeGen &gen)
{
	switch (kind) {
	case kInstance:
		return gen.genLoadInstanceVar(getIndex());

	case kArgument:
		if (!promoted)
			return gen.genLoadArgument(getIndex());
		else
			return gen.genLoadMyHeapVar(promotedIndex);

	case kLocal:
		if (!promoted)
			return gen.genLoadLocal(getIndex());
		else
			return gen.genLoadMyHeapVar(promotedIndex);

	case kParentsHeapVar: {
		std::map<int, int>::iterator it = gen.scope()->
		    parentHeapVarToMyHeapVar.find(getIndex());

		if (promoted || it != gen.scope()->parentHeapVarToMyHeapVar.
		    end())
			return gen.genLoadMyHeapVar(it->second);
		else
			return gen.genLoadParentHeapVar(getIndex());
	}

	case kHeapVar:
		return gen.genLoadMyHeapVar(getIndex());

	default:
		return gen.genLoadGlobal(name);
	}
}

RegisterID
Var::generateAssignOn(CodeGen &gen, ExprNode *expr)
{
	RegisterID val = expr->generateOn(gen);

	switch (kind) {
	case kInstance:
		return gen.genStoreInstanceVar(getIndex(), val);

	case kLocal:
		if (!promoted)
			return gen.genStoreLocal(getIndex(), val);
		else
			return gen.genStoreMyHeapVar(promotedIndex, val);

	case kParentsHeapVar: {
		std::map<int, int>::iterator it = gen.scope()->
		    parentHeapVarToMyHeapVar.find(getIndex());

		if (promoted || it != gen.scope()->
		    parentHeapVarToMyHeapVar.end())
			return gen.genStoreMyHeapVar(it->second, val);
		else
			return gen.genStoreParentHeapVar(getIndex(), val);
	}

	case kHeapVar:
		return gen.genStoreMyHeapVar(getIndex(), val);

	default:
		return gen.genStoreGlobal(name, val);
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
	default:
		throw std::runtime_error("unreachable");
	}
}

RegisterID
IntExprNode::generateOn(CodeGen &gen)
{
	return gen.genLoadInteger(num);
}

RegisterID
CharExprNode::generateOn(CodeGen &gen)
{
	return gen.genLoadLiteralObject(CharOopDesc::newWith(gen.omem(),
	    khar[0]));
}

RegisterID
SymbolExprNode::generateOn(CodeGen &gen)
{
	return gen.genLoadLiteralObject(SymbolOopDesc::fromString(gen.omem(),
	    sym));
}

RegisterID
StringExprNode::generateOn(CodeGen &gen)
{
	return gen.genLoadLiteralObject(StringOopDesc::fromString(gen.omem(),
	    str));
}

RegisterID
FloatExprNode::generateOn(CodeGen &gen)
{
	return gen.genLoadLiteralObject(Oop());
	/*int litNum = gen.genLiteral ((objRef)newFloat (num));
	if (!gen.inLiteralArray ())
	    gen.genInstruction (PushLiteral, litNum);*/
}

RegisterID
ArrayExprNode::generateOn(CodeGen &gen)
{
	return gen.genLoadLiteralObject(Oop());
	/*int litNum;

	gen.beginLiteralArray ();
	for (auto el : elements)
	    el->generateOn (gen);
	litNum = gen.endLiteralArray ();

	if (!gen.inLiteralArray ())
	    gen.genInstruction (PushLiteral, litNum);*/
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

RegisterID
PrimitiveExprNode::generateOn(CodeGen &gen)
{
	std::string name;

	if (num != 0) {
		std::vector<RegisterID> argRegs;
		for (auto arg : args)
			argRegs.push_back(arg->generateOn(gen));
		return gen.genPrimitive(num, argRegs);
	} else
		abort();

#if 0
    name = dynamic_cast<IdentExprNode *> (args[0])->id;

    if (name == "C")
    {
        std::string code = std::string (jitPrelude) +
                           dynamic_cast<StringExprNode *> (args[1])->str;
        TCCState * tcc = atcc_new ();
        size_t codeSize;
        NativeCodeOop nativeCode;
        NativeCodeOopDesc::Fun fun;

        tcc_set_error_func (tcc, NULL, tccErr);
        tcc_set_output_type (tcc, TCC_OUTPUT_MEMORY);
        tcc_define_symbol (tcc, "__MultiVIM_JIT", "true");
        if (tcc_compile_string (tcc, code.c_str ()) == -1)
            abort ();
        codeSize = tcc_relocate (tcc, NULL);
        printf ("Code size: %d\n", codeSize);
        assert (codeSize != -1);

        nativeCode = NativeCodeOopDesc::newWithSize (codeSize);
        tcc_relocate (tcc, nativeCode->funCode ());
        fun = (NativeCodeOopDesc::Fun)tcc_get_symbol (tcc, "main");
        nativeCode->setFun (fun);

        tcc_delete (tcc);

        gen.genPushLiteralObject (nativeCode);
    }
#endif
}

RegisterID
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
IdentExprNode::generateAssignOn(CodeGen &gen, ExprNode *rValue)
{
	return var->generateAssignOn(gen, rValue);
}

RegisterID
AssignExprNode::generateOn(CodeGen &gen)
{
	return left->generateAssignOn(gen, right);
}

RegisterID
MessageExprNode::generateOn(CodeGen &gen)
{
	return generateOn(gen, receiver->generateOn(gen), receiver->isSuper());
}

RegisterID
MessageExprNode::generateOn(CodeGen &gen, RegisterID receiver, bool isSuper)
{
	std::vector<RegisterID> argRegs;

	for (auto a : args)
		argRegs.push_back(a->generateOn(gen));

#if 0
	if (selector == std::string ("ifTrue:ifFalse:"))
		gen.genIfTrueIfFalse ();
	else
#endif
	return gen.genMessage(isSuper, selector, argRegs);
}

RegisterID
CascadeExprNode::generateOn(CodeGen &gen)
{
	RegisterID recv = receiver->generateOn(gen);
	RegisterID first = messages.front()->generateOn(gen, recv,
	    receiver->isSuper());

	for (auto it = ++std::begin(messages); it != std::end(messages); it++)
		(*it)->generateOn(gen);

	return first;
}

void
BlockExprNode::generateReturnPreludeOn(CodeGen &gen)
{
    for (auto & v : scope->myHeapVars)
        v.second->generateRestoreOn (gen);
}

RegisterID
BlockExprNode::generateOn(CodeGen &gen)
{
	BlockOop block = BlockOopDesc::allocate(gen.omem());
	CodeGen blockGen(gen.omem(), args.size(), 0, true);

	blockGen.pushCurrentScope(scope);

	for (auto &v : scope->myHeapVars)
		v.second->generatePromoteOn(blockGen);

	for (auto &s : stmts) {
		if (s != stmts.back())
			s->generateOn(blockGen);
		else if (!dynamic_cast<ReturnStmtNode *>(s)) {
			RegisterID reg = dynamic_cast<ExprStmtNode*>(s)->expr->
			    generateOn(blockGen);
			generateReturnPreludeOn(blockGen);
			blockGen.genReturn(reg);
		}
	}

	if (stmts.empty()) {
		generateReturnPreludeOn(blockGen);
		blockGen.genReturn(blockGen.genLoadNil());
	}

	block->setBytecode(
	    ByteArrayOopDesc::fromVector(gen.omem(), blockGen.bytecode()));
	block->setLiterals(
	    ArrayOopDesc::fromVector(gen.omem(), blockGen.literals()));
	block->setArgumentCount(SmiOop(args.size()));
	block->setTemporarySize(SmiOop((intptr_t)0));
	block->setHeapVarsSize(SmiOop(scope->myHeapVars.size()));
	block->setStackSize(SmiOop(blockGen.nRegs()));

	// gen.popCurrentScope ();
	// if (!blockGen._blockHasBlockReturn)
	//    gen.genPushLiteralObject (block);
	// else
	return gen.genLoadBlockCopy(block);
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
	if (gen.isBlock())
		gen.genBlockReturn(expr->generateOn(gen));
	else
		gen.genReturn(expr->generateOn(gen));
}

#pragma mark decls

MethodOop
MethodNode::generate(ObjectMemory &omem)
{
	bool finalIsReturn;
	MethodOop meth = MethodOopDesc::allocate();
	CodeGen gen(omem, args.size(), locals.size());

	gen.pushCurrentScope(scope);

	for (auto &v : scope->myHeapVars)
		v.second->generatePromoteOn(gen);

	for (auto s : stmts) {
		s->generateOn(gen);
	}

	if (!finalIsReturn) {
		gen.genReturnSelf();
	}

	meth->setSelector(SymbolOopDesc::fromString(omem, sel));
	meth->setBytecode(ByteArrayOopDesc::fromVector(omem, gen.bytecode()));
	meth->setLiterals(ArrayOopDesc::fromVector(omem, gen.literals()));
	meth->setArgumentCount(args.size());
	meth->setTemporarySize(locals.size());
	meth->setHeapVarsSize(scope->myHeapVars.size());
	meth->setStackSize(gen.nRegs());

	gen.popCurrentScope();

	return meth;
}

void
ClassNode::generate(ObjectMemory &omem)
{
	printf("CLASS %s\n", name.c_str());
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