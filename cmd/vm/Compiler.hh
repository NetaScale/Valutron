#ifndef COMPILER_HH_
#define COMPILER_HH_

#include <list>
#include <stack>
#include <string>
#include <vector>

#include "lemon/lemon_base.h"

#include "AST.hh"
#include "Oops.hh"

struct MethodNode;

struct Token {
	Position m_pos;

	Token() = default;
	Token(const Token &) = default;
	Token(Token &&) = default;

	Token(Position pos)
	    : m_pos(pos) {};
	Token(Position pos, double f)
	    : m_pos(pos)
	    , floatValue(f) {};
	Token(Position pos, int i)
	    : m_pos(pos)
	    , intValue(i) {};
	Token(Position pos, const std::string &s)
	    : m_pos(pos)
	    , stringValue(s) {};
	Token(Position pos, std::string &&s)
	    : m_pos(pos)
	    , stringValue(std::move(s)) {};

	Token &operator=(const Token &) = default;
	Token &operator=(Token &&) = default;

	operator std::string() const { return stringValue; }
	operator const char *() const { return stringValue.c_str(); }
	operator double() const { return floatValue; }
	const Position & pos() const { return m_pos; }

	double floatValue = 0.0;
	int intValue = 0;
	std::string stringValue;
};

class ClassNode;
struct NameScope;
class VarNode;
class GlobalVarNode;
class ProgramNode;

class MVST_Parser : public lemon_base<Token> {
    protected:
	std::string fName;
	std::string path;
	std::string &fText;
	ProgramNode *program;
	MethodNode *meth;
	int m_line = 0, m_col = 0, m_pos = 0;
	int m_oldLine = 0, m_oldCol = 0, m_oldPos = 0;

    public:
	using lemon_base::parse;

	int line() const { return m_line; }
	int col() const { return m_col; }
	int pos1() const { return m_pos; }

	static ProgramNode *parseFile(std::string fName);
	static MethodNode *parseText(std::string text);
	static MVST_Parser *create(std::string fName, std::string &fText);

	MVST_Parser(std::string f, std::string &ft)
	    : fName(f)
	    , fText(ft)
	    , program(nullptr) {};

	/* parsing */
	void parse(int major) { parse(major, Token { pos() }); }

	template <class T> void parse(int major, T &&t)
	{
		parse(major, Token(pos(), std::forward<T>(t)));
	}

	virtual void trace(FILE *, const char *) { }

	/* line tracking */
	Position pos();

	void recOldPos()
	{
		m_oldPos = m_pos;
		m_oldLine = m_line;
		m_oldCol = m_col;
	}

	void cr()
	{
		m_pos += m_col + 1;
		m_line++;
		m_col = 0;
	}
	void incCol() { m_col++; }
};

#endif /* COMPILER_HH_ */
