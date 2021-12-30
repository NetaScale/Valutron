#ifndef TYPECHECK_HH_
#define TYPECHECK_HH_

#include <string>
#include <map>
#include <vector>

struct TyClass;

struct Type {
	enum Kind {
		kIdent,

		kBlock,
		kSelf,
		kInstanceType,
		kId,

		/* these are resolved Idents */
		kClass,
		kInstance,
	} m_kind;

	TyClass * m_cls;

	std::vector<Type> m_typeArgs;

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
	TyClass * super;

	std::map<std::string, Type *> m_cVars;
	std::map<std::string, Type *> m_iVars;
	std::vector<Method *> m_iMethods;
	std::vector<std::string> m_typeParams;
};

struct TyEnv {
	TyEnv * m_parent = NULL;
	std::map<std::string, Type *> m_vars; /**< variable names */
	std::map<std::string, Type *> m_types; /**< type names */

	Type * lookup(std::string & txt);
};

struct TyChecker {
	TyEnv *m_globals;

	TyChecker();

	TyClass * findOrCreateClass(std::string name);
};

#endif /* TYPECHECK_HH_ */
