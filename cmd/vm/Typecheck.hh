#ifndef TYPECHECK_HH_
#define TYPECHECK_HH_

#include <string>
#include <map>
#include <vector>

struct ClassNode;
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

		/* these are resolved Idents */
		kTyVar,
		kClass,
		kInstance,
		kMax
	} m_kind;

	static const char *kindStr[kMax];

	bool m_isConstructor = false;

	Type *m_wrapped = NULL; /* if kTyVar */
	TyClass *m_cls = NULL;	/* if kClass or kInstance */

	std::string m_ident;
	std::vector<Type*> m_typeArgs;

	static Type *makeInstanceMaster(TyClass *cls);

	Type() : m_kind(kAsYetUnspecified) {};
	Type(std::string ident, std::vector<Type*> typeArgs);
	Type(TyClass *cls); /**< create kClass type */
	Type(TyClass * cls, std::string ident, std::vector<VarDecl> m_typeArgs); /**< create master kInstance type */
	Type(std::string tyVarIdent, Type * bound);

	/**
	 * Try to get the return type of a message send to this type.
	 */
	Type *typeSend(std::string selector, std::vector<Type *> &argTypes);

	void resolveInTyEnv(TyEnv * env);
	void constructInto(Type *into);
	void print(size_t in);

	friend std::ostream &operator<<(std::ostream &, Type const &);
	std::ostream &niceName(std::ostream &os) const;
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
	ClassNode *m_clsNode;
};

struct TyEnv {
	TyEnv * m_parent = NULL;
	TyClass *m_tyClass = NULL;
	std::map<std::string, Type *> m_vars; /**< variable names */
	std::map<std::string, Type *> m_types; /**< type names */

	Type * lookupVar(std::string & txt);
	Type * lookupType(std::string & txt);

};

struct TyChecker {
	TyEnv *m_globals;
	std::vector<TyEnv*> m_envs;

	TyChecker();

	TyClass *findOrCreateClass(ClassNode *cls);
};

#endif /* TYPECHECK_HH_ */
