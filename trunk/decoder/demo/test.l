%{
#include <stdlib.h>
#include <string.h>
#include "test_share.h"
#include "test_parser.h"
#include "test_mem.h"

typedef CALCSTYPE YYSTYPE;
static int save_stream_context(yyscan_t yyscaner, int from_default);

#define YY_USER_ACTION								\
{													\
	if(yyg->yy_hold_char==YY_END_OF_BUFFER_CHAR &&	\
			save_stream_context(yyg,0))				\
		return TOK_CONTINUE;						\
}

#ifdef YY_RESTORE_YY_MORE_OFFSET
	#undef YY_RESTORE_YY_MORE_OFFSET
	#define YY_RESTORE_YY_MORE_OFFSET				\
	{												\
		if(save_stream_context(yyg,1))				\
			return TOK_CONTINUE;					\
	}
#endif
%}

%option noyywrap
%option reentrant
%option noyyalloc
%option noyyfree
%option noyyrealloc
%option noinput
%option nounput

NUMBER [1-9][0-9]*
SPACE [ \t]+

%x PUNCT
%%
<INITIAL>{
{SPACE}
{NUMBER} {BEGIN(PUNCT);yylval->integer = atoi(yytext); return TOK_NUM;}
. {return yytext[0];}
}

<PUNCT>{
{SPACE}
"\+\+\+" {BEGIN(INITIAL); return TOK_PLUS;}
"\-\-\-" {BEGIN(INITIAL); return TOK_MINUS;}
"\n" {BEGIN(INITIAL); return TOK_NEWLINE;}
. {return yytext[0];}
}
%%

void *calcalloc (yy_size_t sz,yyscan_t yyscanner )
{
	return test_malloc(sz);
}

void *calcrealloc (void *ptr,yy_size_t sz,yyscan_t yyscanner )
{
	return test_realloc(ptr, sz);
}

void calcfree (void *ptr ,yyscan_t yyscanner )
{
	test_free(ptr);
}

struct yy_buffer_state* calc_scan_stream(const char *new_buf, size_t new_buf_len,
	const char *last_buf, size_t last_buf_len, void *scanner)
{
	YY_BUFFER_STATE b;
	char *buf;
	yy_size_t n, _yybytes_len;
    
	_yybytes_len = new_buf_len + last_buf_len;
	n = _yybytes_len + 2;
	buf = (char *)calcalloc(n, scanner);
	if (!buf)
	{
		calcerror("out of dynamic memory in calc_scan_stream()\n" );
		return NULL;
	}

	if(last_buf)
		memcpy(buf, last_buf, last_buf_len);
	
	if(new_buf)
		memcpy(buf+last_buf_len, new_buf, new_buf_len);

	buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;

	b = calc_scan_buffer(buf, n, scanner);
	if ( ! b )
	{
		calcerror("bad buffer in calc_scan_bytes()\n" );
		calcfree(buf, scanner);
		return NULL;
	}

	b->yy_is_our_buffer = 1;
	return b;
}

static int save_stream_context(yyscan_t  yyscanner, int from_default)
{
	struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	calc_share_t *priv = (calc_share_t*)calcget_extra(yyg);
	int len = 0;
	if(!priv || priv->last_frag)
	{
		fprintf(stderr, "======>priv: %p, last_frag: %d\n", priv, priv?priv->last_frag:0);
		return 0;
	}

	if(priv->last_string)
	{
		calcfree(priv->last_string, yyg);
		priv->last_string = NULL;
		priv->last_string_len = 0;
	}
	
	len = from_default?yyleng-1:yyleng;
	if(!len)
	{
		fprintf(stderr, "======>yyleng=0\n");
		return 0;
	}

	priv->last_string = calcalloc(len, yyg);
	if(!priv->last_string)
	{
		calcerror("out of memory while saving context\n");
		return 0;
	}
	memcpy(priv->last_string, yytext, len);
	priv->last_string_len = len;
	fprintf(stderr, "======>save context, yytext: %s, yyleng: %d\n", yytext, len);
	return 1;
}
