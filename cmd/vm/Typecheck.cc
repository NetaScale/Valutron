#include <cassert>
#include <stdexcept>
#include <string>

#include "AST.hh"
#include "Typecheck.hh"

struct Invocation {
	Invocation * m_super = NULL; /**< invocation created for sub type */
	Type *receiver, *trueReceiver;
	bool isClass = false; /**< is it the class or the instance? */
	bool isSuper = false;
	std::map<VarDecl *, Type *> tyParamMap;
	Type * tyParam(VarDecl * varDecl);
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

TyBlock *
TyBlock::deepCopy()
{
	TyBlock *tyBlock = new TyBlock;
	Type *type;

	*tyBlock = *this;

	for (auto &argTy : tyBlock->m_argTypes) {
		Type *type = new Type;
		*type = *argTy;
		argTy = type;
	}

	type = new Type;
	*type = *m_retType;
	tyBlock->m_retType = type;

	return tyBlock;
}

Type * Invocation::tyParam(VarDecl * varDecl)
{
	assert(varDecl);

	if (tyParamMap.find(varDecl) != tyParamMap.end())
		return tyParamMap[varDecl];
	else if (m_super)
		return m_super->tyParam(varDecl);
	else
		return NULL;
}

std::ostream& italic(std::ostream& os)
{
    return os << "\e[3m";
}

std::ostream& normal(std::ostream& os)
{
    return os << "\e[0m";
}

Type::Type(std::string ident, std::vector<Type *> typeArgs)
    : m_ident(ident)
    , m_typeArgs(typeArgs)
{
	if (ident == "self")
		m_kind = kSelf;
	else if (ident == "instancetype")
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
	m_cls = cls;
}

Type * Type::id()
{
	Type * type = new Type;
	type->m_kind = kId;
	return type;
}

Type * Type::makeInstanceMaster(TyClass *cls, std::vector<VarDecl> &tyParams)
{
	Type * type = new Type;
	type->m_kind = kInstance;
	type->m_ident = cls->m_name;
	type->m_cls = cls;
	if (tyParams.size()) {
		type->m_isConstructor = true;
		for (auto &tyParam : tyParams)
			type->m_typeArgs.push_back(Type::makeTyVarReference(
			    &tyParam));
	}
	return type;
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
	if (m_kind == kIdent) {
		Type * type = env->lookupType(m_ident);
		type->resolveInTyEnv(env); // ???
		type->constructInto(this);
		for (auto & arg: m_typeArgs)
			arg->resolveInTyEnv(env); // ???
	} else if (m_kind == kBlock) {
		m_block->TyEnv::m_parent = env;
		/* Register block's generic type parameters */
		for (auto & tyParam: m_block->m_tyParams) {
			m_block->TyEnv::m_types[tyParam.first] =
			     Type::makeTyVarReference(&tyParam);
		}
		for (auto & type: m_block->m_argTypes)
			type->resolveInTyEnv(m_block);
		m_block->m_retType->resolveInTyEnv(m_block);
	} else if (m_kind == kUnion) {
		for (auto &type : m_members)
			type->resolveInTyEnv(env);
	}
}

bool
Type::isSubtypeOf(TyEnv * env, Type *type)
{
	if (type->m_kind == kAsYetUnspecified) {
		std::cerr << "Inferring uninferred RHS to be " << *this << "\n";
		*type = *this;
		type->m_wasInferred = true;
		return true;
	} else if (type->m_kind == kBlock && m_kind != kBlock)
		/* only a block can be a subtype of a block */
		return false;
	else if (m_kind == kUnion) {
		/**
		 * A union $a is a subtype of a type $b if every member of union
		 * $a is a subtype of type $b.
		 */
		for (auto &member : m_members)
			if (!member->isSubtypeOf(env, type)) {
				std::cout << *member << " is not a subtype of "
					  << *type << "\n";
				return false;
			}
		return true;
	} else if (type->m_kind == kUnion) {
		/**
		 * A type $a is a subtype of a union type $b if type $a is a
		 * subtype of at least one member of $b.
		 */
		for (auto &member : type->m_members)
			if (isSubtypeOf(env, member))
				return true;
		return false;
	}

	switch (m_kind) {
	case kIdent:
		abort();

	case kAsYetUnspecified: {
		std::cerr << "Inferring uninferred LHS to be " << *type << "\n";
		*this = *type;
		this->m_wasInferred = true;
		return true;
	}

	case kBlock: {
		/* TODO: expand own tyvars? <- not sure what this means */

		if (type->m_kind != kBlock)
			return false;

		Type *newBlk = new Type;

		*newBlk = *this;

		auto &rhsRType = newBlk->m_block->m_retType;
		auto &lhsRType = type->m_block->m_retType;
		bool unknownRType = rhsRType->m_ident == "%BlockRetType";

		for (int i = 0; i < newBlk->m_block->m_argTypes.size(); i++) {
			auto &rhsArg = newBlk->m_block->m_argTypes[i];
			auto &lhsArg = type->m_block->m_argTypes[i];
			if (rhsArg->isSubtypeOf(env, lhsArg)) {
#if 0
				std::cout <<"RHS arg " <<
					*rhsArg <<
					" is a subtype of LHS arg " <<
					*lhsArg<< "\n";
#endif
			} else {
				std::cout << "RHS arg " << *rhsArg
					  << " is NOT! a subtype of LHS arg "
					  << *lhsArg << "\n";
				return false;
			}
		}

		if (unknownRType) {
			/* TODO typecheck block unconditionally? */
			newBlk->m_block->m_retType = newBlk->typeCheckBlock(
			    type->m_block->m_argTypes);
		}

		if (lhsRType->isSubtypeOf(env, rhsRType)) {
#if 0
				std::cout <<"LHS RType " << *lhsRType <<
				    " is a subtype of RHS RType " <<
				    *rhsRType << "\n";
#endif
		} else {
			std::cout << "LHS RType " << *lhsRType
				  << " is NOT! a subtype of RHS RType "
				  << *rhsRType << "\n";
			return false;
		}

		return true;
	}

	case kSelf:
	case kInstanceType:
		abort();

	case kId:
		return true;

	case kTyVar: {
		if (m_tyVarDecl->second)
			return isSubtypeOf(env, m_tyVarDecl->second);
		else {
			std::cout << "Potentially falsely allowing RHS " <<
				    *this << " to be assigned to LHS " <<
				    *type << "\n";
			return true;
		}
	}

	case kClass:
		abort();


	case kInstance: {
		test:
		switch (type->m_kind) {
		case kIdent:
			abort();

		case kSelf:
		case kInstanceType:
			std::cout << "Falsely allowing RHS " <<
				    *this << " to be assigned to LHS " <<
				    *type << "\n";
				return true;

		case kId:
			return true;

		case kTyVar:
			if (type->m_tyVarDecl->second) {
				type = type->m_tyVarDecl->second;
				goto test;
			}
			else {
				std::cout << "Falsely allowing RHS " <<
				    *this << " to be assigned to LHS " <<
				    *type << "\n";
				return true;
			}

		case kClass:
			return false;

		case kInstance: {
			return instanceIsSubtypeOfInstance(env, type);
		}

		case kBlock: /* handled earlier */
		case kMax:   /* invalid */
			abort();
		}
	}

	case kUnion: /* handled earlier */
	case kMax:
		abort();
	}

	abort();
}

bool
Type::instanceIsSubtypeOfInstance(TyEnv *env, Type *type)
{
	/**
	 * An instance $a is a subclass of an instance $b if:
	 *
	 * 1. $a's class is the same as $b's; and
	 * 2. All type parameters to $a are subtypes of the type parameters to
	 *    $b, if there are any type arguments.
	 *
	 * If condition 1 is not met, then we check again with each supertype of
	 * $a. If none of the supertypes meet conditions 1 and 2, then $a is not
	 * a subtype of $b.
	 */

	if (m_cls == type->m_cls) {
		if (m_typeArgs.size() == 0)
			return true; /** could do inference here? */
		else if (m_typeArgs.size() == type->m_typeArgs.size()) {
			for (int i = 0; i < m_typeArgs.size(); i++) {
				if (!m_typeArgs[i]->isSubtypeOf(env,
					type->m_typeArgs[i]))
					return false;
			}

			return true;
		}
	} else if (m_cls->super != NULL) {
		Invocation invoc;
		Type *superType;

		invoc.receiver = type;
		invoc.trueReceiver = type;

		/* Register other type's type-arguments. */
		for (int i = 0; i < m_typeArgs.size(); i++)
			invoc.tyParamMap[&m_cls->m_clsNode->m_tyParams[i]] =
			    m_typeArgs[i];

		/* Construct the super-type */
		superType = m_cls->m_clsNode->superType->typeInInvocation(
		    invoc);

		/* Try checking the super-type of the other type. */
		return superType->isSubtypeOf(env, type);
	}

	return false;
}

Type *
Type::typeInInvocation(Invocation &invocation)
{
	switch (m_kind) {
	case kIdent:
	case kAsYetUnspecified:

	case kBlock: {
		Type *type = new Type;
		*type = *this;
		type->m_block = type->m_block->deepCopy();

		for (auto &arg : type->m_block->m_argTypes)
			arg = arg->typeInInvocation(invocation);
		type->m_block->m_retType =
		    type->m_block->m_retType->typeInInvocation(invocation);

		return type;
	}

	case kSelf:
		return invocation.receiver;

	case kInstanceType: {
		Type * type = new Type;
		invocation.trueReceiver->m_cls->m_instanceMasterType->
		    constructInto(type);
		return type;
	}

	case kId:
		return this;

	case kTyVar: {
		Type * type = invocation.tyParam(m_tyVarDecl);
		assert(type != NULL);
		return type;
	}

	case kClass:
		return this;

	case kInstance: {
		Type * type = new Type;
		*type = *this;
		for (auto & typeArg: type->m_typeArgs)
			typeArg = typeArg->typeInInvocation(invocation);
		return type;
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
		into->m_ident = m_ident;
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
		into->m_ident = m_ident;
		into->m_kind = kTyVar;
		into->m_tyVarDecl = m_tyVarDecl;
		break;
	}

	default:
		throw std::runtime_error("Unexpected kind " +
		    std::to_string(m_kind));
	}
}

Type *
Type::blockCopyIfNecessary()
{
	if (m_kind == kBlock /* && isGenericBlock() */) {
		Type *type = new Type;
		*type = *this;
		type->m_block = type->m_block->deepCopy();
		return type;
	} else
		return this;
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

	if (m_tyVarDecl && m_tyVarDecl->second != NULL) {
		std::cout << blanks(in) << "Bound: \n" << "\n";
		m_tyVarDecl->second->print(in + 2);
	}

#if 0
	if (m_wrapped != NULL) {
		std::cout << blanks(in) << "Wrapping: \n";
		m_wrapped->print(in + 2);
	}
#endif

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
	if (m_wasInferred)
		os << italic;

	switch (m_kind) {
	case kIdent:
		os << "unresolved-identifier" << m_ident;
		break;

	case kAsYetUnspecified:
		os << "not-yet-inferred";
		break;

	case kUnion: {
		bool first = true;
		os << "(";
		for (auto &type : m_members) {
			if (!first)
				os << " | ";
			else
				first = false;
			os << *type;
		}
		os << ")";
		break;
	}

	case kBlock:
		os << "[^" << *m_block->m_retType;

		for (int i = 0; i < m_block->m_argTypes.size(); i++) {
			os << ", " << *m_block->m_argTypes[i];
		}

		os << "]";

		break;

	case kSelf:
		os << "self";
		break;

	case kInstanceType:
		os << "instancetype";
		break;

	case kId:
		os << "id";
		break;

	case kTyVar:
		os << "typeParameter " << m_ident;
#if 0
		if (m_wrapped)
			os << "(" << (*m_wrapped) << ")";
#endif
		break;

	case kClass:
		os << m_ident << " class";
		break;

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
		break;
	}

	case kMax:
		abort();
	}
	if (m_wasInferred)
		os << normal;

	return os;
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
		tyc->m_classType = new Type(tyc);
		tyc->m_instanceMasterType = Type::makeInstanceMaster(tyc,
		    cls->m_tyParams);

		m_globals->m_vars[cls->name] = tyc->m_classType;
		m_globals->m_types[cls->name] = tyc->m_instanceMasterType;

		return tyc;
	}
}

TyChecker::TyChecker()
{
	m_globals = new TyEnv;
	m_envs.push_back(m_globals);
}

static void resolveOrId(Type *& type, TyEnv * env)
{
	if (!type)
		type = new Type("id", {});
	else
		type->resolveInTyEnv(env);
}

static void resolveOrUninferred(Type *& type, TyEnv * env)
{
	if (!type)
		type = new Type;
	else
		type->resolveInTyEnv(env);
}

void
MethodNode::typeReg(TyChecker &tyc)
{
	TyEnv::m_parent = tyc.m_envs.back();
	auto tyClass = m_parent->m_tyClass;
	tyc.m_envs.push_back(this);

	assert(tyClass);

	/* Register method's generic type parameters */
	for (auto & tyParam: m_tyParams) {
		TyEnv::m_types[tyParam.first] = Type::makeTyVarReference(
		    &tyParam);
	}

	resolveOrId(m_retType, this);

	for (auto &var : args) {
		resolveOrId(var.second, this);
		m_vars[var.first] = var.second;
	}

	for (auto &var : locals) {
		resolveOrUninferred(var.second, this);
		m_vars[var.first] = var.second;
	}

	m_vars["self"] = m_parent->m_tyClass->m_instanceMasterType;
	m_vars["instancetype"] = m_parent->m_tyClass->m_instanceMasterType;

	tyc.m_envs.pop_back();
}

void
ClassNode::typeReg(TyChecker &tyc)
{
	TyEnv::m_parent = tyc.m_envs.back();
	TyEnv::m_tyClass = tyClass;
	tyc.m_envs.push_back(this);

	for (auto & tyParam : m_tyParams) {
		TyEnv::m_types[tyParam.first] = Type::makeTyVarReference(
		    &tyParam);
	}

	if (superName != "nil") {
		superType->resolveInTyEnv(this);
		tyClass->super = superType->m_cls;
	}
	else {
		delete superType;
		superType = NULL;
		tyClass->super = NULL;
	}

	for (auto &var : iVars) {
		resolveOrId(var.second, this);
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
#define GLOBAL(NAME, CLASS)                                \
	tyc.m_globals->m_vars[NAME] = new Type(CLASS, {}); \
	tyc.m_globals->m_vars[NAME]->resolveInTyEnv(tyc.m_globals)
#define TYPE(VAR, NAME)               \
	tyc.VAR = new Type(NAME, {}); \
	tyc.VAR->resolveInTyEnv(tyc.m_globals)
	GLOBAL("true", "True");
	GLOBAL("false", "False");
	TYPE(m_smiType, "Integer");
	TYPE(m_symType, "Symbol");
	TYPE(m_stringType, "String");
}

/**
 * Type inferencing and type checking
 */

Type *
SymbolExprNode::type(TyChecker &tyc)
{
	return tyc.symType();
}

Type *
IntExprNode::type(TyChecker &tyc)
{
	return tyc.smiType();
}

Type *
StringExprNode::type(TyChecker &tyc)
{
	return tyc.stringType();
}


Type *
IdentExprNode::type(TyChecker &tyc)
{
	return tyc.m_envs.back()->lookupVar(id);
}

Type *
AssignExprNode::type(TyChecker &tyc)
{
	Type * lhs = left->type(tyc);
	Type * rhs = right->type(tyc);

	if (!rhs->isSubtypeOf(tyc.m_envs.back(), lhs)) {
		std::cerr << "RHS of type " << *rhs <<
		    " is not a subtype of LHS " << *lhs << "\n";
	}

	return lhs;
}

Type *
MessageExprNode::type(TyChecker &tyc)
{
	Type *recvType = receiver->type(tyc);
	std::vector<Type *> argTypes;
	Type *res;

	std::cout<< "Typechecking a send of #" << selector << " to " <<
	    *recvType << "\n";

	for (auto &arg : args)
		argTypes.push_back(arg->type(tyc));

#if 0
	printf("RECEIVER TYPE:\n");
	recvType->print(10);
	printf("ARG TYPES\n");
	for (auto &argType : argTypes)
		argType->print(8);
#endif

	if (argTypes.size()) {
		std::cout << "Argument types: ";
		for (auto &type : argTypes)
			std::cout << *type;
		std::cout << "\n";
	}

	res = recvType->typeSend(tyc.env(), selector, argTypes);

	std::cout << " -> RESULT OF SEND: " << *res << "\n\n";

	return res;
}

Type *
Type::typeSend(TyEnv * env, std::string selector, std::vector<Type *> &argTypes,
    Type * trueReceiver, Invocation * prev_invoc)
{
	if (!trueReceiver)
		trueReceiver = this;

	switch (m_kind) {
	case kUnion: {
		Type *result = new Type;

		result->m_kind = kUnion;
		for (auto &member : m_members)
			result->m_members.push_back(member->typeSend(env,
			    selector, argTypes, trueReceiver, NULL));

		return result;
	}

	case kInstance: {
		MethodNode *meth = NULL;
		Invocation invoc;

		if (prev_invoc)
			invoc.m_super = prev_invoc;
		invoc.receiver = this;
		invoc.trueReceiver = trueReceiver;

		/* Register this (the receiver) type's type-arguments. */
		for (int i = 0; i < m_typeArgs.size(); i++)
			invoc.tyParamMap[&m_cls->m_clsNode->m_tyParams[i]] =
			    m_typeArgs[i];

		/* Search for the instance method in the class node. */
		for (auto &aMeth : m_cls->m_clsNode->iMethods)
			if (aMeth->sel == selector)
				meth = aMeth;

		/* Method not found? */
		if (!meth) {
			if (m_cls->super) {
				/* Construct the super-type */
				Type * superRecv = m_cls->m_clsNode->superType->
				    typeInInvocation(invoc);
				/* Try checking the super-type as receiver */
				return superRecv->typeSend(env, selector,
				    argTypes, trueReceiver, &invoc);
			}

			std::cerr << "Object of type " << *trueReceiver
				  << " does not appear to understand message "
				  << selector << "\n";
			return Type::id();
		}

		/* Register any type parameters of the generic argument. */
		for (auto & tyParam: meth->m_tyParams) {
			invoc.tyParamMap[&tyParam] = new Type;
		}

		for (int i = 0; i < argTypes.size(); i++) {
			Type *formal = meth->args[i].second->typeInInvocation(
			    invoc);
			Type *actual = argTypes[i]->blockCopyIfNecessary();
			std::cout << "FORMAL: " << *formal << "\n";
			std::cout << "ACTUAL: " << *actual << "\n";
			if (!actual->isSubtypeOf(env, formal))
				std::cerr << "Argument of type " << *actual
					  << " is not a subtype of " << *formal
					  << "\n";
		}

		return meth->m_retType->typeInInvocation(invoc);
	}

	case kClass: {
		MethodNode *meth = NULL;
		Invocation invoc;

		if (prev_invoc)
			invoc.m_super = prev_invoc;
		invoc.receiver = this;
		invoc.trueReceiver = trueReceiver;
		invoc.isClass = true;

		for (auto &aMeth : m_cls->m_clsNode->cMethods)
			if (aMeth->sel == selector)
				meth = aMeth;

		if (!meth) {
			if (m_cls->super) {
				return m_cls->super->m_classType->typeSend(
					env, selector, argTypes, trueReceiver,
					&invoc);
			}

			std::cerr << "Object of type " << *this
				  << " does not appear to understand message "
				  << selector << "\n";
			return NULL;
		}

		for (int i = 0; i < argTypes.size(); i++) {
			Type *formal = meth->args[i].second->typeInInvocation(
			    invoc);
			if (!argTypes[i]->isSubtypeOf(env, formal))
				std::cerr << "Argument of type " << *this
					  << " is not a subtype of " << *formal
					  << "\n";
		}

		return meth->m_retType->typeInInvocation(invoc);
	}

	default:
		std::cerr <<"Warning: can't typecheck message #" << selector <<
		    " sent to receiver of unknown type\n";
		return Type::id();
	}
}

Type * Type::typeCheckBlock(std::vector<Type *> & argTypes)
{
	auto tyc = m_block->m_node->m_tyc;
	TyEnv * env = new TyEnv;
	std::vector<VarDecl> argDecls;
	Type * rType = NULL;

	env->m_parent = tyc.m_envs.back();
	tyc.m_envs.push_back(env);

	for (int i = 0; i < m_block->m_argTypes.size(); i++) {
#if 0
		std::cout <<"Declaring " << m_block->m_node->args[i].first <<
		    " to be " << *argTypes[i] << "\n";
#endif
		env->m_vars[m_block->m_node->args[i].first] = argTypes[i];
	}

	for (auto &stmt : m_block->m_node->stmts) {
		ExprStmtNode * last = NULL;

		if (stmt == m_block->m_node->stmts.back())
			last = dynamic_cast<ExprStmtNode*>(stmt);

		if (!last)
			stmt->typeCheck(tyc);
		else
			rType = last->expr->type(tyc);
	}

	tyc.m_envs.pop_back();

	if (!rType)
		rType = Type::id();
	std::cout << "Inferred block return type to be " << *rType << "\n";

	return rType;
}

Type *
BlockExprNode::type(TyChecker &tyc)
{
	TyBlock * blk = new TyBlock;
	Type * type = new Type;

	m_tyc = tyc;

	blk->m_argTypes.resize(args.size(), NULL);
	blk->m_node = this;

	for (int i = 0; i < args.size(); i++) {
		if (!args[i].second)
			blk->m_tyParams.push_back({"%BlockParam1", NULL});
		args[i].second = blk->m_argTypes[i] = Type::makeTyVarReference(
			&blk->m_tyParams.back());
	}
	if (!m_retType) {
		blk->m_tyParams.push_back({"%BlockRetType", NULL});
		blk->m_retType = m_retType = Type::makeTyVarReference(
			&blk->m_tyParams.back());
	}

	type->m_kind = Type::kBlock;
	type->m_block = blk;

	return type;
}

void
ExprStmtNode::typeCheck(TyChecker &tyc)
{
	expr->type(tyc);
}


void
ReturnStmtNode::typeCheck(TyChecker &tyc)
{
	Type * type = expr->type(tyc);
	Type * retType = tyc.method()->m_retType;
	if (!type->isSubtypeOf(tyc.env(), retType)) {
		std::cerr << "Type " << *type << " is not a subtype of expected"
		    " return type " << *retType << "\n";
	}
}

void
MethodNode::typeCheck(TyChecker &tyc)
{
	tyc.m_envs.push_back(this);
	tyc.m_method = this;

	for (auto &stmt : stmts)
		stmt->typeCheck(tyc);

	tyc.m_method = NULL;
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
