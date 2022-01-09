#ifndef AST_HH_
#define AST_HH_

#include <iostream>
#include <list>
#include <map>
#include <vector>

#include "Misc.hh"
#include "ObjectMemory.hh"
#include "Oops.hh"
#include "Typecheck.hh"

/* Details of the position of some source code. */
class Position {
	size_t m_oldLine, m_oldCol, m_oldPos;
	size_t m_line, m_col, m_pos;

    public:
	Position() {};
	Position(size_t oldLine, size_t oldCol, size_t oldPos, size_t line,
	    size_t col, size_t pos)
	    : m_oldLine(oldLine)
	    , m_oldCol(oldCol)
	    , m_oldPos(oldPos)
	    , m_line(line)
	    , m_col(col)
	    , m_pos(pos) {};
	Position(const Position &a, const Position &b)
	    : m_oldLine(a.m_oldLine)
	    , m_oldCol(a.m_oldCol)
	    , m_oldPos(a.m_oldPos)
	    , m_line(b.m_line)
	    , m_col(b.m_col)
	    , m_pos(b.m_pos) {};

	/* Get line number */
	size_t line() const;
	/* Get column number*/
	size_t col() const;
	/* Get absolute position in source-file */
	size_t pos() const;
};

class CodeGen;
class CompilationContext;
class ExprNode;
struct Var;
struct Scope;
struct BlockScope;
struct MethodScope;
struct ClassScope;
struct ClassNode;

typedef int RegisterID;

class SynthContext {
	ObjectMemory &m_omem;
	TyChecker m_tyChecker;

    public:
	SynthContext(ObjectMemory &omem)
	    : m_omem(omem) {};

	ObjectMemory &omem() { return m_omem; }
	TyChecker &tyChecker() { return m_tyChecker; }
};

struct Var {
	bool promoted;
	int promotedIndex;

	enum Kind {
		kHeapVar,
		kParentsHeapVar,
		kSpecial,
		kLocal,
		kArgument,
		kInstance,
		kGlobalConstant,
		kGlobal,
	} kind;

	std::string name;

	virtual int getIndex() { throw; }

	Var(Kind kind, std::string name)
	    : kind(kind)
	    , name(name)
	    , promoted(false)
	{
	}

	void generateOn(CodeGen &gen);
	void generateAssignOn(CodeGen &gen, ExprNode *expr);

	/*
	 * Generates code to move up this variable into myHeapVars.
	 */
	void generatePromoteOn(CodeGen &gen);
	void generateRestoreOn(CodeGen &gen);
};

struct SpecialVar : Var {
	SpecialVar(std::string name)
	    : Var(kSpecial, name)
	{
	}
};

struct IndexedVar : Var {
	int index;

	virtual int getIndex() { return index; }

	IndexedVar(int index, Kind kind, std::string name)
	    : index(index)
	    , Var(kind, name)
	{
	}
};

struct LocalVar : IndexedVar {
	LocalVar(int index, std::string name)
	    : IndexedVar(index, kLocal, name)
	{
	}
};

struct ArgumentVar : IndexedVar {
	ArgumentVar(int index, std::string name)
	    : IndexedVar(index, kArgument, name)
	{
	}
};

struct InstanceVar : IndexedVar {
	InstanceVar(int idx, std::string name)
	    : IndexedVar(idx, kInstance, name)
	{
	}
};

struct HeapVar : IndexedVar {
	HeapVar(int idx, std::string name)
	    : IndexedVar(idx, kHeapVar, name)
	{
	}
};

struct ParentsHeapVar : IndexedVar {
	ParentsHeapVar(int idx, std::string name)
	    : IndexedVar(idx, kParentsHeapVar, name)
	{
	}
};

struct GlobalVar : Var {
	GlobalVar(std::string name)
	    : Var(kGlobal, name)
	{
	}
};

struct Scope {
	virtual Scope *parent() = 0;

	/* FIXME: We store here mappings from parent heapvar numbers to local
	 * heapvar numbers following promotions, because we create new parent
	 * heapvar references all the time and can't just update them by setting
	 * 'promoted' field as with others. This is an ugly and unpleasant hack.
	 */
	std::map<int, int> parentHeapVarToMyHeapVar;

	GlobalVar *addClass(ClassNode *aClass);
	virtual Var *lookup(std::string aName);
};

struct ClassScope : Scope {
	/* Index + 1 = Smalltalk index */
	std::vector<InstanceVar *> iVars;

	Scope *parent() { return NULL; }

	void addIvar(std::string name);
	virtual Var *lookup(std::string aName);
};

struct BlockNode;

struct AbstractCodeScope : Scope {

    protected:
	ParentsHeapVar *promote(Var *aNode);

    public:
	/* Our heapvars that we have promoted herein.
	 * We always search here first.
	 * Pair of MyHeapVar to original Var. */
	// if SECOND is a ParentsHeapVar, then that's fine
	std::vector<std::pair<HeapVar *, Var *>> myHeapVars;

	std::vector<ArgumentVar *> args;
	std::vector<LocalVar *> locals;

	void addArg(std::string name);
	void addLocal(std::string name);
	virtual Var *lookupFromBlock(std::string aName) = 0;
};

struct MethodScope : public AbstractCodeScope {
	ClassScope *_parent;

	MethodScope(ClassScope *parent)
	    : _parent(parent)
	{
	}

	ClassScope *parent() { return _parent; }

	virtual Var *lookup(std::string aName);
	Var *lookupFromBlock(std::string aName);
};

struct BlockScope : public AbstractCodeScope {
	AbstractCodeScope *_parent;
	//   std::vector<ParentsHeapVar *> parentsHeapVars;
	/**
	 * Imagine nested blocks within a method like so:
	 * | x |
	 * [ [ x + 1 ] ]
	 * The outer block does NOT touch the outer method's temporary `x`;
	 * therefore it would not get it promoted. But we need it to be!
	 * Therefore if our lookupFromBlock() doesn't find a variable locally,
	 * we add the parent's heapvar to our list of heapvars that we need to
	 * promote ourselves.
	 */

	AbstractCodeScope *parent() { return _parent; }

	BlockScope(AbstractCodeScope *parent)
	    : _parent(parent)
	{
	}

	virtual Var *lookup(std::string aName);
	Var *lookupFromBlock(std::string aName);
};

struct Node {
	Position m_pos;

	Node() = default;
	Node(Position pos)
	    : m_pos(pos) {};

	virtual void print(int in)
	{
		std::cout << blanks(in) << "<node: " << typeid(*this).name()
			  << "/>\n";
	}
};

struct SelectorNode {
	Type *m_retType;
	std::string m_sel;
	std::vector<VarDecl> m_params;
};

struct StmtNode : Node {
	virtual void synthInScope(Scope *scope) = 0;
	virtual void typeCheck(TyChecker &tyc) = 0;
	virtual void generateOn(CodeGen &gen) = 0;
};

/**
 * \defgroup Expression nodes
 * @{
 */

struct ExprNode : Node {
	ExprNode() = default;
	ExprNode(Position pos)
	    : Node(pos) {};

	virtual void synthInScope(Scope *scope) = 0;
	virtual void typeCheck(TyChecker &tyc)
	{
		std::cout << "Unimplemented typecheck for "
			  << typeid(*this).name() << "\n";
	}
	virtual void generateOn(CodeGen &gen) = 0;

	virtual Type *type(TyChecker &tyc)
	{
		throw std::runtime_error(
		    std::string("No type() for ") + typeid(*this).name());
	}

	virtual bool isSuper() { return false; }
};

struct LiteralExprNode : public ExprNode {
	LiteralExprNode(Position pos)
	    : ExprNode(pos) {};

	virtual void synthInScope(Scope *parentScope) { }
};

/* Character literal */
struct CharExprNode : LiteralExprNode {
	std::string khar;

	CharExprNode(Position pos, std::string aChar)
	    : LiteralExprNode(pos)
	    , khar(aChar)
	{
	}

	void generateOn(CodeGen &gen) override;
};

/* Symbol literal */
struct SymbolExprNode : LiteralExprNode {
	std::string sym;

	SymbolExprNode(Position pos, std::string aSymbol)
	    : LiteralExprNode(pos)
	    , sym(aSymbol)
	{
	}

	Type *type(TyChecker &tyc) override;
	void generateOn(CodeGen &gen) override;
};

/* Integer literal */
struct IntExprNode : LiteralExprNode {
	int num;

	IntExprNode(Position pos, int aNum)
	    : LiteralExprNode(pos)
	    , num(aNum)
	{
	}

	Type *type(TyChecker &tyc) override;
	void generateOn(CodeGen &gen) override;
};

/* String literal */
struct StringExprNode : LiteralExprNode {
	std::string str;

	StringExprNode(Position pos, std::string aString)
	    : LiteralExprNode(pos)
	    , str(aString)
	{
	}

	Type *type(TyChecker &tyc) override;
	void generateOn(CodeGen &gen) override;
};

/* Integer literal */
struct FloatExprNode : LiteralExprNode {
	double num;

	FloatExprNode(Position pos, double aNum)
	    : LiteralExprNode(pos)
	    , num(aNum)
	{
	}

	virtual void generateOn(CodeGen &gen);
};

struct ArrayExprNode : LiteralExprNode {
	std::vector<ExprNode *> elements;

	ArrayExprNode(Position pos, std::vector<ExprNode *> exprs)
	    : LiteralExprNode(pos)
	    , elements(exprs)
	{
	}

	virtual void generateOn(CodeGen &gen);
};

struct IdentExprNode : ExprNode {
	std::string id;
	Var *var;

	virtual bool isSuper() override { return id == "super"; }

	IdentExprNode(std::string id)
	    : id(id)
	    , var(NULL)
	{
	}

	void synthInScope(Scope *scope) override;
	Type *type(TyChecker &tyc) override;
	void generateOn(CodeGen &gen) override;
	void generateAssignOn(CodeGen &gen, ExprNode *rValue);

	void print(int in) override;
};

struct AssignExprNode : ExprNode {
	IdentExprNode *left;
	ExprNode *right;

	AssignExprNode(IdentExprNode *l, ExprNode *r)
	    : left(l)
	    , right(r)
	{
	}

	void synthInScope(Scope *scope) override;
	Type *type(TyChecker &tyc) override;
	void generateOn(CodeGen &gen) override;

	void print(int in) override
	{
		std::cout << blanks(in) << "<assign>\n";
		std::cout << blanks(in + 1) << "<left: " << left->id << ">\n";
		std::cout << blanks(in + 1) << "<right>\n";
		right->print(in + 2);
		std::cout << blanks(in + 1) << "</right>\n";
		std::cout << "</assign>";
	}
};

struct MessageExprNode : ExprNode {
	ExprNode *receiver;
	std::string selector;
	std::vector<ExprNode *> args;

	MessageExprNode(ExprNode *receiver, std::string selector,
	    std::vector<ExprNode *> args = {})
	    : receiver(receiver)
	    , selector(selector)
	    , args(args)
	{
	}

	MessageExprNode(ExprNode *receiver,
	    std::vector<std::pair<std::string, ExprNode *>> list)
	    : receiver(receiver)
	{
		for (auto &p : list) {
			selector += p.first;
			args.push_back(p.second);
		}
	}

	virtual void synthInScope(Scope *scope) override;
	virtual void generateOn(CodeGen &gen) override;
	// void typeCheck(TyChecker &tyc);
	Type *type(TyChecker &tyc) override;
	Type *fullType(TyChecker &tyc, bool cascade = false,
	    Type *recvType = NULL);
	/* if receiver = -1, then assumed to be in accumulator */
	void generateOn(CodeGen &gen, RegisterID receiver, bool isSuper);

	void print(int in) override
	{
		std::cout << blanks(in) << "<message>\n";

		std::cout << blanks(in + 1) << "<receiver>\n";
		receiver->print(in + 2);
		std::cout << blanks(in + 1) << "</receiver>\n";

		std::cout << blanks(in + 1) << "<selector:#" << selector
			  << "\n";

		for (auto e : args) {
			std::cout << blanks(in + 1) << "<arg>\n";
			e->print(in + 2);
			std::cout << blanks(in + 1) << "</arg>\n";
		}

		std::cout << blanks(in) << "</message>\n";
	}
};

struct CascadeExprNode : ExprNode {
	ExprNode *receiver;
	std::vector<MessageExprNode *> messages;

	CascadeExprNode(ExprNode *r)
	    : receiver(r)
	{
		MessageExprNode *m = dynamic_cast<MessageExprNode *>(r);
		if (m) {
			receiver = m->receiver;
			messages.push_back(m);
		}
	}

	void synthInScope(Scope *scope) override;
	Type *type(TyChecker &tyc) override;
	void generateOn(CodeGen &gen) override;

	void print(int in) override
	{
		std::cout << blanks(in) << "<cascade>\n";

		std::cout << blanks(in + 1) << "<receiver>\n";
		receiver->print(in + 2);
		std::cout << blanks(in + 1) << "</receiver>\n";

		for (auto e : messages) {
			e->print(in + 1);
		}

		std::cout << blanks(in) << "</cascade>\n";
	}
};

struct PrimitiveExprNode : ExprNode {
	int num;
	std::vector<ExprNode *> args;

	PrimitiveExprNode(int num, std::vector<ExprNode *> args)
	    : num(num)
	    , args(args)
	{
	}

	void print(int in) override;

	void synthInScope(Scope *scope) override;
	Type *type(TyChecker & tyc) override;
	void generateOn(CodeGen &gen) override;
};

struct BlockExprNode : public ExprNode {
    private:
	friend class Type;

	/**< data needed to set up a TypeChecker for checking */
	TyChecker m_tyc;

	void generateReturnPreludeOn(CodeGen &gen);

    public:
	BlockScope *scope;
	Type *m_retType = NULL;
	std::vector<VarDecl> args;
	std::vector<StmtNode *> stmts;

	BlockExprNode(std::vector<VarDecl> args, std::vector<StmtNode *> stmts)
	    : args(args)
	    , stmts(stmts)
	{
	}

	virtual void synthInScope(Scope *parentScope) override;
	Type *type(TyChecker &tyc) override;
	virtual void generateOn(CodeGen &gen) override;

	void print(int in) override;
};

/**
 * @}
 */

struct ExprStmtNode : StmtNode {
	ExprNode *expr;

	ExprStmtNode(ExprNode *e)
	    : expr(e)
	{
	}

	virtual void synthInScope(Scope *scope);
	void typeCheck(TyChecker &tyc);
	virtual void generateOn(CodeGen &gen);

	virtual void print(int in);
};

struct ReturnStmtNode : StmtNode {
	ExprNode *expr;

	ReturnStmtNode(ExprNode *e)
	    : expr(e)
	{
	}

	void synthInScope(Scope *scope) override;
	void typeCheck(TyChecker &tyc) override;
	void generateOn(CodeGen &gen) override;

	void print(int in) override;
};

struct DeclNode : Node {
	virtual void registerNamesIn(SynthContext &sctx, DictionaryOop ns) = 0;
	virtual void synthInNamespace(SynthContext &sctx, DictionaryOop ns) = 0;
	virtual void typeReg(TyChecker &tyc) = 0;
	virtual void typeCheck(TyChecker &tyc) = 0;
	virtual void generate(ObjectMemory &omem) = 0;
};

struct MethodNode : public Node, public TyEnv {
	MethodScope *scope;

	bool isClassMethod;
	Type *m_retType;
	std::string sel;
	std::vector<VarDecl> m_tyParams;
	std::vector<VarDecl> args;
	std::vector<VarDecl> locals;
	std::vector<StmtNode *> stmts;

	MethodNode(bool isClassMethod, Type *retType, std::string sel,
	    std::vector<VarDecl> tyParams, std::vector<VarDecl> args,
	    std::vector<VarDecl> locals, std::vector<StmtNode *> stmts)
	    : isClassMethod(isClassMethod)
	    , m_retType(retType)
	    , sel(sel)
	    , m_tyParams(tyParams)
	    , args(args)
	    , locals(locals)
	    , stmts(stmts)
	{
	}

	MethodNode *synthInClassScope(ClassScope *clsScope);
	void typeReg(TyChecker &tyc);
	void typeCheck(TyChecker &tyc);
	MethodOop generate(ObjectMemory &omem);

	void print(int in);
};

struct ClassNode : public DeclNode, public TyEnv {
	ClassScope *scope;
	ClassOop cls;

	std::string name;
	std::string superName;
	Type *superType;
	std::vector<VarDecl> m_tyParams;
	std::vector<VarDecl> cVars;
	std::vector<VarDecl> iVars;
	std::vector<MethodNode *> cMethods;
	std::vector<MethodNode *> iMethods;

	TyClass *tyClass;

	/* Resolved later */
	// GlobalVar * superClass;

	ClassNode(std::string name, std::vector<VarDecl> m_tyParams,
	    std::string superName, std::vector<Type *> superTyArgs,
	    std::vector<VarDecl> cVars, std::vector<VarDecl> iVars);

	void addMethods(std::vector<MethodNode *> meths);

	void registerNamesIn(SynthContext &sctx, DictionaryOop ns);
	void synthInNamespace(SynthContext &sctx, DictionaryOop ns);
	void typeReg(TyChecker &tyc);
	void typeCheck(TyChecker &tyc);

	void generate(ObjectMemory &omem);

	void print(int in);
};

struct NamespaceNode : public DeclNode {
	std::string name;
	std::vector<DeclNode *> decls;

	NamespaceNode(std::string name, std::vector<DeclNode *> decls)
	    : name(name)
	    , decls(decls)
	{
	}

	void registerNamesIn(SynthContext &sctx, DictionaryOop ns);
	void synthInNamespace(SynthContext &sctx, DictionaryOop ns);
	void typeReg(TyChecker &tyc);
	void typeCheck(TyChecker &tyc);
	void generate(ObjectMemory &omem);
};

struct ProgramNode : public DeclNode {
	std::vector<DeclNode *> decls;

	ProgramNode(std::vector<DeclNode *> decls)
	    : decls(decls)
	{
	}

	void registerNamesIn(SynthContext &sctx, DictionaryOop ns);
	void registerNames(SynthContext &sctx)
	{
		registerNamesIn(sctx, ObjectMemory::objGlobals);
	}

	void synthInNamespace(SynthContext &sctx, DictionaryOop ns);
	void synth(SynthContext &sctx)
	{
		synthInNamespace(sctx, ObjectMemory::objGlobals);
	}
	void typeReg(TyChecker &tyc);
	void typeCheck(TyChecker &tyc);
	void generate(ObjectMemory &omem);

	void print(int in);
};

#endif /* AST_HH_ */
