#ifndef HTML_MEM_H
#define HTML_MEM_H 1

/*memory mgt system init api for system initializing*/
struct html_priv_data;
extern void html_mem_init();
extern void html_mem_finit();

/*slab mgt api*/
struct html_node;
struct html_node_prot;
extern struct html_node* html_alloc_node(struct html_priv_data *priv);
extern struct html_node_prot* html_alloc_prot(struct html_priv_data *priv);

extern struct html_priv_data* html_priv_alloc(int source);
extern void html_priv_free(struct html_priv_data *priv);

#ifndef HTML_DEBUG
	#include "dp_common.h"
	#include "core_drv.h"
	#include "dp_cvmx_mem.h"
	#include "mem_slab.h"
#else
	#include <stdlib.h>
	#define MEM_MODULE_HTML_DECODER 0
	#define MEM_PRIO_LOW 0
	typedef int* mem_slab_t;

	static inline void* MODULE_KMALLOC(size_t size, int from, int pri, const char *file, const char *func, int line)
	{
		return malloc(size);
	}

	static inline void* MODULE_KREALLOC2(void *ptr, size_t size, int from, int pri, const char *file, const char *func, int line)
	{
		return realloc(ptr,size);
	}

	static inline void KFREE(void *p)
	{
		free(p);
	}

	static inline void *zalloc(mem_slab_t slab)
	{
		char *ret=NULL;
		if(slab && *slab>0)
			ret = malloc(*slab);
		return ret;
	}

	static inline void zfree(mem_slab_t slab, void *p)
	{
		free(p);
	}

	static inline mem_slab_t zinit(char *name, int size, int entry, int flag, int zalloc)
	{
		mem_slab_t slab = (mem_slab_t)malloc(sizeof(*slab));
		if(slab)
			*slab = size;

		return slab;
	}

	static inline void zfinit(mem_slab_t slab)
	{
		if(slab)
			free(slab);
	}

	static inline void zclear(mem_slab_t slab)
	{
		return;
	}
#endif

/*KMALLOC/KFREE*/
#define html_malloc(sz) MODULE_KMALLOC(sz,MEM_MODULE_HTML_DECODER,MEM_PRIO_LOW,(char*)__FILE__,(char*)__FUNCTION__,__LINE__)
#define html_realloc(ptr,sz) MODULE_KREALLOC2(ptr,sz,MEM_MODULE_HTML_DECODER,MEM_PRIO_LOW,(char*)__FILE__,(char*)__FUNCTION__,__LINE__)
#define html_free(p) KFREE(p)

/*for slab*/
#define html_zalloc(slab) zalloc(slab)
#define html_zfree(slab,p) zfree(slab,p)
#define html_zclear(slab) zclear(slab)
#define html_zfinit(slab) zfinit(slab)
#define html_zinit(name,size,entry,flag,zalloc) zinit((name),(size),(entry),(flag),(zalloc))

#endif
