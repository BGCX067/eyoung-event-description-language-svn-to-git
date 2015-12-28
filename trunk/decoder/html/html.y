%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "queue.h"
#include "html.h"
#include "html_parser.h"
#include "html_mem.h"
#include "html_lex.h"
#include "libhtml.h"

#define YYINITDEPTH 32
#define YYMAXDEPTH 256

static char *html_value_string_decode(char *value);
static int do_html_check(html_priv_data *priv, enum htmltokentype type, html_node_prot_t *prots);
#ifndef HTML_DEBUG
	#include "dp_common.h"
	#include "stream.h"
	#include "stream_mem.h"
	#include "strm_task_ctx.h"
	#include "stream_decoder.h"
	#include "stream_types.h"
	#include "strmengine_debug.h"
	#include "common.h"
	#include "others.h"
	#include "../http/http_decoder.h"
	#include "idp_global.h"
	#include "dp_idp_http.h"
	#include "idp_profile.h"
	#include "idp_syslog.h"
#endif

#ifdef yydebug
	#undef yydebug
	#define yydebug debug_strmengine_decoder_html_basic
#endif

static inline int nocopy_break(void *p)
{
	html_priv_data* priv = (html_priv_data*)p;
	switch(priv->source)
	{
	case HTML_FROM_BUFFER:
		return 1;
	case HTML_FROM_STREAM:
		if(!priv->copy)
			return 1;
		return 0;
	default:
		return 0;
	}
}

#define NOCOPY_BREAK(priv)										\
{																\
	if(nocopy_break(priv))										\
		break;													\
}

#define SET_COPY_FLAG(priv,flag)								\
{																\
	if(((html_priv_data*)priv)->source==HTML_FROM_STREAM)		\
		((html_priv_data*)priv)->copy=flag;						\
}
%}

/*TOKEN declaration*/
%token SYM_LEX_CONTINUE					/*	<lex-continue>	*/
%token SYM_TEXT							/*	text			*/
%token SYM_TAG_START_FLAG				/*	<				*/
%token SYM_TAG_START_FLAG2				/*	</				*/
%token SYM_TAG_END_FLAG					/*	>				*/
%token SYM_TAG_END_FLAG2				/*	/>				*/
%token SYM_EQUAL						/*	=				*/

/*for html tag*/
%token SYM_TAG_A						/*	a				*/
%token SYM_TAG_ABBR						/*	abbr			*/
%token SYM_TAG_ACRONYM					/*	acronym			*/
%token SYM_TAG_ADDRESS					/*	address			*/
%token SYM_TAG_APPLET					/*	applet			*/
%token SYM_TAG_AREA						/*	area			*/
%token SYM_TAG_ARTICLE					/*	article			*/
%token SYM_TAG_ASIDE					/*	aside			*/
%token SYM_TAG_AUDIO					/*	audio			*/
%token SYM_TAG_B						/*	b				*/
%token SYM_TAG_BASE						/*	base			*/
%token SYM_TAG_BASEFONT					/*	basefont		*/
%token SYM_TAG_BDO						/*	bdo				*/
%token SYM_TAG_BIG						/*	big				*/
%token SYM_TAG_BLOCKQUOTE				/*	blockquote		*/
%token SYM_TAG_BODY						/*	body			*/
%token SYM_TAG_BR						/*	br				*/
%token SYM_TAG_BUTTON					/*	button			*/
%token SYM_TAG_CANVAS					/*	canvas			*/
%token SYM_TAG_CAPTION					/*	caption			*/
%token SYM_TAG_CENTER					/*	center			*/
%token SYM_TAG_CITE						/*	cite			*/
%token SYM_TAG_CODE						/*	code			*/
%token SYM_TAG_COL						/*	col				*/
%token SYM_TAG_COLGROUP					/*	colgroup		*/
%token SYM_TAG_COMMAND					/*	command			*/
%token SYM_TAG_DATALIST					/*	datalist		*/
%token SYM_TAG_DD						/*	dd				*/
%token SYM_TAG_DEL						/*	del				*/
%token SYM_TAG_DETAILS					/*	details			*/
%token SYM_TAG_DFN						/*	dfn				*/
%token SYM_TAG_DIR						/*	dir				*/
%token SYM_TAG_DIV						/*	div				*/
%token SYM_TAG_DL						/*	dl				*/
%token SYM_TAG_DT						/*	dt				*/
%token SYM_TAG_EM						/*	em				*/
%token SYM_TAG_EMBED					/*	embed			*/
%token SYM_TAG_FIELDSET					/*	fieldset		*/
%token SYM_TAG_FIGCAPTION				/*	figcaption		*/
%token SYM_TAG_FIGURE					/*	figure			*/
%token SYM_TAG_FONT						/*	font			*/
%token SYM_TAG_FOOTER					/*	footer			*/
%token SYM_TAG_FORM						/*	form			*/
%token SYM_TAG_FRAME					/*	frame			*/
%token SYM_TAG_FRAMESET					/*	frameset		*/
%token SYM_TAG_H1						/*	h1				*/
%token SYM_TAG_H2						/*	h2				*/
%token SYM_TAG_H3						/*	h3				*/
%token SYM_TAG_H4						/*	h4				*/
%token SYM_TAG_H5						/*	h5				*/
%token SYM_TAG_H6						/*	h6				*/
%token SYM_TAG_HEAD						/*	head			*/
%token SYM_TAG_HEADER					/*	header			*/
%token SYM_TAG_HGROUP					/*	hgroup			*/
%token SYM_TAG_HR						/*	hr				*/
%token SYM_TAG_HTML						/*	html			*/
%token SYM_TAG_I						/*	i				*/
%token SYM_TAG_IFRAME					/*	iframe			*/
%token SYM_TAG_IMG						/*	img				*/
%token SYM_TAG_INPUT					/*	input			*/
%token SYM_TAG_INS						/*	ins				*/
%token SYM_TAG_KEYGEN					/*	keygen			*/
%token SYM_TAG_KBD						/*	kbd				*/
%token SYM_TAG_LABEL					/*	label			*/
%token SYM_TAG_LEGEND					/*	legend			*/
%token SYM_TAG_LI						/*	li				*/
%token SYM_TAG_LINK						/*	link			*/
%token SYM_TAG_MAP						/*	map				*/
%token SYM_TAG_MARK						/*	mark			*/
%token SYM_TAG_MENU						/*	menu			*/
%token SYM_TAG_META						/*	meta			*/
%token SYM_TAG_METER					/*	meter			*/
%token SYM_TAG_NAV						/*	nav				*/
%token SYM_TAG_NOFRAME					/*	noframe			*/
%token SYM_TAG_NOSCRIPT					/*	noscript		*/
%token SYM_TAG_OBJECT					/*	object			*/
%token SYM_TAG_OL						/*	ol				*/
%token SYM_TAG_OPTGROUP					/*	optgroup		*/
%token SYM_TAG_OPTION					/*	option			*/
%token SYM_TAG_OUTPUT					/*	output			*/
%token SYM_TAG_P						/*	p				*/
%token SYM_TAG_PARAM					/*	param			*/
%token SYM_TAG_PRE						/*	pre				*/
%token SYM_TAG_PROGRESS					/*	progress		*/
%token SYM_TAG_Q						/*	q				*/
%token SYM_TAG_RP						/*	rp				*/
%token SYM_TAG_RT						/*	rt				*/
%token SYM_TAG_RUBY						/*	ruby			*/
%token SYM_TAG_S						/*	s				*/
%token SYM_TAG_U						/*	u				*/
%token SYM_TAG_SAMP						/*	samp			*/
%token SYM_TAG_SCRIPT					/*	script			*/
%token SYM_TAG_SECTION					/*	section			*/
%token SYM_TAG_SELECT					/*	select			*/
%token SYM_TAG_SMALL					/*	small			*/
%token SYM_TAG_SOURCE					/*	source			*/
%token SYM_TAG_SPAN						/*	span			*/
%token SYM_TAG_STRIKE					/*	strike			*/
%token SYM_TAG_STRONG					/*	strong			*/
%token SYM_TAG_STYLE					/*	style			*/
%token SYM_TAG_SUB						/*	sub				*/
%token SYM_TAG_SUMMARY					/*	summary			*/
%token SYM_TAG_SUP						/*	sup				*/
%token SYM_TAG_TABLE					/*	table			*/
%token SYM_TAG_TBODY					/*	tbody			*/
%token SYM_TAG_TD						/*	td				*/
%token SYM_TAG_TH						/*	th				*/
%token SYM_TAG_TR						/*	tr				*/
%token SYM_TAG_TT						/*	tt				*/
%token SYM_TAG_TEXTAREA					/*	textarea		*/
%token SYM_TAG_TFOOT					/*	tfoot			*/
%token SYM_TAG_THEAD					/*	thead			*/
%token SYM_TAG_TIME						/*	time			*/
%token SYM_TAG_TITLE					/*	title			*/
%token SYM_TAG_UL						/*	ul				*/
%token SYM_TAG_VAR						/*	var				*/
%token SYM_TAG_VIDEO					/*	video			*/
%token SYM_TAG_USERDATA					/*	others			*/

/*
 * for html event token
 */
%token SYM_EVT_ONAFTERPRINT				/*	onafterprint	*/
%token SYM_EVT_ONBEFOREPRINT			/*	onbeforeprint	*/
%token SYM_EVT_ONBEFOREONLOAD			/*	onbeforeonload	*/
%token SYM_EVT_ONBLUR					/*	onblur			*/
%token SYM_EVT_ONFOCUS					/*	onfocus			*/
%token SYM_EVT_ONHASCHANGE				/*	onhaschange		*/
%token SYM_EVT_ONLOAD					/*	onload			*/
%token SYM_EVT_ONMESSAGE				/*	onmessage		*/
%token SYM_EVT_ONOFFLINE				/*	onoffline		*/
%token SYM_EVT_ONONLINE					/*	ononline		*/
%token SYM_EVT_ONPAGEHIDE				/*	onpagehide		*/
%token SYM_EVT_ONPAGESHOW				/*	onpageshow		*/
%token SYM_EVT_ONPOPSTATE				/*	onpopstate		*/
%token SYM_EVT_ONREDO					/*	onredo			*/
%token SYM_EVT_ONRESIZE					/*	onresize		*/
%token SYM_EVT_ONSTORAGE				/*	onstorage		*/
%token SYM_EVT_ONUNDO					/*	onundo			*/
%token SYM_EVT_ONUNLOAD					/*	onunload		*/
%token SYM_EVT_ONCHANGE					/*	onchange		*/
%token SYM_EVT_ONCONTEXTMENU			/*	oncontextmenu	*/
%token SYM_EVT_ONFORMCHANGE				/*	onformchange	*/
%token SYM_EVT_ONFORMINPUT				/*	onforminput		*/
%token SYM_EVT_ONINPUT					/*	oninput			*/
%token SYM_EVT_ONINVALID				/*	oninvalid		*/
%token SYM_EVT_ONRESET					/*	onreset			*/
%token SYM_EVT_ONSELECT					/*	onselect		*/
%token SYM_EVT_ONSUBMIT					/*	onsubmit		*/
%token SYM_EVT_ONKEYDOWN				/*	onkeydown		*/
%token SYM_EVT_ONKEYPRESS				/*	onkeypress		*/
%token SYM_EVT_ONKEYUP					/*	onkeyup			*/
%token SYM_EVT_ONCLICK					/*	onclick			*/
%token SYM_EVT_ONDBLCLICK				/*	ondblclick		*/
%token SYM_EVT_ONDRAG					/*	ondrag			*/
%token SYM_EVT_ONDRAGEND				/*	ondragend		*/
%token SYM_EVT_ONDRAGENTER				/*	ondragenter		*/
%token SYM_EVT_ONDRAGLEAVE				/*	ondragleave		*/
%token SYM_EVT_ONDRAGOVER				/*	ondragover		*/
%token SYM_EVT_ONDRAGSTART				/*	ondragstart		*/
%token SYM_EVT_ONDROP					/*	ondrop			*/
%token SYM_EVT_ONMOUSEDOWN				/*	onmousedown		*/
%token SYM_EVT_ONMOUSEMOVE				/*	onmousemove		*/
%token SYM_EVT_ONMOUSEOUT				/*	onmouseout		*/
%token SYM_EVT_ONMOUSEOVER				/*	onmouseover		*/
%token SYM_EVT_ONMOUSEUP				/*	onmouseup		*/
%token SYM_EVT_ONMOUSEWHEEL				/*	onmousewheel	*/
%token SYM_EVT_ONSCROLL					/*	onscroll		*/
%token SYM_EVT_ONABORT					/*	onabort			*/
%token SYM_EVT_ONCANPLAY				/*	oncanplay		*/
%token SYM_EVT_ONCANPLAYTHROUGH			/*	oncanplaythrough*/
%token SYM_EVT_ONDURATIONCHANGE			/*	ondurationchange*/
%token SYM_EVT_ONEMPTIED				/*	onemptied		*/
%token SYM_EVT_ONENDED					/*	onended			*/
%token SYM_EVT_ONERROR					/*	onerror			*/
%token SYM_EVT_ONLOADEDDATA				/*	onloadeddata	*/
%token SYM_EVT_ONLOADEDMETADATA			/*	onloadedmetadata*/
%token SYM_EVT_ONLOADSTART				/*	onloadstart		*/
%token SYM_EVT_ONPAUSE					/*	onpause			*/
%token SYM_EVT_ONPLAY					/*	onplay			*/
%token SYM_EVT_ONPLAYING				/*	onplaying		*/
%token SYM_EVT_ONPROGRESS				/*	onprogress		*/
%token SYM_EVT_ONRATECHANGE				/*	onratechange	*/
%token SYM_EVT_ONREADYSTATECHANGE		/*	onreadystatechange*/
%token SYM_EVT_ONSEEKED					/*	onseeked		*/
%token SYM_EVT_ONSEEKING				/*	onseeking		*/
%token SYM_EVT_ONSTALLED				/*	onstalled		*/
%token SYM_EVT_ONSUSPEND				/*	onsuspend		*/
%token SYM_EVT_ONTIMEUPDATE				/*	ontimeupdate	*/
%token SYM_EVT_ONVOLUMECHANGE			/*	onvolumechange	*/
%token SYM_EVT_ONWAITING				/*	onwaiting		*/

/*
 * common prot
 */
%token SYM_PROT_ACCESSKEY				/*	accesskey		*/
%token SYM_PROT_CLASS					/*	class			*/
%token SYM_PROT_CONTENTEDITABLE			/*	contenteditable	*/
%token SYM_PROT_CONTEXTMENU				/*	contextmenu		*/
%token SYM_PROT_DIR						/*	dir				*/
%token SYM_PROT_DRAGGABLE				/*	draggable		*/
%token SYM_PROT_HIDDEN					/*	hidden			*/
%token SYM_PROT_ID						/*	id				*/
%token SYM_PROT_ITEM					/*	item			*/
%token SYM_PROT_ITEMPROP				/*	itemprop		*/
%token SYM_PROT_LANG					/*	lang			*/
%token SYM_PROT_SPELLCHECK				/*	spellcheck		*/
%token SYM_PROT_STYLE					/*	style			*/
%token SYM_PROT_SUBJECT					/*	subject			*/
%token SYM_PROT_TABINDEX				/*	tabindex		*/
%token SYM_PROT_TITLE					/*	title			*/
%token SYM_PROT_USERDATA				/*	data-'userdef'	*/
%token SYM_PROT_TEMPLATE				/*	template		*/
%token SYM_PROT_REGISTRATIONMARK		/*	registrationmark*/
%token SYM_PROT_IRRELEVANT				/*	irrelevant		*/

/*
 * tag private prop
 */
%token SYM_PROT_OPEN					/*	open			*/
%token SYM_PROT_DATA					/*	data			*/
%token SYM_PROT_NOWRAP					/*	nowrap			*/
%token SYM_PROT_DATETIME				/*	datetime		*/
%token SYM_PROT_ROWS					/*	rows			*/
%token SYM_PROT_LIST					/*	list			*/
%token SYM_PROT_FORMTARGETNEW			/*	formtargetNew	*/
%token SYM_PROT_AUTOFOCUSNEW			/*	autofocusNew	*/
%token SYM_PROT_ICON					/*	icon			*/
%token SYM_PROT_MAXLENGTH				/*	maxlength		*/
%token SYM_PROT_WIDTH					/*	width			*/
%token SYM_PROT_ARCHIVE					/*	archive			*/
%token SYM_PROT_HREF					/*	href			*/
%token SYM_PROT_PRELOAD					/*	preload			*/
%token SYM_PROT_MULTIPLE				/*	multiple		*/
%token SYM_PROT_HREFLANG				/*	hreflang		*/
%token SYM_PROT_CELLSPACING				/*	cellspacing		*/
%token SYM_PROT_COLSPAN					/*	colspan			*/
%token SYM_PROT_ACTION					/*	action			*/
%token SYM_PROT_CLASSID					/*	classid			*/
%token SYM_PROT_PATTERN					/*	pattern			*/
%token SYM_PROT_COLOR					/*	color			*/
%token SYM_PROT_HIGH					/*	high			*/
%token SYM_PROT_PING					/*	ping			*/
%token SYM_PROT_ISMAP					/*	ismap			*/
%token SYM_PROT_HTTPEQUIV				/*	http-equiv		*/
%token SYM_PROT_HSPACE					/*	hspace			*/
%token SYM_PROT_COMPACT					/*	compact			*/
%token SYM_PROT_LANGUAGE				/*	language		*/
%token SYM_PROT_REQUIRED				/*	required		*/
%token SYM_PROT_SPAN					/*	span			*/
%token SYM_PROT_FORMACTIONNEW			/*	formactionNew	*/
%token SYM_PROT_RULES					/*	rules			*/
%token SYM_PROT_AXIS					/*	axis			*/
%token SYM_PROT_METHOD					/*	method			*/
%token SYM_PROT_BGCOLOR					/*	bgcolor			*/
%token SYM_PROT_SHAPE					/*	shape			*/
%token SYM_PROT_USEMAP					/*	usemap			*/
%token SYM_PROT_FOR						/*	for				*/
%token SYM_PROT_SCOPED					/*	scoped			*/
%token SYM_PROT_FORMNOVALIDATENEW		/*	fornovalidateNew*/
%token SYM_PROT_CONTENT					/*	content			*/
%token SYM_PROT_INPUTMODE				/*	inputmode		*/
%token SYM_PROT_CITE					/*	cite			*/
%token SYM_PROT_VSPACE					/*	vspace			*/
%token SYM_PROT_XMLNS					/*	xmlns			*/
%token SYM_PROT_CODETYPE				/*	codetype		*/
%token SYM_PROT_TARGET					/*	target			*/
%token SYM_PROT_VALUE					/*	value			*/
%token SYM_PROT_AUTOFOCUS				/*	autofocus		*/
%token SYM_PROT_MEDIA					/*	media			*/
%token SYM_PROT_COORDS					/*	coords			*/
%token SYM_PROT_PROFILE					/*	profile			*/
%token SYM_PROT_HEADERS					/*	headers			*/
%token SYM_PROT_VALUETYPE				/*	valuetype		*/
%token SYM_PROT_REPLACE					/*	replace			*/
%token SYM_PROT_MARGINHEIGHT			/*	marginheight	*/
%token SYM_PROT_BORDER					/*	border			*/
%token SYM_PROT_FRAMEBORDER				/*	frameborder		*/
%token SYM_PROT_ASYNC					/*	async			*/
%token SYM_PROT_FACE					/*	face			*/
%token SYM_PROT_CELLPADDING				/*	cellpadding		*/
%token SYM_PROT_STANDBY					/*	standby			*/
%token SYM_PROT_ALT						/*	alt				*/
%token SYM_PROT_ACCEPTCHARSET			/*	accept-charset	*/
%token SYM_PROT_FORMMETHODNEW			/*	formmethodNew	*/
%token SYM_PROT_AUTOPLAY				/*	autoplay		*/
%token SYM_PROT_REV						/*	rev				*/
%token SYM_PROT_LOOP					/*	loop			*/
%token SYM_PROT_CODE					/*	code			*/
%token SYM_PROT_SRC						/*	src				*/
%token SYM_PROT_CHECKED					/*	checked			*/
%token SYM_PROT_SCROLLING				/*	scrolling		*/
%token SYM_PROT_SCOPE					/*	scope			*/
%token SYM_PROT_DEFER					/*	defer			*/
%token SYM_PROT_XMLSPACE				/*	xml:space		*/
%token SYM_PROT_CHALLENGE				/*	challenge		*/
%token SYM_PROT_SCHEME					/*	scheme			*/
%token SYM_PROT_DECLARE					/*	declare			*/
%token SYM_PROT_CHAR					/*	char			*/
%token SYM_PROT_READONLY				/*	readonly		*/
%token SYM_PROT_XMLLANG					/*	xml:lang		*/
%token SYM_PROT_MAX						/*	max				*/
%token SYM_PROT_ROWSPAN					/*	rowspan			*/
%token SYM_PROT_KEYTYPE					/*	keytype			*/
%token SYM_PROT_AUTOCOMPLETE			/*	autocomplete	*/
%token SYM_PROT_SELECTED				/*	selected		*/
%token SYM_PROT_CODEBASE				/*	codebase		*/
%token SYM_PROT_STEP					/*	step			*/
%token SYM_PROT_NOHREF					/*	nohref			*/
%token SYM_PROT_CHARSET					/*	charset			*/
%token SYM_PROT_FORMNEW					/*	formNew			*/
%token SYM_PROT_FORMENCTYPENEW			/*	formenctypeNew	*/
%token SYM_PROT_REL						/*	rel				*/
%token SYM_PROT_MIN						/*	min				*/
%token SYM_PROT_NAME					/*	name			*/
%token SYM_PROT_TYPE					/*	type			*/
%token SYM_PROT_NOSHADE					/*	noshade			*/
%token SYM_PROT_MANIFEST				/*	manifest		*/
%token SYM_PROT_ALIGN					/*	align			*/
%token SYM_PROT_HEIGHT					/*	height			*/
%token SYM_PROT_ACCEPT					/*	accept			*/
%token SYM_PROT_ENCTYPE					/*	enctype			*/
%token SYM_PROT_DISABLED				/*	disabled		*/
%token SYM_PROT_CONTROLS				/*	controls		*/
%token SYM_PROT_LONGDESC				/*	longdesc		*/
%token SYM_PROT_MARGINWIDTH				/*	marginwidth		*/
%token SYM_PROT_NORESIZE				/*	noresize		*/
%token SYM_PROT_COLS					/*	cols			*/
%token SYM_PROT_SIZE					/*	size			*/
%token SYM_PROT_RADIOGROUP				/*	radiogroup		*/
%token SYM_PROT_VALIGN					/*	valign			*/
%token SYM_PROT_CHAROFF					/*	charoff			*/
%token SYM_PROT_LOW						/*	low				*/
%token SYM_PROT_START					/*	start			*/
%token SYM_PROT_SUMMARY					/*	summary			*/
%token SYM_PROT_OPTIMUM					/*	optimum			*/
%token SYM_PROT_ABBR					/*	abbr			*/
%token SYM_PROT_FORM					/*	form			*/
%token SYM_PROT_LABEL					/*	label			*/
%token SYM_PROT_FRAME					/*	frame			*/
%token SYM_PROT_ALLOWSCRIPTACCESS		/*	AllowScriptAccess*/

%union
{
	struct
	{
		char *str;
		int str_len;
	};
	enum htmltokentype id;
	struct html_node *node;
	struct html_node_prot *prot;
	STAILQ_HEAD(__prot_type, html_node_prot) prot_list;
	STAILQ_HEAD(__node, html_node) node_list;
}

%type <str> html_tag_event_value_ html_tag_prot_value_
%type <id> html_tag_name_ html_tag_event_name_ html_tag_prot_name_
%type <node> html_tag
%type <node> html_tag_begin_
%type <prot> html_tag_prot_event_ 
%type <prot> html_tag_prot_ 
%type <prot> html_tag_event_
%type <prot_list> html_tag_prot_event_list_ 
%type <node_list> html_doc 

%parse-param {void *this_priv}

%destructor {if($$) html_free($$);} html_tag_event_value_ html_tag_prot_value_

%define api.prefix html
%define api.pure full
%define api.push-pull push

%start html_init
%%
html_init: 
	html_doc
	{
		NOCOPY_BREAK(this_priv);
		STAILQ_CONCAT(&((html_priv_data*)this_priv)->html_root, &$1);
	}
	;

html_doc:
	empty
	{
		STAILQ_INIT(&$$);
	}
	| html_doc html_tag
	{
		NOCOPY_BREAK(this_priv);

		STAILQ_INSERT_TAIL(&$$, $2, sib);
	}
	| html_doc SYM_TEXT
	{
		NOCOPY_BREAK(this_priv);

		char *value = (char*)html_malloc(yylval.str_len + 1);
		if(!value)
		{
			YYFPRINTF(stderr, "alloc doc value failed\n");
			YYABORT;
		}
		memcpy(value, yylval.str, yylval.str_len);
		value[yylval.str_len] = '\0';

		html_node_t *node = html_alloc_node((html_priv_data*)this_priv);
		if(!node)
		{
			YYFPRINTF(stderr, "alloc doc node failed\n");
			html_free(value);
			YYABORT;
		}

		node->type = SYM_TEXT;
		node->text = value;
		STAILQ_INSERT_TAIL(&$$, node, sib);
	}
	;

empty:
	 ;

html_tag:
	html_tag_begin_ html_doc html_tag_finish_
	{
		ADD_SCORE(this_priv);

		NOCOPY_BREAK(this_priv);
		STAILQ_CONCAT(&$1->child, &$2);
		$$ = $1;
	}
	;

html_tag_begin_:
	html_tag_begin_start_ html_tag_name_
	{
		if(IS_DETECT_NODE($2))
			SET_COPY_FLAG(this_priv,1);
	}
	html_tag_prot_event_list_ html_tag_begin_end_
	{
		if(FILE_CHECK(this_priv))
		{
			html_node_t *node = html_alloc_node((html_priv_data*)this_priv);
			if(!node)
			{
				YYFPRINTF(stderr, "alloc tag node failed\n");
				YYABORT;
			}

			node->type = $2;
			STAILQ_CONCAT(&node->prot, &$4);
			$$ = node;
		}
		else if(HTTP_CHECK(this_priv) && ((html_priv_data*)this_priv)->copy)
		{
			/*do html check*/
			if(do_html_check((html_priv_data*)this_priv, $2, STAILQ_FIRST(&$4)) < 0)
				return -1;
		}
		SET_COPY_FLAG(this_priv, 0);
	}
	;

html_tag_begin_start_:
	SYM_TAG_START_FLAG
	;

html_tag_begin_end_:
	SYM_TAG_END_FLAG
	| SYM_TAG_END_FLAG2
	;

html_tag_finish_:
	empty
	| html_tag_finish_start_ html_tag_name_
	{
		SET_COPY_FLAG(this_priv,0);
	}
	html_tag_prot_event_list_ html_tag_finish_end_
	;

html_tag_finish_start_:
	SYM_TAG_START_FLAG2
	;

html_tag_finish_end_:
	SYM_TAG_END_FLAG
	| SYM_TAG_END_FLAG2
	;

html_tag_prot_event_list_:
	empty
	{
		STAILQ_INIT(&$$);
	}
	| html_tag_prot_event_list_ html_tag_prot_event_
	{
		ADD_SCORE(this_priv);

		NOCOPY_BREAK(this_priv);
		STAILQ_INSERT_TAIL(&$$, $2, next);
	}
	;

html_tag_prot_event_:
	html_tag_prot_
	{
		$$ = $1;
	}
	| html_tag_event_
	{
		$$ = $1;
	}
	;

html_tag_prot_:
	html_tag_prot_name_ SYM_EQUAL html_tag_prot_value_
	{
		NOCOPY_BREAK(this_priv);

		html_node_prot_t *prot = html_alloc_prot((html_priv_data*)this_priv);
		if(!prot)
		{
			YYFPRINTF(stderr, "alloc node prot failed\n");
			html_free($3);
			$3 = NULL;
			YYABORT;
		}
		prot->type = $1;
		prot->value = $3;
		$$ = prot;
	}
	| html_tag_prot_name_
	{
		NOCOPY_BREAK(this_priv);

		html_node_prot_t *prot = html_alloc_prot((html_priv_data*)this_priv);
		if(!prot)
		{
			YYFPRINTF(stderr, "alloc node prot failed\n");
			YYABORT;
		}
		prot->type = $1;
		$$ = prot;
	}
	;

html_tag_prot_value_:
	SYM_TEXT
	{
		if(XSS_CHECK(this_priv))
		{
			html_value_string_decode(yylval.str);
			if(!strncasecmp(yylval.str, JAVASCRIPT_FLAG, sizeof(JAVASCRIPT_FLAG)-1))
				((html_priv_data*)this_priv)->score += (NO_XSS+1);
			break;
		}
		else
		{
			NOCOPY_BREAK(this_priv);
		}

		$$ = (char*)html_malloc(yylval.str_len + 1);
		if(!$$)
			YYABORT;
		memcpy($$, yylval.str, yylval.str_len);
		$$[yylval.str_len] = '\0';
		html_value_string_decode($$);
	}
	;

html_tag_event_:
	html_tag_event_name_ SYM_EQUAL html_tag_event_value_
	{
		NOCOPY_BREAK(this_priv);

		html_node_prot_t *prot = html_alloc_prot((html_priv_data*)this_priv);
		if(!prot)
		{
			YYFPRINTF(stderr, "alloc node prot failed\n");
			html_free($3);
			$3 = NULL;
			YYABORT;
		}
		prot->type = $1;
		prot->value = $3;
		$$ = prot;
	}
	| html_tag_event_name_
	{
		NOCOPY_BREAK(this_priv);

		html_node_prot_t *prot = html_alloc_prot((html_priv_data*)this_priv);
		if(!prot)
		{
			YYFPRINTF(stderr, "alloc node prot failed\n");
			YYABORT;
		}
		prot->type = $1;
		$$ = prot;
	}
	;

html_tag_event_value_:
	SYM_TEXT
	{
		if(XSS_CHECK(this_priv))
		{
			html_value_string_decode(yylval.str);
			if(!strncasecmp(yylval.str, JAVASCRIPT_FLAG, sizeof(JAVASCRIPT_FLAG)-1))
				((html_priv_data*)this_priv)->score += (NO_XSS+1);
			break;
		}
		else
		{
			NOCOPY_BREAK(this_priv);
		}

		$$ = (char*)html_malloc(yylval.str_len + 1);
		if(!$$)
			YYABORT;
		memcpy($$, yylval.str, yylval.str_len);
		$$[yylval.str_len] = '\0';
		html_value_string_decode($$);
	}
	;

html_tag_event_name_:
	SYM_EVT_ONAFTERPRINT
	{
		$$ = SYM_EVT_ONAFTERPRINT;
	}
	| SYM_EVT_ONBEFOREPRINT
	{
		$$ = SYM_EVT_ONBEFOREPRINT;
	}
	| SYM_EVT_ONBEFOREONLOAD
	{
		$$ = SYM_EVT_ONBEFOREONLOAD;
	}
	| SYM_EVT_ONBLUR
	{
		$$ = SYM_EVT_ONBLUR;
	}
	| SYM_EVT_ONFOCUS
	{
		$$ = SYM_EVT_ONFOCUS;
	}
	| SYM_EVT_ONHASCHANGE
	{
		$$ = SYM_EVT_ONHASCHANGE;
	}
	| SYM_EVT_ONLOAD
	{
		$$ = SYM_EVT_ONLOAD;
	}
	| SYM_EVT_ONMESSAGE
	{
		$$ = SYM_EVT_ONMESSAGE;
	}
	| SYM_EVT_ONOFFLINE
	{
		$$ = SYM_EVT_ONOFFLINE;
	}
	| SYM_EVT_ONONLINE
	{
		$$ = SYM_EVT_ONONLINE;
	}
	| SYM_EVT_ONPAGEHIDE
	{
		$$ = SYM_EVT_ONPAGEHIDE;
	}
	| SYM_EVT_ONPAGESHOW
	{
		$$ = SYM_EVT_ONPAGESHOW;
	}
	| SYM_EVT_ONPOPSTATE
	{
		$$ = SYM_EVT_ONPOPSTATE;
	}
	| SYM_EVT_ONREDO
	{
		$$ = SYM_EVT_ONREDO;
	}
	| SYM_EVT_ONRESIZE
	{
		$$ = SYM_EVT_ONRESIZE;
	}
	| SYM_EVT_ONSTORAGE
	{
		$$ = SYM_EVT_ONSTORAGE;
	}
	| SYM_EVT_ONUNDO
	{
		$$ = SYM_EVT_ONUNDO;
	}
	| SYM_EVT_ONUNLOAD
	{
		$$ = SYM_EVT_ONUNLOAD;
	}
	| SYM_EVT_ONCHANGE
	{
		$$ = SYM_EVT_ONCHANGE;
	}
	| SYM_EVT_ONCONTEXTMENU
	{
		$$ = SYM_EVT_ONCONTEXTMENU;
	}
	| SYM_EVT_ONFORMCHANGE
	{
		$$ = SYM_EVT_ONFORMCHANGE;
	}
	| SYM_EVT_ONFORMINPUT
	{
		$$ = SYM_EVT_ONFORMINPUT;
	}
	| SYM_EVT_ONINPUT
	{
		$$ = SYM_EVT_ONINPUT;
	}
	| SYM_EVT_ONINVALID
	{
		$$ = SYM_EVT_ONINVALID;
	}
	| SYM_EVT_ONRESET
	{
		$$ = SYM_EVT_ONRESET;
	}
	| SYM_EVT_ONSELECT
	{
		$$ = SYM_EVT_ONSELECT;
	}
	| SYM_EVT_ONSUBMIT
	{
		$$ = SYM_EVT_ONSUBMIT;
	}
	| SYM_EVT_ONKEYDOWN
	{
		$$ = SYM_EVT_ONKEYDOWN;
	}
	| SYM_EVT_ONKEYPRESS
	{
		$$ = SYM_EVT_ONKEYPRESS;
	}
	| SYM_EVT_ONKEYUP
	{
		$$ = SYM_EVT_ONKEYUP;
	}
	| SYM_EVT_ONCLICK
	{
		$$ = SYM_EVT_ONCLICK;
	}
	| SYM_EVT_ONDBLCLICK
	{
		$$ = SYM_EVT_ONDBLCLICK;
	}
	| SYM_EVT_ONDRAG
	{
		$$ = SYM_EVT_ONDRAG;
	}
	| SYM_EVT_ONDRAGEND
	{
		$$ = SYM_EVT_ONDRAGEND;
	}
	| SYM_EVT_ONDRAGENTER
	{
		$$ = SYM_EVT_ONDRAGENTER;
	}
	| SYM_EVT_ONDRAGLEAVE
	{
		$$ = SYM_EVT_ONDRAGLEAVE;
	}
	| SYM_EVT_ONDRAGOVER
	{
		$$ = SYM_EVT_ONDRAGOVER;
	}
	| SYM_EVT_ONDRAGSTART
	{
		$$ = SYM_EVT_ONDRAGSTART;
	}
	| SYM_EVT_ONDROP
	{
		$$ = SYM_EVT_ONDROP;
	}
	| SYM_EVT_ONMOUSEDOWN
	{
		$$ = SYM_EVT_ONMOUSEDOWN;
	}
	| SYM_EVT_ONMOUSEMOVE
	{
		$$ = SYM_EVT_ONMOUSEMOVE;
	}
	| SYM_EVT_ONMOUSEOUT
	{
		$$ = SYM_EVT_ONMOUSEOUT;
	}
	| SYM_EVT_ONMOUSEOVER
	{
		$$ = SYM_EVT_ONMOUSEOVER;
	}
	| SYM_EVT_ONMOUSEUP
	{
		$$ = SYM_EVT_ONMOUSEUP;
	}
	| SYM_EVT_ONMOUSEWHEEL
	{
		$$ = SYM_EVT_ONMOUSEWHEEL;
	}
	| SYM_EVT_ONSCROLL
	{
		$$ = SYM_EVT_ONSCROLL;
	}
	| SYM_EVT_ONABORT
	{
		$$ = SYM_EVT_ONABORT;
	}
	| SYM_EVT_ONCANPLAY
	{
		$$ = SYM_EVT_ONCANPLAY;
	}
	| SYM_EVT_ONCANPLAYTHROUGH
	{
		$$ = SYM_EVT_ONCANPLAYTHROUGH;
	}
	| SYM_EVT_ONDURATIONCHANGE
	{
		$$ = SYM_EVT_ONDURATIONCHANGE;
	}
	| SYM_EVT_ONEMPTIED
	{
		$$ = SYM_EVT_ONEMPTIED;
	}
	| SYM_EVT_ONENDED
	{
		$$ = SYM_EVT_ONENDED;
	}
	| SYM_EVT_ONERROR
	{
		$$ = SYM_EVT_ONERROR;
	}
	| SYM_EVT_ONLOADEDDATA
	{
		$$ = SYM_EVT_ONLOADEDDATA;
	}
	| SYM_EVT_ONLOADEDMETADATA
	{
		$$ = SYM_EVT_ONLOADEDMETADATA;
	}
	| SYM_EVT_ONLOADSTART
	{
		$$ = SYM_EVT_ONLOADSTART;
	}
	| SYM_EVT_ONPAUSE
	{
		$$ = SYM_EVT_ONPAUSE;
	}
	| SYM_EVT_ONPLAY
	{
		$$ = SYM_EVT_ONPLAY;
	}
	| SYM_EVT_ONPLAYING
	{
		$$ = SYM_EVT_ONPLAYING;
	}
	| SYM_EVT_ONPROGRESS
	{
		$$ = SYM_EVT_ONPROGRESS;
	}
	| SYM_EVT_ONRATECHANGE
	{
		$$ = SYM_EVT_ONRATECHANGE;
	}
	| SYM_EVT_ONREADYSTATECHANGE
	{
		$$ = SYM_EVT_ONREADYSTATECHANGE;
	}
	| SYM_EVT_ONSEEKED
	{
		$$ = SYM_EVT_ONSEEKED;
	}
	| SYM_EVT_ONSEEKING
	{
		$$ = SYM_EVT_ONSEEKING;
	}
	| SYM_EVT_ONSTALLED
	{
		$$ = SYM_EVT_ONSTALLED;
	}
	| SYM_EVT_ONSUSPEND
	{
		$$ = SYM_EVT_ONSUSPEND;
	}
	| SYM_EVT_ONTIMEUPDATE
	{
		$$ = SYM_EVT_ONTIMEUPDATE;
	}
	| SYM_EVT_ONVOLUMECHANGE
	{
		$$ = SYM_EVT_ONVOLUMECHANGE;
	}
	| SYM_EVT_ONWAITING
	{
		$$ = SYM_EVT_ONWAITING;
	}
	;

html_tag_name_:
	SYM_TAG_A
	{
		$$ = SYM_TAG_A;
	}
	| SYM_TAG_ABBR
	{
		$$ = SYM_TAG_ABBR;
	}
	| SYM_TAG_ACRONYM
	{
		$$ = SYM_TAG_ACRONYM;
	}
	| SYM_TAG_ADDRESS
	{
		$$ = SYM_TAG_ADDRESS;
	}
	| SYM_TAG_APPLET
	{
		$$ = SYM_TAG_APPLET;
	}
	| SYM_TAG_AREA
	{
		$$ = SYM_TAG_AREA;
	}
	| SYM_TAG_ARTICLE
	{
		$$ = SYM_TAG_ARTICLE;
	}
	| SYM_TAG_ASIDE
	{
		$$ = SYM_TAG_ASIDE;
	}
	| SYM_TAG_AUDIO
	{
		$$ = SYM_TAG_AUDIO;
	}
	| SYM_TAG_B
	{
		$$ = SYM_TAG_B;
	}
	| SYM_TAG_BASE
	{
		$$ = SYM_TAG_BASE;
	}
	| SYM_TAG_BASEFONT
	{
		$$ = SYM_TAG_BASEFONT;
	}
	| SYM_TAG_BDO
	{
		$$ = SYM_TAG_BDO;
	}
	| SYM_TAG_BIG
	{
		$$ = SYM_TAG_BIG;
	}
	| SYM_TAG_BLOCKQUOTE
	{
		$$ = SYM_TAG_BLOCKQUOTE;
	}
	| SYM_TAG_BODY
	{
		$$ = SYM_TAG_BODY;
	}
	| SYM_TAG_BR
	{
		$$ = SYM_TAG_BR;
	}
	| SYM_TAG_BUTTON
	{
		$$ = SYM_TAG_BUTTON;
	}
	| SYM_TAG_CANVAS
	{
		$$ = SYM_TAG_CANVAS;
	}
	| SYM_TAG_CAPTION
	{
		$$ = SYM_TAG_CAPTION;
	}
	| SYM_TAG_CENTER
	{
		$$ = SYM_TAG_CENTER;
	}
	| SYM_TAG_CITE
	{
		$$ = SYM_TAG_CITE;
	}
	| SYM_TAG_CODE
	{
		$$ = SYM_TAG_CODE;
	}
	| SYM_TAG_COL
	{
		$$ = SYM_TAG_COL;
	}
	| SYM_TAG_COLGROUP
	{
		$$ = SYM_TAG_COLGROUP;
	}
	| SYM_TAG_COMMAND
	{
		$$ = SYM_TAG_COMMAND;
	}
	| SYM_TAG_DATALIST
	{
		$$ = SYM_TAG_DATALIST;
	}
	| SYM_TAG_DD
	{
		$$ = SYM_TAG_DD;
	}
	| SYM_TAG_DEL
	{
		$$ = SYM_TAG_DEL;
	}
	| SYM_TAG_DETAILS
	{
		$$ = SYM_TAG_DETAILS;
	}
	| SYM_TAG_DFN
	{
		$$ = SYM_TAG_DFN;
	}
	| SYM_TAG_DIR
	{
		$$ = SYM_TAG_DIR;
	}
	| SYM_TAG_DIV
	{
		$$ = SYM_TAG_DIV;
	}
	| SYM_TAG_DL
	{
		$$ = SYM_TAG_DL;
	}
	| SYM_TAG_DT
	{
		$$ = SYM_TAG_DT;
	}
	| SYM_TAG_EM
	{
		$$ = SYM_TAG_EM;
	}
	| SYM_TAG_EMBED
	{
		$$ = SYM_TAG_EMBED;
	}
	| SYM_TAG_FIELDSET
	{
		$$ = SYM_TAG_FIELDSET;
	}
	| SYM_TAG_FIGCAPTION
	{
		$$ = SYM_TAG_FIGCAPTION;
	}
	| SYM_TAG_FIGURE
	{
		$$ = SYM_TAG_FIGURE;
	}
	| SYM_TAG_FONT
	{
		$$ = SYM_TAG_FONT;
	}
	| SYM_TAG_FOOTER
	{
		$$ = SYM_TAG_FOOTER;
	}
	| SYM_TAG_FORM
	{
		$$ = SYM_TAG_FORM;
	}
	| SYM_TAG_FRAME
	{
		$$ = SYM_TAG_FRAME;
	}
	| SYM_TAG_FRAMESET
	{
		$$ = SYM_TAG_FRAMESET;
	}
	| SYM_TAG_H1
	{
		$$ = SYM_TAG_H1;
	}
	| SYM_TAG_H2
	{
		$$ = SYM_TAG_H2;
	}
	| SYM_TAG_H3
	{
		$$ = SYM_TAG_H3;
	}
	| SYM_TAG_H4
	{
		$$ = SYM_TAG_H4;
	}
	| SYM_TAG_H5
	{
		$$ = SYM_TAG_H5;
	}
	| SYM_TAG_H6
	{
		$$ = SYM_TAG_H6;
	}
	| SYM_TAG_HEAD
	{
		$$ = SYM_TAG_HEAD;
	}
	| SYM_TAG_HEADER
	{
		$$ = SYM_TAG_HEADER;
	}
	| SYM_TAG_HGROUP
	{
		$$ = SYM_TAG_HGROUP;
	}
	| SYM_TAG_HR
	{
		$$ = SYM_TAG_HR;
	}
	| SYM_TAG_HTML
	{
		$$ = SYM_TAG_HTML;
	}
	| SYM_TAG_I
	{
		$$ = SYM_TAG_I;
	}
	| SYM_TAG_IFRAME
	{
		$$ = SYM_TAG_IFRAME;
	}
	| SYM_TAG_IMG
	{
		$$ = SYM_TAG_IMG;
	}
	| SYM_TAG_INPUT
	{
		$$ = SYM_TAG_INPUT;
	}
	| SYM_TAG_INS
	{
		$$ = SYM_TAG_INS;
	}
	| SYM_TAG_KEYGEN
	{
		$$ = SYM_TAG_KEYGEN;
	}
	| SYM_TAG_KBD
	{
		$$ = SYM_TAG_KBD;
	}
	| SYM_TAG_LABEL
	{
		$$ = SYM_TAG_LABEL;
	}
	| SYM_TAG_LEGEND
	{
		$$ = SYM_TAG_LEGEND;
	}
	| SYM_TAG_LI
	{
		$$ = SYM_TAG_LI;
	}
	| SYM_TAG_LINK
	{
		$$ = SYM_TAG_LINK;
	}
	| SYM_TAG_MAP
	{
		$$ = SYM_TAG_MAP;
	}
	| SYM_TAG_MARK
	{
		$$ = SYM_TAG_MARK;
	}
	| SYM_TAG_MENU
	{
		$$ = SYM_TAG_MENU;
	}
	| SYM_TAG_META
	{
		$$ = SYM_TAG_META;
	}
	| SYM_TAG_METER
	{
		$$ = SYM_TAG_METER;
	}
	| SYM_TAG_NAV
	{
		$$ = SYM_TAG_NAV;
	}
	| SYM_TAG_NOFRAME
	{
		$$ = SYM_TAG_NOFRAME;
	}
	| SYM_TAG_NOSCRIPT
	{
		$$ = SYM_TAG_NOSCRIPT;
	}
	| SYM_TAG_OBJECT
	{
		$$ = SYM_TAG_OBJECT;
	}
	| SYM_TAG_OL
	{
		$$ = SYM_TAG_OL;
	}
	| SYM_TAG_OPTGROUP
	{
		$$ = SYM_TAG_OPTGROUP;
	}
	| SYM_TAG_OPTION
	{
		$$ = SYM_TAG_OPTION;
	}
	| SYM_TAG_OUTPUT
	{
		$$ = SYM_TAG_OUTPUT;
	}
	| SYM_TAG_P
	{
		$$ = SYM_TAG_P;
	}
	| SYM_TAG_PARAM
	{
		$$ = SYM_TAG_PARAM;
	}
	| SYM_TAG_PRE
	{
		$$ = SYM_TAG_PRE;
	}
	| SYM_TAG_PROGRESS
	{
		$$ = SYM_TAG_PROGRESS;
	}
	| SYM_TAG_Q
	{
		$$ = SYM_TAG_Q;
	}
	| SYM_TAG_RP
	{
		$$ = SYM_TAG_RP;
	}
	| SYM_TAG_RT
	{
		$$ = SYM_TAG_RT;
	}
	| SYM_TAG_RUBY
	{
		$$ = SYM_TAG_RUBY;
	}
	| SYM_TAG_S
	{
		$$ = SYM_TAG_S;
	}
	| SYM_TAG_U
	{
		$$ = SYM_TAG_U;
	}
	| SYM_TAG_SAMP
	{
		$$ = SYM_TAG_SAMP;
	}
	| SYM_TAG_SCRIPT
	{
		$$ = SYM_TAG_SCRIPT;
	}
	| SYM_TAG_SECTION
	{
		$$ = SYM_TAG_SECTION;
	}
	| SYM_TAG_SELECT
	{
		$$ = SYM_TAG_SELECT;
	}
	| SYM_TAG_SMALL
	{
		$$ = SYM_TAG_SMALL;
	}
	| SYM_TAG_SOURCE
	{
		$$ = SYM_TAG_SOURCE;
	}
	| SYM_TAG_SPAN
	{
		$$ = SYM_TAG_SPAN;
	}
	| SYM_TAG_STRIKE
	{
		$$ = SYM_TAG_STRIKE;
	}
	| SYM_TAG_STRONG
	{
		$$ = SYM_TAG_STRONG;
	}
	| SYM_TAG_STYLE
	{
		$$ = SYM_TAG_STYLE;
	}
	| SYM_TAG_SUB
	{
		$$ = SYM_TAG_SUB;
	}
	| SYM_TAG_SUMMARY
	{
		$$ = SYM_TAG_SUMMARY;
	}
	| SYM_TAG_SUP
	{
		$$ = SYM_TAG_SUP;
	}
	| SYM_TAG_TABLE
	{
		$$ = SYM_TAG_TABLE;
	}
	| SYM_TAG_TBODY
	{
		$$ = SYM_TAG_TBODY;
	}
	| SYM_TAG_TD
	{
		$$ = SYM_TAG_TD;
	}
	| SYM_TAG_TH
	{
		$$ = SYM_TAG_TH;
	}
	| SYM_TAG_TR
	{
		$$ = SYM_TAG_TR;
	}
	| SYM_TAG_TT
	{
		$$ = SYM_TAG_TT;
	}
	| SYM_TAG_TEXTAREA
	{
		$$ = SYM_TAG_TEXTAREA;
	}
	| SYM_TAG_TFOOT
	{
		$$ = SYM_TAG_TFOOT;
	}
	| SYM_TAG_THEAD
	{
		$$ = SYM_TAG_THEAD;
	}
	| SYM_TAG_TIME
	{
		$$ = SYM_TAG_TIME;
	}
	| SYM_TAG_TITLE
	{
		$$ = SYM_TAG_TITLE;
	}
	| SYM_TAG_UL
	{
		$$ = SYM_TAG_UL;
	}
	| SYM_TAG_VAR
	{
		$$ = SYM_TAG_VAR;
	}
	| SYM_TAG_VIDEO
	{
		$$ = SYM_TAG_VIDEO;
	}
	| SYM_TAG_USERDATA
	{
		$$ = SYM_TAG_USERDATA;
	}
	;

html_tag_prot_name_:
	SYM_PROT_ACCESSKEY
	{
		$$ = SYM_PROT_ACCESSKEY;
	}
	| SYM_PROT_CLASS
	{
		$$ = SYM_PROT_CLASS;
	}
	| SYM_PROT_CONTENTEDITABLE
	{
		$$ = SYM_PROT_CONTENTEDITABLE;
	}
	| SYM_PROT_CONTEXTMENU
	{
		$$ = SYM_PROT_CONTEXTMENU;
	}
	| SYM_PROT_DIR
	{
		$$ = SYM_PROT_DIR;
	}
	| SYM_PROT_DRAGGABLE
	{
		$$ = SYM_PROT_DRAGGABLE;
	}
	| SYM_PROT_HIDDEN
	{
		$$ = SYM_PROT_HIDDEN;
	}
	| SYM_PROT_ID
	{
		$$ = SYM_PROT_ID;
	}
	| SYM_PROT_ITEM
	{
		$$ = SYM_PROT_ITEM;
	}
	| SYM_PROT_ITEMPROP
	{
		$$ = SYM_PROT_ITEMPROP;
	}
	| SYM_PROT_LANG
	{
		$$ = SYM_PROT_LANG;
	}
	| SYM_PROT_SPELLCHECK
	{
		$$ = SYM_PROT_SPELLCHECK;
	}
	| SYM_PROT_STYLE
	{
		$$ = SYM_PROT_STYLE;
	}
	| SYM_PROT_SUBJECT
	{
		$$ = SYM_PROT_SUBJECT;
	}
	| SYM_PROT_TABINDEX
	{
		$$ = SYM_PROT_TABINDEX;
	}
	| SYM_PROT_TITLE
	{
		$$ = SYM_PROT_TITLE;
	}
	| SYM_PROT_USERDATA
	{
		$$ = SYM_PROT_USERDATA;
	}
	| SYM_PROT_TEMPLATE
	{
		$$ = SYM_PROT_TEMPLATE;
	}
	| SYM_PROT_REGISTRATIONMARK
	{
		$$ = SYM_PROT_REGISTRATIONMARK;
	}
	| SYM_PROT_IRRELEVANT
	{
		$$ = SYM_PROT_IRRELEVANT;
	}
	| SYM_PROT_OPEN
	{
		$$ = SYM_PROT_OPEN;
	}
	| SYM_PROT_DATA
	{
		$$ = SYM_PROT_DATA;
	}
	| SYM_PROT_NOWRAP
	{
		$$ = SYM_PROT_NOWRAP;
	}
	| SYM_PROT_DATETIME
	{
		$$ = SYM_PROT_DATETIME;
	}
	| SYM_PROT_ROWS
	{
		$$ = SYM_PROT_ROWS;
	}
	| SYM_PROT_LIST
	{
		$$ = SYM_PROT_LIST;
	}
	| SYM_PROT_FORMTARGETNEW
	{
		$$ = SYM_PROT_FORMTARGETNEW;
	}
	| SYM_PROT_AUTOFOCUSNEW
	{
		$$ = SYM_PROT_AUTOFOCUSNEW;
	}
	| SYM_PROT_ICON
	{
		$$ = SYM_PROT_ICON;
	}
	| SYM_PROT_MAXLENGTH
	{
		$$ = SYM_PROT_MAXLENGTH;
	}
	| SYM_PROT_WIDTH
	{
		$$ = SYM_PROT_WIDTH;
	}
	| SYM_PROT_ARCHIVE
	{
		$$ = SYM_PROT_ARCHIVE;
	}
	| SYM_PROT_HREF
	{
		$$ = SYM_PROT_HREF;
	}
	| SYM_PROT_PRELOAD
	{
		$$ = SYM_PROT_PRELOAD;
	}
	| SYM_PROT_MULTIPLE
	{
		$$ = SYM_PROT_MULTIPLE;
	}
	| SYM_PROT_HREFLANG
	{
		$$ = SYM_PROT_HREFLANG;
	}
	| SYM_PROT_CELLSPACING
	{
		$$ = SYM_PROT_CELLSPACING;
	}
	| SYM_PROT_COLSPAN
	{
		$$ = SYM_PROT_COLSPAN;
	}
	| SYM_PROT_ACTION
	{
		$$ = SYM_PROT_ACTION;
	}
	| SYM_PROT_CLASSID
	{
		$$ = SYM_PROT_CLASSID;
	}
	| SYM_PROT_PATTERN
	{
		$$ = SYM_PROT_PATTERN;
	}
	| SYM_PROT_COLOR
	{
		$$ = SYM_PROT_COLOR;
	}
	| SYM_PROT_HIGH
	{
		$$ = SYM_PROT_HIGH;
	}
	| SYM_PROT_PING
	{
		$$ = SYM_PROT_PING;
	}
	| SYM_PROT_ISMAP
	{
		$$ = SYM_PROT_ISMAP;
	}
	| SYM_PROT_HTTPEQUIV
	{
		$$ = SYM_PROT_HTTPEQUIV;
	}
	| SYM_PROT_HSPACE
	{
		$$ = SYM_PROT_HSPACE;
	}
	| SYM_PROT_COMPACT
	{
		$$ = SYM_PROT_COMPACT;
	}
	| SYM_PROT_LANGUAGE
	{
		$$ = SYM_PROT_LANGUAGE;
	}
	| SYM_PROT_REQUIRED
	{
		$$ = SYM_PROT_REQUIRED;
	}
	| SYM_PROT_SPAN
	{
		$$ = SYM_PROT_SPAN;
	}
	| SYM_PROT_FORMACTIONNEW
	{
		$$ = SYM_PROT_FORMACTIONNEW;
	}
	| SYM_PROT_RULES
	{
		$$ = SYM_PROT_RULES;
	}
	| SYM_PROT_AXIS
	{
		$$ = SYM_PROT_AXIS;
	}
	| SYM_PROT_METHOD
	{
		$$ = SYM_PROT_METHOD;
	}
	| SYM_PROT_BGCOLOR
	{
		$$ = SYM_PROT_BGCOLOR;
	}
	| SYM_PROT_SHAPE
	{
		$$ = SYM_PROT_SHAPE;
	}
	| SYM_PROT_USEMAP
	{
		$$ = SYM_PROT_USEMAP;
	}
	| SYM_PROT_FOR
	{
		$$ = SYM_PROT_FOR;
	}
	| SYM_PROT_SCOPED
	{
		$$ = SYM_PROT_SCOPED;
	}
	| SYM_PROT_FORMNOVALIDATENEW
	{
		$$ = SYM_PROT_FORMNOVALIDATENEW;
	}
	| SYM_PROT_CONTENT
	{
		$$ = SYM_PROT_CONTENT;
	}
	| SYM_PROT_INPUTMODE
	{
		$$ = SYM_PROT_INPUTMODE;
	}
	| SYM_PROT_CITE
	{
		$$ = SYM_PROT_CITE;
	}
	| SYM_PROT_VSPACE
	{
		$$ = SYM_PROT_VSPACE;
	}
	| SYM_PROT_XMLNS
	{
		$$ = SYM_PROT_XMLNS;
	}
	| SYM_PROT_CODETYPE
	{
		$$ = SYM_PROT_CODETYPE;
	}
	| SYM_PROT_TARGET
	{
		$$ = SYM_PROT_TARGET;
	}
	| SYM_PROT_VALUE
	{
		$$ = SYM_PROT_VALUE;
	}
	| SYM_PROT_AUTOFOCUS
	{
		$$ = SYM_PROT_AUTOFOCUS;
	}
	| SYM_PROT_MEDIA
	{
		$$ = SYM_PROT_MEDIA;
	}
	| SYM_PROT_COORDS
	{
		$$ = SYM_PROT_COORDS;
	}
	| SYM_PROT_PROFILE
	{
		$$ = SYM_PROT_PROFILE;
	}
	| SYM_PROT_HEADERS
	{
		$$ = SYM_PROT_HEADERS;
	}
	| SYM_PROT_VALUETYPE
	{
		$$ = SYM_PROT_VALUETYPE;
	}
	| SYM_PROT_REPLACE
	{
		$$ = SYM_PROT_REPLACE;
	}
	| SYM_PROT_MARGINHEIGHT
	{
		$$ = SYM_PROT_MARGINHEIGHT;
	}
	| SYM_PROT_BORDER
	{
		$$ = SYM_PROT_BORDER;
	}
	| SYM_PROT_FRAMEBORDER
	{
		$$ = SYM_PROT_FRAMEBORDER;
	}
	| SYM_PROT_ASYNC
	{
		$$ = SYM_PROT_ASYNC;
	}
	| SYM_PROT_FACE
	{
		$$ = SYM_PROT_FACE;
	}
	| SYM_PROT_CELLPADDING
	{
		$$ = SYM_PROT_CELLPADDING;
	}
	| SYM_PROT_STANDBY
	{
		$$ = SYM_PROT_STANDBY;
	}
	| SYM_PROT_ALT
	{
		$$ = SYM_PROT_ALT;
	}
	| SYM_PROT_ACCEPTCHARSET
	{
		$$ = SYM_PROT_ACCEPTCHARSET;
	}
	| SYM_PROT_FORMMETHODNEW
	{
		$$ = SYM_PROT_FORMMETHODNEW;
	}
	| SYM_PROT_AUTOPLAY
	{
		$$ = SYM_PROT_AUTOPLAY;
	}
	| SYM_PROT_REV
	{
		$$ = SYM_PROT_REV;
	}
	| SYM_PROT_LOOP
	{
		$$ = SYM_PROT_LOOP;
	}
	| SYM_PROT_CODE
	{
		$$ = SYM_PROT_CODE;
	}
	| SYM_PROT_SRC
	{
		$$ = SYM_PROT_SRC;
	}
	| SYM_PROT_CHECKED
	{
		$$ = SYM_PROT_CHECKED;
	}
	| SYM_PROT_SCROLLING
	{
		$$ = SYM_PROT_SCROLLING;
	}
	| SYM_PROT_SCOPE
	{
		$$ = SYM_PROT_SCOPE;
	}
	| SYM_PROT_DEFER
	{
		$$ = SYM_PROT_DEFER;
	}
	| SYM_PROT_XMLSPACE
	{
		$$ = SYM_PROT_XMLSPACE;
	}
	| SYM_PROT_CHALLENGE
	{
		$$ = SYM_PROT_CHALLENGE;
	}
	| SYM_PROT_SCHEME
	{
		$$ = SYM_PROT_SCHEME;
	}
	| SYM_PROT_DECLARE
	{
		$$ = SYM_PROT_DECLARE;
	}
	| SYM_PROT_CHAR
	{
		$$ = SYM_PROT_CHAR;
	}
	| SYM_PROT_READONLY
	{
		$$ = SYM_PROT_READONLY;
	}
	| SYM_PROT_XMLLANG
	{
		$$ = SYM_PROT_XMLLANG;
	}
	| SYM_PROT_MAX
	{
		$$ = SYM_PROT_MAX;
	}
	| SYM_PROT_ROWSPAN
	{
		$$ = SYM_PROT_ROWSPAN;
	}
	| SYM_PROT_KEYTYPE
	{
		$$ = SYM_PROT_KEYTYPE;
	}
	| SYM_PROT_AUTOCOMPLETE
	{
		$$ = SYM_PROT_AUTOCOMPLETE;
	}
	| SYM_PROT_SELECTED
	{
		$$ = SYM_PROT_SELECTED;
	}
	| SYM_PROT_CODEBASE
	{
		$$ = SYM_PROT_CODEBASE;
	}
	| SYM_PROT_STEP
	{
		$$ = SYM_PROT_STEP;
	}
	| SYM_PROT_NOHREF
	{
		$$ = SYM_PROT_NOHREF;
	}
	| SYM_PROT_CHARSET
	{
		$$ = SYM_PROT_CHARSET;
	}
	| SYM_PROT_FORMNEW
	{
		$$ = SYM_PROT_FORMNEW;
	}
	| SYM_PROT_FORMENCTYPENEW
	{
		$$ = SYM_PROT_FORMENCTYPENEW;
	}
	| SYM_PROT_REL
	{
		$$ = SYM_PROT_REL;
	}
	| SYM_PROT_MIN
	{
		$$ = SYM_PROT_MIN;
	}
	| SYM_PROT_NAME
	{
		$$ = SYM_PROT_NAME;
	}
	| SYM_PROT_TYPE
	{
		$$ = SYM_PROT_TYPE;
	}
	| SYM_PROT_NOSHADE
	{
		$$ = SYM_PROT_NOSHADE;
	}
	| SYM_PROT_MANIFEST
	{
		$$ = SYM_PROT_MANIFEST;
	}
	| SYM_PROT_ALIGN
	{
		$$ = SYM_PROT_ALIGN;
	}
	| SYM_PROT_HEIGHT
	{
		$$ = SYM_PROT_HEIGHT;
	}
	| SYM_PROT_ACCEPT
	{
		$$ = SYM_PROT_ACCEPT;
	}
	| SYM_PROT_ENCTYPE
	{
		$$ = SYM_PROT_ENCTYPE;
	}
	| SYM_PROT_DISABLED
	{
		$$ = SYM_PROT_DISABLED;
	}
	| SYM_PROT_CONTROLS
	{
		$$ = SYM_PROT_CONTROLS;
	}
	| SYM_PROT_LONGDESC
	{
		$$ = SYM_PROT_LONGDESC;
	}
	| SYM_PROT_MARGINWIDTH
	{
		$$ = SYM_PROT_MARGINWIDTH;
	}
	| SYM_PROT_NORESIZE
	{
		$$ = SYM_PROT_NORESIZE;
	}
	| SYM_PROT_COLS
	{
		$$ = SYM_PROT_COLS;
	}
	| SYM_PROT_SIZE
	{
		$$ = SYM_PROT_SIZE;
	}
	| SYM_PROT_RADIOGROUP
	{
		$$ = SYM_PROT_RADIOGROUP;
	}
	| SYM_PROT_VALIGN
	{
		$$ = SYM_PROT_VALIGN;
	}
	| SYM_PROT_CHAROFF
	{
		$$ = SYM_PROT_CHAROFF;
	}
	| SYM_PROT_LOW
	{
		$$ = SYM_PROT_LOW;
	}
	| SYM_PROT_START
	{
		$$ = SYM_PROT_START;
	}
	| SYM_PROT_SUMMARY
	{
		$$ = SYM_PROT_SUMMARY;
	}
	| SYM_PROT_OPTIMUM
	{
		$$ = SYM_PROT_OPTIMUM;
	}
	| SYM_PROT_ABBR
	{
		$$ = SYM_PROT_ABBR;
	}
	| SYM_PROT_FORM
	{
		$$ = SYM_PROT_FORM;
	}
	| SYM_PROT_LABEL
	{
		$$ = SYM_PROT_LABEL;
	}
	| SYM_PROT_FRAME
	{
		$$ = SYM_PROT_FRAME;
	}
	| SYM_PROT_ALLOWSCRIPTACCESS
	{
		$$ = SYM_PROT_ALLOWSCRIPTACCESS;
	}
	;
%%
static int do_html_external_link_check(html_priv_data *priv, char *url)
{
#ifndef HTML_DEBUG
	if(!priv->virtual_host || !priv->virtual_host->external_link_check)
	{
		YYFPRINTF(stderr, "no need do external link check for virtual host %p %s\n",
			priv->virtual_host, priv->virtual_host?priv->virtual_host->name:"(NULL)");
		return 0;
	}

	if(url[0]=='/')
	{
		YYFPRINTF(stderr, "no need do external check, for the url %s starts with /, which is a local url\n", url);
		return 0;
	}

	if(dp_external_link_match(priv->virtual_host, url))
	{
		YYFPRINTF(stderr, "pass external link check for virtual host: %s, url: %s\n",
			priv->virtual_host->name, url);
		return 0;
	}

	idp_action action;
	memset(&action, 0, sizeof(action));
	action.action_type = priv->virtual_host->external_link_action;
	if(idp_global_mode == IDP_MODE_IPS_LOGONLY)
		action.action_type = IDP_ACTION_LOGONLY;
	idp_external_link_syslog("", priv->full_uri, &action, url);

	priv->clean = 0;
	if(!priv->req_dynamic_page)
		dp_virtual_host_add_black(priv->virtual_host, priv->uri_md5_32bit, url);
	
	if(action.action_type == IDP_ACTION_RESET)
	{
		YYFPRINTF(stderr, "ATTACK: violate against the external link policy, reset session\n");
		idp_do_action(0, action.action_type, IDP_PROTOCOL_HTTP, BLOCK_NONE, 0, priv->pf_id, priv->task_ctx);
		return -1;
	}

	YYFPRINTF(stderr, "ATTACK: violate against the external link policy, continue scan\n");
#endif
	return 0;
}

static int do_html_web_acl_check(html_priv_data *priv, char *url)
{
#ifndef HTML_DEBUG
	if(!priv->virtual_host || !priv->virtual_host->web_acl_check)
	{
		YYFPRINTF(stderr, "no need do web acl check for virtual host %p %s\n",
			priv->virtual_host, priv->virtual_host?priv->virtual_host->name:"(NULL)");
		return 0;
	}

	if(dp_web_acl_match(priv->virtual_host, url)!=WEB_ACL_STATIC)
	{
		YYFPRINTF(stderr, "pass web acl check for virtual host: %s, url: %s\n",
			priv->virtual_host->name, url);
		return 0;
	}

	idp_action action;
	memset(&action, 0, sizeof(action));
	action.action_type = priv->virtual_host->web_acl_action;
	if(idp_global_mode == IDP_MODE_IPS_LOGONLY)
		action.action_type = IDP_ACTION_LOGONLY;
	idp_web_acl_syslog(WEB_ACL_STATIC, &action, "", priv->full_uri);

	priv->clean = 0;
	if(!priv->req_dynamic_page)
		dp_virtual_host_add_black(priv->virtual_host, priv->uri_md5_32bit, NULL);

	if(action.action_type == IDP_ACTION_RESET)
	{
		YYFPRINTF(stderr, "ATTACK: violate against the static web acl policy, reset session\n");
		idp_do_action(0, action.action_type, IDP_PROTOCOL_HTTP, BLOCK_NONE, 0, priv->pf_id, priv->task_ctx);
		return -1;
	}
	YYFPRINTF(stderr, "ATTACK: violate against the static web acl policy, continue scan\n");
#endif
	return 0;
}

static int do_html_frame_check(html_priv_data *priv, html_node_prot_t *prots)
{
	for(; prots && prots->type!=SYM_PROT_SRC; prots=STAILQ_NEXT(prots, next));

	if(!prots && !prots->value)
	{
		YYFPRINTF(stderr, "Error: %s not found\n", prots?"src value":"src prot");
		return 0;
	}

	return do_html_external_link_check(priv, prots->value);
}

static int do_html_iframe_check(html_priv_data *priv, html_node_prot_t *prots)
{
	return do_html_frame_check(priv, prots);
}

static int do_html_script_check(html_priv_data *priv, html_node_prot_t *prots)
{
	return do_html_frame_check(priv, prots);
}

static int do_html_link_check(html_priv_data *priv, html_node_prot_t *prots)
{
	int dyn=0;
	char *url=NULL;
	while(prots)
	{
		if(prots->type==SYM_PROT_TYPE && prots->value && strcasestr(prots->value, "script"))
			dyn=1;
		if(prots->type==SYM_PROT_HREF)
			url=prots->value;
		prots = STAILQ_NEXT(prots, next);
	}

	if(!url)
	{
		YYFPRINTF(stderr, "Error: cannot find src property value\n");
		return 0;
	}

	if(do_html_external_link_check(priv, url) < 0)
		return -1;

	if(!dyn)
		return 0;

	return do_html_web_acl_check(priv, url);
}

static int do_html_applet_check(html_priv_data *priv, html_node_prot_t *prots)
{
	for(; prots && prots->type!=SYM_PROT_CODE; prots=STAILQ_NEXT(prots, next));

	if(!prots && !prots->value)
	{
		YYFPRINTF(stderr, "Error: %s not found\n", prots?"src value":"src prot");
		return 0;
	}

	if(do_html_external_link_check(priv, prots->value) < 0)
		return -1;
	return do_html_web_acl_check(priv, prots->value);
}

static int do_html_object_check(html_priv_data *priv, html_node_prot_t *prots)
{
	for(; prots && prots->type!=SYM_PROT_DATA; prots=STAILQ_NEXT(prots, next));

	if(!prots && !prots->value)
	{
		YYFPRINTF(stderr, "Error: %s not found\n", prots?"src value":"src prot");
		return 0;
	}

	if(do_html_external_link_check(priv, prots->value) < 0)
		return -1;

	return do_html_web_acl_check(priv, prots->value);
}

static int do_html_check(html_priv_data *priv, enum htmltokentype type, html_node_prot_t *prots)
{
	if(!priv || !prots)
	{
		YYFPRINTF(stderr, "bad data priv: %p, prots: %p\n", priv, prots);
		return 0;
	}

	switch(type)
	{
		case SYM_TAG_FRAME:
			return do_html_frame_check(priv, prots);
		case SYM_TAG_IFRAME:
			return do_html_iframe_check(priv, prots);
		case SYM_TAG_APPLET:
			return do_html_applet_check(priv, prots);
		case SYM_TAG_LINK:
			return do_html_link_check(priv, prots);
		case SYM_TAG_OBJECT:
			return do_html_object_check(priv, prots);
		case SYM_TAG_SCRIPT:
			return do_html_script_check(priv, prots);
		default:
			return 0;
	}
}

static void html_tab_print(int tab)
{
	int i;
	for(i=0; i<tab; i++)
		YYFPRINTF(stderr, "  ");
}

static void html_node_print(html_node_t *node, int tab)
{
	html_node_t *child = NULL;
	html_node_prot_t *prot = NULL;

	if(!node)
		return;
	
	if(IS_TAG_NODE(node))
	{
		html_tab_print(tab);
		YYFPRINTF(stderr, "<%s ", get_token_text(node->type));

		STAILQ_FOREACH(prot, &node->prot, next)
			YYFPRINTF(stderr, "%s=\"%s\" ", get_token_text(prot->type), prot->value?prot->value:"");
		YYFPRINTF(stderr, "/>\n");

		STAILQ_FOREACH(child, &node->child, sib)
			html_node_print(child, tab+1);
		
		html_tab_print(tab);
		YYFPRINTF(stderr, "</%s>\n", get_token_text(node->type));
	}
	else
	{
		html_tab_print(tab);
		YYFPRINTF(stderr, "%s\n", node->text?node->text:"");
	}
}

static void html_print(html_priv_data *priv)
{
	html_node_t *node = NULL;

	if(!priv || STAILQ_EMPTY(&priv->html_root))
		return;

	STAILQ_FOREACH(node, &priv->html_root, sib)
		html_node_print(node, 0);
}

static char *html_value_string_decode(char *value)
{
	char *rd=value, *wt=value;
	int num=0;
	char idx=0, again=0;
	if(!value)
		return NULL;
	
AGAIN:
	/*eat starting space*/
	while(isspace(*rd))
		rd++;

	while(*rd)
	{
		if(*rd == '&' && *(rd+1) == '#')
		{
			if((*(rd+2) == 'x' || *(rd+2) == 'X') && isxdigit(*(rd+3)))
			{
				/*for hex num encoding, &#x00061;*/
				idx = 0;
				char c = *(rd+3+idx);
				num = 0;
				while(isxdigit(c))
				{
					if(isdigit(c))
						num = num*16+(c-'0');
					else if(isupper(c))
						num = num*16 + (c-'A'+0x0A);
					else
						num = num*16 + (c-'a'+0x0A);
					idx++;
					c = *(rd+3+idx);
				}
				rd = rd + 3 + idx;
				if(c == ';')
					rd++;
				if(num>255)
					*wt++ = ' ';	/*for unicode, add a simple SPACE*/
				else
					*wt++ = num;
				continue;
			}
			else if(isdigit(*(rd+2)))
			{
				/*for decimal encoding, &#00097;*/
				idx = 0;
				char c = *(rd+2+idx);
				num=0;
				while(isdigit(c))
				{
					num = num*10 + (c-'0');
					idx++;
					c = *(rd+2+idx);
				}
				rd = rd + 2 + idx;
				if(c == ';')
					rd++;
				if(num>255)
					*wt++ = ' ';
				else
					*wt++ = num;
				continue;
			}
			else if(isalpha(*(rd+2)))
			{
				/*for html encoding symbol, we find ';'*/
				idx=0;
				char c = *(rd+2+idx);
				while(c && c != ';')
				{
					idx++;
					c = *(rd+2+idx);
				}
				*wt++ = ' ';
				rd = rd + 2 + idx;
				if(c == ';')
					rd++;
				continue;
			}
			else
			{
				*wt++ = '&';
				rd++;
				continue;
			}
		}
		else
		{
			if(*rd == '\r' || *rd == '\n')
				rd++;
			else
				*wt++ = *rd++;
			continue;
		}
	}
	*wt = '\0';
	if(!again)
	{
		again = 1;
		rd = wt = value;
		num=idx=0;
		goto AGAIN;
	}
	
	YYFPRINTF(stderr, "After decoding, we get property value: %s\n", value);
	return (value);
}

static void htmlpstate_fast_init(html_priv_data *priv)
{
	htmlpstate *yyps = (htmlpstate*)priv->parser;
	if(!yyps->yynew && yyss != yyssa)
		YYSTACK_FREE(yyss);
	/*fast init parser*/
	yyps->yynew = 1;

	/*fast init lexer*/
	html_lex_fast_init(priv->lexer);

	/*fast init result*/
	INIT_SCORE(priv);
	INIT_ERROR(priv);
}

static int parse_html_stream(html_priv_data *priv, const char *buf, size_t buf_len, int last_frag)
{
	htmlpstate *parser = (htmlpstate*)priv->parser;
	yyscan_t lexer = (yyscan_t)priv->lexer;
	YY_BUFFER_STATE input = NULL;
	int token = 0, parser_ret = 0;
	HTMLSTYPE value;

	priv->last_frag = last_frag;
	input = html_scan_stream(buf, buf_len, priv);
	if(!input)
	{
		YYFPRINTF(stderr, "create stream buffer failed\n");
		return 1;
	}

	while(1)
	{
		token = htmllex(&value, lexer);
		if(token == SYM_LEX_CONTINUE)
			break;
		parser_ret = htmlpush_parse(parser, token, &value, (void*)priv);
		if(parser_ret != YYPUSH_MORE)
			break;
	}
	html_delete_buffer(input, lexer);

	YYFPRINTF(stderr, "error: %d, score: %d, xss: %s\n", GET_ERROR(priv), GET_SCORE(priv), FIND_XSS(priv)?"TRUE":"FALSE");
	if(GET_ERROR(priv) || (parser_ret != YYPUSH_MORE && parser_ret != 0) )
	{
		YYFPRINTF(stderr, "find error while parsing\n");
		return 2;
	}
	return 0;
}

int htmlerror(void *this_priv, const char *format, ...)
{
	char buffer[4096];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, args);
	buffer[sizeof(buffer)-1] = '\0';
	YYFPRINTF(stderr, "%s\n", buffer);
	va_end(args);
	ADD_ERROR((html_priv_data*)this_priv);
	return 0;
}

#define MAX_XSS_BUFFER (4096)
int xss_injection_check(const char *buf, int buf_len, int sensitive)
{
	const char html_first_part[] = "<img\tsrc=\x20";
	const char html_second_part[] = "\x20/>";
	char html_buf[MAX_XSS_BUFFER+128];

	if(buf_len >= MAX_XSS_BUFFER)
	{
		YYFPRINTF(stderr, "too long(%d) buffer to check\n", buf_len);
		return 0;
	}
	
	html_priv_data *priv = html_priv_alloc(HTML_FROM_BUFFER);
	if(!priv)
	{
		YYFPRINTF(stderr, "failed to alloc xss check priv data\n");
		return 0;
	}
	
	priv->sensitive = sensitive;

	/*first check buf not in html <img> context*/
	int parse_error = parse_html_stream(priv, buf, buf_len, 1);
	if(!parse_error && FIND_XSS(priv))
	{
		YYFPRINTF(stderr, "find xss in formt0\n");
		return 1;
	}

	int html_len = (sizeof(html_first_part)-1) + buf_len + (sizeof(html_second_part)-1);
	int first_blank = sizeof(html_first_part) - 2;
	int second_blank = (sizeof(html_first_part)-1) + buf_len;
	memcpy(html_buf, html_first_part, sizeof(html_first_part)-1);
	memcpy(html_buf + first_blank + 1, buf, buf_len);
	memcpy(html_buf + second_blank, html_second_part, sizeof(html_second_part)-1);
	
	/*xss check in html <img> context without any quote*/
	htmlpstate_fast_init(priv);
	parse_error = parse_html_stream(priv, html_buf, html_len, 1);
	if(!parse_error && FIND_XSS(priv))
	{
		YYFPRINTF(stderr, "find xss in formt1\n");
		return 1;
	}
	
	/*stat quote in buf*/
	int index=0, find_quote=0, find_dquote=0;
	for(index=0; index<buf_len; index++)
	{
		if(buf[index] == '\'')
			find_quote = 1;
		if(buf[index] == '\"')
			find_dquote = 1;
	}

	YYFPRINTF(stderr, "find_quote: %d, find_dquote: %d\n", find_quote, find_dquote);
	if(find_quote)
	{
		/*xss check in html <img> context with single-quote*/
		htmlpstate_fast_init(priv);
		html_buf[first_blank] = '\'';
		html_buf[second_blank] = '\'';
		parse_error = parse_html_stream(priv, html_buf, html_len, 1);
		if(!parse_error && FIND_XSS(priv))
		{
			YYFPRINTF(stderr, "find xss in formt2\n");
			return 1;
		}
	}

	if(find_dquote)
	{
		/*xss check in html <img> context with double-quote*/
		htmlpstate_fast_init(priv);
		html_buf[first_blank] = '\"';
		html_buf[second_blank] = '\"';
		parse_error = parse_html_stream(priv, html_buf, html_len, 1);
		if(!parse_error && FIND_XSS(priv))
		{
			YYFPRINTF(stderr, "find xss in formt3\n");
			return 1;
		}
	}

	html_priv_free(priv);
	return 0;
}

int parse_html_file(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if(!fp)
	{
		YYFPRINTF(stderr, "open file %s failed\n", filename?filename:"null");
		return 1;
	}

	html_priv_data *priv = html_priv_alloc(HTML_FROM_FILE);
	if(!priv)
	{
		YYFPRINTF(stderr, "alloc priv data failed\n");
		fclose(fp);
		return 2;
	}

	char read_buf[READ_BUF_LEN];
	int last_frag = 0, find_error = 0;
	size_t read_len = 0;
	while(1)
	{
		read_len = fread(read_buf, 1, READ_BUF_LEN, fp);
		if(ferror(fp) || feof(fp))
			last_frag = 1;
		
		find_error = parse_html_stream(priv, read_buf, read_len, last_frag);
		if(last_frag || find_error)
			break;
	}

	html_print(priv);
	html_priv_free(priv);
	fclose(fp);
	return find_error;
}

int parse_html_frags(char **frags, int frags_num)
{
	html_priv_data *priv = html_priv_alloc(HTML_FROM_STREAM);
	if(!priv)
	{
		YYFPRINTF(stderr, "alloc priv data failed\n");
		return 2;
	}
	
	int index;
	int last_frag = 0, find_error = 0;
	for(index=0; index<frags_num; index++)
	{
		last_frag = (index==frags_num-1);
		find_error = parse_html_stream(priv, frags[index], strlen(frags[index]), last_frag);
		if(find_error)
			break;
	}

	html_priv_free(priv);
	return find_error;
}

#ifndef HTML_DEBUG
static int decoder_html_file_init(void *strm, stream_decoder_entry_t *decoder_entry, task_ctx_t *ctx)
{
	stream_refer_t *in_sp = NULL;
	html_priv_data *priv = NULL;
	http_html_strm_priv_t *strm_priv = NULL;
	dp_idp_ruleset *ruleset = NULL;
	dp_virtual_host_t *virtual_host = NULL;
	int idp_pf_id = 0;
	
	/* open input stream */
	in_sp = stream_open(strm, decoder_entry, STREAM_O_RDONLY);
	if (NULL == in_sp) {
		YYFPRINTF(stderr, "Error: failed in openning stream\n");
		goto failed;
	}

	if(stream_get_priv2(in_sp, (void**)&strm_priv) < 0)
	{
		YYFPRINTF(stderr, "Error: failed to get stream private data\n");
		goto failed;
	}
	
	YYFPRINTF(stderr, "magic: %x, md5:%x, virtual host id:%d, full uri:%s\n", 
				strm_priv->magic,strm_priv->uri_md5_32bit, strm_priv->virtual_host_id, in_sp->stream->name);
	if(strm_priv->magic != HTML_DECODER_MAGIC)
	{
		YYFPRINTF(stderr, "Error: bad magic data, not from http response decoder\n");
		goto failed;
	}
	
	idp_pf_id = task_ctx2sub_pf_id(ctx, SUB_PROFILE_IDP);
	ruleset = idp_profile_get_ruleset(idp_pf_id, IDP_PROTOCOL_HTTP);
	virtual_host = dp_get_virtual_host_by_id(ruleset, strm_priv->virtual_host_id);
	if(!virtual_host)
	{
		YYFPRINTF(stderr, "Error: cannot get virtual host by id %d\n", strm_priv->virtual_host_id);
		goto failed;
	}
	
	if(!virtual_host->enable || (!virtual_host->web_acl_check && !virtual_host->external_link_check))
	{
		YYFPRINTF(stderr, "Error: no need do html parser(host %d enable:%d, acl enabled: %d, external enable: %d)\n", 
			virtual_host->virtual_host_id, virtual_host->enable, virtual_host->web_acl_check, virtual_host->external_link_check);
		goto failed;
	}

	/*alloc priv and set it*/
	priv = html_priv_alloc(HTML_FROM_STREAM);
	if(!priv)
	{
		YYFPRINTF(stderr, "Error: alloc html private data failed\n");
		goto failed;
	}

	priv->virtual_host = virtual_host;
	priv->full_uri = in_sp->stream->name;
	priv->virtual_host_id = strm_priv->virtual_host_id;
	priv->pf_id = idp_pf_id;
	sprintf(priv->uri_md5_32bit, "%x", strm_priv->uri_md5_32bit);
	priv->req_dynamic_page = strm_priv->req_dynamic_page;
	priv->task_ctx = ctx;
	priv->in_stream = in_sp;

	decoder_entry->d_state = (void*)priv;
	return 0;

failed:
	if(in_sp)
		stream_close(in_sp, decoder_entry);
	
	if(priv)
		html_priv_free(priv);
	
	return -2;
}

static int decoder_html_file_process(void *strm, stream_decoder_entry_t *decoder_entry, task_ctx_t *ctx)
{
	strm_buf_t *p_buf = NULL;
	html_priv_data *priv = (html_priv_data*)decoder_entry->d_state;
	dp_idp_ruleset *ruleset = idp_profile_get_ruleset(priv->pf_id, IDP_PROTOCOL_HTTP);
	dp_virtual_host_t *virtual_host = dp_get_virtual_host_by_id(ruleset, priv->virtual_host_id);
	char *read_buf = NULL;
	int read_len = 0;
	int ret = STREAM_NORMAL;

	if(!virtual_host)
	{
		YYFPRINTF(stderr, "cannot find virtual host %d\n", priv->virtual_host_id);
		ret = STREAM_CLEAN;
		goto end;
	}

	if(!virtual_host->enable || (!virtual_host->web_acl_check && !virtual_host->external_link_check))
	{
		YYFPRINTF(stderr, "no need do html parser(host %d enable:%d, acl enabled: %d, external enable: %d)\n", 
			virtual_host->virtual_host_id, virtual_host->enable, virtual_host->web_acl_check, virtual_host->external_link_check);
		ret = STREAM_CLEAN;
		goto end;
	}
	priv->virtual_host = virtual_host;
	
	while(1)
	{
		read_buf = stream_read_no_cp(priv->in_stream, -1, &read_len, &p_buf);
		if(read_len > 0)
		{
			stream_flush(priv->in_stream);
			if(parse_html_stream(priv, read_buf, read_len, 0))
			{
				ret = STREAM_CLEAN;
				goto end;
			}
		}
		else if(read_len == 0)
		{
			ret = STREAM_NORMAL;
			goto end;
		}
		else if(read_len == E_STREAM_END)
		{
			if(!parse_html_stream(priv, NULL, 0, 1) && priv->clean)
			{
				YYFPRINTF(stderr, "clean html file %s(%s), add to virtual host %s's white list\n",
					priv->uri_md5_32bit, priv->full_uri, priv->virtual_host->name);
				dp_virtual_host_add_white(priv->virtual_host, priv->uri_md5_32bit);
			}
			ret = STREAM_CLEAN;
			goto end;
		}
		else
		{
			ret = STREAM_CLEAN;
			goto end;
		}
	}
end:
	return ret;
}

static int decoder_html_file_finit(void *strm, stream_decoder_entry_t *decoder_entry, task_ctx_t *ctx)
{
	html_priv_data *priv = (html_priv_data*)decoder_entry->d_state;
	if(priv)
	{
		if(priv->in_stream)
			stream_close(priv->in_stream, decoder_entry);
		html_priv_free(priv);
	}
	return 0;
}

int decoder_html_file_reg()
{
	stream_decoder_t strm_decoder = {
		DECODER_HTML,
		{0},
		STREAM_DO_IDP,
		decoder_html_file_init,
		decoder_html_file_process,
		decoder_html_file_finit,
	};

	strm_decoder_set_intersting_strm(&strm_decoder, STREAM_HTML);
	strm_decoder_reg(&strm_decoder);
	html_mem_init();
	return 0;
}

void decoder_html_file_unreg()
{
	html_mem_finit();
	strm_decoder_unset_intersting_strm(DECODER_HTML);
}
#endif
