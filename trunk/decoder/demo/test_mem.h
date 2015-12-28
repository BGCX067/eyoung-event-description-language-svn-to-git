#ifndef TEST_MEM_H
#define TEST_MEM_H

#include <stdlib.h>

static inline void* test_malloc(size_t size)
{
	return malloc(size);
}

static inline void test_free(void *ptr)
{
	free(ptr);
}

static inline void* test_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

#endif
