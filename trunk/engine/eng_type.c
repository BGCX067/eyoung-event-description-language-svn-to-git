#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "eng_priv.h"

#define complete_type(dst, n, t)			\
	do										\
	{										\
		(dst)->type = (n);					\
		(dst)->size = sizeof(t);			\
		(dst)->alignment = alignment_of(t);	\
	}while(0)

int ey_type_init(ey_engine_t *eng)
{
	if(!type_slab(eng))
	{
		char slab_name[64];
		snprintf(slab_name, 63, "%s type slab", eng->name);
		slab_name[63] = '\0';
		type_slab(eng) = engine_zinit(slab_name, sizeof(ey_type_t));
		if(!type_slab(eng))
		{
			engine_init_error("init type slab failed\n");
			return -1;
		}
	}

	ey_char_type(eng) = ey_alloc_simple_type(eng, TYPE_SIGNED_CHAR, TYPE_QUALIFIER_NORMAL, sizeof(char), alignment_of(char), NULL);
	if(!ey_char_type(eng))
	{
		engine_init_error("init char type failed\n");
		return -1;
	}

	ey_uchar_type(eng) = ey_alloc_simple_type(eng, TYPE_UNSIGNED_CHAR, TYPE_QUALIFIER_NORMAL, sizeof(unsigned char), alignment_of(unsigned char), NULL);
	if(!ey_uchar_type(eng))
	{
		engine_init_error("init unsigned char type failed\n");
		return -1;
	}

	ey_short_type(eng) = ey_alloc_simple_type(eng, TYPE_SIGNED_SHORT, TYPE_QUALIFIER_NORMAL, sizeof(short), alignment_of(short), NULL);
	if(!ey_short_type(eng))
	{
		engine_init_error("init short type failed\n");
		return -1;
	}

	ey_ushort_type(eng) = ey_alloc_simple_type(eng, TYPE_UNSIGNED_SHORT, TYPE_QUALIFIER_NORMAL, sizeof(unsigned short), alignment_of(unsigned short), NULL);
	if(!ey_ushort_type(eng))
	{
		engine_init_error("init unsigned short type failed\n");
		return -1;
	}

	ey_int_type(eng) = ey_alloc_simple_type(eng, TYPE_SIGNED_INT, TYPE_QUALIFIER_NORMAL, sizeof(int), alignment_of(int), NULL);
	if(!ey_int_type(eng))
	{
		engine_init_error("init int type failed\n");
		return -1;
	}

	ey_uint_type(eng) = ey_alloc_simple_type(eng, TYPE_UNSIGNED_INT, TYPE_QUALIFIER_NORMAL, sizeof(unsigned int), alignment_of(unsigned int), NULL);
	if(!ey_uint_type(eng))
	{
		engine_init_error("init unsigned int type failed\n");
		return -1;
	}

	ey_long_type(eng) = ey_alloc_simple_type(eng, TYPE_SIGNED_LONG, TYPE_QUALIFIER_NORMAL, sizeof(long), alignment_of(long), NULL);
	if(!ey_long_type(eng))
	{
		engine_init_error("init long type failed\n");
		return -1;
	}

	ey_ulong_type(eng) = ey_alloc_simple_type(eng, TYPE_UNSIGNED_LONG, TYPE_QUALIFIER_NORMAL, sizeof(unsigned long), alignment_of(unsigned long), NULL);
	if(!ey_ulong_type(eng))
	{
		engine_init_error("init unsigned long type failed\n");
		return -1;
	}

	ey_float_type(eng) = ey_alloc_simple_type(eng, TYPE_FLOAT, TYPE_QUALIFIER_NORMAL, sizeof(float), alignment_of(float), NULL);
	if(!ey_float_type(eng))
	{
		engine_init_error("init float type failed\n");
		return -1;
	}

	ey_double_type(eng) = ey_alloc_simple_type(eng, TYPE_DOUBLE, TYPE_QUALIFIER_NORMAL, sizeof(double), alignment_of(double), NULL);
	if(!ey_double_type(eng))
	{
		engine_init_error("init double type failed\n");
		return -1;
	}

	ey_void_type(eng) = ey_alloc_simple_type(eng, TYPE_VOID, TYPE_QUALIFIER_NORMAL, 0, 0, NULL);
	if(!ey_void_type(eng))
	{
		engine_init_error("init void type failed\n");
		return -1;
	}

	ey_pvoid_type(eng) = ey_alloc_pointer_type(eng, TYPE_POINTER, TYPE_QUALIFIER_NORMAL, sizeof(void*), alignment_of(void*), NULL, ey_void_type(eng));
	if(!ey_pvoid_type(eng))
	{
		engine_init_error("init void* type failed\n");
		return -1;
	}

	ey_pchar_type(eng) = ey_alloc_pointer_type(eng, TYPE_POINTER, TYPE_QUALIFIER_NORMAL, sizeof(char*), alignment_of(char*), NULL, ey_char_type(eng));
	if(!ey_pchar_type(eng))
	{
		engine_init_error("init char* type failed\n");
		return -1;
	}

	ey_const_char_type(eng) = ey_alloc_simple_type(eng, TYPE_SIGNED_CHAR, TYPE_QUALIFIER_CONST, sizeof(char), alignment_of(char), NULL);
	if(!ey_const_char_type(eng))
	{
		engine_init_error("init const char type failed\n");
		return -1;
	}

	ey_enum_const_type(eng) = ey_alloc_simple_type(eng, TYPE_ENUM_CONST, TYPE_QUALIFIER_CONST, sizeof(int), alignment_of(int), NULL);
	if(!ey_enum_const_type(eng))
	{
		engine_init_error("init enum const type failed\n");
		return -1;
	}
	return 0;
}

void ey_type_finit(ey_engine_t *eng)
{
	if(type_slab(eng))
		engine_zclear(type_slab(eng));
	ey_char_type(eng) = NULL;
	ey_uchar_type(eng) = NULL;
	ey_short_type(eng) = NULL;
	ey_ushort_type(eng) = NULL;
	ey_int_type(eng) = NULL;
	ey_uint_type(eng) = NULL;
	ey_long_type(eng) = NULL;
	ey_ulong_type(eng) = NULL;
	ey_float_type(eng) = NULL;
	ey_double_type(eng) = NULL;
	ey_void_type(eng) = NULL;
	ey_pvoid_type(eng) = NULL;
	ey_pchar_type(eng) = NULL;
	ey_const_char_type(eng) = NULL;
	ey_enum_const_type(eng) = NULL;
}

static ey_type_t *ey_alloc_type(ey_engine_t *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location)
{
	ey_type_t *ret = NULL;
	if(type_slab(eng))
	{
		ret = engine_zalloc(type_slab(eng));
		if(ret)
		{
			memset(ret, 0, sizeof(*ret));
			ret->type = type;
			ret->qualifier_class = qualifier_class;
			ret->size = size;
			ret->alignment = alignment;
			ret->location = location?*location:default_location;
		}
	}
	return ret;
}

ey_type_t *ey_alloc_simple_type(ey_engine_t *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location)
{
	ey_type_t *ret = ey_alloc_type(eng, type, qualifier_class, size, alignment, location);
	if(ret)
		ret->class = TYPE_CLASS_SIMPLE;
	return ret;
}

ey_type_t *ey_alloc_array_type(ey_engine_t *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_type_t *base_type, size_t count, int undefined)
{
	ey_type_t *ret = ey_alloc_type(eng, type, qualifier_class, size, alignment, location);
	if(ret)
	{
		ret->class = TYPE_CLASS_ARRAY;
		ret->array_type.base_type = base_type;
		ret->array_type.count = count;
		ret->array_type.undefined = undefined;
	}
	return ret;
}

ey_type_t *ey_alloc_enum_type(ey_engine_t *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_symbol_t *tag, ey_symbol_list_t *const_list)
{
	ey_type_t *ret = ey_alloc_type(eng, type, qualifier_class, size, alignment, location);
	if(ret)
	{
		ret->class = TYPE_CLASS_ENUM;
		ret->enum_type.tag = tag;
		TAILQ_INIT(&ret->enum_type.enum_const_list);
		if(const_list)
			TAILQ_CONCAT(&ret->enum_type.enum_const_list, const_list, list_next);
	}
	return ret;
}

ey_type_t *ey_alloc_function_type(ey_engine_t *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_type_t *return_type, int var_args, ey_symbol_list_t *arg_list)
{
	ey_type_t *ret = ey_alloc_type(eng, type, qualifier_class, size, alignment, location);
	if(ret)
	{
		ret->class = TYPE_CLASS_FUNCTION;
		ret->function_type.return_type = return_type;
		ret->function_type.var_args = var_args;
		TAILQ_INIT(&ret->function_type.arg_list);
		if(arg_list)
			TAILQ_CONCAT(&ret->function_type.arg_list, arg_list, list_next);
	}
	return ret;
}

ey_type_t *ey_alloc_pointer_type(ey_engine_t *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_type_t *deref_type)
{
	ey_type_t *ret = ey_alloc_type(eng, type, qualifier_class, size, alignment, location);
	if(ret)
	{
		ret->class = TYPE_CLASS_POINTER;
		ret->pointer_type.deref_type = deref_type;
	}
	return ret;
}

ey_type_t *ey_alloc_su_type(ey_engine_t *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_symbol_t *tag, ey_member_list_t *member_list)
{
	ey_type_t *ret = ey_alloc_type(eng, type, qualifier_class, size, alignment, location);
	if(ret)
	{
		ret->class = TYPE_CLASS_SU;
		ret->su_type.tag = tag;
		TAILQ_INIT(&ret->su_type.member_list);
		if(member_list)
			TAILQ_CONCAT(&ret->su_type.member_list, member_list, member_next);
	}
	return ret;
}

ey_type_t *ey_alloc_tag_type(ey_engine_t *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_type_t *descriptor)
{
	ey_type_t *ret = ey_alloc_type(eng, type, qualifier_class, size, alignment, location);
	if(ret)
	{
		ret->class = TYPE_CLASS_TAG;
		ret->tag_type.descriptor_type = descriptor;
	}
	return ret;
}

ey_type_t *ey_alloc_typedef_type(ey_engine_t *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_type_t *descriptor)
{
	ey_type_t *ret = ey_alloc_type(eng, type, qualifier_class, size, alignment, location);
	if(ret)
	{
		ret->class = TYPE_CLASS_TYPEDEF;
		ret->typedef_type.descriptor_type = descriptor;
	}
	return ret;
}

void ey_free_type(ey_engine_t *eng, ey_type_t *type)
{
	if(!type)
		return;
	switch(type->class)
	{
		case TYPE_CLASS_ARRAY:
			ey_free_type(eng, type->array_type.base_type);
			break;
		case TYPE_CLASS_ENUM:
			ey_free_symbol(eng, type->enum_type.tag);
			ey_free_symbol_list(eng, &type->enum_type.enum_const_list);
			break;
		case TYPE_CLASS_FUNCTION:
			ey_free_type(eng, type->function_type.return_type);
			ey_free_symbol_list(eng, &type->function_type.arg_list);
			break;
		case TYPE_CLASS_POINTER:
			ey_free_type(eng, type->pointer_type.deref_type);
			break;
		case TYPE_CLASS_SU:
			ey_free_symbol(eng, type->su_type.tag);
			ey_free_member_list(eng, &type->su_type.member_list);
			break;
		case TYPE_CLASS_TAG:
			ey_free_type(eng, type->tag_type.descriptor_type);
			break;
		case TYPE_CLASS_TYPEDEF:
			ey_free_type(eng, type->typedef_type.descriptor_type);
			break;
		default:
			*(int*)0 = 0;
			break;
	}
	engine_zfree(type_slab(eng), type);
}

void ey_type_print(ey_engine_t *eng, ey_type_t *type, int tab)
{
	/*TODO*/
	return;
}

int ey_type_assignment_compatible(ey_engine_t *eng, ey_type_t *dst, ey_type_t *src)
{
	assert(dst!=NULL);
	assert(src!=NULL);
	
	ey_type_type_t dst_type = dst->type;
	ey_type_type_t src_type = src->type;
	if(ey_type_is_arithmetic(dst_type))
	{
		if(ey_type_is_arithmetic(src_type))
			return 1;
	}
	else if(ey_type_is_su(dst_type))
	{
		return ey_type_tag_equal(eng, dst, src, 0);
	}
	else if(ey_type_is_pointer(dst_type))
	{
		if(ey_type_is_pointer_array(src_type))
		{
			return ey_type_pointer_compatible(eng, dst, src, 0);
		}
		else if(src_type==TYPE_FUNCTION)
		{
			if(dst->pointer_type.deref_type->type==TYPE_FUNCTION)	/*function name=>function pointer*/
				return ey_type_function_equal(eng, dst->pointer_type.deref_type, src, 1);
			else if(dst->pointer_type.deref_type->type==TYPE_VOID)  /*function name=>void**/
				return 1;
		}
	}
	
	return 0;
}

int ey_type_pointer_compatible(ey_engine_t *eng, ey_type_t *dst, ey_type_t *src, int compare_qulifier)
{
	assert(dst!=NULL && src!=NULL);

	if(!ey_type_is_pointer_array(dst->type))
		return 0;

	if(!ey_type_is_pointer_array(src->type))
		return 0;

	if(ey_type_is_pointer(dst->type) && dst->pointer_type.deref_type->type==TYPE_VOID)
		return 1;
	if(ey_type_is_pointer(src->type) && src->pointer_type.deref_type->type==TYPE_VOID)
		return 1;
	
	ey_type_t *type1 = ey_type_is_pointer(dst->type)?(dst->pointer_type.deref_type):(dst->array_type.base_type);
	ey_type_t *type2 = ey_type_is_pointer(src->type)?(src->pointer_type.deref_type):(src->array_type.base_type);
	if(compare_qulifier && (type1->qualifier_class | type2->qualifier_class) != type1->qualifier_class)
		return 0;
	
	return ey_type_equal(eng, type1, type2, 0, 1);
}

int ey_type_tag_equal(ey_engine_t *eng, ey_type_t *dst, ey_type_t *src, int compare_qulifier)
{
	assert(dst!=NULL && src!=NULL);

	if(!ey_type_is_tag(dst->type))
		return 0;

	if(!ey_type_is_tag(src->type))
		return 0;

	if(dst->type!=src->type)
		return 0;
	
	if(compare_qulifier && dst->qualifier_class!=src->qualifier_class)
		return 0;
	
	if(!ey_type_is_su(dst->type))
		return 0;
	
	return dst->tag_type.descriptor_type == src->tag_type.descriptor_type;
}

int ey_type_pointer_equal(ey_engine_t *eng, ey_type_t *dst, ey_type_t *src, int compare_qulifier)
{
	assert(dst!=NULL && src!=NULL);
	
	if(!ey_type_is_pointer(dst->type))
		return 0;

	if(!ey_type_is_pointer(src->type))
		return 0;

	if(compare_qulifier && dst->qualifier_class!=src->qualifier_class)
		return 0;
	
	return ey_type_equal(eng, dst->pointer_type.deref_type, src->pointer_type.deref_type, compare_qulifier, 1);
}

int ey_type_function_equal(ey_engine_t *eng, ey_type_t *dst, ey_type_t *src, int compare_qulifier)
{
	assert(dst!=NULL && src!=NULL);
	
	if(dst->type!=TYPE_FUNCTION)
		return 0;

	if(src->type!=TYPE_FUNCTION)
		return 0;

	if(!ey_type_equal(eng, dst->function_type.return_type, src->function_type.return_type, compare_qulifier, 1))
		return 0;
	
	if(dst->function_type.var_args != src->function_type.var_args)
		return 0;
	
	ey_symbol_t *dst_arg=NULL, *src_arg=NULL;
	for(dst_arg=TAILQ_FIRST(&dst->function_type.arg_list), src_arg=TAILQ_FIRST(&src->function_type.arg_list);
		dst_arg!=NULL && src_arg!=NULL;
		dst_arg=TAILQ_NEXT(dst_arg, list_next), src_arg=TAILQ_NEXT(src_arg, list_next))
	{
		if(!ey_type_equal(eng, dst_arg->type, src_arg->type, compare_qulifier, 0))
			break;
	}
	if(dst_arg || src_arg)
		return 0;
	return 1;
}

int ey_type_array_equal(ey_engine_t *eng, ey_type_t *dst, ey_type_t *src, int compare_qulifier, int compare_count)
{
	assert(dst!=NULL && src!=NULL);

	if(dst->type!=TYPE_ARRAY)
		return 0;

	if(src->type!=TYPE_ARRAY)
		return 0;

	if(compare_count && dst->array_type.count!=src->array_type.count)
		return 0;
	
	if(compare_qulifier && dst->qualifier_class!=src->qualifier_class)
		return 0;

	ey_type_t *dst_base = dst->array_type.base_type;
	ey_type_t *src_base = src->array_type.base_type;
	return ey_type_equal(eng, dst_base, src_base, compare_qulifier, compare_count);
}

int ey_type_equal(ey_engine_t *eng, ey_type_t *dst, ey_type_t *src, int compare_qulifier, int compare_count)
{
	assert(dst!=NULL);
	assert(src!=NULL);

	if(dst->type!=src->type)
		return 0;
	if(compare_qulifier && dst->qualifier_class!=src->qualifier_class)
		return 0;
	
	switch(dst->class)
	{
		case TYPE_CLASS_SIMPLE:
			return 1;
		case TYPE_CLASS_ARRAY:
			return ey_type_array_equal(eng, dst, src, compare_qulifier, compare_count);
		case TYPE_CLASS_POINTER:
			return ey_type_pointer_equal(eng, dst, src, compare_qulifier);
		case TYPE_CLASS_FUNCTION:
			return ey_type_function_equal(eng, dst, src, 1);
		case TYPE_CLASS_TAG:
			return ey_type_tag_equal(eng, dst, src, compare_qulifier);
		case TYPE_CLASS_ENUM:
		case TYPE_CLASS_SU:
		case TYPE_CLASS_TYPEDEF:
		default:
			return 0;
	}
}

ey_type_t *ey_type_merge(ey_engine_t *eng, ey_type_t *dst, ey_type_t *src)
{
	assert(dst!=NULL || src!=NULL);

	if(!src)
		return dst;
	if(!dst)
		return src;
	
	ey_type_t *tmp_type = NULL;
	switch(dst->type)
	{
		case TYPE_ARRAY:
		{
			tmp_type = dst->array_type.base_type;
			tmp_type = ey_type_merge(eng, tmp_type, src);
			if(!tmp_type)
				return NULL;
			dst->array_type.base_type = tmp_type;
			dst->size = dst->array_type.count * tmp_type->size;
			dst->alignment = tmp_type->alignment;
			break;
		}
		case TYPE_POINTER:
		{
			tmp_type = dst->pointer_type.deref_type;
			tmp_type = ey_type_merge(eng, tmp_type, src);
			if(!tmp_type)
				return NULL;
			dst->pointer_type.deref_type = tmp_type;
			break;
		}
		case TYPE_FUNCTION:
		{
			tmp_type = dst->function_type.return_type;
			tmp_type = ey_type_merge(eng, tmp_type, src);
			if(!tmp_type)
				return NULL;
			dst->function_type.return_type = tmp_type;
			break;
		}
		default:
		{
			return NULL;
		}
	}
	return dst;
}

ey_type_t *ey_type_normalize(ey_engine_t *eng, ey_type_t *dst)
{
	assert(dst!=NULL);

	ey_type_type_t dst_type = dst->type;
	switch(dst_type)
	{
		case TYPE_CHAR:
		case TYPE_SIGNED_CHAR:
			complete_type(dst, TYPE_SIGNED_CHAR, char);
			break;
		case TYPE_SIGNED_SHORT:
		case TYPE_SHORT:
		case TYPE_SHORT_INT:
		case TYPE_SIGNED_SHORT_INT:
			complete_type(dst, TYPE_SIGNED_SHORT, short);
			break;
		case TYPE_UNSIGNED_SHORT:
		case TYPE_UNSIGNED_SHORT_INT:
			complete_type(dst, TYPE_UNSIGNED_SHORT, short);
			break;
		case TYPE_SIGNED_INT:
		case TYPE_INT:
		case TYPE_SIGNED:
		case TYPE_PESUDO:
			complete_type(dst, TYPE_SIGNED_INT, int);
			break;
		case TYPE_UNSIGNED_INT:
		case TYPE_UNSIGNED:
			complete_type(dst, TYPE_UNSIGNED_INT, int);
			break;
		case TYPE_SIGNED_LONG:
		case TYPE_LONG:
		case TYPE_LONG_INT:
		case TYPE_SIGNED_LONG_INT:
			complete_type(dst, TYPE_SIGNED_LONG, long);
			break;
		case TYPE_UNSIGNED_LONG:
		case TYPE_UNSIGNED_LONG_INT:
			complete_type(dst, TYPE_UNSIGNED_LONG, long);
			break;
		case TYPE_SIGNED_LONG_LONG:
		case TYPE_LONG_LONG:
		case TYPE_LONG_LONG_INT:
		case TYPE_SIGNED_LONG_LONG_INT:
			complete_type(dst, TYPE_SIGNED_LONG_LONG, long long);
			break;
		case TYPE_UNSIGNED_LONG_LONG:
		case TYPE_UNSIGNED_LONG_LONG_INT:
			complete_type(dst, TYPE_UNSIGNED_LONG_LONG, long long);
			break;
		case TYPE_UNSIGNED_CHAR:
		case TYPE_FLOAT:
		case TYPE_DOUBLE:
		case TYPE_VOID:
		case TYPE_STRUCT_TAG:
		case TYPE_ENUM_TAG:
		case TYPE_UNION_TAG:
			break;
		default:
			return NULL;
	}
	return dst;
}

ey_type_t *ey_type_combine(ey_engine_t *eng, ey_type_t *dst, ey_type_t *src)
{
	assert(dst!=NULL && src!=NULL);

	ey_type_type_t dst_type = dst->type;
	ey_type_type_t src_type = src->type;

	switch(dst_type)
	{
		case TYPE_CHAR:
		{
			switch(src_type)
			{
				case TYPE_PESUDO:
					break;
				case TYPE_SIGNED:
					complete_type(dst, TYPE_SIGNED_CHAR, char);
					break;
				case TYPE_UNSIGNED:
					complete_type(dst, TYPE_UNSIGNED_CHAR, char);
					break;
				default:
					return NULL;
			}
		}
		case TYPE_SHORT:
		{
			switch(src_type)
			{
				case TYPE_PESUDO:
					break;
				case TYPE_INT:
					complete_type(dst, TYPE_SHORT_INT, short);
					break;
				case TYPE_UNSIGNED_INT:
					complete_type(dst, TYPE_UNSIGNED_SHORT_INT, short);
					break;
				case TYPE_SIGNED_INT:
					complete_type(dst, TYPE_SIGNED_SHORT_INT, short);
					break;
				case TYPE_SIGNED:
					complete_type(dst, TYPE_SIGNED_SHORT, short);
					break;
				case TYPE_UNSIGNED:
					complete_type(dst, TYPE_UNSIGNED_SHORT, short);
					break;
				default:
					return NULL;
			}
			break;
		}
		case TYPE_INT:
		{
			switch(src_type)
			{
				case TYPE_PESUDO:
					break;
				case TYPE_SIGNED:
					complete_type(dst, TYPE_SIGNED_INT, int);
					break;
				case TYPE_UNSIGNED:
					complete_type(dst, TYPE_UNSIGNED_INT, int);
					break;
				case TYPE_SHORT:
					complete_type(dst, TYPE_SHORT_INT, short);
					break;
				case TYPE_SIGNED_SHORT:
					complete_type(dst, TYPE_SIGNED_SHORT_INT, short);
					break;
				case TYPE_UNSIGNED_SHORT:
					complete_type(dst, TYPE_UNSIGNED_SHORT_INT, short);
					break;
				case TYPE_LONG:
					complete_type(dst, TYPE_LONG_INT, long);
					break;
				case TYPE_SIGNED_LONG:
					complete_type(dst, TYPE_SIGNED_LONG_INT, long);
					break;
				case TYPE_UNSIGNED_LONG:
					complete_type(dst, TYPE_UNSIGNED_LONG_INT, long);
					break;
				case TYPE_LONG_LONG:
					complete_type(dst, TYPE_LONG_LONG_INT, long long);
					break;
				case TYPE_SIGNED_LONG_LONG:
					complete_type(dst, TYPE_SIGNED_LONG_LONG_INT, long long);
					break;
				case TYPE_UNSIGNED_LONG_LONG:
					complete_type(dst, TYPE_UNSIGNED_LONG_LONG_INT, long long);
					break;
				default:
					return NULL;
			}
			break;
		}

		case TYPE_LONG:
		{
			switch(src_type)
			{
				case TYPE_PESUDO:
					break;
				case TYPE_INT:
					complete_type(dst, TYPE_LONG_INT, long);
					break;
				case TYPE_SIGNED_INT:
					complete_type(dst, TYPE_SIGNED_LONG_INT, long);
					break;
				case TYPE_UNSIGNED_INT:
					complete_type(dst, TYPE_UNSIGNED_LONG_INT, long);
					break;
				case TYPE_SIGNED:
					complete_type(dst, TYPE_SIGNED_LONG, long);
					break;
				case TYPE_UNSIGNED:
					complete_type(dst, TYPE_UNSIGNED_LONG, long);
					break;
				case TYPE_LONG:
					complete_type(dst, TYPE_LONG_LONG, long long);
					break;
				case TYPE_SIGNED_LONG:
					complete_type(dst, TYPE_SIGNED_LONG_LONG, long long);
					break;
				case TYPE_UNSIGNED_LONG:
					complete_type(dst, TYPE_UNSIGNED_LONG_LONG, long long);
					break;
				case TYPE_LONG_INT:
					complete_type(dst, TYPE_LONG_LONG_INT, long long);
					break;
				case TYPE_SIGNED_LONG_INT:
					complete_type(dst, TYPE_SIGNED_LONG_LONG_INT, long long);
					break;
				case TYPE_UNSIGNED_LONG_INT:
					complete_type(dst, TYPE_UNSIGNED_LONG_LONG_INT, long long);
					break;
				default:
					return NULL;
			}
			break;
		}

		case TYPE_SIGNED:
		{
			switch(src_type)
			{
				case TYPE_PESUDO:
					break;
				case TYPE_CHAR:
					complete_type(dst, TYPE_SIGNED_CHAR, char);
					break;
				case TYPE_SHORT:
					complete_type(dst, TYPE_SIGNED_SHORT, short);
					break;
				case TYPE_SHORT_INT:
					complete_type(dst, TYPE_SIGNED_SHORT_INT, short);
					break;
				case TYPE_INT:
					complete_type(dst, TYPE_SIGNED_INT, int);
					break;
				case TYPE_LONG:
					complete_type(dst, TYPE_SIGNED_LONG, long);
					break;
				case TYPE_LONG_INT:
					complete_type(dst, TYPE_SIGNED_LONG_INT, long);
					break;
				case TYPE_LONG_LONG:
					complete_type(dst, TYPE_SIGNED_LONG_LONG, long long);
					break;
				case TYPE_LONG_LONG_INT:
					complete_type(dst, TYPE_SIGNED_LONG_LONG_INT, long long);
					break;
				default:
					return NULL;
			}
			break;
		}

		case TYPE_UNSIGNED:
		{
			switch(src_type)
			{
				case TYPE_PESUDO:
					break;
				case TYPE_CHAR:
					complete_type(dst, TYPE_UNSIGNED_CHAR, char);
					break;
				case TYPE_SHORT:
					complete_type(dst, TYPE_UNSIGNED_SHORT, short);
					break;
				case TYPE_SHORT_INT:
					complete_type(dst, TYPE_UNSIGNED_SHORT_INT, short);
					break;
				case TYPE_INT:
					complete_type(dst, TYPE_UNSIGNED_INT, int);
					break;
				case TYPE_LONG:
					complete_type(dst, TYPE_UNSIGNED_LONG, long);
					break;
				case TYPE_LONG_INT:
					complete_type(dst, TYPE_UNSIGNED_LONG_INT, long);
					break;
				case TYPE_LONG_LONG:
					complete_type(dst, TYPE_UNSIGNED_LONG_LONG, long long);
					break;
				case TYPE_LONG_LONG_INT:
					complete_type(dst, TYPE_UNSIGNED_LONG_LONG_INT, long long);
					break;
				default:
					return NULL;
			}
			break;
		}

		case TYPE_VOID:
		case TYPE_FLOAT:
		case TYPE_DOUBLE:
		case TYPE_STRUCT_TAG:
		case TYPE_UNION_TAG:
		case TYPE_ENUM_TAG:
		{
			switch(src_type)
			{
				case TYPE_PESUDO:
					break;
				default:
					return NULL;
			}
			break;
		}

		default:
		{
			return NULL;
		}
	}
	dst->qualifier_class |= src->qualifier_class;
	return dst;
}

ey_type_t *ey_type_integer_promotion(ey_engine_t *eng, ey_type_t *dst)
{
	if(!dst)
		return NULL;
	
	if(!ey_type_is_integer(dst->type))
	{
		ey_parser_set_error(eng, &dst->location, "type %s cannot do integer promotion\n", ey_type_type_name(dst->type));
		return NULL;
	}
	
	if(dst->size < ey_int_type(eng)->size)
	{
		if(ey_type_is_signed_integer(dst->type))
			complete_type(dst, TYPE_SIGNED_INT, int);
		else
			complete_type(dst, TYPE_UNSIGNED_INT, unsigned int);
	}
	return dst;
}

ey_type_t *ey_type_promotion(ey_engine_t *eng, ey_type_t *dst, ey_type_t *src)
{
	ey_type_t *ret = NULL;
	if(!dst || !src)
		goto end;
	
	ey_type_type_t dst_type = dst->type;
	ey_type_type_t src_type = src->type;
	ey_location_t *new_loc = ey_combine_location(eng, &dst->location, &src->location);
	if(!ey_type_is_arithmetic(dst_type) || !ey_type_is_arithmetic(src_type))
	{
		ey_parser_set_error(eng, new_loc, "%s, %s, only arithmetic can do implicit type promotion\n",
			ey_type_type_name(dst_type), ey_type_type_name(src_type));
		goto end;
	}
	
	ret = ey_alloc_simple_type(eng, TYPE_SIGNED_INT, TYPE_QUALIFIER_NORMAL, sizeof(int), alignment_of(int), new_loc);
	if(!ret)
	{
		ey_parser_set_error(eng, new_loc, "alloc implicit type promotion result type failed\n");
		goto end;
	}
	
	if(dst_type==TYPE_DOUBLE || src_type==TYPE_DOUBLE)
	{
		complete_type(ret, TYPE_DOUBLE, double);
		goto end;
	}
	else if(dst_type==TYPE_FLOAT || src_type==TYPE_FLOAT)
	{
		complete_type(ret, TYPE_FLOAT, float);
		goto end;
	}
	else if(dst_type==TYPE_UNSIGNED_LONG_LONG || src_type==TYPE_UNSIGNED_LONG_LONG)
	{
		complete_type(ret, TYPE_UNSIGNED_LONG_LONG, unsigned long long);
		goto end;
	}
	else if(dst_type==TYPE_SIGNED_LONG_LONG || src_type==TYPE_SIGNED_LONG_LONG)
	{
		complete_type(ret, TYPE_LONG_LONG, signed long long);
		goto end;
	}
	else if(dst_type==TYPE_UNSIGNED_LONG || src_type==TYPE_UNSIGNED_LONG)
	{
		complete_type(ret, TYPE_UNSIGNED_LONG, unsigned long);
		goto end;
	}
	else if(dst_type==TYPE_SIGNED_LONG || src_type==TYPE_SIGNED_LONG)
	{
		complete_type(ret, TYPE_SIGNED_LONG, signed long);
		goto end;
	}
	else if(dst_type==TYPE_UNSIGNED_INT || src_type==TYPE_UNSIGNED_INT)
	{
		complete_type(ret, TYPE_UNSIGNED_INT, unsigned int);
		goto end;
	}
	else
	{
		complete_type(ret, TYPE_SIGNED_INT, signed int);
		goto end;
	}

end:
	return ret;
}

int ey_type_has_bitwise_member(ey_engine_t *eng, ey_type_t *su_type)
{
	assert(su_type!=NULL && (su_type->type==TYPE_STRUCT || su_type->type==TYPE_UNION));

	ey_member_t *member = NULL;
	TAILQ_FOREACH(member, &su_type->su_type.member_list, member_next)
	{
		if(member->bit_size)
			return 1;
	}

	return 0;
}

ey_type_t *ey_type_set_union_type(ey_engine_t *eng, ey_type_t *su_type)
{
	int size = 0;
	int alignment = 0;
	ey_member_t *member = NULL;

	assert(su_type!=NULL && su_type->type==TYPE_UNION);

	if(ey_type_is_declared(eng, su_type))
		return su_type;

	TAILQ_FOREACH(member, &su_type->su_type.member_list, member_next)
	{
		ey_type_t *member_type = member->member->type;
		int member_size = member_type->size;
		int member_alignment = member_type->alignment;
		
		if(member->bit_size)
		{
			if(!ey_type_is_integer(member_type->type))
			{
				ey_parser_set_error(eng, &member->member->location, "bitwise member %s type %s is not integer\n",
					member->member->name, ey_type_type_name(member_type->type));
				return NULL;
			}

			if(member->bit_size > member_type->size*CHAR_BIT)
			{
				ey_parser_set_error(eng, &member->member->location, "bitwise member %s type %s is not integer\n",
					member->member->name, ey_type_type_name(member_type->type));
				return NULL;
			}
		}
		else
		{
			if(!ey_type_is_declared(eng, member_type))
			{
				int is_last = (NULL == TAILQ_NEXT(member, member_next));
				if(!is_last)
				{
					ey_parser_set_error(eng, &member->member->location, "member %s type is not defined\n", member->member->name);
					return NULL;
				}

				if(member_type->type != TYPE_ARRAY)
				{
					ey_parser_set_error(eng, &member->member->location, "only incomplete array type can be as the type of last member\n");
					return NULL;
				}

				if(!ey_type_is_declared(eng, member_type->array_type.base_type))
				{
					ey_parser_set_error(eng, &member->member->location, "base type is not defined\n");
					return NULL;
				}

				assert(member_type->array_type.count==0 || member_type->array_type.undefined);
			}

			if(member_type->type==TYPE_FUNCTION)
			{
				ey_parser_set_error(eng, &member->member->location, "member %s type is function\n", ey_type_type_name(member_type->type));
				return NULL;
			}
		}

		member->offset = 0;
		if(member_size > size)
			size = member_size;

		if(member_alignment > alignment)
			alignment = member_alignment;
	}

	su_type->size = align_offset(size, alignment);
	su_type->alignment = alignment;
	return su_type;
}

ey_type_t *ey_type_set_struct_type(ey_engine_t *eng, ey_type_t *su_type)
{
	int offset = 0;
	int bit_offset = 0;
	int alignment = 0;
	int size = 0;
	ey_member_t *member = NULL;

	assert(su_type!=NULL && su_type->type==TYPE_STRUCT);

	if(ey_type_is_declared(eng, su_type))
		return su_type;
	
	TAILQ_FOREACH(member, &su_type->su_type.member_list, member_next)
	{
		ey_type_t *member_type = member->member->type;
		int member_size = member_type->size;
		int member_alignment = member_type->alignment;
		if(member->bit_size)
		{
			if(!ey_type_is_integer(member_type->type))
			{
				ey_parser_set_error(eng, &member->member->location, "bitwise member %s type %s is not integer\n",
					member->member->name, ey_type_type_name(member_type->type));
				return NULL;
			}

			if(member->bit_size > member_type->size*CHAR_BIT)
			{
				ey_parser_set_error(eng, &member->member->location, "bitwise member %s type %s is not integer\n",
					member->member->name, ey_type_type_name(member_type->type));
				return NULL;
			}

			int next_boudary = 0;
			if(bit_offset)
				next_boudary = align_offset((offset+((bit_offset-1)/CHAR_BIT+1)), member_alignment);
			else
				next_boudary = align_offset(offset, member_alignment);

			if((((next_boudary*CHAR_BIT)-(bit_offset+(CHAR_BIT*offset)))>=member->bit_size) && (member->bit_size>0))
			{
				bit_offset += (offset-next_boudary+member_alignment)*CHAR_BIT;
				offset = next_boudary-member_alignment;
			}
			else
			{
				offset = next_boudary;
				bit_offset = 0;
			}

			member->bit_start = bit_offset;
			member->offset = offset;
			bit_offset += member->bit_size;

			if(offset+member_size > size)
				size = offset+member_size;
			if(member_alignment>alignment)
				alignment = member_alignment;
		}
		else
		{
			if(!ey_type_is_declared(eng, member_type))
			{
				int is_last = (NULL == TAILQ_NEXT(member, member_next));
				if(!is_last)
				{
					ey_parser_set_error(eng, &member->member->location, "member %s type is not defined\n", member->member->name);
					return NULL;
				}

				if(member_type->type != TYPE_ARRAY)
				{
					ey_parser_set_error(eng, &member->member->location, "only incomplete array type can be as the type of last member\n");
					return NULL;
				}

				if(!ey_type_is_declared(eng, member_type->array_type.base_type))
				{
					ey_parser_set_error(eng, &member->member->location, "base type is not defined\n");
					return NULL;
				}

				assert(member_type->array_type.count==0 || member_type->array_type.undefined);
			}

			if(member_type->type==TYPE_FUNCTION)
			{
				ey_parser_set_error(eng, &member->member->location, "member %s type is function\n", ey_type_type_name(member_type->type));
				return NULL;
			}

			if(bit_offset)
			{
				offset += ((bit_offset - 1) / CHAR_BIT + 1);
				bit_offset = 0;
			}

			offset = align_offset(offset, member_alignment);
			member->offset = offset;
			offset += member_size;
			member->bit_start = 0;
			member->bit_size = 0;

			if(offset>size)
				size = offset;

			if (member_alignment > alignment)
				alignment = member_alignment;
		}
	}

	su_type->size = align_offset(size, alignment);
	su_type->alignment = alignment;
	return su_type;
}

int ey_type_is_declared(ey_engine_t *eng, ey_type_t *type)
{
	if(!type)
		return 0;
	if(type->size || type->type==TYPE_VOID || type->type==TYPE_FUNCTION)
		return 1;
	return 0;
}

ey_member_t *ey_type_find_member(ey_engine_t *eng, ey_type_t *su_tag, ey_symbol_t *member_symbol)
{
	assert(su_tag!=NULL && ey_type_is_su(su_tag->type));
	assert(member_symbol!=NULL && member_symbol->name!=NULL);
	
	ey_type_t *su_type = su_tag->tag_type.descriptor_type;
	assert(su_type!=NULL);

	ey_member_t *ret = NULL;
	TAILQ_FOREACH(ret, &su_type->su_type.member_list, member_next)
	{
		if(ret->member && ret->member->name && !strcmp(ret->member->name, member_symbol->name))
			break;
	}
	return ret;
}

ey_type_t *ey_type_copy(ey_engine_t *eng, ey_type_t *type)
{
	assert(type!=NULL);

	ey_type_t *ret = ey_alloc_type(eng, type->type, type->qualifier_class,
		type->size, type->alignment, &type->location);
	if(!ret)
		return NULL;
	
	switch(type->class)
	{
		case TYPE_CLASS_SIMPLE:
		{
			break;
		}
		case TYPE_CLASS_POINTER:
		{
			ret->pointer_type.deref_type = type->pointer_type.deref_type;
			break;
		}
		case TYPE_CLASS_TAG:
		{
			ret->tag_type.descriptor_type = type->tag_type.descriptor_type;
			break;
		}
		case TYPE_CLASS_TYPEDEF:
		{
			ret->typedef_type.descriptor_type = type->typedef_type.descriptor_type;
			break;
		}
		case TYPE_CLASS_ARRAY:
		{
			ret->array_type.base_type = type->array_type.base_type;
			break;
		}
		case TYPE_CLASS_ENUM:
		case TYPE_CLASS_FUNCTION:
		case TYPE_CLASS_SU:
		default:
		{
			return NULL;
		}
	}
	return ret;
}

ey_type_t *ey_type_array2pointer(ey_engine_t *eng, ey_type_t *type)
{
	assert(type!=NULL && type->type==TYPE_ARRAY && type->array_type.base_type!=NULL);

	ey_type_t *ret = NULL;
	ey_type_t *deref_type = NULL;

	deref_type = ey_type_copy(eng, type->array_type.base_type);
	if(!deref_type)
		return NULL;
	
	ret = ey_alloc_pointer_type(eng, TYPE_POINTER, TYPE_QUALIFIER_NORMAL, 
		sizeof(char*), alignment_of(char*), &type->location, deref_type);
	if(!ret)
		return NULL;

	return ret;
}
