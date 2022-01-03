#include <cassert>
#include <stdexcept>
#include <string>

#include "AST.hh"
#include "Typecheck.hh"

struct Invocation {
	Type *receiver;
	bool isClass = false; /**< is it the class or the instance? */
	std::map<std::string, Type *> tyParamMap;
};

const char *Type::kindStr[kMax] = {
	"UnresolvedIdentifier",

	"NotYetInferred",

	"Block",
	"Self",
	"InstanceType",
	"id",

	"TyVar",
	"Class",
	"Instance",
};

Type::Type(std::string ident, std::vector<Type *> typeArgs)
    : m_ident(ident)
    , m_typeArgs(typeArgs)
{
	if (ident == "Self")
		m_kind = kSelf;
	else if (ident == "InstanceType")
		m_kind = kInstanceType;
	else if (ident == "id")
		m_kind = kId;
	else
		m_kind = kIdent;
}

Type::Type(TyClass *cls)
{
	m_ident = cls->m_name;
	m_kind = kClass;
}

Type::Type(TyClass *cls, std::string ident, std::vector<VarDecl> &tyParams)
{
	m_kind = kInstance;
	m_ident = ident;
	m_cls = cls;
	if (tyParams.size()) {
		m_isConstructor = true;
		for (auto &tyParam : tyParams) {
			m_typeArgs.push_back(Type::makeTyVarReference(&tyParam));
		}
	}
}

Type *Type::makeTyVarReference(VarDecl * varDecl)
{
	Type * type = new Type;
	type->m_ident = varDecl->first;
	type->m_kind = kTyVar;
	type->m_tyVarDecl = varDecl;
	return type;
}

void Type::resolveInTyEnv(TyEnv * env)
{
	if (m_kind == kIdent)
	{
		Type * type =  env->lookupType(m_ident);
		type->resolveInTyEnv(env); // ???
		type->constructInto(this);
		for (auto & arg: m_typeArgs)
		{
			arg->resolveInTyEnv(env); // ???
		}
	}
	else {
	printf("\n\nType already resolved??\n");
	print(5);
	printf("\n");
	}
}

bool
Type::isSubtypeOf(Type *type)
{
	switch (m_kind) {
	case kIdent:
	case kAsYetUnspecified: {
		std::cerr << "Inferring uninferred LHS to be " << *type;
		*this = *type;
		return true;
	}
	case kBlock:
	case kSelf:
	case kInstanceType:
		abort();

	case kId:
		return true;

	case kTyVar:
		abort();

	case kClass:
		abort();

	case kInstance: {
		switch (type->m_kind) {
		case kIdent:
			abort();

		case kAsYetUnspecified:
			std::cerr << "Inferring uninferred RHS to be " << *this;
			*type = *this;
			return true;

		case kBlock:
		case kSelf:
		case kInstanceType:
			abort();

		case kId:
			return true;

		case kTyVar:
			abort();

		case kClass:
			abort();

		case kInstance:
			printf("Hello\n");

		case kMax:
			abort();
		}
	}

	case kMax:
		abort();
	}
}

Type *
Type::typeInInvocation(Invocation &invocation)
{
	switch (m_kind) {
	case kIdent:
	case kAsYetUnspecified:
	case kBlock:
		abort();

	case kSelf:
		return invocation.receiver;

	case kInstanceType:
		abort();

	case kId:
		return this;

	case kTyVar:
		return invocation.tyParamMap[m_ident];

	case kClass:
	case kInstance: {
		abort();
	}

	default:
		abort();
	}
}

void
Type::constructInto(Type *into)
{
	switch (m_kind) {
	case kInstance:
		into->m_cls = m_cls;
		into->m_kind = kInstance;
		if (!into->m_typeArgs.empty() && into->m_typeArgs.size() !=
		    m_typeArgs.size())
			throw std::runtime_error(
			    "Unexpected type argument count " +
			    std::to_string(into->m_typeArgs.size()));
		else if (into->m_typeArgs.empty() && !m_typeArgs.empty())
		{
			into->m_typeArgs.resize(m_typeArgs.size(), NULL);
			for (int i = 0; i < m_typeArgs.size(); i++)
				into->m_typeArgs[i] = new Type;
		}
		break;

	case kTyVar: {
		into->m_kind = kTyVar;
		into->m_tyVarDecl = m_tyVarDecl;
		break;
	}

	default:
		throw std::runtime_error("Unexpected kind " +
		    std::to_string(m_kind));
	}
}

void
Type::print(size_t in)
{
	std::cout << blanks(in) << "Type {\n";
	in += 2;

	std::cout << blanks(in) << "Kind: " << kindStr[m_kind];
	if (m_isConstructor)
		std::cout << " Constructor";
	std::cout << "\n";

	std::cout << blanks(in) << "Ident: " << m_ident << "\n";

	if(m_tyVarDecl && m_tyVarDecl->second != NULL)
	{
		std::cout << blanks(in) << "Bound: \n" << "\n";
		m_tyVarDecl->second->print(in + 2);
	}

	if (m_wrapped != NULL) {
		std::cout << blanks(in) << "Wrapping: \n";
		m_wrapped->print(in + 2);
	}

	if (!m_typeArgs.empty()) {
		std::cout << blanks(in) << "Args: [\n";
		for (auto &arg : m_typeArgs)
			arg->print(in + 2);
		std::cout << blanks(in) << "]\n";
	}

	if (m_cls)
		std::cout << blanks(in) << "Class: " << m_cls->m_name << "\n";

	in -= 2;
	std::cout << blanks(in) << "}\n";
}

std::ostream &
operator<<(std::ostream &os, Type const &type)
{
	return type.niceName(os);
}

std::ostream &
Type::niceName(std::ostream &os) const
{
	switch (m_kind) {
	case kIdent:
		return os << "{unresolved-identifier" << m_ident << "}";

	case kAsYetUnspecified:
		return os << "{not-yet-inferred}";

	case kBlock:
		return os << "[{block}]";

	case kSelf:
		return os << "self";

	case kInstanceType:
		return os << "instancetype";

	case kId:
		return os << "id";

	case kTyVar:
		os << "typeParameter " << m_ident; 
		
		if (m_wrapped)
			return os << "(" << (*m_wrapped) << ")";
		else
			return os;

	case kClass:
		return os << m_ident << " class";

	case kInstance: {
		os << m_ident;
		if (m_typeArgs.size()) {
			bool isFirst = true;
			os << "<";
			for (auto &arg : m_typeArgs) {
				if (isFirst)
					isFirst = false;
				else
					os << " ,";
				os << *arg;
			}
			os << ">";
		}
		return os;
	}

	case kMax:
		abort();
	}
}

Type *
TyEnv::lookupVar(std::string &txt)
{
	if (m_vars.find(txt) != m_vars.end())
		return m_vars[txt];
	else if (m_parent)
		return m_parent->lookupVar(txt);
	else
		return NULL;
}

Type *
TyEnv::lookupType(std::string &txt)
{
	if (m_types.find(txt) != m_types.end())
		return m_types[txt];
	else if (m_parent)
		return m_parent->lookupType(txt);
	else
		return NULL;
}

TyClass *
TyChecker::findOrCreateClass(ClassNode *cls)
{
	Type *type = m_globals->lookupVar(cls->name);
	if (type && type->m_kind == Type::kClass)
		return type->m_cls;
	else {
		TyClass *tyc = new TyClass;
		tyc->m_name = cls->name;
		tyc->m_clsNode = cls;
		m_globals->m_vars[cls->name] = new Type(tyc);
		m_globals->m_types[cls->name] = new Type(tyc, cls->name,
		    cls->m_tyParams);
		return tyc;
	}
}

TyChecker::TyChecker()
{
	m_globals = new TyEnv;
	m_envs.push_back(m_globals);
}

void
MethodNode::typeReg(TyChecker &tyc)
{
	TyEnv::m_parent = tyc.m_envs.back();
	auto tyClass = m_parent->m_tyClass;
	tyc.m_envs.push_back(this);

	assert(tyClass);

	m_retType->resolveInTyEnv(this);

	for (auto &var : args) {
		var.second->resolveInTyEnv(this);
		m_vars[var.first] = var.second;
	}

	for (auto & var: locals)
	{
		printf("METHOD LOCAL %s: \n", var.first.c_str());
		var.second->print(2);
		var.second->resolveInTyEnv(this);
		printf("NEU TYPE: \n");
		var.second->print(2);
		printf("END METHOD TYPE\n");
		m_vars[var.first] = var.second;
	}

	tyc.m_envs.pop_back();
}

void
ClassNode::typeReg(TyChecker &tyc)
{
	TyEnv::m_parent = tyc.m_envs.back();
	TyEnv::m_tyClass = tyClass;
	tyc.m_envs.push_back(this);

	for (auto & tyParam : m_tyParams) {
		TyEnv::m_types[tyParam.first] = Type::makeTyVarReference(&tyParam);
	}

	if (superName != "nil") {
		printf("ORIGINAL TYPE: \n");
		superType->print(2);
		superType->resolveInTyEnv(this);
		printf("NEU TYPE: \n");
		superType->print(2);
	} else {
		delete superType;
		superType = NULL;
	}

	for (auto &var : iVars) {
		if (var.second) {
			printf("  IVAR %s: \n", var.first.c_str());
			var.second->print(4);
			var.second->resolveInTyEnv(this);
			printf("  NEU TYPE: \n");
			var.second->print(4);
			printf("  END IVAR TYPE\n");
		} else
			var.second = new Type();
		m_vars[var.first] = var.second;
	}

	for (auto &meth : cMethods)
		meth->typeReg(tyc);

	for (auto &meth : iMethods)
		meth->typeReg(tyc);

	tyc.m_envs.pop_back();
}

void
NamespaceNode::typeReg(TyChecker &tyc)
{
	for (auto cls : decls)
		cls->typeReg(tyc);
}

void
ProgramNode::typeReg(TyChecker &tyc)
{
	for (auto cls : decls)
		cls->typeReg(tyc);
}

/**
 * Type inferencing and type checking
 */

Type *
IdentExprNode::type(TyChecker &tyc)
{
	return tyc.m_envs.back()->lookupVar(id);
}

void
MessageExprNode::typeCheck(TyChecker &tyc)
{
	Type *recvType = receiver->type(tyc);
	std::vector<Type *> argTypes;
	Type *res;

	for (auto &arg : args)
		argTypes.push_back(arg->type(tyc));

	printf("RECEIVER TYPE:\n");
	recvType->print(10);
	printf("ARG TYPES\n");
	for (auto &argType : argTypes)
		argType->print(8);

	res = recvType->typeSend(selector, argTypes);

	std::cout << " -> RESULT OF SEND: " << *res << "\n";
}

Type *
Type::typeSend(std::string selector, std::vector<Type *> &argTypes)
{
	switch (m_kind) {
	case kInstance: {
		MethodNode *meth = NULL;
		Invocation invoc;

		for (auto &aMeth : m_cls->m_clsNode->iMethods)
			if (aMeth->sel == selector)
				meth = aMeth;

		if (!meth) {
			std::cerr << "Object of type " << *this
				  << " does not appear to understand message "
				  << selector << "\n";
			return NULL;
		}

		for (int i = 0; i < m_typeArgs.size(); i++)
			invoc.tyParamMap[m_cls->m_clsNode->m_tyParams[i].
			    first] = m_typeArgs[i];

		for (int i = 0; i < argTypes.size(); i++) {
			Type *formal = meth->args[i].second->typeInInvocation(
			    invoc);
			if (!argTypes[i]->isSubtypeOf(formal))
				std::cerr << "Argument of type " << *this
					  << "is not a subtype of " << *formal
					  << "\n";
		}

		return meth->m_retType->typeInInvocation(invoc);
	}

	default:
		std::cout << "Unexpected kind " << kindStr[m_kind] << "\n";
		return NULL;
		break;
	}
}

void
ReturnStmtNode::typeCheck(TyChecker &tyc)
{
	expr->typeCheck(tyc);
}

void
MethodNode::typeCheck(TyChecker &tyc)
{
	tyc.m_envs.push_back(this);

	for (auto &stmt : stmts)
		stmt->typeCheck(tyc);

	tyc.m_envs.pop_back();
}

void
ClassNode::typeCheck(TyChecker &tyc)
{
	tyc.m_envs.push_back(this);

	for (auto & meth: iMethods)
		meth->typeCheck(tyc);

	tyc.m_envs.pop_back();
}

void
NamespaceNode::typeCheck(TyChecker &tyc)
{
	for (auto cls : decls)
		cls->typeCheck(tyc);
}

void
ProgramNode::typeCheck(TyChecker &tyc)
{
	for (auto cls : decls)
		cls->typeCheck(tyc);
}
