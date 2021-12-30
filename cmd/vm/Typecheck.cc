#include <stdexcept>

#include "Typecheck.hh"

Type::Type(std::string ident, std::vector<Type *> typeArgs)
    : m_ident(ident)
    , m_typeArgs(typeArgs)
{
}

Type::Type(TyClass *cls)
{
	m_ident = cls->m_name;
	m_kind = kClass;
}

Type *
TyEnv::lookup(std::string &txt)
{
	if (m_vars.find(txt) != m_vars.end())
		return m_vars[txt];
	else if (m_parent)
		return m_parent->lookup(txt);
	else
		return NULL;
}

TyClass *
TyChecker::findOrCreateClass(std::string name)
{
	Type *type = m_globals->lookup(name);
	if (type && type->m_kind == Type::kClass)
		return type->m_cls;
	else {
		TyClass *tyc = new TyClass;
		m_globals->m_vars[name] = new Type(tyc);
		return tyc;
	}
}

TyChecker::TyChecker()
{
	m_globals = new TyEnv;
}
