#include <string.h>
#include "queue.h"
#include "html.h"
#include "html_mem.h"
#include "html_lex.h"
#include "html_parser.h"

#ifdef HTML_DEBUG
#define FLOW_SHARED
#endif
FLOW_SHARED static mem_slab_t html_priv_slab;
FLOW_SHARED static mem_slab_t html_node_slab;
FLOW_SHARED static mem_slab_t html_node_prot_slab;

/*for memory system init*/
void html_mem_init()
{
	html_priv_slab = html_zinit("html priv data", sizeof(html_priv_data), 0, 0, 0);
	html_node_slab = html_zinit("html node", sizeof(html_node_t), 0, 0, 0);
	html_node_prot_slab = html_zinit("html node property", sizeof(html_node_prot_t), 0, 0, 0);
}

void html_mem_finit()
{
	html_zfinit(html_priv_slab);
	html_zfinit(html_node_slab);
	html_zfinit(html_node_prot_slab);
}

/*for slab mgt*/
struct html_node* html_alloc_node(struct html_priv_data *priv)
{
	struct html_node *ret = (struct html_node*)html_zalloc(html_node_slab);
	if(!ret)
		return NULL;
	
	YYFPRINTF(stderr, "alloc node\n");
	memset(ret, 0, sizeof(*ret));
	STAILQ_INIT(&ret->prot);
	STAILQ_INIT(&ret->child);
	SLIST_INSERT_HEAD(&priv->free_node, ret, free_list);
	return ret;
}

struct html_node_prot* html_alloc_prot(struct html_priv_data *priv)
{
	struct html_node_prot *ret = (struct html_node_prot*)html_zalloc(html_node_prot_slab);
	if(!ret)
		return NULL;

	YYFPRINTF(stderr, "alloc prot\n");
	memset(ret, 0, sizeof(*ret));
	SLIST_INSERT_HEAD(&priv->free_prot, ret, free_list);
	return ret;
}

void html_priv_free(struct html_priv_data *priv)
{
	if(!priv)
		return;

	html_node_t *node=NULL, *tmp_node=NULL;
	html_node_prot_t *prot=NULL, *tmp_prot=NULL;
	htmlpstate *parser = (htmlpstate*)priv->parser;
	yyscan_t lexer = (yyscan_t)priv->lexer;

	/*free node*/
	SLIST_FOREACH_SAFE(node, &priv->free_node, free_list, tmp_node)
	{
		if(node->text)
			html_free(node->text);
		html_zfree(html_node_slab, node);
	}

	/*free prot*/
	SLIST_FOREACH_SAFE(prot, &priv->free_prot, free_list, tmp_prot)
	{
		if(prot->value)
			html_free(prot->value);
		html_zfree(html_node_prot_slab, prot);
	}

	/*free last_string*/
	if(priv->last_string)
	{
		html_free(priv->last_string);
		priv->last_string = NULL;
	}

	/*free lexer*/
	if(lexer)
	{
		htmllex_destroy(lexer);
		priv->lexer = NULL;
	}

	/*free parser*/
	if(parser)
	{
		htmlpstate_delete(parser);
		priv->parser = NULL;
	}
	
	/*free parser itself*/
	html_zfree(html_priv_slab, priv);
}

html_priv_data* html_priv_alloc(int source)
{
	htmlpstate *parser = NULL;
	yyscan_t lexer = NULL;
	html_priv_data *priv = NULL;
	
	parser = htmlpstate_new();
	if(!parser)
	{
		YYFPRINTF(stderr, "alloc parser failed\n");
		goto failed;
	}

	if(htmllex_init(&lexer))
	{
		YYFPRINTF(stderr, "alloc lexer failed\n");
		goto failed;
	}
	
	priv = html_zalloc(html_priv_slab);
	if(!priv)
	{
		YYFPRINTF(stderr, "alloc html priv data failed\n");
		goto failed;
	}

	memset(priv, 0, sizeof(*priv));
	STAILQ_INIT(&priv->html_root);
	SLIST_INIT(&priv->free_node);
	SLIST_INIT(&priv->free_prot);
	priv->parser = (void*)parser;
	priv->lexer = (void*)lexer;
	htmlset_extra((void*)priv, lexer);
	priv->source = source;
	priv->clean = 1;
	INIT_SCORE(priv);
	INIT_ERROR(priv);

	if(priv->source==HTML_FROM_FILE)
		priv->copy=1;
	return priv;

failed:
	if(parser)
		htmlpstate_delete(parser);
	if(lexer)
		htmllex_destroy(lexer);
	if(priv)
		html_zfree(html_priv_slab, priv);
	return NULL;
}
