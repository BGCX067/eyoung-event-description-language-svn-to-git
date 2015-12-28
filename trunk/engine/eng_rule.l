%{
#include <stdio.h>
#include <stdlib.h>
#include "eng_priv.h"
#include "eng_rule_parser.h"

static int ident_type(const char *str);
static int string_type(const char *str);
static int number_type(const char *str);
static void comment(yyscan_t yyscanner);

#define YY_USER_ACTION							\
	{											\
		yylloc->first_line = yylineno;			\
		yylloc->first_column = yycolumn;		\
		yylloc->last_line = yylineno;			\
		yylloc->last_column = yycolumn+yyleng;	\
		yycolumn += yyleng;						\
	}
%}

%option header-file="eng_rule_lex.h"
%option outfile="eng_rule_lex.c"
%option prefix="eng_rule_"
%option bison-bridge
%option bison-locations
%option noyywrap
%option reentrant
%option ansi-definitions
%option ansi-prototypes
%option nounput
%option noyyalloc
%option noyyfree
%option noyyrealloc
%option yylineno

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			([Ee][+-]?{D}+)
P           ([Pp][+-]?{D}+)
FS			(f|F|l|L)
IS          ((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))
%%
"/*"						{ comment(yyscanner); }
"//"[^\n]*					{ /* consume //-comment */ }
"auto"						{ return(TOKEN_AUTO); }
"_Bool"						{ return(TOKEN_BOOL); }
"break"						{ return(TOKEN_BREAK); }
"case"						{ return(TOKEN_CASE); }
"char"						{ return(TOKEN_CHAR); }
"_Complex"					{ return(TOKEN_COMPLEX); }
"const"						{ return(TOKEN_CONST); }
"continue"					{ return(TOKEN_CONTINUE); }
"default"					{ return(TOKEN_DEFAULT); }
"do"						{ return(TOKEN_DO); }
"double"					{ return(TOKEN_DOUBLE); }
"else"						{ return(TOKEN_ELSE); }
"enum"						{ return(TOKEN_ENUM); }
"extern"					{ return(TOKEN_EXTERN); }
"float"						{ return(TOKEN_FLOAT); }
"for"						{ return(TOKEN_FOR); }
"goto"						{ return(TOKEN_GOTO); }
"if"						{ return(TOKEN_IF); }
"_Imaginary"				{ return(TOKEN_IMAGINARY); }
"inline"					{ return(TOKEN_INLINE); }
"int"						{ return(TOKEN_INT); }
"long"						{ return(TOKEN_LONG); }
"register"					{ return(TOKEN_REGISTER); }
"restrict"					{ return(TOKEN_RESTRICT); }
"return"					{ return(TOKEN_RETURN); }
"short"						{ return(TOKEN_SHORT); }
"signed"					{ return(TOKEN_SIGNED); }
"sizeof"					{ return(TOKEN_SIZEOF); }
"static"					{ return(TOKEN_STATIC); }
"struct"					{ return(TOKEN_STRUCT); }
"switch"					{ return(TOKEN_SWITCH); }
"typedef"					{ return(TOKEN_TYPEDEF); }
"union"						{ return(TOKEN_UNION); }
"unsigned"					{ return(TOKEN_UNSIGNED); }
"void"						{ return(TOKEN_VOID); }
"volatile"					{ return(TOKEN_VOLATILE); }
"while"						{ return(TOKEN_WHILE); }

{L}({L}|{D})*				{ return(ident_type(yytext)); }
0[xX]{H}+{IS}?				{ return(number_type(yytext)); }
0[0-7]*{IS}?				{ return(number_type(yytext)); }
[1-9]{D}*{IS}?				{ return(number_type(yytext)); }
L?'(\\.|[^\\'\n])+'			{ return(string_type(yytext)); }
{D}+{E}{FS}?				{ return(number_type(yytext)); }
{D}*"."{D}+{E}?{FS}?		{ return(number_type(yytext)); }
{D}+"."{D}*{E}?{FS}?		{ return(number_type(yytext)); }
0[xX]{H}+{P}{FS}?			{ return(number_type(yytext)); }
0[xX]{H}*"."{H}+{P}?{FS}?	{ return(number_type(yytext)); }
0[xX]{H}+"."{H}*{P}?{FS}?	{ return(number_type(yytext)); }
L?\"(\\.|[^\\"\n])*\"		{ return(string_type(yytext)); }

"..."						{ return(TOKEN_ELLIPSIS); }
">>="						{ return(TOKEN_DRARROWEQ); }
"<<="						{ return(TOKEN_DLARROWEQ); }
"+="						{ return(TOKEN_PLUSEQ); }
"-="						{ return(TOKEN_MINUSEQ); }
"*="						{ return(TOKEN_STAREQ); }
"/="						{ return(TOKEN_SLASHEQ); }
"%="						{ return(TOKEN_PERCENTEQ); }
"&="						{ return(TOKEN_AMPEREQ); }
"^="						{ return(TOKEN_UPARROWEQ); }
"|="						{ return(TOKEN_BAREQ); }
">>"						{ return(TOKEN_DRARROW); }
"<<"						{ return(TOKEN_DLARROW); }
"++"						{ return(TOKEN_DPLUS); }
"--"						{ return(TOKEN_DMINUS); }
"->"						{ return(TOKEN_MINUSARROW); }
"&&"						{ return(TOKEN_DAMPER); }
"||"						{ return(TOKEN_DBAR); }
"<="						{ return(TOKEN_LARROWEQ); }
">="						{ return(TOKEN_RARROWEQ); }
"=="						{ return(TOKEN_DEQ); }
"!="						{ return(TOKEN_EXCLAMEQ); }
";"							{ return(TOKEN_SEMI); }
("{"|"<%")					{ return(TOKEN_LBRACE); }
("}"|"%>")					{ return(TOKEN_RBRACE); }
","							{ return(TOKEN_COMMA); }
":"							{ return(TOKEN_COLON); }
"="							{ return(TOKEN_EQ); }
"("							{ return(TOKEN_LPAREN); }
")"							{ return(TOKEN_RPAREN); }
("["|"<:")					{ return(TOKEN_LBRACK); }
("]"|":>")					{ return(TOKEN_RBRACK); }
"."							{ return(TOKEN_DOT); }
"&"							{ return(TOKEN_AMPER); }
"!"							{ return(TOKEN_EXCLAM); }
"~"							{ return(TOKEN_TILDE); }
"-"							{ return(TOKEN_MINUS); }
"+"							{ return(TOKEN_PLUS); }
"*"							{ return(TOKEN_STAR); }
"/"							{ return(TOKEN_SLASH); }
"%"							{ return(TOKEN_PERCENT); }
"<"							{ return(TOKEN_LARROW); }
">"							{ return(TOKEN_RARROW); }
"^"							{ return(TOKEN_UPARROW); }
"|"							{ return(TOKEN_BAR); }
"?"							{ return(TOKEN_QUEST); }

[ \t\v\f]					{/*do nothing*/}
[\n]						{yycolumn=1;}
.							{/* error handler for unrecognized char*/ }
%%
static void comment(yyscan_t yyscanner)
{
	char c, prev = 0;
	while ((c = input(yyscanner)) != 0)
	{
		if (c == '/' && prev == '*')
			return;
		prev = c;
	}
	engine_lexier_error("unterminated comment\n");
}

static int ident_type(const char *str)
{
	return TOKEN_IDENT;
}

static int number_type(const char *str)
{
	return TOKEN_CONST_INT;
}

static int string_type(const char *str)
{
	return TOKEN_CONST_STRING;
}

void *eng_rule_alloc(yy_size_t sz, yyscan_t yyscanner)
{
	return lexier_malloc(sz);
}

void *eng_rule_realloc(void *ptr, yy_size_t sz, yyscan_t yyscanner)
{
	return lexier_realloc(ptr,sz);
}

void eng_rule_free(void *ptr, yyscan_t yyscanner)
{
	lexier_free(ptr);
}
