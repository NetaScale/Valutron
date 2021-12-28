#ifndef AST_HH_
#define AST_HH_

#include <iostream>
#include <list>
#include <map>
#include <vector>

#include "Misc.hh"
#include "Oops.hh"
#include "ObjectMemory.hh"

class CodeGen;
class CompilationContext;
class ExprNode;
struct Var;
struct Scope;
struct BlockScope;
struct MethodScope;
struct ClassScope;
struct ClassNode;

class SynthContext {
	ObjectMemory & m_omem;

	public:
	SynthContext(ObjectMemory & omem) : m_omem(omem) {};

	ObjectMemory&omem() { return m_omem; }
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
	virtual void print(int in)
	{
		std::cout << blanks(in) << "<node: " << typeid(*this).name()
			  << "/>\n";
	}
};

struct StmtNode : Node {
	virtual void synthInScope(Scope *scope) = 0;
	virtual void generateOn(CodeGen &gen) = 0;
};

struct ExprStmtNode : StmtNode {
	ExprNode *expr;

	ExprStmtNode(ExprNode *e)
	    : expr(e)
	{
	}

	virtual void synthInScope(Scope *scope);
	virtual void generateOn(CodeGen &gen);

	virtual void print(int in);
};

struct ReturnStmtNode : StmtNode {
	ExprNode *expr;

	ReturnStmtNode(ExprNode *e)
	    : expr(e)
	{
	}

	virtual void synthInScope(Scope *scope);
	virtual void generateOn(CodeGen &gen);

	virtual void print(int in);
};

struct DeclNode : Node {
	virtual void registerNamesIn(DictionaryOop ns) = 0;
	virtual void synthInNamespace(DictionaryOop ns) = 0;
	virtual void generate() = 0;
};

struct MethodNode : public Node {
	MethodScope *scope;

	bool isClassMethod;
	std::string sel;
	std::vector<std::string> args;
	std::vector<std::string> locals;
	std::vector<StmtNode *> stmts;

	MethodNode(bool isClassMethod, std::string sel,
	    std::vector<std::string> args, std::vector<std::string> locals,
	    std::vector<StmtNode *> stmts)
	    : isClassMethod(isClassMethod)
	    , sel(sel)
	    , args(args)
	    , locals(locals)
	    , stmts(stmts)
	{
	}

	MethodNode *synthInClassScope(ClassScope *clsScope);
	MethodOop generate();

	void print(int in);
};

struct ClassNode : public DeclNode {
	ClassScope *scope;
	ClassOop cls;

	std::string name;
	std::string superName;
	std::vector<std::string> cVars;
	std::vector<std::string> iVars;
	std::vector<MethodNode *> cMethods;
	std::vector<MethodNode *> iMethods;

	/* Resolved later */
	// GlobalVar * superClass;

	ClassNode(std::string name, std::string superName,
	    std::vector<std::string> cVars, std::vector<std::string> iVars);

	void addMethods(std::vector<MethodNode *> meths);

	void registerNamesIn(DictionaryOop ns);
	void synthInNamespace(DictionaryOop ns);
	void generate();

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

	void registerNamesIn(DictionaryOop ns);
	void synthInNamespace(DictionaryOop ns);
	void generate();
};

struct ProgramNode : public DeclNode {
	std::vector<DeclNode *> decls;

	ProgramNode(std::vector<DeclNode *> decls)
	    : decls(decls)
	{
	}

	void registerNamesIn(DictionaryOop ns);
	void registerNames()
	{
		registerNamesIn (ObjectMemory::objGlobals);
	}

	void synthInNamespace(DictionaryOop ns);
	void synth()
	{
		synthInNamespace (ObjectMemory::objGlobals);
	}
	void generate();

	void print(int in);
};

/**
 * \defgroup Expression nodes
 * @{
 */

struct ExprNode : Node {
	virtual void synthInScope(Scope *scope) = 0;
	virtual void generateOn(CodeGen &gen) = 0;

	virtual bool isSuper() { return false; }
};

struct LiteralExprNode : ExprNode {
	virtual void synthInScope(Scope *parentScope) { }
};

/* Character literal */
struct CharExprNode : LiteralExprNode {
	std::string khar;

	CharExprNode(std::string aChar)
	    : khar(aChar)
	{
	}

	virtual void generateOn(CodeGen &gen);
};

/* Symbol literal */
struct SymbolExprNode : LiteralExprNode {
	std::string sym;

	SymbolExprNode(std::string aSymbol)
	    : sym(aSymbol)
	{
	}

	virtual void generateOn(CodeGen &gen);
};

/* Integer literal */
struct IntExprNode : LiteralExprNode {
	int num;

	IntExprNode(int aNum)
	    : num(aNum)
	{
	}

	virtual void generateOn(CodeGen &gen);
};

/* String literal */
struct StringExprNode : LiteralExprNode {
	std::string str;

	StringExprNode(std::string aString)
	    : str(aString)
	{
	}

	virtual void generateOn(CodeGen &gen);
};

/* Integer literal */
struct FloatExprNode : LiteralExprNode {
	double num;

	FloatExprNode(double aNum)
	    : num(aNum)
	{
	}

	virtual void generateOn(CodeGen &gen);
};

struct ArrayExprNode : LiteralExprNode {
	std::vector<ExprNode *> elements;

	ArrayExprNode(std::vector<ExprNode *> exprs)
	    : elements(exprs)
	{
	}

	virtual void generateOn(CodeGen &gen);
};

struct PrimitiveExprNode : ExprNode {
	int num;
	std::vector<ExprNode *> args;

	PrimitiveExprNode(int num, std::vector<ExprNode *> args)
	    : num(num)
	    , args(args)
	{
	}

	virtual void print(int in);

	virtual void synthInScope(Scope *scope);
	virtual void generateOn(CodeGen &gen);
};

struct IdentExprNode : ExprNode {
	std::string id;
	Var *var;

	virtual bool isSuper() { return id == "super"; }

	IdentExprNode(std::string id)
	    : id(id)
	    , var(NULL)
	{
	}

	virtual void synthInScope(Scope *scope);
	virtual void generateOn(CodeGen &gen);
	virtual void generateAssignOn(CodeGen &gen, ExprNode *rValue);

	void print(int in);
};

struct AssignExprNode : ExprNode {
	IdentExprNode *left;
	ExprNode *right;

	AssignExprNode(IdentExprNode *l, ExprNode *r)
	    : left(l)
	    , right(r)
	{
	}

	virtual void synthInScope(Scope *scope);
	virtual void generateOn(CodeGen &gen);

	void print(int in)
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

	virtual void synthInScope(Scope *scope);
	virtual void generateOn(CodeGen &gen) { generateOn(gen, false); }
	virtual void generateOn(CodeGen &gen, bool cascade);

	void print(int in)
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

	virtual void synthInScope(Scope *scope);
	virtual void generateOn(CodeGen &gen);

	void print(int in)
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

struct BlockExprNode : ExprNode {
    private:
	void generateReturnPreludeOn(CodeGen &gen);

    public:
	BlockScope *scope;
	std::vector<std::string> args;
	std::vector<StmtNode *> stmts;

	BlockExprNode(std::vector<std::string> a, std::vector<StmtNode *> s)
	    : args(a)
	    , stmts(s)
	{
	}

	virtual void synthInScope(Scope *parentScope);
	virtual void generateOn(CodeGen &gen);

	void print(int in);
};

/**
 * @}
 */

#endif /* AST_HH_ */
