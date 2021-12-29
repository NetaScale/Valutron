/*
 * Copyright 2020-2021 David MacKay.  All rights reserved.
 * Use is subject to license terms.
 */

%include {
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <list>
#include <filesystem>

#include "AST.hh"
#include "Compiler.hh"
#include "Parser.tab.h"
#include "Scanner.l.hh"

#define LEMON_SUPER MVST_Parser

static std::string removeFirstChar (std::string aString)
{
	return aString.substr(1, aString.size() - 1);
}

static std::string removeFirstAndLastChar(std::string aString)
{
	return aString.substr(1, aString.size() - 2);
}
}

%token_type    {Token}
%token_prefix  TOK_

%code {
ProgramNode * MVST_Parser::parseFile (std::string fName)
{
	MVST_Parser *parser;
	std::string src;
	std::ifstream f;
		std::filesystem::path fPath(fName);
	yyscan_t scanner;
	YY_BUFFER_STATE yyb;

	f.exceptions(std::ios::failbit | std::ios::badbit);

	try
	{
		f.open(fName);

		f.seekg(0, std::ios::end);
		src.reserve(f.tellg());
		f.seekg(0, std::ios::beg);

		src.assign((std::istreambuf_iterator<char>(f)),
			std::istreambuf_iterator<char>());
	}

	catch (std::ios_base::failure &e)
	{
		std::cerr << "PDST: Error: File " + fName + " failed:\n\t" +
				e.code().message() + "\n";
	}

	parser = MVST_Parser::create(fName, src);
	if (0)
		parser->trace(stdout, "<parser>: ");

	parser->path =  fPath.parent_path();

	mvstlex_init_extra(parser, &scanner);
	/* Now we need to scan our string into the buffer. */
	yyb = mvst_scan_string(src.c_str(), scanner);

	while (mvstlex(scanner))
		;
	parser->parse(TOK_EOF);
	parser->parse(0);

	return parser->program;
}

MethodNode * MVST_Parser::parseText (std::string src)
{
	MVST_Parser *parser;
	yyscan_t scanner;
	YY_BUFFER_STATE yyb;

	parser = MVST_Parser::create("<no-file>", src);
	if (1)//0)
		//parser->trace(stdout, "<parser>: ");

	mvstlex_init_extra(parser, &scanner);

	printf("Compiling immediate method: [ %s ]\n", src.c_str());
	/* Now we need to scan our string into the buffer. */
	yyb = mvst_scan_string(src.c_str(), scanner);

	parser->parse(TOK_DIRECTMETH);
	while (mvstlex(scanner))
		;
	parser->parse(TOK_EOF);
	parser->parse(0);

	return parser->meth;
}

MVST_Parser * MVST_Parser::create(std::string fName, std::string & fText)
{
	return new yypParser(fName, fText);
}

Position MVST_Parser::pos()
{
	yypParser *self = (yypParser *)this;
	return Position(self->m_oldLine, self->m_oldCol, self->m_oldPos, self->m_line, self->m_col, self->m_pos);
}

}

%syntax_error {
	const YYACTIONTYPE stateno = yytos->stateno;
	size_t eolPos = fText.find("\n", m_pos);
	std::string eLine = fText.substr(m_pos, eolPos - m_pos);
	size_t i;

	std::cerr << "PDST: " << fName << "(" << std::to_string(m_line) + ":" <<
	    std::to_string(m_col) << "): " <<
	    "Error V1001: Syntax error: unexpected " <<
	    yyTokenName[yymajor] << "\n";

	std::cerr << "+ " << eLine << "\n";
	std::cerr << "+ ";

	for (i = 0; i < m_oldCol; i++)
		std::cerr << " ";
	for (; i < m_col; i++)
		std::cerr << "^";

	std::cerr << "\n\texpected one of: \n";

	for (unsigned i = 0; i < YYNTOKEN; ++i) {
		int yyact = yy_find_shift_action(i, stateno);
		if (yyact != YY_ERROR_ACTION && yyact != YY_NO_ACTION)
			std::cerr << "\t" << yyTokenName[i] << "\n";
	}
}

prog ::= file.

file ::= directMeth EOF.


file ::= decl_list(l) EOF.
	{
		program = new ProgramNode(l);
	}

directMeth ::= DIRECTMETH var_defs_opt(locals)
		statements(stmts)
		dot_opt.
	{
		meth = new MethodNode(false, "fakeselector", {}, locals, stmts);
	}

%type decl_list { std::vector<DeclNode *> }
%type decl { DeclNode * }
%type class_def { DeclNode * }

decl_list(L) ::= decl(d). { L = {d}; }
decl_list(L) ::= decl_list(l) decl(d).
	{
		L = l;
		L.push_back(d);
	}

decl(D) ::= ATinclude STRING(name).
	{
		D = parseFile(path + "/" + removeFirstAndLastChar(name));
	}
decl(D) ::= NAMESPACE CURRENTCOLON identifier(name) SQB_OPEN decl_list(l) SQB_CLOSE.
	{
		printf("new namespace node %s\n", name.stringValue.c_str());
		D = new NamespaceNode(name, l);
	}
decl ::= class_def.


class_def(D) ::= identifier(super) SUBCLASSCOLON identifier(name)
	SQB_OPEN
		ivar_cvar_defs_opt(iVars)
		method_defs_opt(iMeths)
	SQB_CLOSE.
	{
		ClassNode * cls = new ClassNode(name, super, {}, iVars);
		cls->addMethods(iMeths);
		D = cls;
	}

%type ivar_cvar_defs_opt { std::vector<std::string>}
%type var_defs_opt { std::vector<std::string> }
%type var_def_list_opt { std::vector<std::string> }
%type var_def_list { std::vector<std::string> }

ivar_cvar_defs_opt ::= .
ivar_cvar_defs_opt(L) ::= BAR var_def_list_opt(l) BAR. { L = l; }
ivar_cvar_defs_opt(L) ::= BAR var_def_list_opt(l) BAR BAR var_def_list BAR. { L = l; }

var_defs_opt ::= .
var_defs_opt(L) ::= BAR var_def_list(l) BAR. { L = l; }

var_def_list_opt ::= .
var_def_list_opt(L) ::= var_def_list(l). { L = l; }

var_def_list(L) ::= IDENTIFIER(i). { L = {i}; }
var_def_list(L) ::= var_def_list(l) IDENTIFIER(i).
	{
		L = l;
		L.push_back(i);	}

%type method_defs_opt { std::vector<MethodNode *> }
%type method_defs { std::vector<MethodNode *> }
%type method_def { MethodNode * }

method_defs_opt(L) ::= method_defs(l). { L = l; }
method_defs_opt ::= .

method_defs(L) ::= method_def(d). { L = {d}; }
method_defs(L) ::= method_defs(l) method_def(d).
	{
		L = l;
		L.push_back(d);	}

method_def(D) ::= opt_class_meth_spec(isClass) selector_pattern(s)
	SQB_OPEN		var_defs_opt(locals)
		statements(stmts)
		dot_opt
	SQB_CLOSE.
	{
		//for (auto s: stmts)
		//s->print(2);
		D = new MethodNode(isClass, s.first, s.second, locals, stmts);
	}

%type opt_class_meth_spec { bool }

opt_class_meth_spec(C) ::= . { C = false; }
opt_class_meth_spec(C) ::= CLASSRR. { C = true; }

%type statements { std::vector<StmtNode *> }
%type statement { StmtNode * }

statements(L) ::= statement(s). { L = {s}; }
statements(L) ::= statements(l) DOT statement(s).
	{
		L = l;
		L.push_back(s);	}

statement(S) ::= UP assign_expr(e). { S = new ReturnStmtNode(e);  }
statement(S) ::= assign_expr(e). { S = new ExprStmtNode(e); }

%type assign_expr { ExprNode * }
%type continuation_expr { ExprNode * }
%type keyw_continuation_expr { CascadeExprNode * }
%type bin_continuation_expr { CascadeExprNode * }
%type unary_continuation_expr { CascadeExprNode * }
%type msg_expr { ExprNode * }

assign_expr(E) ::= identifier(i) ASSIGN continuation_expr(e).
	{
		E = new AssignExprNode(new IdentExprNode (i), e);
	}
assign_expr(E) ::= continuation_expr(e). { E = e;}

continuation_expr(C) ::= msg_expr(e). { C = e; }
continuation_expr(C) ::= keyw_continuation_expr(e). { C = e; }

keyw_continuation_expr(C) ::= bin_continuation_expr(c).
	{
		C = c;
	}
keyw_continuation_expr(C) ::= bin_continuation_expr(c) keyw_msg_body(l).
	{
		C = c;
		C->messages.push_back(
			new MessageExprNode(C->receiver, l));
	}

bin_continuation_expr(C) ::= unary_continuation_expr(c). { C = c; }
bin_continuation_expr(C) ::= bin_continuation_expr(c) binOp(s) unary_expr(e).
	{
		C = c;
		C->messages.push_back(new MessageExprNode(C->receiver, s, {e}));
	}


unary_continuation_expr(C) ::= continuation_expr(r) SEMICOLON.
	{
		/* FIXME: To avoid false positives, put bracketed expressions in their
		 * own thing. */
		CascadeExprNode * c = dynamic_cast<CascadeExprNode *> (r);
		C = c ? c : new CascadeExprNode (r);
	}
unary_continuation_expr(C) ::= unary_continuation_expr(c) identifier(i).
	{
		C = c;
		C->messages.push_back(new MessageExprNode(C->receiver, i));
	}

msg_expr(E) ::= binary_expr(b). { E = b; }
msg_expr(E) ::= binary_expr(e) keyw_msg_body(k).
	{
		E = new MessageExprNode(e, k);
	}

%type keyw_msg_body { std::vector<std::pair<std::string, ExprNode *>> }

keyw_msg_body(L) ::= keyword(k) binary_expr(e).
	{
		L = { {k, e}};
	}
keyw_msg_body(L) ::= keyw_msg_body(l) keyword(k) binary_expr(e).
	{
		L = l;
		L.push_back( { k, e });
	}

%type binary_expr { ExprNode * }
%type unary_expr { ExprNode * }
%type primary_expr { ExprNode * }

binary_expr(E) ::= unary_expr(e). { E = e; }
binary_expr(E) ::= binary_expr(r) binOp(s) unary_expr(a).
	{
		E  = new MessageExprNode(r, s, { a });	}

unary_expr(E) ::= primary_expr(e). { E = e; }
unary_expr(U) ::= unary_expr(p) identifier(i). { U  = new MessageExprNode(p, i); }

primary_expr(S) ::= identifier(i). { S = new IdentExprNode (i); }
primary_expr(S) ::= LBRACKET assign_expr(s) RBRACKET. { S = s; }
primary_expr ::= block_expr.
primary_expr ::= literal_expr.
primary_expr(S) ::= PRIMNUM(n) primary_list_opt(l) RCARET.
	{
		S = new PrimitiveExprNode(n.intValue, l);
	}

%type primary_list_opt { std::vector<ExprNode *> }
%type primary_list { std::vector<ExprNode *> }

primary_list_opt ::= .
primary_list_opt(L) ::= primary_list(l). { L = l; }

primary_list(L) ::= primary_expr(p). { L = {p}; }
primary_list(L) ::= primary_list(l) primary_expr(p).
	{
		L = l;
		L.push_back(p);
	}

%type literal_expr { ExprNode * }
%type array_literal_member { ExprNode * }
%type basic_literal_expr { ExprNode * }

literal_expr(L) ::= HASH LBRACKET array_literal_members_opt(a) RBRACKET.  { L = new ArrayExprNode(a); }
literal_expr ::= basic_literal_expr.

%type array_literal_members { std::vector<ExprNode *> }
%type array_literal_members_opt { std::vector<ExprNode *> }

array_literal_members_opt ::= .
array_literal_members_opt(A) ::= array_literal_members(l). {A = l;}

array_literal_members(L) ::= array_literal_member(l). { L = {l}; }
array_literal_members(L) ::= array_literal_members(l) array_literal_member(lit).
	{
		L = l;
		L.push_back(lit);
	}

array_literal_member(A) ::= basic_literal_expr(i). { A = i; }
array_literal_member(A) ::= identifier(i). { A = new SymbolExprNode(i); }
array_literal_member(A) ::= keyword(k). { A = new SymbolExprNode(k); }
array_literal_member(A) ::= LBRACKET array_literal_members_opt(a) RBRACKET. { A = new ArrayExprNode(a); }
array_literal_member(A) ::= HASH LBRACKET array_literal_members_opt(a) RBRACKET. { A = new ArrayExprNode(a); }

basic_literal_expr(S) ::= STRING(s).
	{
		S = new StringExprNode(removeFirstAndLastChar(s));
	}
basic_literal_expr(S) ::= SYMBOL(s).
	{
		S = new SymbolExprNode(removeFirstChar(s));
	}
basic_literal_expr(S) ::= INTEGER(i).
	{
		S = new IntExprNode(i.intValue);
	}
basic_literal_expr(S) ::= FLOAT(f).
	{
		S = new FloatExprNode(f);
	}
basic_literal_expr(S) ::= CHAR(c).
	{
		S = new CharExprNode(removeFirstChar(c));
	}

%type block_expr { BlockExprNode * }

block_expr(B) ::= SQB_OPEN block_formal_list_opt(v) statements_opt(s) SQB_CLOSE.
	{
		B = new BlockExprNode(v, s);
	}

dot_opt ::= .
dot_opt ::= DOT.

%type statements_opt { std::vector<StmtNode *> }

statements_opt ::= .
statements_opt(L) ::= statements(l). { L = l; }

%type block_formal_list_opt { std::vector<std::string> }
%type colon_var_list { std::vector<std::string> }

block_formal_list_opt(L) ::= colon_var_list(l) BAR. { L = l; }
block_formal_list_opt ::= .

colon_var_list(L) ::= COLONVAR(v).	{
		std::string s = v;
		s.erase(s.begin());
		L = {s};
	}colon_var_list(L) ::= colon_var_list(l) COLONVAR(v).
	{
		L = l;
		std::string s = v;
		s.erase(s.begin());
		L.push_back(s);
	}

%type selector_pattern { std::pair<std::string, std::vector<std::string>> }

selector_pattern(S) ::= identifier(i). { S = {i, {}};  }
selector_pattern(S)
	::= binary_pattern(b).
	{		S = {b.first, {b.second}};
	}
selector_pattern(S)
	::= keyw_pattern(k).
	{
		S = k;
	}

%type keyw_pattern { std::pair<std::string, std::vector<std::string>> }

keyw_pattern(L) ::= keyw_pattern_part(k). {
    L = {k.first, {k.second}};
}
keyw_pattern(L) ::= keyw_pattern(l) keyw_pattern_part(k). {    L = l;
	L.first += k.first;
	L.second.push_back(k.second);
}

%type keyw_pattern_part { std::pair<std::string, std::string> }
%type binary_pattern { std::pair<std::string, std::string> }

keyw_pattern_part(K) ::= keyword(k) identifier(s). {
    K = {k, s};
}

binary_pattern(B) ::= binOp(b) identifier(s). {
    B = {b, s};
}

identifier(I) ::= IDENTIFIER(i). { I = i; }
identifier(I) ::= NAMESPACENAME(i). { I = i; }
identifier(I) ::= NAMESPACE. { I = Token("Namespace"); }

%type keyword { std::string }

keyword(K) ::= KEYWORD(k). { K = k.stringValue; }
keyword(K) ::= SUBCLASSCOLON. { K = "subclass:"; }
keyword(K) ::= CURRENTCOLON. { K = "current:"; }

%type binOp { std::string }
%type binChar { std::string }

binOp(B) ::= binChar(c). { B = c; }
binOp(B) ::= binOp(b) binChar(c). { B = b; B += c; }

binChar(B) ::= BINARY(b). { B = b.stringValue; }
binChar(B) ::= LCARET. { B = std::string("<"); }
binChar(B) ::= RCARET. { B = std::string(">"); }