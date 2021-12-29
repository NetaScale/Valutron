/* Copyright 2020-2021 D. Mackay. All rights reserved. */
/*************************************************************************
 * Little Smalltalk, Version 1
 *
 * The source code for the Little Smalltalk system may be freely copied
 * provided that the source of all files is acknowledged and that this
 * condition is copied with each file.
 *
 * The Little Smalltalk system is distributed without responsibility for
 * the performance of the program and without any guarantee of maintenance.
 *
 * All questions concerning Little Smalltalk should be addressed to:
 *
 *	 Professor Tim Budd
 *	 Department of Computer Science
 *	 Oregon State University
 *	 Corvallis, Oregon
 *	 97331
 *	 USA
 *************************************************************************/

%{
#include "Compiler.hh"
#include "Parser.tab.h"

#define YY_NO_UNISTD_H
#define isatty(X) NULL
%}

%option noyywrap
%option stack
%option yylineno
%option reentrant
%option extra-type = "MVST_Parser *"
%option prefix = "mvst"
%option caseless

%{

#define YY_USER_ACTION \
	yyextra->recOldPos(); \
	for(int i = 0; yytext[i] != '\0'; i++) \
	{ \
	if(yytext[i] == '\n') \
		yyextra->cr(); \
	else \
		yyextra->incCol(); \
	}

#define p(x) yyextra->parse(TOK_##x)
#define ps(x) yyextra->parse(TOK_##x, yytext)
#define pi(x) yyextra->parse(TOK_##x, atoi(yytext))
#define pf(x) yyextra->parse(TOK_##x, atof(yytext))

%}

Digit ([0-9])
Letter ([a-zA-Z_\xA8\xAA\xAD\xAF\xB2\xB5\xB7\xB8\xB9\xBA\xBC\xBD\xBE]|[\xC0-\xFF][\x80-\xBF]*|\\u([0-9a-fA-F]{4}))
Identifier (:?{Letter})(:?{Letter}|{Digit}|::)*
LineTerminator ((\n)|(\r)|(\r\n))
QuotedComment \"(\\.|[^"])*\"

RegularChar [^ \t\n.()\[]

Exponent (e[-+]?[0-9]+)
RadixInteger [0-9]+r-?[0-9A-Z]+

%%

{QuotedComment}			{}

":="				{ p(ASSIGN); }
"<-"				{ p(ASSIGN); }
"@include"			{ p(ATinclude); }

"class>>"			{ p(CLASSRR); }
"subclass:"			{ p(SUBCLASSCOLON); }
"Namespace"			{ p(NAMESPACE); }
"current:"			{ p(CURRENTCOLON); }

${RegularChar}					{ ps(CHAR); }
#{RegularChar}+					{ ps(SYMBOL); }
{RadixInteger}(\.[0-9A-Z]+)?{Exponent}?		{ pf(FLOAT); }
-[0-9]+						{ pi(INTEGER); }
[0-9]+						{ pi(INTEGER); }
[0-9]+(\.[0-9]+)?{Exponent}?			{ pf(FLOAT); }
:{Identifier}					{ ps(COLONVAR); }
{Identifier}:					{ ps(KEYWORD); }
{Identifier}"."{Identifier}			{ ps(NAMESPACENAME); }
{Identifier}					{ ps(IDENTIFIER); }
'''(.*)'''					{ ps(STRING); }
[']([^']|\\.)*[']			 	{ ps(STRING); }

"<"[0-9]+			{ yyextra->parse(TOK_PRIMNUM, atoi(yytext + 1)); }

"^"				{ p(UP); }
"."				{ p(DOT); }
"|"				{ p(BAR); }
";"				{ p(SEMICOLON); }
"("				{ p(LBRACKET); }
")"				{ p(RBRACKET); }
"["				{ p(SQB_OPEN); }
"]"				{ p(SQB_CLOSE); }
"<"				{ p(LCARET); }
">"				{ p(RCARET); }
"#"				{ p(HASH); }

{LineTerminator}	{}
[ \t\h\v]		{}

.			{ ps(BINARY); }