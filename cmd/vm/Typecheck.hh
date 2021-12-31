#ifndef TYPECHECK_HH_
#define TYPECHECK_HH_

#include <string>
#include <map>
#include <vector>

struct TyClass;
struct TyEnv;
struct Type;
typedef std::pair<std::string, Type *> VarDecl;

struct Type {
	enum Kind {
		kIdent,

		kAsYetUnspecified,

		kBlock,
		kSelf,
		kInstanceType,
		kId,

		kTyVar,

		/* these are resolved Idents */
		kClass,
		kInstance,
	} m_kind;

	bool m_isConstructor  = false;

	Type * m_wrapped = NULL;
	std::string m_ident;
	TyClass * m_cls = NULL;
	std::vector<Type*> m_typeArgs;

	Type() : m_kind(kAsYetUnspecified) {};
	Type(std::string ident, std::vector<Type*> typeArgs);
	Type(TyClass *cls); /**< create kClass type */
	Type(TyClass * cls, std::string ident, std::vector<VarDecl> m_typeArgs); /**< create master kInstance type */
	Type(std::string tyVarIdent, Type * bound);

	void resolveInTyEnv(TyEnv * env);
	void construct(Type * into);
	void print(size_t in);
};

struct TypeEnv {
	/* maps type names to types */
	std::map<std::string, Type *> m_types;
};

struct Method {
	Type *m_rType;
	std::vector<Type *> m_argTypes;
};

struct TyClass {
#if 0
	union {
		TyClass *m_meta; /**< if non-null, metaclass of this class */
		TyClass *m_nst;  /**< if non-null, instance class of this class */
	};
#endif
	std::string m_name;
	TyClass * super;

	std::map<std::string, Type *> m_cVars;
	std::map<std::string, Type *> m_iVars;
	std::vector<Method *> m_iMethods;
	std::vector<VarDecl> m_typeParams;
};

struct TyEnv {
	TyEnv * m_parent = NULL;
	std::map<std::string, Type *> m_vars; /**< variable names */
	std::map<std::string, Type *> m_types; /**< type names */

	Type * lookupVar(std::string & txt);
	Type * lookupType(std::string & txt);

};

struct TyChecker {
	TyEnv *m_globals;
	std::vector<TyEnv*> m_envs;

	TyChecker();

	TyClass * findOrCreateClass(std::string name, std::vector<VarDecl> typeParams);
};

#endif /* TYPECHECK_HH_ */
