%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "test_share.h"
#include "test_parser.h"
#include "test_lex.h"
#include "test_mem.h"

#undef YYMALLOC
#undef YYFREE
#define YYMALLOC(sz) test_malloc(sz)
#define YYFREE(ptr) test_free(ptr)
%}

%union
{
	int integer;
}
%token TOK_NUM
%token TOK_PLUS
%token TOK_MINUS
%token TOK_NEWLINE
%token TOK_CONTINUE

%type <integer> TOK_NUM
%type <integer> expr

%destructor {fprintf(stderr, "====>free %d\n", $$);} TOK_NUM expr
%left TOK_PLUS TOK_MINUS

%define api.prefix calc
%define api.pure full
%define api.push-pull push
%start input
%%
input :	empty
	|	input line
	;

empty:
	;

line : TOK_NEWLINE
	{
		fprintf(stderr, "=====>nothing\n");
	}
	| expr TOK_NEWLINE
	{
		fprintf(stderr, "=====>GREAT! value: %d\n", $1);
	}
	;

expr : TOK_NUM
	{
		$$ = $1;
	}
	| expr TOK_PLUS expr
	{
		$$ = $1 + $3;
	}
	| expr TOK_MINUS expr
	{
		$$ = $1 - $3;
	}
	;
%%
int main(int argc, char *argv[])
{
	calcpstate *parser;
	CALCSTYPE value;
	int token;

	calcdebug = 1;
	parser = calcpstate_new();
	if(!parser)
	{
		calcerror("malloc parser failed\n");
		return -1;
	}
	
	yyscan_t scanner;
	calc_share_t priv;
	memset(&priv, 0, sizeof(priv));
	if(calclex_init_extra(&priv, &scanner))
	{
		calcerror("init scanner failed\n");
		return -1;
	}

	/*
	 * 1+++ 32---	23
	 * I use '+++' and '---' is just for testing the START CONDITION feature in FLEX
	 */
	char *strings[] = {"1++", "+ 3", "2-", "--	23", "\n"};
	int strings_len = sizeof(strings)/sizeof(strings[0]);
	int index, parser_ret;
	YY_BUFFER_STATE stream;
	for(index=0 ; index<strings_len; index++)
	{
		priv.last_frag = (index==strings_len-1);

		stream = calc_scan_stream(strings[index], strlen(strings[index]), 
			priv.last_string, priv.last_string_len, scanner);

		if(priv.last_string)
		{
			test_free(priv.last_string);
			priv.last_string = NULL;
			priv.last_string_len = 0;
		}

		if(!stream)
			break;
		while(1)
		{
			token = calclex(&value, scanner);
			if(token == TOK_CONTINUE)
				break;
			parser_ret = calcpush_parse(parser, token, &value);
			if(parser_ret != YYPUSH_MORE)
				break;
		}
		calc_delete_buffer(stream, scanner);
		if(parser_ret != YYPUSH_MORE && parser_ret != 0)
		{
			fprintf(stderr, "find error\n");
			break;
		}
	}
	if(priv.last_string)
	{
		test_free(priv.last_string);
		priv.last_string = NULL;
		priv.last_string_len = 0;
	}
	calclex_destroy(scanner);
	calcpstate_delete(parser);
	return 0;
}

int calcerror(const char *format,...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
	return 0;
}
