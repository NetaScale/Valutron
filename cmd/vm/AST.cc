#include "AST.hh"

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
void
Var::generateOn(CodeGen &gen)
{
#if 0
    switch (kind)
    {
    case kInstance:
        gen.genPushInstanceVar (getIndex ());
        return;
    case kArgument:
        if (!promoted)
            gen.genPushArgument (getIndex ());
        else
            gen.genPushMyHeapVar (promotedIndex);
        return;
    case kLocal:
        if (!promoted)
            gen.genPushLocal (getIndex ());
        else
            gen.genPushMyHeapVar (promotedIndex);
        return;
    case kParentsHeapVar:
    {
        std::map<int, int>::iterator it =
            gen.currentScope ()->parentHeapVarToMyHeapVar.find (getIndex ());
        if (promoted ||
            it != gen.currentScope ()->parentHeapVarToMyHeapVar.end ())
            gen.genPushMyHeapVar (it->second);
        else
            gen.genPushParentHeapVar (getIndex ());
        return;
    }
    case kHeapVar:
        gen.genPushMyHeapVar (getIndex ());
        return;
    }

    gen.genPushGlobal (name);
#endif
}

void
Var::generateAssignOn(CodeGen &gen, ExprNode *expr)
{
#if 0
    expr->generateOn (gen);
    switch (kind)
    {
    case kInstance:
        gen.genStoreInstanceVar (getIndex ());
        return;
    case kLocal:
        if (!promoted)
            gen.genStoreLocal (getIndex ());
        else
            gen.genStoreMyHeapVar (promotedIndex);
        return;
    case kParentsHeapVar:
    {
        std::map<int, int>::iterator it =
            gen.currentScope ()->parentHeapVarToMyHeapVar.find (getIndex ());
        if (promoted ||
            it != gen.currentScope ()->parentHeapVarToMyHeapVar.end ())
            gen.genStoreMyHeapVar (it->second);
        else
            gen.genStoreParentHeapVar (getIndex ());
        return;
    }
    case kHeapVar:
        gen.genStoreMyHeapVar (getIndex ());
        return;
    }

    gen.genStoreGlobal (name);
#endif
}

void
Var::generatePromoteOn(CodeGen &gen)
{
#if 0
    switch (kind)
    {
    case kParentsHeapVar:
        gen.genMoveParentHeapVarToMyHeapVars (getIndex (), promotedIndex);
        gen.currentScope ()->parentHeapVarToMyHeapVar[getIndex ()] =
            promotedIndex;
        return;

    case kLocal:
        gen.genMoveLocalToMyHeapVars (getIndex (), promotedIndex);
        return;

    case kArgument:
        gen.genMoveArgumentToMyHeapVars (getIndex (), promotedIndex);
        return;
    }

    assert (!"Unreached\n");
#endif
}

/**
 * Restores the myheapvar value to the parents' heapvars. Implements the ability
 * to mutate parents' data.
 */
void
Var::generateRestoreOn(CodeGen &gen)
{
#if 0
    switch (kind)
    {
    case kParentsHeapVar:
        gen.genMoveMyHeapVarToParentHeapVars (promotedIndex, getIndex ());
        return;
    }
#endif
}

void
IntExprNode::generateOn(CodeGen &gen)
{
#if 0
    gen.genPushInteger (num);
#endif
}

void
CharExprNode::generateOn(CodeGen &gen)
{
#if 0
    gen.genPushLiteralObject (CharOopDesc::newWith (khar[0]));
#endif
}

void
SymbolExprNode::generateOn(CodeGen &gen)
{
#if 0
    gen.genPushLiteralObject (SymbolOopDesc::fromString (sym));
#endif
}

void
StringExprNode::generateOn(CodeGen &gen)
{
#if 0
    gen.genPushLiteralObject (StringOopDesc::fromString (str));
#endif
}

void
FloatExprNode::generateOn(CodeGen &gen)
{
#if 0
    gen.genPushLiteralObject (Oop::nilObj ());
    /*int litNum = gen.genLiteral ((objRef)newFloat (num));
    if (!gen.inLiteralArray ())
        gen.genInstruction (PushLiteral, litNum);*/
#endif
}

void
ArrayExprNode::generateOn(CodeGen &gen)
{
#if 0
    gen.genPushLiteralObject (Oop::nilObj ());
    /*int litNum;

    gen.beginLiteralArray ();
    for (auto el : elements)
        el->generateOn (gen);
    litNum = gen.endLiteralArray ();

    if (!gen.inLiteralArray ())
        gen.genInstruction (PushLiteral, litNum);*/
#endif
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
#if 0
    std::string name;

    if (num != 0)
    {
        for (auto arg : args)
            arg->generateOn (gen);
        gen.genPrimitive (num, args.size ());
        return;
    }

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

void
IdentExprNode::generateOn(CodeGen &gen)
{
#if 0
    if (isSuper () || id == "self")
        gen.genPushSelf ();
    else if (id == "nil")
        gen.genPushNil ();
    else if (id == "true")
        gen.genPushTrue ();
    else if (id == "false")
        gen.genPushFalse ();
    else if (id == "Smalltalk")
        gen.genPushSmalltalk ();
    else if (id == "thisContext")
        gen.genPushThisContext ();
    else
        var->generateOn (gen);
#endif
}

void
IdentExprNode::generateAssignOn(CodeGen &gen, ExprNode *rValue)
{
#if 0
    var->generateAssignOn (gen, rValue);
#endif
}

void
AssignExprNode::generateOn(CodeGen &gen)
{
#if 0
    left->generateAssignOn (gen, right);
#endif
}

void
MessageExprNode::generateOn(CodeGen &gen, bool cascade)
{
#if 0
    if (!cascade)
        receiver->generateOn (gen);

    for (auto a : args)
        a->generateOn (gen);

    if (selector == std::string ("ifTrue:ifFalse:"))
    {
        gen.genIfTrueIfFalse ();
    }
    else
    {
        gen.genMessage (receiver->isSuper (), args.size (), selector);
    }
#endif
}

void
CascadeExprNode::generateOn(CodeGen &gen)
{
#if 0
    receiver->generateOn (gen);
    messages.front ()->generateOn (gen, true);
    /* duplicate result of first send so it remains after message send */
    gen.genDup ();

    for (auto it = ++std::begin (messages); it != std::end (messages); it++)
    {
        if (*it == messages.back ())
            /* as we are the last one, get rid of the original receiver
             * duplicate; we are consuming it. */
            gen.genPop ();
        (*it)->generateOn (gen, true);
        if (*it != messages.back ())
        {
            /* pop result of this send */
            gen.genPop ();
            /* duplicate receiver so it remains after next send*/
            gen.genDup ();
        }
    }
#endif
}

void
BlockExprNode::generateReturnPreludeOn(CodeGen &gen)
{
#if 0
    for (auto & v : scope->myHeapVars)
        v.second->generateRestoreOn (gen);
#endif
}

void
BlockExprNode::generateOn(CodeGen &gen)
{
#if 0
    BlockOop block = BlockOopDesc::allocate ();
    CodeGen blockGen (true);

    blockGen.pushCurrentScope (scope);

    for (auto & v : scope->myHeapVars)
        v.second->generatePromoteOn (blockGen);

    for (auto & s : stmts)
    {
        s->generateOn (blockGen);
        if (s != stmts.back ())
            blockGen.genPop ();
        else if (!dynamic_cast<ReturnStmtNode *> (s))
        {
            generateReturnPreludeOn (blockGen);
            blockGen.genReturn ();
        }
    }

    if (stmts.empty ())
    {
        generateReturnPreludeOn (blockGen);
        blockGen.genPushNil ();
        blockGen.genReturn ();
    }

    block->setBytecode (ByteArrayOopDesc::fromVector (blockGen.bytecode ()));
    block->setLiterals (ArrayOopDesc::fromVector (blockGen.literals ()));
    block->setArgumentCount (SmiOop (args.size ()));
    block->setTemporarySize (SmiOop ((intptr_t)0));
    block->setHeapVarsSize (SmiOop (scope->myHeapVars.size ()));
    block->setStackSize (SmiOop (blockGen.maxStackSize ()));

    // gen.popCurrentScope ();
    // if (!blockGen._blockHasBlockReturn)
    //    gen.genPushLiteralObject (block);
    // else
    gen.genPushBlockCopy (block);
#endif
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
#if 0
    expr->generateOn (gen);
    //  FIXME: Do we need to do anything with parents' heapvars? Set them back
    //  or something like that?
    if (gen.isBlock ())
        gen.genBlockReturn ();
    else
        gen.genReturn ();
#endif
}

#pragma mark decls

MethodOop
MethodNode::generate(ObjectMemory &omem)
{
#if 0
    bool finalIsReturn;
    MethodOop meth = MethodOopDesc::allocate ();
    CodeGen gen;

    gen.pushCurrentScope (scope);

    for (auto & v : scope->myHeapVars)
        v.second->generatePromoteOn (gen);

    for (auto s : stmts)
    {
        s->generateOn (gen);
        if (s != stmts.back () ||
            !(finalIsReturn = dynamic_cast<ReturnStmtNode *> (s)))
            gen.genPop ();
    }

    if (!finalIsReturn)
    {
        gen.genPushSelf ();
        gen.genReturn ();
    }

    meth->setSelector (SymbolOopDesc::fromString (sel));
    meth->setBytecode (ByteArrayOopDesc::fromVector (gen.bytecode ()));
    meth->setLiterals (ArrayOopDesc::fromVector (gen.literals ()));
    meth->setArgumentCount (args.size ());
    meth->setTemporarySize (locals.size ());
    meth->setHeapVarsSize (scope->myHeapVars.size ());
    meth->setStackSize (gen.maxStackSize ());

    // meth->print (2);

    gen.popCurrentScope ();

    // REMOVE printf ("\nEnd a method\n\n\n\n\n");

    return meth;
#endif
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