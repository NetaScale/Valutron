#include <cassert>

#include "AST.hh"

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

void
MessageExprNode::synthInScope(Scope *scope)
{
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

	for (auto arg : args)
		scope->addArg(arg);

	for (auto stmt : stmts)
		stmt->synthInScope(scope);
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

	for (auto arg : args)
		scope->addArg(arg);
	for (auto local : locals)
		scope->addLocal(local);
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

	for (int i = 1; i <= aClass->nstVars()->size(); i++)
		scope->addIvar(
		    aClass->nstVars()->basicAt(i).as<SymbolOop>()->asString());
}

void
ClassNode::registerNamesIn(SynthContext &sctx, DictionaryOop ns)
{
	cls = ns->findOrCreateClass(sctx.omem(), ClassOop(), name);
	cls->setNstVars(
	    ArrayOopDesc::symbolArrayFromStringVector(sctx.omem(), iVars));
	cls->setDictionary(ns);
	ns->symbolInsert(sctx.omem(), cls->name(), cls);
	tyClass = sctx.tyChecker().findOrCreateClass(name);

}

void
ClassNode::synthInNamespace(SynthContext &sctx, DictionaryOop ns)
{
	int index = 0;
	scope = new ClassScope;
	ClassOop superCls;

	if (superName != "nil") {
		printf("LOOKING UP SUPERCLASS %s\n\n", superName.c_str());
		superCls = ns->symbolLookupNamespaced(sctx.omem(), superName)
			       .as<ClassOop>();
		if (superCls.isNil()) {
			ns->print(5);
			// memMgr.objGlobals->print(5);
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