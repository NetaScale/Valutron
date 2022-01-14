#include <cassert>

#include "AST.hh"
#include "Oops.hh"
#include "Interpreter.hh"

Var *
Scope::lookup(std::string name)
{
	if (parent())
		return parent()->lookup(name);
	else {
		/*printf ("In scope %s: Failed to find %s, assuming GLOBAL.\n",
			typeid (*this).name (),
			name.c_str ());*/
		return new GlobalVar(name);
	}
}

Var *
ClassScope::lookup(std::string name)
{
	for (auto v : iVars)
		if (v->name == name)
			return v;
	return Scope::lookup(name);
}

void
ClassScope::addIvar(std::string name)
{
	iVars.push_back(new InstanceVar(iVars.size() + 1, name));
}

void
AbstractCodeScope::addArg(std::string name)
{
	args.push_back(new ArgumentVar(args.size() + 1, name));
}

void
AbstractCodeScope::addLocal(std::string name)
{
	locals.push_back(new LocalVar(locals.size() + 1, name));
}

ParentsHeapVar *
AbstractCodeScope::promote(Var *aNode)
{
	HeapVar *newVar = new HeapVar(myHeapVars.size() + 1, aNode->name);
	aNode->promoted = true;
	aNode->promotedIndex = newVar->index;
	myHeapVars.push_back({ newVar, aNode });
	return new ParentsHeapVar(myHeapVars.size(), aNode->name);
}

Var *
MethodScope::lookup(std::string aName)
{
	for (auto v : locals)
		if (v->name == aName)
			return v;
	for (auto v : args)
		if (v->name == aName)
			return v;
	return AbstractCodeScope::lookup(aName);
}

Var *
MethodScope::lookupFromBlock(std::string aName)
{
	for (auto v : myHeapVars)
		/* already promoted */
		if (v.first->name == aName)
			return new ParentsHeapVar(v.first->index, aName);
	for (auto v : locals)
		if (v->name == aName)
			return promote(v);
	for (auto v : args)
		if (v->name == aName)
			return promote(v);
	return AbstractCodeScope::lookup(aName);
}

Var *
BlockScope::lookup(std::string aName)
{
	Var *candidate;
	for (auto v : myHeapVars)
		if (v.first->name == aName)
			return v.first;
	for (auto v : args)
		if (v->name == aName)
			return v;
	return parent()->lookupFromBlock(aName);
}

Var *
BlockScope::lookupFromBlock(std::string aName)
{
	Var *par;
	ParentsHeapVar *parCand;
	for (auto v : myHeapVars)
		/* already promoted */
		if (v.first->name == aName)
			return new ParentsHeapVar(v.first->index, aName);
	for (auto v : locals)
		if (v->name == aName)
			return promote(v);
	for (auto v : args)
		if (v->name == aName)
			return promote(v);

	par = parent()->lookupFromBlock(aName);

	if ((parCand = dynamic_cast<ParentsHeapVar *>(par)))
		/*
		 * Parent has a heapvar for it, but we don't have a local
		 * reference! Therefore let us create a local copy in our own
		 * heapvars.
		 */
		return promote(par);
	return par;
}

#pragma expressions

void
PrimitiveExprNode::synthInScope(Scope *scope)
{
	num = Primitive::named(name.c_str());
	if (num == -1)
		throw std::runtime_error("Unknown primitive " + name);

	for (auto arg : args)
		arg->synthInScope(scope);
}

void
IdentExprNode::synthInScope(Scope *scope)
{
	var = scope->lookup(id);
}

void
AssignExprNode::synthInScope(Scope *scope)
{
	left->synthInScope(scope);
	right->synthInScope(scope);
}

static int
isOptimisedBinop(std::string sel)
{
	for (int i = 0; i < sizeof(ObjectMemory::binOpStr) / sizeof(*ObjectMemory::binOpStr); i++)
		if (sel == ObjectMemory::binOpStr[i])
			return i;
	return -1;
}

void
MessageExprNode::synthInScope(Scope *scope)
{
	if (receiver->isSuper())
		goto plain;

	if (selector == "and:") {
		receiver->synthInScope(scope);
		m_specialKind = kAnd;
		args[0]->synthInlineInScope(scope);
		return;
	} else if (selector == "ifTrue:ifFalse:") {
		receiver->synthInScope(scope);
		m_specialKind = kIfTrueIfFalse;
		args[0]->synthInlineInScope(scope);
		args[1]->synthInlineInScope(scope);
		return;
	} else if (selector == "ifFalse:ifTrue:") {
		receiver->synthInScope(scope);
		m_specialKind = kIfFalseIfTrue;
		args[0]->synthInlineInScope(scope);
		args[1]->synthInlineInScope(scope);
		return;
	} else if (selector == "ifTrue:") {
		receiver->synthInScope(scope);
		m_specialKind = kIfTrue;
		args[0]->synthInlineInScope(scope);
		return;
	} else if (selector == "ifFalse:") {
		receiver->synthInScope(scope);
		m_specialKind = kIfFalse;
		args[0]->synthInlineInScope(scope);
		return;
	} else if (selector == "whileTrue") {
		receiver->synthInlineInScope(scope);
		m_specialKind = kWhileTrue0;
		return;
	} else if (selector == "whileFalse") {
		receiver->synthInlineInScope(scope);
		m_specialKind = kWhileFalse0;
		return;
	} else if (selector == "whileTrue:") {
		receiver->synthInlineInScope(scope);
		m_specialKind = kWhileTrue;
		args[0]->synthInlineInScope(scope);
		return;
	} else if (selector == "whileFalse:") {
		receiver->synthInlineInScope(scope);
		m_specialKind = kWhileFalse;
		args[0]->synthInlineInScope(scope);
		return;
	}
	 else {
		int binOp = isOptimisedBinop(selector);

		if (binOp != -1)
			m_specialKind = (SpecialKind) (kBinOp + binOp);
	}

plain:
	receiver->synthInScope(scope);

	for (auto arg : args)
		arg->synthInScope(scope);
}

void
CascadeExprNode::synthInScope(Scope *scope)
{
	receiver->synthInScope(scope);
	for (auto message : messages)
		message->synthInScope(scope);
}

void
BlockExprNode::synthInScope(Scope *parentScope)
{
	scope = new BlockScope(dynamic_cast<AbstractCodeScope *>(parentScope));

	for (auto & arg : args)
		scope->addArg(arg.first);

	for (auto stmt : stmts)
		stmt->synthInScope(scope);
}

void
BlockExprNode::synthInlineInScope(Scope *parentScope)
{
	assert(args.empty());

	m_inlined = true;

	for (auto stmt : stmts)
		stmt->synthInScope(parentScope);
}

#pragma statements

void
ExprStmtNode::synthInScope(Scope *scope)
{
	expr->synthInScope(scope);
}

void
ReturnStmtNode::synthInScope(Scope *scope)
{
	expr->synthInScope(scope);
}

#pragma decls

MethodNode *
MethodNode::synthInClassScope(ClassScope *clsScope)
{
	/* We expect to get a class node already set up with its ivars etc. If
	 * not, no problem. */
	scope = new MethodScope(clsScope);

	for (auto & arg : args)
		scope->addArg(arg.first);
	for (auto & local : locals)
		scope->addLocal(local.first);
	for (auto stmt : stmts)
		stmt->synthInScope(scope);

	return this;
}

static void
classOopAddIvarsToScopeStartingFrom(ClassOop aClass, ClassScope *scope)
{
	ClassOop superClass = aClass->superClass();

	if (!superClass.isNil())
		classOopAddIvarsToScopeStartingFrom(superClass, scope);

	for (int i = 0; i < aClass->nstVars()->size(); i++)
		scope->addIvar(
		    aClass->nstVars()->basicAt0(i).as<SymbolOop>()->asString());
}

void
ClassNode::registerNamesIn(SynthContext &sctx, DictionaryOop ns)
{
	std::vector<std::string> ivarNames;

	for (auto & var: iVars)
		ivarNames.push_back(var.first);

	cls = ns->findOrCreateClass(sctx.omem(), ClassOop(), name);
	cls->setNstVars(ArrayOopDesc::symbolArrayFromStringVector(sctx.omem(),
	    ivarNames));
	cls->setDictionary(ns);
	ns->symbolInsert(sctx.omem(), cls->name(), cls);
	sctx.tyChecker().findOrCreateClass(this);
}

void
ClassNode::synthInNamespace(SynthContext &sctx, DictionaryOop ns)
{
	int index = 0;
	scope = new ClassScope;
	ClassOop superCls;

	if (superName != "nil") {
		superCls = ns->symbolLookupNamespaced(sctx.omem(), superName)
			       .as<ClassOop>();
		if (superCls.isNil()) {

		}
		assert(!superCls.isNil());
	}

	cls->setupSuperclass(superCls);

	classOopAddIvarsToScopeStartingFrom(cls, scope);
	cls->setNstSize(SmiOop(scope->iVars.size()));

	for (auto meth : cMethods)
		meth->synthInClassScope(scope);

	for (auto meth : iMethods) {
		meth->synthInClassScope(scope);
	}
}

void
NamespaceNode::registerNamesIn(SynthContext &sctx, DictionaryOop ns)
{
	DictionaryOop newNs = ns->subNamespaceNamed(sctx.omem(), name);
	for (auto d : decls)
		d->registerNamesIn(sctx, newNs);
}

void
NamespaceNode::synthInNamespace(SynthContext &sctx, DictionaryOop ns)
{
	DictionaryOop newNs = ns->subNamespaceNamed(sctx.omem(), name);
	for (auto d : decls)
		d->synthInNamespace(sctx, newNs);
}

void
ProgramNode::registerNamesIn(SynthContext &sctx, DictionaryOop ns)
{
	for (auto d : decls)
		d->registerNamesIn(sctx, ns);
}

void
ProgramNode::synthInNamespace(SynthContext &sctx, DictionaryOop ns)
{
	for (auto d : decls)
		d->synthInNamespace(sctx, ns);
}