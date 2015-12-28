#ifndef HTML_H
#define HTML_H 1

#define HTML_NO_MEM	2

struct html_node;
struct html_node_prot;
struct yy_buffer_state;

#ifndef HTML_DEBUG
struct stream_refer_;
struct dp_virtual_host;
struct task_ctx_;
#endif

#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "html_parser.h"

#define YYSTYPE HTMLSTYPE

#define READ_BUF_LEN	2048
typedef struct html_priv_data
{
	STAILQ_HEAD(__html_root, html_node) html_root;
	SLIST_HEAD(__free_node, html_node) free_node;
	SLIST_HEAD(__free_prot, html_node_prot) free_prot;

	void *parser;
	void *lexer;

	char *last_string;
	int last_string_len;
	unsigned char last_frag;
	unsigned char clean;
	unsigned char copy;
	unsigned char sensitive;

#define HTML_FROM_BUFFER	1
#define HTML_FROM_FILE		2
#define HTML_FROM_STREAM	3
	unsigned char source;
	unsigned char error;
	unsigned short score;

#ifndef HTML_DEBUG
	struct stream_refer_ *in_stream;
	struct dp_virtual_host *virtual_host;
	char *full_uri;
	struct task_ctx_ *task_ctx;
	int pf_id;
	unsigned int virtual_host_id;
	char uri_md5_32bit[9];
	char req_dynamic_page;
#endif
}html_priv_data;

/*for html tag node*/
typedef struct html_node_prot
{
	enum htmltokentype type;
	char *value;
	STAILQ_ENTRY(html_node_prot) next;
	SLIST_ENTRY(html_node_prot) free_list;
}html_node_prot_t;

typedef struct html_node
{
	enum htmltokentype type;
	char *text;						/*for text between two tags*/
	STAILQ_HEAD(__prot, html_node_prot) prot;	/*for tag*/
	STAILQ_HEAD(__child, html_node) child;
	STAILQ_ENTRY(html_node) sib;
	SLIST_ENTRY(html_node) free_list;
}html_node_t;

#define IS_CLOSING_NODE(node)			\
(										\
	node->type!=SYM_TAG_AREA		&&	\
	noode->type!=SYM_TAG_BASE 		&&	\
	node->type!=SYM_TAG_BASEFONT	&&	\
	node->type!=SYM_TAG_BR			&&	\
	node->type!=SYM_TAG_COL			&&	\
	node->type!=SYM_TAG_FRAME		&&	\
	node->type!=SYM_TAG_IMG			&&	\
	node->type!=SYM_TAG_INPUT		&&	\
	node->type!=SYM_TAG_LINK		&&	\
	node->type!=SYM_TAG_META		&&	\
	node->type!=SYM_TAG_PARAM			\
)

#define IS_DETECT_NODE(type)			\
(										\
	type==SYM_TAG_APPLET			||	\
	type==SYM_TAG_FRAME				||	\
	type==SYM_TAG_IFRAME			||	\
	type==SYM_TAG_LINK				||	\
	type==SYM_TAG_OBJECT			||	\
	type==SYM_TAG_SCRIPT				\
)

#define IS_TEXT_NODE(node) (node->type==SYM_TEXT)
#define IS_TAG_NODE(node) (!IS_TEXT_NODE(node))

#define JAVASCRIPT_FLAG "javascript:"
#define XSS_CHECK(priv) (((html_priv_data*)priv)->source==HTML_FROM_BUFFER)
#define FILE_CHECK(priv) (((html_priv_data*)priv)->source==HTML_FROM_FILE)
#define HTTP_CHECK(priv) (((html_priv_data*)priv)->source==HTML_FROM_STREAM)
#define TARGET_CHECK(priv) (((html_priv_data*)priv)->source)

extern struct yy_buffer_state* html_scan_stream(const char *new_buf, size_t new_buf_len, html_priv_data *priv);
extern int htmlerror(void *this_priv, const char *format,...);
extern void html_lex_fast_init(void* yyscanner);

extern int debug_strmengine_decoder_html_basic;
extern int strm_debug(int flag, const char *format,...);

#ifdef YYFPRINTF
#undef YYFPRINTF
#endif

#define YYFPRINTF(fp,para...) strm_debug(debug_strmengine_decoder_html_basic,para)

static inline void ADD_ERROR(html_priv_data *priv)
{
	priv->error++;
}

static inline unsigned short GET_ERROR(html_priv_data *priv)
{
	return priv->error;
}

static inline void INIT_ERROR(html_priv_data *priv)
{
	priv->error = 0;
}

static inline void INIT_SCORE(html_priv_data *priv)
{
	priv->score = 0;
}

static inline void ADD_SCORE(html_priv_data *priv)
{
	priv->score++;
}

static inline unsigned short GET_SCORE(html_priv_data *priv)
{
	return priv->score;
}

#define NO_XSS 2
static inline int FIND_XSS(html_priv_data *priv)
{
	return GET_SCORE(priv) > NO_XSS;
}

#define _token(t) get_token_name(t)
static inline const char *get_token_name(enum htmltokentype id)
{
	if(id < 256)
		return "ANSI_CHAR";

	switch (id)
	{
	case SYM_TEXT:
		return "SYM_TEXT";
	case SYM_TAG_START_FLAG:
		return "SYM_TAG_START_FLAG";
	case SYM_TAG_START_FLAG2:
		return "SYM_TAG_START_FLAG2";
	case SYM_TAG_END_FLAG:
		return "SYM_TAG_END_FLAG";
	case SYM_TAG_END_FLAG2:
		return "SYM_TAG_END_FLAG2";
	case SYM_EQUAL:
		return "=";
	case SYM_TAG_A:
		return "SYM_TAG_A";
	case SYM_TAG_ABBR:
		return "SYM_TAG_ABBR";
	case SYM_TAG_ACRONYM:
		return "SYM_TAG_ACRONYM";
	case SYM_TAG_ADDRESS:
		return "SYM_TAG_ADDRESS";
	case SYM_TAG_APPLET:
		return "SYM_TAG_APPLET";
	case SYM_TAG_AREA:
		return "SYM_TAG_AREA";
	case SYM_TAG_ARTICLE:
		return "SYM_TAG_ARTICLE";
	case SYM_TAG_ASIDE:
		return "SYM_TAG_ASIDE";
	case SYM_TAG_AUDIO:
		return "SYM_TAG_AUDIO";
	case SYM_TAG_B:
		return "SYM_TAG_B";
	case SYM_TAG_BASE:
		return "SYM_TAG_BASE";
	case SYM_TAG_BASEFONT:
		return "SYM_TAG_BASEFONT";
	case SYM_TAG_BDO:
		return "SYM_TAG_BDO";
	case SYM_TAG_BIG:
		return "SYM_TAG_BIG";
	case SYM_TAG_BLOCKQUOTE:
		return "SYM_TAG_BLOCKQUOTE";
	case SYM_TAG_BODY:
		return "SYM_TAG_BODY";
	case SYM_TAG_BR:
		return "SYM_TAG_BR";
	case SYM_TAG_BUTTON:
		return "SYM_TAG_BUTTON";
	case SYM_TAG_CANVAS:
		return "SYM_TAG_CANVAS";
	case SYM_TAG_CAPTION:
		return "SYM_TAG_CAPTION";
	case SYM_TAG_CENTER:
		return "SYM_TAG_CENTER";
	case SYM_TAG_CITE:
		return "SYM_TAG_CITE";
	case SYM_TAG_CODE:
		return "SYM_TAG_CODE";
	case SYM_TAG_COL:
		return "SYM_TAG_COL";
	case SYM_TAG_COLGROUP:
		return "SYM_TAG_COLGROUP";
	case SYM_TAG_COMMAND:
		return "SYM_TAG_COMMAND";
	case SYM_TAG_DATALIST:
		return "SYM_TAG_DATALIST";
	case SYM_TAG_DD:
		return "SYM_TAG_DD";
	case SYM_TAG_DEL:
		return "SYM_TAG_DEL";
	case SYM_TAG_DETAILS:
		return "SYM_TAG_DETAILS";
	case SYM_TAG_DFN:
		return "SYM_TAG_DFN";
	case SYM_TAG_DIR:
		return "SYM_TAG_DIR";
	case SYM_TAG_DIV:
		return "SYM_TAG_DIV";
	case SYM_TAG_DL:
		return "SYM_TAG_DL";
	case SYM_TAG_DT:
		return "SYM_TAG_DT";
	case SYM_TAG_EM:
		return "SYM_TAG_EM";
	case SYM_TAG_EMBED:
		return "SYM_TAG_EMBED";
	case SYM_TAG_FIELDSET:
		return "SYM_TAG_FIELDSET";
	case SYM_TAG_FIGCAPTION:
		return "SYM_TAG_FIGCAPTION";
	case SYM_TAG_FIGURE:
		return "SYM_TAG_FIGURE";
	case SYM_TAG_FONT:
		return "SYM_TAG_FONT";
	case SYM_TAG_FOOTER:
		return "SYM_TAG_FOOTER";
	case SYM_TAG_FORM:
		return "SYM_TAG_FORM";
	case SYM_TAG_FRAME:
		return "SYM_TAG_FRAME";
	case SYM_TAG_FRAMESET:
		return "SYM_TAG_FRAMESET";
	case SYM_TAG_H1:
		return "SYM_TAG_H1";
	case SYM_TAG_H2:
		return "SYM_TAG_H2";
	case SYM_TAG_H3:
		return "SYM_TAG_H3";
	case SYM_TAG_H4:
		return "SYM_TAG_H4";
	case SYM_TAG_H5:
		return "SYM_TAG_H5";
	case SYM_TAG_H6:
		return "SYM_TAG_H6";
	case SYM_TAG_HEAD:
		return "SYM_TAG_HEAD";
	case SYM_TAG_HEADER:
		return "SYM_TAG_HEADER";
	case SYM_TAG_HGROUP:
		return "SYM_TAG_HGROUP";
	case SYM_TAG_HR:
		return "SYM_TAG_HR";
	case SYM_TAG_HTML:
		return "SYM_TAG_HTML";
	case SYM_TAG_I:
		return "SYM_TAG_I";
	case SYM_TAG_IFRAME:
		return "SYM_TAG_IFRAME";
	case SYM_TAG_IMG:
		return "SYM_TAG_IMG";
	case SYM_TAG_INPUT:
		return "SYM_TAG_INPUT";
	case SYM_TAG_INS:
		return "SYM_TAG_INS";
	case SYM_TAG_KEYGEN:
		return "SYM_TAG_KEYGEN";
	case SYM_TAG_KBD:
		return "SYM_TAG_KBD";
	case SYM_TAG_LABEL:
		return "SYM_TAG_LABEL";
	case SYM_TAG_LEGEND:
		return "SYM_TAG_LEGEND";
	case SYM_TAG_LI:
		return "SYM_TAG_LI";
	case SYM_TAG_LINK:
		return "SYM_TAG_LINK";
	case SYM_TAG_MAP:
		return "SYM_TAG_MAP";
	case SYM_TAG_MARK:
		return "SYM_TAG_MARK";
	case SYM_TAG_MENU:
		return "SYM_TAG_MENU";
	case SYM_TAG_META:
		return "SYM_TAG_META";
	case SYM_TAG_METER:
		return "SYM_TAG_METER";
	case SYM_TAG_NAV:
		return "SYM_TAG_NAV";
	case SYM_TAG_NOFRAME:
		return "SYM_TAG_NOFRAME";
	case SYM_TAG_NOSCRIPT:
		return "SYM_TAG_NOSCRIPT";
	case SYM_TAG_OBJECT:
		return "SYM_TAG_OBJECT";
	case SYM_TAG_OL:
		return "SYM_TAG_OL";
	case SYM_TAG_OPTGROUP:
		return "SYM_TAG_OPTGROUP";
	case SYM_TAG_OPTION:
		return "SYM_TAG_OPTION";
	case SYM_TAG_OUTPUT:
		return "SYM_TAG_OUTPUT";
	case SYM_TAG_P:
		return "SYM_TAG_P";
	case SYM_TAG_PARAM:
		return "SYM_TAG_PARAM";
	case SYM_TAG_PRE:
		return "SYM_TAG_PRE";
	case SYM_TAG_PROGRESS:
		return "SYM_TAG_PROGRESS";
	case SYM_TAG_Q:
		return "SYM_TAG_Q";
	case SYM_TAG_RP:
		return "SYM_TAG_RP";
	case SYM_TAG_RT:
		return "SYM_TAG_RT";
	case SYM_TAG_RUBY:
		return "SYM_TAG_RUBY";
	case SYM_TAG_S:
		return "SYM_TAG_S";
	case SYM_TAG_U:
		return "SYM_TAG_U";
	case SYM_TAG_SAMP:
		return "SYM_TAG_SAMP";
	case SYM_TAG_SCRIPT:
		return "SYM_TAG_SCRIPT";
	case SYM_TAG_SECTION:
		return "SYM_TAG_SECTION";
	case SYM_TAG_SELECT:
		return "SYM_TAG_SELECT";
	case SYM_TAG_SMALL:
		return "SYM_TAG_SMALL";
	case SYM_TAG_SOURCE:
		return "SYM_TAG_SOURCE";
	case SYM_TAG_SPAN:
		return "SYM_TAG_SPAN";
	case SYM_TAG_STRIKE:
		return "SYM_TAG_STRIKE";
	case SYM_TAG_STRONG:
		return "SYM_TAG_STRONG";
	case SYM_TAG_STYLE:
		return "SYM_TAG_STYLE";
	case SYM_TAG_SUB:
		return "SYM_TAG_SUB";
	case SYM_TAG_SUMMARY:
		return "SYM_TAG_SUMMARY";
	case SYM_TAG_SUP:
		return "SYM_TAG_SUP";
	case SYM_TAG_TABLE:
		return "SYM_TAG_TABLE";
	case SYM_TAG_TBODY:
		return "SYM_TAG_TBODY";
	case SYM_TAG_TD:
		return "SYM_TAG_TD";
	case SYM_TAG_TH:
		return "SYM_TAG_TH";
	case SYM_TAG_TR:
		return "SYM_TAG_TR";
	case SYM_TAG_TT:
		return "SYM_TAG_TT";
	case SYM_TAG_TEXTAREA:
		return "SYM_TAG_TEXTAREA";
	case SYM_TAG_TFOOT:
		return "SYM_TAG_TFOOT";
	case SYM_TAG_THEAD:
		return "SYM_TAG_THEAD";
	case SYM_TAG_TIME:
		return "SYM_TAG_TIME";
	case SYM_TAG_TITLE:
		return "SYM_TAG_TITLE";
	case SYM_TAG_UL:
		return "SYM_TAG_UL";
	case SYM_TAG_VAR:
		return "SYM_TAG_VAR";
	case SYM_TAG_VIDEO:
		return "SYM_TAG_VIDEO";
	case SYM_EVT_ONAFTERPRINT:
		return "SYM_EVT_ONAFTERPRINT";
	case SYM_EVT_ONBEFOREPRINT:
		return "SYM_EVT_ONBEFOREPRINT";
	case SYM_EVT_ONBEFOREONLOAD:
		return "SYM_EVT_ONBEFOREONLOAD";
	case SYM_EVT_ONBLUR:
		return "SYM_EVT_ONBLUR";
	case SYM_EVT_ONFOCUS:
		return "SYM_EVT_ONFOCUS";
	case SYM_EVT_ONHASCHANGE:
		return "SYM_EVT_ONHASCHANGE";
	case SYM_EVT_ONLOAD:
		return "SYM_EVT_ONLOAD";
	case SYM_EVT_ONMESSAGE:
		return "SYM_EVT_ONMESSAGE";
	case SYM_EVT_ONOFFLINE:
		return "SYM_EVT_ONOFFLINE";
	case SYM_EVT_ONONLINE:
		return "SYM_EVT_ONONLINE";
	case SYM_EVT_ONPAGEHIDE:
		return "SYM_EVT_ONPAGEHIDE";
	case SYM_EVT_ONPAGESHOW:
		return "SYM_EVT_ONPAGESHOW";
	case SYM_EVT_ONPOPSTATE:
		return "SYM_EVT_ONPOPSTATE";
	case SYM_EVT_ONREDO:
		return "SYM_EVT_ONREDO";
	case SYM_EVT_ONRESIZE:
		return "SYM_EVT_ONRESIZE";
	case SYM_EVT_ONSTORAGE:
		return "SYM_EVT_ONSTORAGE";
	case SYM_EVT_ONUNDO:
		return "SYM_EVT_ONUNDO";
	case SYM_EVT_ONUNLOAD:
		return "SYM_EVT_ONUNLOAD";
	case SYM_EVT_ONCHANGE:
		return "SYM_EVT_ONCHANGE";
	case SYM_EVT_ONCONTEXTMENU:
		return "SYM_EVT_ONCONTEXTMENU";
	case SYM_EVT_ONFORMCHANGE:
		return "SYM_EVT_ONFORMCHANGE";
	case SYM_EVT_ONFORMINPUT:
		return "SYM_EVT_ONFORMINPUT";
	case SYM_EVT_ONINPUT:
		return "SYM_EVT_ONINPUT";
	case SYM_EVT_ONINVALID:
		return "SYM_EVT_ONINVALID";
	case SYM_EVT_ONRESET:
		return "SYM_EVT_ONRESET";
	case SYM_EVT_ONSELECT:
		return "SYM_EVT_ONSELECT";
	case SYM_EVT_ONSUBMIT:
		return "SYM_EVT_ONSUBMIT";
	case SYM_EVT_ONKEYDOWN:
		return "SYM_EVT_ONKEYDOWN";
	case SYM_EVT_ONKEYPRESS:
		return "SYM_EVT_ONKEYPRESS";
	case SYM_EVT_ONKEYUP:
		return "SYM_EVT_ONKEYUP";
	case SYM_EVT_ONCLICK:
		return "SYM_EVT_ONCLICK";
	case SYM_EVT_ONDBLCLICK:
		return "SYM_EVT_ONDBLCLICK";
	case SYM_EVT_ONDRAG:
		return "SYM_EVT_ONDRAG";
	case SYM_EVT_ONDRAGEND:
		return "SYM_EVT_ONDRAGEND";
	case SYM_EVT_ONDRAGENTER:
		return "SYM_EVT_ONDRAGENTER";
	case SYM_EVT_ONDRAGLEAVE:
		return "SYM_EVT_ONDRAGLEAVE";
	case SYM_EVT_ONDRAGOVER:
		return "SYM_EVT_ONDRAGOVER";
	case SYM_EVT_ONDRAGSTART:
		return "SYM_EVT_ONDRAGSTART";
	case SYM_EVT_ONDROP:
		return "SYM_EVT_ONDROP";
	case SYM_EVT_ONMOUSEDOWN:
		return "SYM_EVT_ONMOUSEDOWN";
	case SYM_EVT_ONMOUSEMOVE:
		return "SYM_EVT_ONMOUSEMOVE";
	case SYM_EVT_ONMOUSEOUT:
		return "SYM_EVT_ONMOUSEOUT";
	case SYM_EVT_ONMOUSEOVER:
		return "SYM_EVT_ONMOUSEOVER";
	case SYM_EVT_ONMOUSEUP:
		return "SYM_EVT_ONMOUSEUP";
	case SYM_EVT_ONMOUSEWHEEL:
		return "SYM_EVT_ONMOUSEWHEEL";
	case SYM_EVT_ONSCROLL:
		return "SYM_EVT_ONSCROLL";
	case SYM_EVT_ONABORT:
		return "SYM_EVT_ONABORT";
	case SYM_EVT_ONCANPLAY:
		return "SYM_EVT_ONCANPLAY";
	case SYM_EVT_ONCANPLAYTHROUGH:
		return "SYM_EVT_ONCANPLAYTHROUGH";
	case SYM_EVT_ONDURATIONCHANGE:
		return "SYM_EVT_ONDURATIONCHANGE";
	case SYM_EVT_ONEMPTIED:
		return "SYM_EVT_ONEMPTIED";
	case SYM_EVT_ONENDED:
		return "SYM_EVT_ONENDED";
	case SYM_EVT_ONERROR:
		return "SYM_EVT_ONERROR";
	case SYM_EVT_ONLOADEDDATA:
		return "SYM_EVT_ONLOADEDDATA";
	case SYM_EVT_ONLOADEDMETADATA:
		return "SYM_EVT_ONLOADEDMETADATA";
	case SYM_EVT_ONLOADSTART:
		return "SYM_EVT_ONLOADSTART";
	case SYM_EVT_ONPAUSE:
		return "SYM_EVT_ONPAUSE";
	case SYM_EVT_ONPLAY:
		return "SYM_EVT_ONPLAY";
	case SYM_EVT_ONPLAYING:
		return "SYM_EVT_ONPLAYING";
	case SYM_EVT_ONPROGRESS:
		return "SYM_EVT_ONPROGRESS";
	case SYM_EVT_ONRATECHANGE:
		return "SYM_EVT_ONRATECHANGE";
	case SYM_EVT_ONREADYSTATECHANGE:
		return "SYM_EVT_ONREADYSTATECHANGE";
	case SYM_EVT_ONSEEKED:
		return "SYM_EVT_ONSEEKED";
	case SYM_EVT_ONSEEKING:
		return "SYM_EVT_ONSEEKING";
	case SYM_EVT_ONSTALLED:
		return "SYM_EVT_ONSTALLED";
	case SYM_EVT_ONSUSPEND:
		return "SYM_EVT_ONSUSPEND";
	case SYM_EVT_ONTIMEUPDATE:
		return "SYM_EVT_ONTIMEUPDATE";
	case SYM_EVT_ONVOLUMECHANGE:
		return "SYM_EVT_ONVOLUMECHANGE";
	case SYM_EVT_ONWAITING:
		return "SYM_EVT_ONWAITING";
	case SYM_PROT_ACCESSKEY:
		return "SYM_PROT_ACCESSKEY";
	case SYM_PROT_CLASS:
		return "SYM_PROT_CLASS";
	case SYM_PROT_CONTENTEDITABLE:
		return "SYM_PROT_CONTENTEDITABLE";
	case SYM_PROT_CONTEXTMENU:
		return "SYM_PROT_CONTEXTMENU";
	case SYM_PROT_DIR:
		return "SYM_PROT_DIR";
	case SYM_PROT_DRAGGABLE:
		return "SYM_PROT_DRAGGABLE";
	case SYM_PROT_HIDDEN:
		return "SYM_PROT_HIDDEN";
	case SYM_PROT_ID:
		return "SYM_PROT_ID";
	case SYM_PROT_ITEM:
		return "SYM_PROT_ITEM";
	case SYM_PROT_ITEMPROP:
		return "SYM_PROT_ITEMPROP";
	case SYM_PROT_LANG:
		return "SYM_PROT_LANG";
	case SYM_PROT_SPELLCHECK:
		return "SYM_PROT_SPELLCHECK";
	case SYM_PROT_STYLE:
		return "SYM_PROT_STYLE";
	case SYM_PROT_SUBJECT:
		return "SYM_PROT_SUBJECT";
	case SYM_PROT_TABINDEX:
		return "SYM_PROT_TABINDEX";
	case SYM_PROT_TITLE:
		return "SYM_PROT_TITLE";
	case SYM_PROT_USERDATA:
		return "SYM_PROT_USERDATA";
	case SYM_PROT_TEMPLATE:
		return "SYM_PROT_TEMPLATE";
	case SYM_PROT_REGISTRATIONMARK:
		return "SYM_PROT_REGISTRATIONMARK";
	case SYM_PROT_IRRELEVANT:
		return "SYM_PROT_IRRELEVANT";
	case SYM_PROT_OPEN:
		return "SYM_PROT_OPEN";
	case SYM_PROT_DATA:
		return "SYM_PROT_DATA";
	case SYM_PROT_NOWRAP:
		return "SYM_PROT_NOWRAP";
	case SYM_PROT_DATETIME:
		return "SYM_PROT_DATETIME";
	case SYM_PROT_ROWS:
		return "SYM_PROT_ROWS";
	case SYM_PROT_LIST:
		return "SYM_PROT_LIST";
	case SYM_PROT_FORMTARGETNEW:
		return "SYM_PROT_FORMTARGETNEW";
	case SYM_PROT_AUTOFOCUSNEW:
		return "SYM_PROT_AUTOFOCUSNEW";
	case SYM_PROT_ICON:
		return "SYM_PROT_ICON";
	case SYM_PROT_MAXLENGTH:
		return "SYM_PROT_MAXLENGTH";
	case SYM_PROT_WIDTH:
		return "SYM_PROT_WIDTH";
	case SYM_PROT_ARCHIVE:
		return "SYM_PROT_ARCHIVE";
	case SYM_PROT_HREF:
		return "SYM_PROT_HREF";
	case SYM_PROT_PRELOAD:
		return "SYM_PROT_PRELOAD";
	case SYM_PROT_MULTIPLE:
		return "SYM_PROT_MULTIPLE";
	case SYM_PROT_HREFLANG:
		return "SYM_PROT_HREFLANG";
	case SYM_PROT_CELLSPACING:
		return "SYM_PROT_CELLSPACING";
	case SYM_PROT_COLSPAN:
		return "SYM_PROT_COLSPAN";
	case SYM_PROT_ACTION:
		return "SYM_PROT_ACTION";
	case SYM_PROT_CLASSID:
		return "SYM_PROT_CLASSID";
	case SYM_PROT_PATTERN:
		return "SYM_PROT_PATTERN";
	case SYM_PROT_COLOR:
		return "SYM_PROT_COLOR";
	case SYM_PROT_HIGH:
		return "SYM_PROT_HIGH";
	case SYM_PROT_PING:
		return "SYM_PROT_PING";
	case SYM_PROT_ISMAP:
		return "SYM_PROT_ISMAP";
	case SYM_PROT_HTTPEQUIV:
		return "SYM_PROT_HTTPEQUIV";
	case SYM_PROT_HSPACE:
		return "SYM_PROT_HSPACE";
	case SYM_PROT_COMPACT:
		return "SYM_PROT_COMPACT";
	case SYM_PROT_LANGUAGE:
		return "SYM_PROT_LANGUAGE";
	case SYM_PROT_REQUIRED:
		return "SYM_PROT_REQUIRED";
	case SYM_PROT_SPAN:
		return "SYM_PROT_SPAN";
	case SYM_PROT_FORMACTIONNEW:
		return "SYM_PROT_FORMACTIONNEW";
	case SYM_PROT_RULES:
		return "SYM_PROT_RULES";
	case SYM_PROT_AXIS:
		return "SYM_PROT_AXIS";
	case SYM_PROT_METHOD:
		return "SYM_PROT_METHOD";
	case SYM_PROT_BGCOLOR:
		return "SYM_PROT_BGCOLOR";
	case SYM_PROT_SHAPE:
		return "SYM_PROT_SHAPE";
	case SYM_PROT_USEMAP:
		return "SYM_PROT_USEMAP";
	case SYM_PROT_FOR:
		return "SYM_PROT_FOR";
	case SYM_PROT_SCOPED:
		return "SYM_PROT_SCOPED";
	case SYM_PROT_FORMNOVALIDATENEW:
		return "SYM_PROT_FORMNOVALIDATENEW";
	case SYM_PROT_CONTENT:
		return "SYM_PROT_CONTENT";
	case SYM_PROT_INPUTMODE:
		return "SYM_PROT_INPUTMODE";
	case SYM_PROT_CITE:
		return "SYM_PROT_CITE";
	case SYM_PROT_VSPACE:
		return "SYM_PROT_VSPACE";
	case SYM_PROT_XMLNS:
		return "SYM_PROT_XMLNS";
	case SYM_PROT_CODETYPE:
		return "SYM_PROT_CODETYPE";
	case SYM_PROT_TARGET:
		return "SYM_PROT_TARGET";
	case SYM_PROT_VALUE:
		return "SYM_PROT_VALUE";
	case SYM_PROT_AUTOFOCUS:
		return "SYM_PROT_AUTOFOCUS";
	case SYM_PROT_MEDIA:
		return "SYM_PROT_MEDIA";
	case SYM_PROT_COORDS:
		return "SYM_PROT_COORDS";
	case SYM_PROT_PROFILE:
		return "SYM_PROT_PROFILE";
	case SYM_PROT_HEADERS:
		return "SYM_PROT_HEADERS";
	case SYM_PROT_VALUETYPE:
		return "SYM_PROT_VALUETYPE";
	case SYM_PROT_REPLACE:
		return "SYM_PROT_REPLACE";
	case SYM_PROT_MARGINHEIGHT:
		return "SYM_PROT_MARGINHEIGHT";
	case SYM_PROT_BORDER:
		return "SYM_PROT_BORDER";
	case SYM_PROT_FRAMEBORDER:
		return "SYM_PROT_FRAMEBORDER";
	case SYM_PROT_ASYNC:
		return "SYM_PROT_ASYNC";
	case SYM_PROT_FACE:
		return "SYM_PROT_FACE";
	case SYM_PROT_CELLPADDING:
		return "SYM_PROT_CELLPADDING";
	case SYM_PROT_STANDBY:
		return "SYM_PROT_STANDBY";
	case SYM_PROT_ALT:
		return "SYM_PROT_ALT";
	case SYM_PROT_ACCEPTCHARSET:
		return "SYM_PROT_ACCEPTCHARSET";
	case SYM_PROT_FORMMETHODNEW:
		return "SYM_PROT_FORMMETHODNEW";
	case SYM_PROT_AUTOPLAY:
		return "SYM_PROT_AUTOPLAY";
	case SYM_PROT_REV:
		return "SYM_PROT_REV";
	case SYM_PROT_LOOP:
		return "SYM_PROT_LOOP";
	case SYM_PROT_CODE:
		return "SYM_PROT_CODE";
	case SYM_PROT_SRC:
		return "SYM_PROT_SRC";
	case SYM_PROT_CHECKED:
		return "SYM_PROT_CHECKED";
	case SYM_PROT_SCROLLING:
		return "SYM_PROT_SCROLLING";
	case SYM_PROT_SCOPE:
		return "SYM_PROT_SCOPE";
	case SYM_PROT_DEFER:
		return "SYM_PROT_DEFER";
	case SYM_PROT_XMLSPACE:
		return "SYM_PROT_XMLSPACE";
	case SYM_PROT_CHALLENGE:
		return "SYM_PROT_CHALLENGE";
	case SYM_PROT_SCHEME:
		return "SYM_PROT_SCHEME";
	case SYM_PROT_DECLARE:
		return "SYM_PROT_DECLARE";
	case SYM_PROT_CHAR:
		return "SYM_PROT_CHAR";
	case SYM_PROT_READONLY:
		return "SYM_PROT_READONLY";
	case SYM_PROT_XMLLANG:
		return "SYM_PROT_XMLLANG";
	case SYM_PROT_MAX:
		return "SYM_PROT_MAX";
	case SYM_PROT_ROWSPAN:
		return "SYM_PROT_ROWSPAN";
	case SYM_PROT_KEYTYPE:
		return "SYM_PROT_KEYTYPE";
	case SYM_PROT_AUTOCOMPLETE:
		return "SYM_PROT_AUTOCOMPLETE";
	case SYM_PROT_SELECTED:
		return "SYM_PROT_SELECTED";
	case SYM_PROT_CODEBASE:
		return "SYM_PROT_CODEBASE";
	case SYM_PROT_STEP:
		return "SYM_PROT_STEP";
	case SYM_PROT_NOHREF:
		return "SYM_PROT_NOHREF";
	case SYM_PROT_CHARSET:
		return "SYM_PROT_CHARSET";
	case SYM_PROT_FORMNEW:
		return "SYM_PROT_FORMNEW";
	case SYM_PROT_FORMENCTYPENEW:
		return "SYM_PROT_FORMENCTYPENEW";
	case SYM_PROT_REL:
		return "SYM_PROT_REL";
	case SYM_PROT_MIN:
		return "SYM_PROT_MIN";
	case SYM_PROT_NAME:
		return "SYM_PROT_NAME";
	case SYM_PROT_TYPE:
		return "SYM_PROT_TYPE";
	case SYM_PROT_NOSHADE:
		return "SYM_PROT_NOSHADE";
	case SYM_PROT_MANIFEST:
		return "SYM_PROT_MANIFEST";
	case SYM_PROT_ALIGN:
		return "SYM_PROT_ALIGN";
	case SYM_PROT_HEIGHT:
		return "SYM_PROT_HEIGHT";
	case SYM_PROT_ACCEPT:
		return "SYM_PROT_ACCEPT";
	case SYM_PROT_ENCTYPE:
		return "SYM_PROT_ENCTYPE";
	case SYM_PROT_DISABLED:
		return "SYM_PROT_DISABLED";
	case SYM_PROT_CONTROLS:
		return "SYM_PROT_CONTROLS";
	case SYM_PROT_LONGDESC:
		return "SYM_PROT_LONGDESC";
	case SYM_PROT_MARGINWIDTH:
		return "SYM_PROT_MARGINWIDTH";
	case SYM_PROT_NORESIZE:
		return "SYM_PROT_NORESIZE";
	case SYM_PROT_COLS:
		return "SYM_PROT_COLS";
	case SYM_PROT_SIZE:
		return "SYM_PROT_SIZE";
	case SYM_PROT_RADIOGROUP:
		return "SYM_PROT_RADIOGROUP";
	case SYM_PROT_VALIGN:
		return "SYM_PROT_VALIGN";
	case SYM_PROT_CHAROFF:
		return "SYM_PROT_CHAROFF";
	case SYM_PROT_LOW:
		return "SYM_PROT_LOW";
	case SYM_PROT_START:
		return "SYM_PROT_START";
	case SYM_PROT_SUMMARY:
		return "SYM_PROT_SUMMARY";
	case SYM_PROT_OPTIMUM:
		return "SYM_PROT_OPTIMUM";
	case SYM_PROT_ABBR:
		return "SYM_PROT_ABBR";
	case SYM_PROT_FORM:
		return "SYM_PROT_FORM";
	case SYM_PROT_LABEL:
		return "SYM_PROT_LABEL";
	case SYM_PROT_FRAME:
		return "SYM_PROT_FRAME";
	default:
		return "##UNKNOWN TOKEN##";
	}
}

static inline const char *get_token_text(enum htmltokentype id)
{
	switch(id)
	{
	case SYM_TAG_A:
		return "A";
	case SYM_TAG_ABBR:
		return "ABBR";
	case SYM_TAG_ACRONYM:
		return "ACRONYM";
	case SYM_TAG_ADDRESS:
		return "ADDRESS";
	case SYM_TAG_APPLET:
		return "APPLET";
	case SYM_TAG_AREA:
		return "AREA";
	case SYM_TAG_ARTICLE:
		return "ARTICLE";
	case SYM_TAG_ASIDE:
		return "ASIDE";
	case SYM_TAG_AUDIO:
		return "AUDIO";
	case SYM_TAG_B:
		return "B";
	case SYM_TAG_BASE:
		return "BASE";
	case SYM_TAG_BASEFONT:
		return "BASEFONT";
	case SYM_TAG_BDO:
		return "BDO";
	case SYM_TAG_BIG:
		return "BIG";
	case SYM_TAG_BLOCKQUOTE:
		return "BLOCKQUOTE";
	case SYM_TAG_BODY:
		return "BODY";
	case SYM_TAG_BR:
		return "BR";
	case SYM_TAG_BUTTON:
		return "BUTTON";
	case SYM_TAG_CANVAS:
		return "CANVAS";
	case SYM_TAG_CAPTION:
		return "CAPTION";
	case SYM_TAG_CENTER:
		return "CENTER";
	case SYM_TAG_CITE:
		return "CITE";
	case SYM_TAG_CODE:
		return "CODE";
	case SYM_TAG_COL:
		return "COL";
	case SYM_TAG_COLGROUP:
		return "COLGROUP";
	case SYM_TAG_COMMAND:
		return "COMMAND";
	case SYM_TAG_DATALIST:
		return "DATALIST";
	case SYM_TAG_DD:
		return "DD";
	case SYM_TAG_DEL:
		return "DEL";
	case SYM_TAG_DETAILS:
		return "DETAILS";
	case SYM_TAG_DFN:
		return "DFN";
	case SYM_TAG_DIR:
		return "DIR";
	case SYM_TAG_DIV:
		return "DIV";
	case SYM_TAG_DL:
		return "DL";
	case SYM_TAG_DT:
		return "DT";
	case SYM_TAG_EM:
		return "EM";
	case SYM_TAG_EMBED:
		return "EMBED";
	case SYM_TAG_FIELDSET:
		return "FIELDSET";
	case SYM_TAG_FIGCAPTION:
		return "FIGCAPTION";
	case SYM_TAG_FIGURE:
		return "FIGURE";
	case SYM_TAG_FONT:
		return "FONT";
	case SYM_TAG_FOOTER:
		return "FOOTER";
	case SYM_TAG_FORM:
		return "FORM";
	case SYM_TAG_FRAME:
		return "FRAME";
	case SYM_TAG_FRAMESET:
		return "FRAMESET";
	case SYM_TAG_H1:
		return "H1";
	case SYM_TAG_H2:
		return "H2";
	case SYM_TAG_H3:
		return "H3";
	case SYM_TAG_H4:
		return "H4";
	case SYM_TAG_H5:
		return "H5";
	case SYM_TAG_H6:
		return "H6";
	case SYM_TAG_HEAD:
		return "HEAD";
	case SYM_TAG_HEADER:
		return "HEADER";
	case SYM_TAG_HGROUP:
		return "HGROUP";
	case SYM_TAG_HR:
		return "HR";
	case SYM_TAG_HTML:
		return "HTML";
	case SYM_TAG_I:
		return "I";
	case SYM_TAG_IFRAME:
		return "IFRAME";
	case SYM_TAG_IMG:
		return "IMG";
	case SYM_TAG_INPUT:
		return "INPUT";
	case SYM_TAG_INS:
		return "INS";
	case SYM_TAG_KEYGEN:
		return "KEYGEN";
	case SYM_TAG_KBD:
		return "KBD";
	case SYM_TAG_LABEL:
		return "LABEL";
	case SYM_TAG_LEGEND:
		return "LEGEND";
	case SYM_TAG_LI:
		return "LI";
	case SYM_TAG_LINK:
		return "LINK";
	case SYM_TAG_MAP:
		return "MAP";
	case SYM_TAG_MARK:
		return "MARK";
	case SYM_TAG_MENU:
		return "MENU";
	case SYM_TAG_META:
		return "META";
	case SYM_TAG_METER:
		return "METER";
	case SYM_TAG_NAV:
		return "NAV";
	case SYM_TAG_NOFRAME:
		return "NOFRAME";
	case SYM_TAG_NOSCRIPT:
		return "NOSCRIPT";
	case SYM_TAG_OBJECT:
		return "OBJECT";
	case SYM_TAG_OL:
		return "OL";
	case SYM_TAG_OPTGROUP:
		return "OPTGROUP";
	case SYM_TAG_OPTION:
		return "OPTION";
	case SYM_TAG_OUTPUT:
		return "OUTPUT";
	case SYM_TAG_P:
		return "P";
	case SYM_TAG_PARAM:
		return "PARAM";
	case SYM_TAG_PRE:
		return "PRE";
	case SYM_TAG_PROGRESS:
		return "PROGRESS";
	case SYM_TAG_Q:
		return "Q";
	case SYM_TAG_RP:
		return "RP";
	case SYM_TAG_RT:
		return "RT";
	case SYM_TAG_RUBY:
		return "RUBY";
	case SYM_TAG_S:
		return "S";
	case SYM_TAG_U:
		return "U";
	case SYM_TAG_SAMP:
		return "SAMP";
	case SYM_TAG_SCRIPT:
		return "SCRIPT";
	case SYM_TAG_SECTION:
		return "SECTION";
	case SYM_TAG_SELECT:
		return "SELECT";
	case SYM_TAG_SMALL:
		return "SMALL";
	case SYM_TAG_SOURCE:
		return "SOURCE";
	case SYM_TAG_SPAN:
		return "SPAN";
	case SYM_TAG_STRIKE:
		return "STRIKE";
	case SYM_TAG_STRONG:
		return "STRONG";
	case SYM_TAG_STYLE:
		return "STYLE";
	case SYM_TAG_SUB:
		return "SUB";
	case SYM_TAG_SUMMARY:
		return "SUMMARY";
	case SYM_TAG_SUP:
		return "SUP";
	case SYM_TAG_TABLE:
		return "TABLE";
	case SYM_TAG_TBODY:
		return "TBODY";
	case SYM_TAG_TD:
		return "TD";
	case SYM_TAG_TH:
		return "TH";
	case SYM_TAG_TR:
		return "TR";
	case SYM_TAG_TT:
		return "TT";
	case SYM_TAG_TEXTAREA:
		return "TEXTAREA";
	case SYM_TAG_TFOOT:
		return "TFOOT";
	case SYM_TAG_THEAD:
		return "THEAD";
	case SYM_TAG_TIME:
		return "TIME";
	case SYM_TAG_TITLE:
		return "TITLE";
	case SYM_TAG_UL:
		return "UL";
	case SYM_TAG_VAR:
		return "VAR";
	case SYM_TAG_VIDEO:
		return "VIDEO";
	case SYM_EVT_ONAFTERPRINT:
		return "ONAFTERPRINT";
	case SYM_EVT_ONBEFOREPRINT:
		return "ONBEFOREPRINT";
	case SYM_EVT_ONBEFOREONLOAD:
		return "ONBEFOREONLOAD";
	case SYM_EVT_ONBLUR:
		return "ONBLUR";
	case SYM_EVT_ONFOCUS:
		return "ONFOCUS";
	case SYM_EVT_ONHASCHANGE:
		return "ONHASCHANGE";
	case SYM_EVT_ONLOAD:
		return "ONLOAD";
	case SYM_EVT_ONMESSAGE:
		return "ONMESSAGE";
	case SYM_EVT_ONOFFLINE:
		return "ONOFFLINE";
	case SYM_EVT_ONONLINE:
		return "ONONLINE";
	case SYM_EVT_ONPAGEHIDE:
		return "ONPAGEHIDE";
	case SYM_EVT_ONPAGESHOW:
		return "ONPAGESHOW";
	case SYM_EVT_ONPOPSTATE:
		return "ONPOPSTATE";
	case SYM_EVT_ONREDO:
		return "ONREDO";
	case SYM_EVT_ONRESIZE:
		return "ONRESIZE";
	case SYM_EVT_ONSTORAGE:
		return "ONSTORAGE";
	case SYM_EVT_ONUNDO:
		return "ONUNDO";
	case SYM_EVT_ONUNLOAD:
		return "ONUNLOAD";
	case SYM_EVT_ONCHANGE:
		return "ONCHANGE";
	case SYM_EVT_ONCONTEXTMENU:
		return "ONCONTEXTMENU";
	case SYM_EVT_ONFORMCHANGE:
		return "ONFORMCHANGE";
	case SYM_EVT_ONFORMINPUT:
		return "ONFORMINPUT";
	case SYM_EVT_ONINPUT:
		return "ONINPUT";
	case SYM_EVT_ONINVALID:
		return "ONINVALID";
	case SYM_EVT_ONRESET:
		return "ONRESET";
	case SYM_EVT_ONSELECT:
		return "ONSELECT";
	case SYM_EVT_ONSUBMIT:
		return "ONSUBMIT";
	case SYM_EVT_ONKEYDOWN:
		return "ONKEYDOWN";
	case SYM_EVT_ONKEYPRESS:
		return "ONKEYPRESS";
	case SYM_EVT_ONKEYUP:
		return "ONKEYUP";
	case SYM_EVT_ONCLICK:
		return "ONCLICK";
	case SYM_EVT_ONDBLCLICK:
		return "ONDBLCLICK";
	case SYM_EVT_ONDRAG:
		return "ONDRAG";
	case SYM_EVT_ONDRAGEND:
		return "ONDRAGEND";
	case SYM_EVT_ONDRAGENTER:
		return "ONDRAGENTER";
	case SYM_EVT_ONDRAGLEAVE:
		return "ONDRAGLEAVE";
	case SYM_EVT_ONDRAGOVER:
		return "ONDRAGOVER";
	case SYM_EVT_ONDRAGSTART:
		return "ONDRAGSTART";
	case SYM_EVT_ONDROP:
		return "ONDROP";
	case SYM_EVT_ONMOUSEDOWN:
		return "ONMOUSEDOWN";
	case SYM_EVT_ONMOUSEMOVE:
		return "ONMOUSEMOVE";
	case SYM_EVT_ONMOUSEOUT:
		return "ONMOUSEOUT";
	case SYM_EVT_ONMOUSEOVER:
		return "ONMOUSEOVER";
	case SYM_EVT_ONMOUSEUP:
		return "ONMOUSEUP";
	case SYM_EVT_ONMOUSEWHEEL:
		return "ONMOUSEWHEEL";
	case SYM_EVT_ONSCROLL:
		return "ONSCROLL";
	case SYM_EVT_ONABORT:
		return "ONABORT";
	case SYM_EVT_ONCANPLAY:
		return "ONCANPLAY";
	case SYM_EVT_ONCANPLAYTHROUGH:
		return "ONCANPLAYTHROUGH";
	case SYM_EVT_ONDURATIONCHANGE:
		return "ONDURATIONCHANGE";
	case SYM_EVT_ONEMPTIED:
		return "ONEMPTIED";
	case SYM_EVT_ONENDED:
		return "ONENDED";
	case SYM_EVT_ONERROR:
		return "ONERROR";
	case SYM_EVT_ONLOADEDDATA:
		return "ONLOADEDDATA";
	case SYM_EVT_ONLOADEDMETADATA:
		return "ONLOADEDMETADATA";
	case SYM_EVT_ONLOADSTART:
		return "ONLOADSTART";
	case SYM_EVT_ONPAUSE:
		return "ONPAUSE";
	case SYM_EVT_ONPLAY:
		return "ONPLAY";
	case SYM_EVT_ONPLAYING:
		return "ONPLAYING";
	case SYM_EVT_ONPROGRESS:
		return "ONPROGRESS";
	case SYM_EVT_ONRATECHANGE:
		return "ONRATECHANGE";
	case SYM_EVT_ONREADYSTATECHANGE:
		return "ONREADYSTATECHANGE";
	case SYM_EVT_ONSEEKED:
		return "ONSEEKED";
	case SYM_EVT_ONSEEKING:
		return "ONSEEKING";
	case SYM_EVT_ONSTALLED:
		return "ONSTALLED";
	case SYM_EVT_ONSUSPEND:
		return "ONSUSPEND";
	case SYM_EVT_ONTIMEUPDATE:
		return "ONTIMEUPDATE";
	case SYM_EVT_ONVOLUMECHANGE:
		return "ONVOLUMECHANGE";
	case SYM_EVT_ONWAITING:
		return "ONWAITING";
	case SYM_PROT_ACCESSKEY:
		return "ACCESSKEY";
	case SYM_PROT_CLASS:
		return "CLASS";
	case SYM_PROT_CONTENTEDITABLE:
		return "CONTENTEDITABLE";
	case SYM_PROT_CONTEXTMENU:
		return "CONTEXTMENU";
	case SYM_PROT_DIR:
		return "DIR";
	case SYM_PROT_DRAGGABLE:
		return "DRAGGABLE";
	case SYM_PROT_HIDDEN:
		return "HIDDEN";
	case SYM_PROT_ID:
		return "ID";
	case SYM_PROT_ITEM:
		return "ITEM";
	case SYM_PROT_ITEMPROP:
		return "ITEMPROP";
	case SYM_PROT_LANG:
		return "LANG";
	case SYM_PROT_SPELLCHECK:
		return "SPELLCHECK";
	case SYM_PROT_STYLE:
		return "STYLE";
	case SYM_PROT_SUBJECT:
		return "SUBJECT";
	case SYM_PROT_TABINDEX:
		return "TABINDEX";
	case SYM_PROT_TITLE:
		return "TITLE";
	case SYM_PROT_USERDATA:
		return "USERDATA";
	case SYM_PROT_TEMPLATE:
		return "TEMPLATE";
	case SYM_PROT_REGISTRATIONMARK:
		return "REGISTRATIONMARK";
	case SYM_PROT_IRRELEVANT:
		return "IRRELEVANT";
	case SYM_PROT_OPEN:
		return "OPEN";
	case SYM_PROT_DATA:
		return "DATA";
	case SYM_PROT_NOWRAP:
		return "NOWRAP";
	case SYM_PROT_DATETIME:
		return "DATETIME";
	case SYM_PROT_ROWS:
		return "ROWS";
	case SYM_PROT_LIST:
		return "LIST";
	case SYM_PROT_FORMTARGETNEW:
		return "FORMTARGETNEW";
	case SYM_PROT_AUTOFOCUSNEW:
		return "AUTOFOCUSNEW";
	case SYM_PROT_ICON:
		return "ICON";
	case SYM_PROT_MAXLENGTH:
		return "MAXLENGTH";
	case SYM_PROT_WIDTH:
		return "WIDTH";
	case SYM_PROT_ARCHIVE:
		return "ARCHIVE";
	case SYM_PROT_HREF:
		return "HREF";
	case SYM_PROT_PRELOAD:
		return "PRELOAD";
	case SYM_PROT_MULTIPLE:
		return "MULTIPLE";
	case SYM_PROT_HREFLANG:
		return "HREFLANG";
	case SYM_PROT_CELLSPACING:
		return "CELLSPACING";
	case SYM_PROT_COLSPAN:
		return "COLSPAN";
	case SYM_PROT_ACTION:
		return "ACTION";
	case SYM_PROT_CLASSID:
		return "CLASSID";
	case SYM_PROT_PATTERN:
		return "PATTERN";
	case SYM_PROT_COLOR:
		return "COLOR";
	case SYM_PROT_HIGH:
		return "HIGH";
	case SYM_PROT_PING:
		return "PING";
	case SYM_PROT_ISMAP:
		return "ISMAP";
	case SYM_PROT_HTTPEQUIV:
		return "HTTPEQUIV";
	case SYM_PROT_HSPACE:
		return "HSPACE";
	case SYM_PROT_COMPACT:
		return "COMPACT";
	case SYM_PROT_LANGUAGE:
		return "LANGUAGE";
	case SYM_PROT_REQUIRED:
		return "REQUIRED";
	case SYM_PROT_SPAN:
		return "SPAN";
	case SYM_PROT_FORMACTIONNEW:
		return "FORMACTIONNEW";
	case SYM_PROT_RULES:
		return "RULES";
	case SYM_PROT_AXIS:
		return "AXIS";
	case SYM_PROT_METHOD:
		return "METHOD";
	case SYM_PROT_BGCOLOR:
		return "BGCOLOR";
	case SYM_PROT_SHAPE:
		return "SHAPE";
	case SYM_PROT_USEMAP:
		return "USEMAP";
	case SYM_PROT_FOR:
		return "FOR";
	case SYM_PROT_SCOPED:
		return "SCOPED";
	case SYM_PROT_FORMNOVALIDATENEW:
		return "FORMNOVALIDATENEW";
	case SYM_PROT_CONTENT:
		return "CONTENT";
	case SYM_PROT_INPUTMODE:
		return "INPUTMODE";
	case SYM_PROT_CITE:
		return "CITE";
	case SYM_PROT_VSPACE:
		return "VSPACE";
	case SYM_PROT_XMLNS:
		return "XMLNS";
	case SYM_PROT_CODETYPE:
		return "CODETYPE";
	case SYM_PROT_TARGET:
		return "TARGET";
	case SYM_PROT_VALUE:
		return "VALUE";
	case SYM_PROT_AUTOFOCUS:
		return "AUTOFOCUS";
	case SYM_PROT_MEDIA:
		return "MEDIA";
	case SYM_PROT_COORDS:
		return "COORDS";
	case SYM_PROT_PROFILE:
		return "PROFILE";
	case SYM_PROT_HEADERS:
		return "HEADERS";
	case SYM_PROT_VALUETYPE:
		return "VALUETYPE";
	case SYM_PROT_REPLACE:
		return "REPLACE";
	case SYM_PROT_MARGINHEIGHT:
		return "MARGINHEIGHT";
	case SYM_PROT_BORDER:
		return "BORDER";
	case SYM_PROT_FRAMEBORDER:
		return "FRAMEBORDER";
	case SYM_PROT_ASYNC:
		return "ASYNC";
	case SYM_PROT_FACE:
		return "FACE";
	case SYM_PROT_CELLPADDING:
		return "CELLPADDING";
	case SYM_PROT_STANDBY:
		return "STANDBY";
	case SYM_PROT_ALT:
		return "ALT";
	case SYM_PROT_ACCEPTCHARSET:
		return "ACCEPTCHARSET";
	case SYM_PROT_FORMMETHODNEW:
		return "FORMMETHODNEW";
	case SYM_PROT_AUTOPLAY:
		return "AUTOPLAY";
	case SYM_PROT_REV:
		return "REV";
	case SYM_PROT_LOOP:
		return "LOOP";
	case SYM_PROT_CODE:
		return "CODE";
	case SYM_PROT_SRC:
		return "SRC";
	case SYM_PROT_CHECKED:
		return "CHECKED";
	case SYM_PROT_SCROLLING:
		return "SCROLLING";
	case SYM_PROT_SCOPE:
		return "SCOPE";
	case SYM_PROT_DEFER:
		return "DEFER";
	case SYM_PROT_XMLSPACE:
		return "XML:SPACE";
	case SYM_PROT_CHALLENGE:
		return "CHALLENGE";
	case SYM_PROT_SCHEME:
		return "SCHEME";
	case SYM_PROT_DECLARE:
		return "DECLARE";
	case SYM_PROT_CHAR:
		return "CHAR";
	case SYM_PROT_READONLY:
		return "READONLY";
	case SYM_PROT_XMLLANG:
		return "XML:LANG";
	case SYM_PROT_MAX:
		return "MAX";
	case SYM_PROT_ROWSPAN:
		return "ROWSPAN";
	case SYM_PROT_KEYTYPE:
		return "KEYTYPE";
	case SYM_PROT_AUTOCOMPLETE:
		return "AUTOCOMPLETE";
	case SYM_PROT_SELECTED:
		return "SELECTED";
	case SYM_PROT_CODEBASE:
		return "CODEBASE";
	case SYM_PROT_STEP:
		return "STEP";
	case SYM_PROT_NOHREF:
		return "NOHREF";
	case SYM_PROT_CHARSET:
		return "CHARSET";
	case SYM_PROT_FORMNEW:
		return "FORMNEW";
	case SYM_PROT_FORMENCTYPENEW:
		return "FORMENCTYPENEW";
	case SYM_PROT_REL:
		return "REL";
	case SYM_PROT_MIN:
		return "MIN";
	case SYM_PROT_NAME:
		return "NAME";
	case SYM_PROT_TYPE:
		return "TYPE";
	case SYM_PROT_NOSHADE:
		return "NOSHADE";
	case SYM_PROT_MANIFEST:
		return "MANIFEST";
	case SYM_PROT_ALIGN:
		return "ALIGN";
	case SYM_PROT_HEIGHT:
		return "HEIGHT";
	case SYM_PROT_ACCEPT:
		return "ACCEPT";
	case SYM_PROT_ENCTYPE:
		return "ENCTYPE";
	case SYM_PROT_DISABLED:
		return "DISABLED";
	case SYM_PROT_CONTROLS:
		return "CONTROLS";
	case SYM_PROT_LONGDESC:
		return "LONGDESC";
	case SYM_PROT_MARGINWIDTH:
		return "MARGINWIDTH";
	case SYM_PROT_NORESIZE:
		return "NORESIZE";
	case SYM_PROT_COLS:
		return "COLS";
	case SYM_PROT_SIZE:
		return "SIZE";
	case SYM_PROT_RADIOGROUP:
		return "RADIOGROUP";
	case SYM_PROT_VALIGN:
		return "VALIGN";
	case SYM_PROT_CHAROFF:
		return "CHAROFF";
	case SYM_PROT_LOW:
		return "LOW";
	case SYM_PROT_START:
		return "START";
	case SYM_PROT_SUMMARY:
		return "SUMMARY";
	case SYM_PROT_OPTIMUM:
		return "OPTIMUM";
	case SYM_PROT_ABBR:
		return "ABBR";
	case SYM_PROT_FORM:
		return "FORM";
	case SYM_PROT_LABEL:
		return "LABEL";
	case SYM_PROT_FRAME:
		return "FRAME";
	default:
		return "##UNKNOWN TOKEN##";
	}
}
#endif