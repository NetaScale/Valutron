#ifndef TYPECHECK_HH_
#define TYPECHECK_HH_

#include <map>
#include <string>
#include <vector>

struct ClassNode;
struct MethodNode;
struct BlockExprNode;
struct TyClass;
struct TyBlock;
struct TyEnv;
struct Type;
typedef std::pair<std::string, Type *> VarDecl;
struct Invocation;
struct FlowInference;

struct Type {
	enum Kind {
		kIdent,

		kAsYetUnspecified,

		kUnion,

		kBlock,
		kSelf,
		kInstanceType,
		kId,

		/* these are resolved Idents */
		kTyVar,
		kClass,
		kInstance,
		kSuper,
		kMax
	} m_kind;

	static const char *kindStr[kMax];

	bool m_isConstructor = false;
	bool m_wasInferred = false; /**< was this type inferred? */

	std::vector<Type *> m_members; /**< if kUnion, its types */
	VarDecl *m_tyVarDecl =
	    NULL;		 /**< if a kTyVar, the VarDecl defining it */
	TyClass *m_cls = NULL;	 /* if kClass or kInstance */
	TyBlock *m_block = NULL; /* if kBlock */

	/**
	 * A generic block has to be copied before binding to any variable.
	 */
	Type *blockCopyIfNecessary();

	/**
	 * Type checks a block invocation with a particular set of argument
	 * types, returning whatever type is inferred to be the block's return
	 * type.
	 */
	Type *typeCheckBlock(std::vector<Type *> &argTypes);

	std::string m_ident;
	std::vector<Type *> m_typeArgs;

	static Type *id();
	static Type *super();
	static Type *makeTyVarReference(VarDecl *tyVarDecl);
	static Type *makeInstanceMaster(TyClass *cls,
	    std::vector<VarDecl> &tyParams);

	Type()
	    : m_kind(kAsYetUnspecified) {};
	Type(std::string ident, std::vector<Type *> typeArgs);
	Type(TyClass *cls); /**< create kClass type */

	Type *copy() const;

	/**
	 * Try to get the return type of a message send to this type.
	 */
	Type *typeSend(TyEnv *env, std::string selector,
	    std::vector<Type *> &argTypes, Type *trueReceiver = NULL,
	    Invocation *prev_invoc = NULL, bool skipFirst = false);

	bool isSubtypeOf(TyEnv *env, Type *type);
	bool instanceIsSubtypeOfInstance(TyEnv *env, Type *type);
	Type * narrow(TyEnv * env, Type * type);

	Type *typeInInvocation(Invocation &invocation);
	void registerIVars(Invocation &invoc,
	    std::map<std::string, Type *> &vars);
	void resolveInTyEnv(TyEnv *env);
	void constructInto(Type *into);
	void print(size_t in);

	friend std::ostream &operator<<(std::ostream &, Type const &);
	std::ostream &niceName(std::ostream &os) const;
};

struct TyEnv {
	TyEnv *m_parent = NULL;
	TyClass *m_tyClass = NULL;
	std::map<std::string, Type *> m_vars;  /**< variable names */
	std::map<std::string, Type *> m_types; /**< type names */

	Type *lookupVar(std::string &txt);
	Type *lookupType(std::string &txt);
};

struct TyBlock : public TyEnv {
	BlockExprNode *m_node;
	std::vector<VarDecl> m_tyParams;
	Type *m_retType;
	std::vector<Type *> m_argTypes;

	TyBlock *deepCopy();
};

struct TyClass {
	std::string m_name;
	TyClass *super;
	ClassNode *m_clsNode;

	Type *m_classType;
	Type *m_instanceMasterType;
};

struct TyChecker {
	Type *m_charType, *m_floatType, *m_smiType, *m_symType, *m_stringType;
	TyEnv *m_globals;
	std::vector<TyEnv *> m_envs;
	MethodNode *m_method = NULL;
	std::vector<BlockExprNode *> m_blocks;

	TyChecker();
	TyChecker(Type *smiType, TyEnv *globals, std::vector<TyEnv *> envs,
	    MethodNode *method, std::vector<BlockExprNode *> m_blocks);

	Type *charType() { return m_charType; }
	Type *floatType() { return m_floatType; }
	Type *smiType() { return m_smiType; }
	Type *symType() { return m_symType; }
	Type *stringType() { return m_stringType; }
	TyEnv *env() { return m_envs.back(); }

	TyClass *findOrCreateClass(ClassNode *cls);
	MethodNode *method() { return m_method; }
};

struct FlowInference {
	bool isNegated;
	bool isExact; /* whether is isMemberOf: rather than isKindOf: */
	std::string ident;
	Type *inferredType;
};

#endif /* TYPECHECK_HH_ */
