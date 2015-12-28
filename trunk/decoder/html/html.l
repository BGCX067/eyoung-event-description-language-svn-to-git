%{
#include <stdlib.h>
#include <string.h>
#include "html.h"
#include "html_mem.h"
#include "html_parser.h"

static int save_stream_context(yyscan_t yyscanner, int from_default);
#define YY_USER_ACTION									\
	if(yyg->yy_hold_char==YY_END_OF_BUFFER_CHAR &&		\
			save_stream_context(yyg,0))					\
		return SYM_LEX_CONTINUE;

#ifdef YY_RESTORE_YY_MORE_OFFSET
	#undef YY_RESTORE_YY_MORE_OFFSET
	#define YY_RESTORE_YY_MORE_OFFSET					\
	{													\
		if(save_stream_context(yyg,1))					\
			return SYM_LEX_CONTINUE;					\
	}
#endif

#ifdef HTML_DEBUG
#define YY_USER_INIT									\
	{													\
		yyout = stderr;									\
	}
#endif

#define ret(val)	do									\
	{													\
		YYFPRINTF(stderr, "line %d get token %s\n",		\
			__LINE__, get_token_name(val));				\
		if(val==SYM_TEXT)								\
			YYFPRINTF(stderr, "get text: %s\n",yytext);	\
		return (val);									\
	}while(0)
%}

%option prefix="html"
%option bison-bridge
%option outfile="html_lex.c"
%option header-file="html_lex.h"
%option noyywrap
%option reentrant
%option noyyalloc
%option noyyfree
%option noyyrealloc
%option caseless
%option noinput
%option ansi-definitions

%x HTML_LEX_COMMENT
%x HTML_LEX_META
%x HTML_LEX_TAG_NAME
%x HTML_LEX_PROT_NAME
%x HTML_LEX_PROT_VALUE

SPACE			[ \t\r\n]*
HTML_CHARSET	[[:space:][:alnum:]\-_:<>=%]

%%
<INITIAL>{
"<!--"			{BEGIN HTML_LEX_COMMENT; YYFPRINTF(stderr, "start comment at [%d,%d]\n", yylineno, yycolumn);}
"<!"			{BEGIN HTML_LEX_META; YYFPRINTF(stderr, "start meta data at [%d,%d]\n", yylineno, yycolumn);}
"</"			{BEGIN HTML_LEX_TAG_NAME; ret(SYM_TAG_START_FLAG2);}
"<"				{BEGIN HTML_LEX_TAG_NAME; ret(SYM_TAG_START_FLAG);}
[^<]+			{yylval->str = yytext; yylval->str_len = yyleng; ret(SYM_TEXT);}
}

<HTML_LEX_COMMENT>{
"-->"			{BEGIN INITIAL; YYFPRINTF(stderr, "end comment at [%d,%d]\n", yylineno, yycolumn);}
.				{/*eat all comment*/}
}

<HTML_LEX_META>{
">"				{BEGIN INITIAL; YYFPRINTF(stderr, "end meta data at [%d,%d]\n", yylineno, yycolumn);}
.				{/*eat all meta data*/}
}

<HTML_LEX_TAG_NAME>{
{SPACE}			{/*eat space*/}

"ABBR"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_ABBR);}
"ACRONYM"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_ACRONYM);}
"ADDRESS"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_ADDRESS);}
"APPLET"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_APPLET);}
"AREA"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_AREA);}
"ARTICLE"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_ARTICLE);}
"ASIDE"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_ASIDE);}
"AUDIO"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_AUDIO);}
"A"				{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_A);}
"BASEFONT"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_BASEFONT);}
"BASE"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_BASE);}
"BDO"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_BDO);}
"BIG"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_BIG);}
"BLOCKQUOTE"	{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_BLOCKQUOTE);}
"BODY"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_BODY);}
"BR"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_BR);}
"BUTTON"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_BUTTON);}
"B"				{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_B);}
"CANVAS"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_CANVAS);}
"CAPTION"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_CAPTION);}
"CENTER"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_CENTER);}
"CITE"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_CITE);}
"CODE"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_CODE);}
"COLGROUP"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_COLGROUP);}
"COL"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_COL);}
"COMMAND"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_COMMAND);}
"DATALIST"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_DATALIST);}
"DD"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_DD);}
"DEL"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_DEL);}
"DETAILS"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_DETAILS);}
"DFN"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_DFN);}
"DIR"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_DIR);}
"DIV"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_DIV);}
"DL"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_DL);}
"DT"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_DT);}
"EM"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_EM);}
"EMBED"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_EMBED);}
"FIELDSET"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_FIELDSET);}
"FIGCAPTION"	{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_FIGCAPTION);}
"FIGURE"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_FIGURE);}
"FONT"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_FONT);}
"FOOTER"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_FOOTER);}
"FORM"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_FORM);}
"FRAME"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_FRAME);}
"FRAMESET"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_FRAMESET);}
"H1"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_H1);}
"H2"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_H2);}
"H3"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_H3);}
"H4"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_H4);}
"H5"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_H5);}
"H6"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_H6);}
"HEAD"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_HEAD);}
"HEADER"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_HEADER);}
"HGROUP"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_HGROUP);}
"HR"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_HR);}
"HTML"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_HTML);}
"IFRAME"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_IFRAME);}
"IMG"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_IMG);}
"INPUT"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_INPUT);}
"INS"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_INS);}
"I"				{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_I);}
"KEYGEN"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_KEYGEN);}
"KBD"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_KBD);}
"LABEL"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_LABEL);}
"LEGEND"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_LEGEND);}
"LINK"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_LINK);}
"LI"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_LI);}
"MAP"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_MAP);}
"MARK"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_MARK);}
"MENU"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_MENU);}
"META"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_META);}
"METER"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_METER);}
"NAV"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_NAV);}
"NOFRAME"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_NOFRAME);}
"NOSCRIPT"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_NOSCRIPT);}
"OBJECT"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_OBJECT);}
"OL"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_OL);}
"OPTGROUP"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_OPTGROUP);}
"OPTION"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_OPTION);}
"OUTPUT"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_OUTPUT);}
"PARAM"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_PARAM);}
"PRE"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_PRE);}
"PROGRESS"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_PROGRESS);}
"P"				{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_P);}
"Q"				{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_Q);}
"RP"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_RP);}
"RT"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_RT);}
"RUBY"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_RUBY);}
"SAMP"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_SAMP);}
"SCRIPT"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_SCRIPT);}
"SECTION"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_SECTION);}
"SELECT"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_SELECT);}
"SMALL"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_SMALL);}
"SOURCE"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_SOURCE);}
"SPAN"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_SPAN);}
"STRIKE"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_STRIKE);}
"STRONG"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_STRONG);}
"STYLE"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_STYLE);}
"SUB"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_SUB);}
"SUMMARY"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_SUMMARY);}
"SUP"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_SUP);}
"S"				{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_S);}
"TABLE"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_TABLE);}
"TBODY"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_TBODY);}
"TD"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_TD);}
"TH"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_TH);}
"TR"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_TR);}
"TT"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_TT);}
"TEXTAREA"		{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_TEXTAREA);}
"TFOOT"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_TFOOT);}
"THEAD"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_THEAD);}
"TIME"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_TIME);}
"TITLE"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_TITLE);}
"UL"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_UL);}
"U"				{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_U);}
"VAR"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_VAR);}
"VIDEO"			{BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_VIDEO);}
[[:alnum:]\-_:]* {BEGIN HTML_LEX_PROT_NAME; ret(SYM_TAG_USERDATA);}
.				{/*eat others*/}
}

<HTML_LEX_PROT_NAME>{
{SPACE}					/*eat space*/

"="						{BEGIN HTML_LEX_PROT_VALUE; ret(SYM_EQUAL);}

">"						{BEGIN INITIAL; ret(SYM_TAG_END_FLAG);}
"/"{SPACE}">"			{BEGIN INITIAL; ret(SYM_TAG_END_FLAG2);}

"ONAFTERPRINT"			{ret(SYM_EVT_ONAFTERPRINT);}
"ONBEFOREPRINT"			{ret(SYM_EVT_ONBEFOREPRINT);}
"ONBEFOREONLOAD"		{ret(SYM_EVT_ONBEFOREONLOAD);}
"ONBLUR"				{ret(SYM_EVT_ONBLUR);}
"ONFOCUS"				{ret(SYM_EVT_ONFOCUS);}
"ONHASCHANGE"			{ret(SYM_EVT_ONHASCHANGE);}
"ONLOAD"				{ret(SYM_EVT_ONLOAD);}
"ONMESSAGE"				{ret(SYM_EVT_ONMESSAGE);}
"ONOFFLINE"				{ret(SYM_EVT_ONOFFLINE);}
"ONONLINE"				{ret(SYM_EVT_ONONLINE);}
"ONPAGEHIDE"			{ret(SYM_EVT_ONPAGEHIDE);}
"ONPAGESHOW"			{ret(SYM_EVT_ONPAGESHOW);}
"ONPOPSTATE"			{ret(SYM_EVT_ONPOPSTATE);}
"ONREDO"				{ret(SYM_EVT_ONREDO);}
"ONRESIZE"				{ret(SYM_EVT_ONRESIZE);}
"ONSTORAGE"				{ret(SYM_EVT_ONSTORAGE);}
"ONUNDO"				{ret(SYM_EVT_ONUNDO);}
"ONUNLOAD"				{ret(SYM_EVT_ONUNLOAD);}
"ONCHANGE"				{ret(SYM_EVT_ONCHANGE);}
"ONCONTEXTMENU"			{ret(SYM_EVT_ONCONTEXTMENU);}
"ONFORMCHANGE"			{ret(SYM_EVT_ONFORMCHANGE);}
"ONFORMINPUT"			{ret(SYM_EVT_ONFORMINPUT);}
"ONINPUT"				{ret(SYM_EVT_ONINPUT);}
"ONINVALID"				{ret(SYM_EVT_ONINVALID);}
"ONRESET"				{ret(SYM_EVT_ONRESET);}
"ONSELECT"				{ret(SYM_EVT_ONSELECT);}
"ONSUBMIT"				{ret(SYM_EVT_ONSUBMIT);}
"ONKEYDOWN"				{ret(SYM_EVT_ONKEYDOWN);}
"ONKEYPRESS"			{ret(SYM_EVT_ONKEYPRESS);}
"ONKEYUP"				{ret(SYM_EVT_ONKEYUP);}
"ONCLICK"				{ret(SYM_EVT_ONCLICK);}
"ONDBLCLICK"			{ret(SYM_EVT_ONDBLCLICK);}
"ONDRAG"				{ret(SYM_EVT_ONDRAG);}
"ONDRAGEND"				{ret(SYM_EVT_ONDRAGEND);}
"ONDRAGENTER"			{ret(SYM_EVT_ONDRAGENTER);}
"ONDRAGLEAVE"			{ret(SYM_EVT_ONDRAGLEAVE);}
"ONDRAGOVER"			{ret(SYM_EVT_ONDRAGOVER);}
"ONDRAGSTART"			{ret(SYM_EVT_ONDRAGSTART);}
"ONDROP"				{ret(SYM_EVT_ONDROP);}
"ONMOUSEDOWN"			{ret(SYM_EVT_ONMOUSEDOWN);}
"ONMOUSEMOVE"			{ret(SYM_EVT_ONMOUSEMOVE);}
"ONMOUSEOUT"			{ret(SYM_EVT_ONMOUSEOUT);}
"ONMOUSEOVER"			{ret(SYM_EVT_ONMOUSEOVER);}
"ONMOUSEUP"				{ret(SYM_EVT_ONMOUSEUP);}
"ONMOUSEWHEEL"			{ret(SYM_EVT_ONMOUSEWHEEL);}
"ONSCROLL"				{ret(SYM_EVT_ONSCROLL);}
"ONABORT"				{ret(SYM_EVT_ONABORT);}
"ONCANPLAY"				{ret(SYM_EVT_ONCANPLAY);}
"ONCANPLAYTHROUGH"		{ret(SYM_EVT_ONCANPLAYTHROUGH);}
"ONDURATIONCHANGE"		{ret(SYM_EVT_ONDURATIONCHANGE);}
"ONEMPTIED"				{ret(SYM_EVT_ONEMPTIED);}
"ONENDED"				{ret(SYM_EVT_ONENDED);}
"ONERROR"				{ret(SYM_EVT_ONERROR);}
"ONLOADEDDATA"			{ret(SYM_EVT_ONLOADEDDATA);}
"ONLOADEDMETADATA"		{ret(SYM_EVT_ONLOADEDMETADATA);}
"ONLOADSTART"			{ret(SYM_EVT_ONLOADSTART);}
"ONPAUSE"				{ret(SYM_EVT_ONPAUSE);}
"ONPLAY"				{ret(SYM_EVT_ONPLAY);}
"ONPLAYING"				{ret(SYM_EVT_ONPLAYING);}
"ONPROGRESS"			{ret(SYM_EVT_ONPROGRESS);}
"ONRATECHANGE"			{ret(SYM_EVT_ONRATECHANGE);}
"ONREADYSTATECHANGE"	{ret(SYM_EVT_ONREADYSTATECHANGE);}
"ONSEEKED"				{ret(SYM_EVT_ONSEEKED);}
"ONSEEKING"				{ret(SYM_EVT_ONSEEKING);}
"ONSTALLED"				{ret(SYM_EVT_ONSTALLED);}
"ONSUSPEND"				{ret(SYM_EVT_ONSUSPEND);}
"ONTIMEUPDATE"			{ret(SYM_EVT_ONTIMEUPDATE);}
"ONVOLUMECHANGE"		{ret(SYM_EVT_ONVOLUMECHANGE);}
"ONWAITING"				{ret(SYM_EVT_ONWAITING);}

"ACCESSKEY"				{ret(SYM_PROT_ACCESSKEY);}
"CLASS"					{ret(SYM_PROT_CLASS);}
"CONTENTEDITABLE"		{ret(SYM_PROT_CONTENTEDITABLE);}
"CONTEXTMENU"			{ret(SYM_PROT_CONTEXTMENU);}
"DIR"					{ret(SYM_PROT_DIR);}
"DRAGGABLE"				{ret(SYM_PROT_DRAGGABLE);}
"HIDDEN"				{ret(SYM_PROT_HIDDEN);}
"ID"					{ret(SYM_PROT_ID);}
"ITEMPROP"				{ret(SYM_PROT_ITEMPROP);}
"ITEM"					{ret(SYM_PROT_ITEM);}
"LANG"					{ret(SYM_PROT_LANG);}
"SPELLCHECK"			{ret(SYM_PROT_SPELLCHECK);}
"STYLE"					{ret(SYM_PROT_STYLE);}
"SUBJECT"				{ret(SYM_PROT_SUBJECT);}
"TABINDEX"				{ret(SYM_PROT_TABINDEX);}
"TITLE"					{ret(SYM_PROT_TITLE);}
"TEMPLATE"				{ret(SYM_PROT_TEMPLATE);}
"REGISTRATIONMARK"		{ret(SYM_PROT_REGISTRATIONMARK);}
"IRRELEVANT"			{ret(SYM_PROT_IRRELEVANT);}

"OPEN"					{ret(SYM_PROT_OPEN);}
"DATA"					{ret(SYM_PROT_DATA);}
"NOWRAP"				{ret(SYM_PROT_NOWRAP);}
"DATETIME"				{ret(SYM_PROT_DATETIME);}
"ROWS"					{ret(SYM_PROT_ROWS);}
"LIST"					{ret(SYM_PROT_LIST);}
"FORMTARGETNEW"			{ret(SYM_PROT_FORMTARGETNEW);}
"AUTOFOCUSNEW"			{ret(SYM_PROT_AUTOFOCUSNEW);}
"ICON"					{ret(SYM_PROT_ICON);}
"MAXLENGTH"				{ret(SYM_PROT_MAXLENGTH);}
"WIDTH"					{ret(SYM_PROT_WIDTH);}
"ARCHIVE"				{ret(SYM_PROT_ARCHIVE);}
"HREF"					{ret(SYM_PROT_HREF);}
"PRELOAD"				{ret(SYM_PROT_PRELOAD);}
"MULTIPLE"				{ret(SYM_PROT_MULTIPLE);}
"HREFLANG"				{ret(SYM_PROT_HREFLANG);}
"CELLSPACING"			{ret(SYM_PROT_CELLSPACING);}
"COLSPAN"				{ret(SYM_PROT_COLSPAN);}
"ACTION"				{ret(SYM_PROT_ACTION);}
"CLASSID"				{ret(SYM_PROT_CLASSID);}
"PATTERN"				{ret(SYM_PROT_PATTERN);}
"COLOR"					{ret(SYM_PROT_COLOR);}
"HIGH"					{ret(SYM_PROT_HIGH);}
"PING"					{ret(SYM_PROT_PING);}
"ISMAP"					{ret(SYM_PROT_ISMAP);}
"HTTP-EQUIV"			{ret(SYM_PROT_HTTPEQUIV);}
"HSPACE"				{ret(SYM_PROT_HSPACE);}
"COMPACT"				{ret(SYM_PROT_COMPACT);}
"LANGUAGE"				{ret(SYM_PROT_LANGUAGE);}
"REQUIRED"				{ret(SYM_PROT_REQUIRED);}
"SPAN"					{ret(SYM_PROT_SPAN);}
"FORMACTIONNEW"			{ret(SYM_PROT_FORMACTIONNEW);}
"RULES"					{ret(SYM_PROT_RULES);}
"AXIS"					{ret(SYM_PROT_AXIS);}
"METHOD"				{ret(SYM_PROT_METHOD);}
"BGCOLOR"				{ret(SYM_PROT_BGCOLOR);}
"SHAPE"					{ret(SYM_PROT_SHAPE);}
"USEMAP"				{ret(SYM_PROT_USEMAP);}
"FOR"					{ret(SYM_PROT_FOR);}
"SCOPED"				{ret(SYM_PROT_SCOPED);}
"FORMNOVALIDATENEW"		{ret(SYM_PROT_FORMNOVALIDATENEW);}
"CONTENT"				{ret(SYM_PROT_CONTENT);}
"INPUTMODE"				{ret(SYM_PROT_INPUTMODE);}
"CITE"					{ret(SYM_PROT_CITE);}
"VSPACE"				{ret(SYM_PROT_VSPACE);}
"XMLNS"					{ret(SYM_PROT_XMLNS);}
"CODETYPE"				{ret(SYM_PROT_CODETYPE);}
"TARGET"				{ret(SYM_PROT_TARGET);}
"VALUE"					{ret(SYM_PROT_VALUE);}
"AUTOFOCUS"				{ret(SYM_PROT_AUTOFOCUS);}
"MEDIA"					{ret(SYM_PROT_MEDIA);}
"COORDS"				{ret(SYM_PROT_COORDS);}
"PROFILE"				{ret(SYM_PROT_PROFILE);}
"HEADERS"				{ret(SYM_PROT_HEADERS);}
"VALUETYPE"				{ret(SYM_PROT_VALUETYPE);}
"REPLACE"				{ret(SYM_PROT_REPLACE);}
"MARGINHEIGHT"			{ret(SYM_PROT_MARGINHEIGHT);}
"BORDER"				{ret(SYM_PROT_BORDER);}
"FRAMEBORDER"			{ret(SYM_PROT_FRAMEBORDER);}
"ASYNC"					{ret(SYM_PROT_ASYNC);}
"FACE"					{ret(SYM_PROT_FACE);}
"CELLPADDING"			{ret(SYM_PROT_CELLPADDING);}
"STANDBY"				{ret(SYM_PROT_STANDBY);}
"ALT"					{ret(SYM_PROT_ALT);}
"ACCEPT-CHARSET"		{ret(SYM_PROT_ACCEPTCHARSET);}
"FORMMETHODNEW"			{ret(SYM_PROT_FORMMETHODNEW);}
"AUTOPLAY"				{ret(SYM_PROT_AUTOPLAY);}
"REV"					{ret(SYM_PROT_REV);}
"LOOP"					{ret(SYM_PROT_LOOP);}
"CODE"					{ret(SYM_PROT_CODE);}
"SRC"					{ret(SYM_PROT_SRC);}
"CHECKED"				{ret(SYM_PROT_CHECKED);}
"SCROLLING"				{ret(SYM_PROT_SCROLLING);}
"SCOPE"					{ret(SYM_PROT_SCOPE);}
"DEFER"					{ret(SYM_PROT_DEFER);}
"XML:SPACE"				{ret(SYM_PROT_XMLSPACE);}
"CHALLENGE"				{ret(SYM_PROT_CHALLENGE);}
"SCHEME"				{ret(SYM_PROT_SCHEME);}
"DECLARE"				{ret(SYM_PROT_DECLARE);}
"CHAR"					{ret(SYM_PROT_CHAR);}
"READONLY"				{ret(SYM_PROT_READONLY);}
"XML:LANG"				{ret(SYM_PROT_XMLLANG);}
"MAX"					{ret(SYM_PROT_MAX);}
"ROWSPAN"				{ret(SYM_PROT_ROWSPAN);}
"KEYTYPE"				{ret(SYM_PROT_KEYTYPE);}
"AUTOCOMPLETE"			{ret(SYM_PROT_AUTOCOMPLETE);}
"SELECTED"				{ret(SYM_PROT_SELECTED);}
"CODEBASE"				{ret(SYM_PROT_CODEBASE);}
"STEP"					{ret(SYM_PROT_STEP);}
"NOHREF"				{ret(SYM_PROT_NOHREF);}
"CHARSET"				{ret(SYM_PROT_CHARSET);}
"FORMNEW"				{ret(SYM_PROT_FORMNEW);}
"FORMENCTYPENEW"		{ret(SYM_PROT_FORMENCTYPENEW);}
"REL"					{ret(SYM_PROT_REL);}
"MIN"					{ret(SYM_PROT_MIN);}
"NAME"					{ret(SYM_PROT_NAME);}
"TYPE"					{ret(SYM_PROT_TYPE);}
"NOSHADE"				{ret(SYM_PROT_NOSHADE);}
"MANIFEST"				{ret(SYM_PROT_MANIFEST);}
"ALIGN"					{ret(SYM_PROT_ALIGN);}
"HEIGHT"				{ret(SYM_PROT_HEIGHT);}
"ACCEPT"				{ret(SYM_PROT_ACCEPT);}
"ENCTYPE"				{ret(SYM_PROT_ENCTYPE);}
"DISABLED"				{ret(SYM_PROT_DISABLED);}
"CONTROLS"				{ret(SYM_PROT_CONTROLS);}
"LONGDESC"				{ret(SYM_PROT_LONGDESC);}
"MARGINWIDTH"			{ret(SYM_PROT_MARGINWIDTH);}
"NORESIZE"				{ret(SYM_PROT_NORESIZE);}
"COLS"					{ret(SYM_PROT_COLS);}
"SIZE"					{ret(SYM_PROT_SIZE);}
"RADIOGROUP"			{ret(SYM_PROT_RADIOGROUP);}
"VALIGN"				{ret(SYM_PROT_VALIGN);}
"CHAROFF"				{ret(SYM_PROT_CHAROFF);}
"LOW"					{ret(SYM_PROT_LOW);}
"START"					{ret(SYM_PROT_START);}
"SUMMARY"				{ret(SYM_PROT_SUMMARY);}
"OPTIMUM"				{ret(SYM_PROT_OPTIMUM);}
"ABBR"					{ret(SYM_PROT_ABBR);}
"FORM"					{ret(SYM_PROT_FORM);}
"LABEL"					{ret(SYM_PROT_LABEL);}
"FRAME"					{ret(SYM_PROT_FRAME);}
"ALLOWSCRIPTACCESS"		{ret(SYM_PROT_ALLOWSCRIPTACCESS);}

[[:alnum:]\-_:]*		{ret(SYM_PROT_USERDATA);}
.						{/*eat others*/}
}

<HTML_LEX_PROT_VALUE>{
{SPACE}					/*eat space*/
"\""[^"]*"\""			{BEGIN HTML_LEX_PROT_NAME; yylval->str = yytext+1; yylval->str_len = yyleng - 2; ret(SYM_TEXT);}
"'"[^']*"'"				{BEGIN HTML_LEX_PROT_NAME; yylval->str = yytext+1; yylval->str_len = yyleng - 2; ret(SYM_TEXT);}
"`"[^`]*"`"				{BEGIN HTML_LEX_PROT_NAME; yylval->str = yytext+1; yylval->str_len = yyleng - 2; ret(SYM_TEXT);}
[^[:space:]/>]*			{BEGIN HTML_LEX_PROT_NAME; yylval->str = yytext; yylval->str_len = yyleng; ret(SYM_TEXT);}
[/>]					{BEGIN HTML_LEX_PROT_NAME; unput(yytext[0]);}
.						{/*eat others*/}
}

%%
void *htmlalloc (yy_size_t sz,yyscan_t yyscanner )
{
	return html_malloc(sz);
}

void *htmlrealloc (void *ptr,yy_size_t sz,yyscan_t yyscanner )
{
	return html_realloc(ptr, sz);
}

void htmlfree (void *ptr ,yyscan_t yyscanner )
{
	html_free(ptr);
}

struct yy_buffer_state* html_scan_stream(const char *new_buf, size_t new_buf_len, html_priv_data *priv)
{
	YY_BUFFER_STATE b;
	char *buf;
	yy_size_t n, _yybytes_len;
	char *last_buf = priv->last_string;
	size_t last_buf_len = priv->last_string_len;
	yyscan_t scanner = (yyscan_t)priv->lexer;
    
	_yybytes_len = new_buf_len + last_buf_len;
	n = _yybytes_len + 2;
	buf = (char *)htmlalloc(n, scanner);
	if (!buf)
	{
		YYFPRINTF(stderr, "out of dynamic memory in html_scan_stream()\n");
		return NULL;
	}

	if(last_buf)
		memcpy(buf, last_buf, last_buf_len);
	
	if(new_buf)
		memcpy(buf+last_buf_len, new_buf, new_buf_len);

	buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;
	YYFPRINTF(stderr, "[HTML]: %s\n", buf);

	b = html_scan_buffer(buf, n, scanner);
	if ( ! b )
	{
		YYFPRINTF(stderr, "bad buffer in html_scan_bytes()\n");
		htmlfree(buf, scanner);
		return NULL;
	}

	b->yy_is_our_buffer = 1;

	if(priv->last_string)
	{
		htmlfree(priv->last_string, scanner);
		priv->last_string = NULL;
		priv->last_string_len = 0;
	}
	return b;
}

static int save_stream_context(yyscan_t yyscanner, int from_default)
{
	struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	html_priv_data *priv = (html_priv_data*)htmlget_extra(yyg);
	int len = 0;
	if(!priv || priv->last_frag)
	{
		YYFPRINTF(stderr, "No need to save stream context, priv: %p, last_frag: %d\n",
			priv, priv?priv->last_frag:0);
		return 0;
	}

	if(priv->last_string)
	{
		htmlfree(priv->last_string, yyg);
		priv->last_string = NULL;
		priv->last_string_len = 0;
	}

	len = from_default?yyleng-1:yyleng;
	if(!len)
	{
		YYFPRINTF(stderr, "Exit save stream context for ZERO length yytext\n");
		return 0;
	}

	priv->last_string = htmlalloc(len, yyg);
	if(!priv->last_string)
	{
		YYFPRINTF(stderr, "out of memory while saving context\n");
		return 0;
	}
	memcpy(priv->last_string, yytext, len);
	priv->last_string_len = len;
	YYFPRINTF(stderr, "Save stream context, string: %s, len: %d\n", yytext, len);
	return 1;
}

void html_lex_fast_init(yyscan_t yyscanner)
{
	struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	if(!yyg)
		return;
	memset(yyg, 0, sizeof(*yyg));
	yy_init_globals(yyscanner);
}
