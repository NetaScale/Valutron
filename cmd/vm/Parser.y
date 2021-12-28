/*
 * Copyright 2020 David MacKay.  All rights reserved.
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
    if (1)//0)
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

	std::cerr << "PDST: " << fName << "(" << std::to_string(m_line) + ","			  << std::to_string(m_col) << "): "
			  << "Error V1001: Syntax error: unexpected "			  << yyTokenName[yymajor] << "\n";

	std::cerr << "+ " << eLine << "\n";
	std::cerr << "+ ";
	for (i = 0; i < m_oldCol; i++)
		std::cerr << " ";
	for (; i < m_col; i++)
		std::cerr << "^";

	std::cerr << "\n\texpected one of: \n";

	for (unsigned i = 0; i < YYNTOKEN; ++i)
	{
		int yyact = yy_find_shift_action(i, stateno);
		if (yyact != YY_ERROR_ACTION && yyact != YY_NO_ACTION)
			std::cerr << "\t" << yyTokenName[i] << "\n";
	}
}

prog ::= file.

file ::= directMeth EOF.


file ::= lDecl(l) EOF.
	{
		program = new ProgramNode(l);
	}

directMeth ::= DIRECTMETH oIvarDefs(locals)
		statementList(stmts)
		oFinalDot.
	{
		meth = new MethodNode(false, "fakeselector", {}, locals, stmts);
	}

%type lDecl { std::vector<DeclNode *> }
%type decl { DeclNode * }
%type clsDecl { DeclNode * }

lDecl(L) ::= decl(d). { L = {d}; }
lDecl(L) ::= lDecl(l) decl(d).
	{
		L = l;
		L.push_back(d);
	}

decl(D) ::= ATinclude STRING(name).
	{
		D = parseFile(path + "/" + removeFirstAndLastChar(name));
	}
decl(D) ::= NAMESPACE CURRENTCOLON identifier(name) SQB_OPEN lDecl(l) SQB_CLOSE.
	{
		printf("new namespace node %s\n", name.stringValue.c_str());
		D = new NamespaceNode(name, l);
	}
decl ::= clsDecl.


clsDecl(D) ::= identifier(super) SUBCLASSCOLON identifier(name)
	SQB_OPEN
		oCAndIvarDefs(iVars)
		olMethDecl(iMeths)
	SQB_CLOSE.
	{
		ClassNode * cls = new ClassNode(name, super, {}, iVars);
		cls->addMethods(iMeths);
		D = cls;
	}

%type oCAndIvarDefs { std::vector<std::string>}
%type oIvarDefs { std::vector<std::string> }
%type oLIvar { std::vector<std::string> }
%type lIvar { std::vector<std::string> }

oCAndIvarDefs ::= .
oCAndIvarDefs(L) ::= BAR oLIvar(l) BAR. { L = l; }
oCAndIvarDefs(L) ::= BAR oLIvar(l) BAR BAR lIvar BAR. { L = l; }

oIvarDefs ::= .
oIvarDefs(L) ::= BAR lIvar(l) BAR. { L = l; }

oLIvar ::= .
oLIvar(L) ::= lIvar(l). { L = l; }

lIvar(L) ::= IDENTIFIER(i). { L = {i}; }
lIvar(L) ::= lIvar(l) IDENTIFIER(i).
	{
		L = l;
		L.push_back(i);	}

%type olMethDecl { std::vector<MethodNode *> }
%type lMethDecl { std::vector<MethodNode *> }
%type methDecl { MethodNode * }

olMethDecl(L) ::= lMethDecl(l). { L = l; }
olMethDecl ::= .

lMethDecl(L) ::= methDecl(d). { L = {d}; }
lMethDecl(L) ::= lMethDecl(l) methDecl(d).
	{
		L = l;
		L.push_back(d);	}

methDecl(D) ::= oCLASSRR(isClass) sel_decl(s)
	SQB_OPEN		oIvarDefs(locals)
		statementList(stmts)
		oFinalDot
	SQB_CLOSE.
	{
		//for (auto s: stmts)
		//s->print(2);
		D = new MethodNode(isClass, s.first, s.second, locals, stmts);
	}

%type oCLASSRR { bool }

oCLASSRR(C) ::= . { C = false; }
oCLASSRR(C) ::= CLASSRR. { C = true; }

%type statementList { std::vector<StmtNode *> }
%type statement { StmtNode * }

statementList(L) ::= statement(s). { L = {s}; }
statementList(L) ::= statementList(l) DOT statement(s).
	{
		L = l;
		L.push_back(s);	}

statement(S) ::= UP sExpression(e). { S = new ReturnStmtNode(e);  }
statement(S) ::= sExpression(e). { S = new ExprStmtNode(e); }

%type sExpression { ExprNode * }
%type cExpression { ExprNode * }
%type kContinuation { CascadeExprNode * }
%type bContinuation { CascadeExprNode * }
%type uContinuation { CascadeExprNode * }
%type expression { ExprNode * }

sExpression(E) ::= identifier(i) ASSIGN expression(e).
	{
		E = new AssignExprNode(new IdentExprNode (i), e);
	}
sExpression(E) ::= cExpression(e). { E = e;}

cExpression(C) ::= expression(e). { C = e; }
cExpression(C) ::= kContinuation(e). { C = e; }

kContinuation(C) ::= bContinuation(c).
	{
		C = c;
	}
kContinuation(C) ::= bContinuation(c) keywordList(l).
	{
		C = c;
		C->messages.push_back(
			new MessageExprNode(C->receiver, l));
	}

bContinuation(C) ::= uContinuation(c). { C = c; }
bContinuation(C) ::= bContinuation(c) binOp(s) unary(e).
	{
		C = c;
		C->messages.push_back(new MessageExprNode(C->receiver, s, {e}));
	}


uContinuation(C) ::= cExpression(r) SEMICOLON.
	{
		/* FIXME: To avoid false positives, put bracketed expressions in their
		 * own thing. */
        CascadeExprNode * c = dynamic_cast<CascadeExprNode *> (r);
        C = c ? c : new CascadeExprNode (r);	
	}
uContinuation(C) ::= uContinuation(c) identifier(i).
	{
		C = c;
		C->messages.push_back(new MessageExprNode(C->receiver, i));
	}

expression(E) ::= binary(b). { E = b; }
expression(E) ::= binary(e) keywordList(k).
	{
		E = new MessageExprNode(e, k);
	}

%type keywordList { std::vector<std::pair<std::string, ExprNode *>> }

keywordList(L) ::= keyword(k) binary(e).
	{
		L = { {k, e}};
	}
keywordList(L) ::= keywordList(l) keyword(k) binary(e).
	{
		L = l;
		L.push_back( { k, e });
	}

%type binary { ExprNode * }
%type unary { ExprNode * }
%type primary { ExprNode * }

binary(E) ::= unary(e). { E = e; }
binary(E) ::= binary(r) binOp(s) unary(a).
	{
		E  = new MessageExprNode(r, s, { a });	}

unary(E) ::= primary(e). { E = e; }
unary(U) ::= unary(p) identifier(i). { U  = new MessageExprNode(p, i); }

primary(S) ::= identifier(i). { S = new IdentExprNode (i); }
primary(S) ::= LBRACKET sExpression(s) RBRACKET. { S = s; }
primary ::= block.
primary ::= literal.
primary(S) ::= PRIMNUM(n) oLPrimary(l) RCARET.
	{
		S = new PrimitiveExprNode(n.intValue, l);
	}

%type oLPrimary { std::vector<ExprNode *> }
%type lPrimary { std::vector<ExprNode *> }

oLPrimary ::= .
oLPrimary(L) ::= lPrimary(l). { L = l; }

lPrimary(L) ::= primary(p). { L = {p}; }
lPrimary(L) ::= lPrimary(l) primary(p).
	{
		L = l;
		L.push_back(p);
	}

%type literal { ExprNode * }
%type aLiteral { ExprNode * }
%type iLiteral { ExprNode * }

literal(L) ::= HASH LBRACKET oLALiteral(a) RBRACKET.  { L = new ArrayExprNode(a); }
literal ::= iLiteral.

%type lALiteral { std::vector<ExprNode *> }
%type oLALiteral { std::vector<ExprNode *> }

oLALiteral ::= .
oLALiteral(A) ::= lALiteral(l). {A = l;}

lALiteral(L) ::= aLiteral(l). { L = {l}; }
lALiteral(L) ::= lALiteral(l) aLiteral(lit).
	{
		L = l;
		L.push_back(lit);
	}

aLiteral(A) ::= iLiteral(i). { A = i; }
aLiteral(A) ::= identifier(i). { A = new SymbolExprNode(i); }
aLiteral(A) ::= keyword(k). { A = new SymbolExprNode(k); }
aLiteral(A) ::= ias oLALiteral(a) RBRACKET. { A = new ArrayExprNode(a); }

ias ::= HASH LBRACKET.
ias ::= LBRACKET.

iLiteral(S) ::= STRING(s).
	{
		S = new StringExprNode(removeFirstAndLastChar(s));
	}
iLiteral(S) ::= SYMBOL(s).
	{
		S = new SymbolExprNode(removeFirstChar(s));
	}
iLiteral(S) ::= INTEGER(i).
	{
		S = new IntExprNode(i.intValue);
	}
iLiteral(S) ::= FLOAT(f).
	{
		S = new FloatExprNode(f);
	}
iLiteral(S) ::= CHAR(c).
	{
		S = new CharExprNode(removeFirstChar(c));
	}

%type block { BlockExprNode * }

block(B) ::= SQB_OPEN oBlockVarList(v) oStatementList(s) SQB_CLOSE.
	{
		B = new BlockExprNode(v, s);
	}

oFinalDot ::= .
oFinalDot ::= DOT.

%type oStatementList { std::vector<StmtNode *> }

oStatementList ::= .
oStatementList(L) ::= statementList(l). { L = l; }

%type oBlockVarList { std::vector<std::string> }
%type colonVarList { std::vector<std::string> }

oBlockVarList(L) ::= colonVarList(l) BAR. { L = l; }
oBlockVarList ::= .

colonVarList(L) ::= COLONVAR(v).	{
		std::string s = v;
		s.erase(s.begin());
		L = {s};
	}colonVarList(L) ::= colonVarList(l) COLONVAR(v).
	{
		L = l;
		std::string s = v;
		s.erase(s.begin());
		L.push_back(s);
	}

%type sel_decl { std::pair<std::string, std::vector<std::string>> }

sel_decl(S) ::= identifier(i). { S = {i, {}};  }
sel_decl(S)
	::= binary_decl(b).
	{		S = {b.first, {b.second}};
	}
sel_decl(S)
	::= keyw_decl_list(k).
	{
		S = k;
	}

%type keyw_decl_list { std::pair<std::string, std::vector<std::string>> }

keyw_decl_list(L) ::= keyw_decl(k). {
    L = {k.first, {k.second}};
}
keyw_decl_list(L) ::= keyw_decl_list(l) keyw_decl(k). {    L = l;
	L.first += k.first;
	L.second.push_back(k.second);
}

%type keyw_decl { std::pair<std::string, std::string> }
%type binary_decl { std::pair<std::string, std::string> }

keyw_decl(K) ::= keyword(k) identifier(s). {
    K = {k, s};
}

binary_decl(B) ::= binOp(b) identifier(s). {
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