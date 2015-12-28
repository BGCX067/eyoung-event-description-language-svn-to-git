#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>

#include "eng_priv.h"


static int ey_symbol_init_simple(ey_engine_t *eng, ey_expr_t *left, ey_expr_t *right, 
	ey_expr_list_t *head, int check_const);
static int ey_symbol_init_su(ey_engine_t *eng, ey_expr_t *left, ey_expr_t *right,
	ey_expr_list_t *head, int check_const);
static int ey_symbol_init_array(ey_engine_t *eng, ey_expr_t *left, ey_expr_t *right,
	ey_expr_list_t *head, int check_const, int allow_undefined);

static unsigned int hash_symbol_name(void *k)
{
	ey_symbol_key_t *key = (ey_symbol_key_t*)k;
	return ((unsigned long)(key->name))>>3;
}

static int compare_symbol(void *k, void *v)
{
	ey_symbol_key_t *key = (ey_symbol_key_t*)k;
	ey_symbol_t *symbol = (ey_symbol_t*)v;

	if(!key || !key->name || !symbol || !symbol->name)
		return 1;
	
	if(key->level == symbol->level && !strcmp(key->name, symbol->name))
		return 0;
	
	return 1;
}

int ey_symbol_init(ey_engine_t *eng)
{
	char hash_name[64];
	if(!ey_ident_hash(eng))
	{
		snprintf(hash_name, 63, "%s ident hash", eng->name);
		hash_name[63] = '\0';
		ey_ident_hash(eng) = ey_hash_create(hash_name, 20, -1, hash_symbol_name, compare_symbol, NULL, NULL);
		if(!ey_ident_hash(eng))
		{
			engine_init_error("create global ident hash failed\n");
			return -1;
		}
	}

	if(!ey_tag_hash(eng))
	{
		snprintf(hash_name, 63, "%s tag hash", eng->name);
		hash_name[63] = '\0';
		ey_tag_hash(eng) = ey_hash_create(hash_name, 10, -1, hash_symbol_name, compare_symbol, NULL, NULL);
		if(!ey_tag_hash(eng))
		{
			engine_init_error("create global tag hash failed\n");
			return -1;
		}
	}

	if(!ey_label_hash(eng))
	{
		snprintf(hash_name, 63, "%s label hash", eng->name);
		hash_name[63] = '\0';
		ey_label_hash(eng) = ey_hash_create(hash_name, 10, -1, hash_symbol_name, compare_symbol, NULL, NULL);
		if(!ey_label_hash(eng))
		{
			engine_init_error("create global label hash failed\n");
			return -1;
		}
	}

	if(!symbol_slab(eng))
	{
		char slab_name[64];
		snprintf(slab_name, sizeof(slab_name)-1, "%s eyoung symbol slab", eng->name);
		slab_name[63] = '\0';
		symbol_slab(eng) = engine_zinit(slab_name, sizeof(ey_symbol_t));
		if(!symbol_slab(eng))
		{
			engine_init_error("init symbol slab failed\n");
			return -1;
		}
	}

	if(!member_slab(eng))
	{
		char slab_name[64];
		snprintf(slab_name, sizeof(slab_name)-1, "%s eyoung member slab", eng->name);
		slab_name[63] = '\0';
		member_slab(eng) = engine_zinit(slab_name, sizeof(ey_member_t));
		if(!member_slab(eng))
		{
			engine_init_error("init member slab failed\n");
			return -1;
		}
	}

	if(!value_fslab(eng))
	{
		char slab_name[64];
		snprintf(slab_name, sizeof(slab_name)-1, "%s eyoung value fslab", eng->name);
		slab_name[63] = '\0';
		value_fslab(eng) = engine_fzinit(slab_name, sizeof(unsigned long), NULL);
		if(!value_fslab(eng))
		{
			engine_init_error("init value fslab failed\n");
			return -1;
		}
	}

	if(!name_fslab(eng))
	{
		char slab_name[64];
		snprintf(slab_name, sizeof(slab_name)-1, "%s eyoung name fslab", eng->name);
		slab_name[63] = '\0';
		name_fslab(eng) = engine_fzinit(slab_name, sizeof(unsigned long), NULL);
		if(!name_fslab(eng))
		{
			engine_init_error("init name fslab failed\n");
			return -1;
		}
	}

	void *true_symbol_value = ey_alloc_symbol_value(eng, ey_int_type(eng));
	if(!true_symbol_value)
	{
		engine_init_error("init true symbol value failed\n");
		return -1;
	}
	*(int*)true_symbol_value = 1;

	char *true_symbol_name = ey_alloc_symbol_name(eng, "true");
	if(!true_symbol_name)
	{
		engine_init_error("init true symbol name failed\n");
		return -1;
	}
	ey_true_symbol(eng) = ey_alloc_symbol(eng, true_symbol_name, 0, SYMBOL_CONST, SYMBOL_STORAGE_NONE, 
								SYMBOL_FLAG_DEFINE, ey_int_type(eng), true_symbol_value, NULL, NULL);
	if(!ey_true_symbol(eng))
	{
		engine_init_error("init true symbol failed\n");
		return -1;
	}

	void *false_symbol_value = ey_alloc_symbol_value(eng, ey_int_type(eng));
	if(!false_symbol_value)
	{
		engine_init_error("init false symbol value failed\n");
		return -1;
	}
	*(int*)false_symbol_value = 0;

	char *false_symbol_name = ey_alloc_symbol_name(eng, "false");
	if(!false_symbol_name)
	{
		engine_init_error("init false symbol name failed\n");
		return -1;
	}
	ey_false_symbol(eng) = ey_alloc_symbol(eng, false_symbol_name, 0, SYMBOL_CONST, SYMBOL_STORAGE_NONE, 
								SYMBOL_FLAG_DEFINE, ey_int_type(eng), false_symbol_value, NULL, NULL);
	if(!ey_false_symbol(eng))
	{
		engine_init_error("init false symbol failed\n");
		return -1;
	}

	void *null_symbol_value = ey_alloc_symbol_value(eng, ey_pvoid_type(eng));
	if(!null_symbol_value)
	{
		engine_init_error("init null symbol value failed\n");
		return -1;
	}
	*(void**)null_symbol_value = NULL;

	char *null_symbol_name = ey_alloc_symbol_name(eng, "null");
	if(!null_symbol_name)
	{
		engine_init_error("init null symbol name failed\n");
		return -1;
	}
	ey_null_symbol(eng) = ey_alloc_symbol(eng, null_symbol_name, 0, SYMBOL_CONST, SYMBOL_STORAGE_NONE, 
								SYMBOL_FLAG_DEFINE, ey_pvoid_type(eng), null_symbol_value, NULL, NULL);
	if(!ey_null_symbol(eng))
	{
		engine_init_error("init null symbol failed\n");
		return -1;
	}
	return 0;
}

void ey_symbol_finit(ey_engine_t *eng)
{
	if(symbol_slab(eng))
		engine_zclear(symbol_slab(eng));
	if(member_slab(eng))
		engine_zclear(member_slab(eng));
	if(value_fslab(eng))
		engine_fzclear(value_fslab(eng));
	if(name_fslab(eng))
		engine_fzclear(name_fslab(eng));
	ey_true_symbol(eng) = NULL;
	ey_false_symbol(eng) = NULL;
	ey_null_symbol(eng) = NULL;
	ey_ident_hash(eng) = NULL;
	ey_tag_hash(eng) = NULL;
	ey_label_hash(eng) = NULL;
}

void ey_free_symbol(ey_engine_t *eng, ey_symbol_t *symbol)
{
	if(!symbol)
		return;
	
	if(symbol->hash)
		ey_remove_symbol(eng, symbol);
	symbol->hash = NULL;

	if(symbol->name)
		engine_fzfree(name_fslab(eng), symbol->name);
	symbol->name = NULL;

	if(symbol->type)
		ey_free_type(eng, symbol->type);
	symbol->type = NULL;

	if(symbol->value)
		engine_fzfree(value_fslab(eng), symbol->value);
	symbol->value = NULL;

	if(symbol->init_value)
		ey_free_expr(eng, symbol->init_value);
	symbol->init_value = NULL;

	engine_zfree(symbol_slab(eng), symbol);
}

void ey_free_symbol_list(ey_engine_t *eng, ey_symbol_list_t *symbol_list)
{
	if(!symbol_list)
		return;
	
	ey_symbol_t *del=NULL, *next=NULL;
	TAILQ_FOREACH_SAFE(del, symbol_list, list_next, next)
	{
		ey_free_symbol(eng, del);
	}
}

void ey_free_member_list(ey_engine_t *eng, ey_member_list_t *member_list)
{
	if(!member_list)
		return;
	
	ey_member_t *del=NULL, *next=NULL;
	TAILQ_FOREACH_SAFE(del, member_list, member_next, next)
	{
		if(del->member)
			ey_free_symbol(eng, del->member);
		engine_zfree(member_slab(eng), del);
	}
}

ey_symbol_t *ey_alloc_symbol(ey_engine_t *eng, char *name, unsigned int level,
	ey_symbol_class_t class, ey_symbol_storage_class_t storage_class,
	unsigned int flag, ey_type_t *type, void *value, ey_expr_t *init_value, ey_location_t *location)
{
	ey_symbol_t *ret = (ey_symbol_t*)engine_zalloc(symbol_slab(eng));
	if(ret)
	{
		memset(ret, 0, sizeof(*ret));
		ret->name = name;
		ret->level = level;
		ret->class = class;
		ret->storage_class = storage_class;
		ret->flag = flag;
		ret->type = type;
		ret->value = value;
		ret->init_value = init_value;
		ret->location = location?*location:default_location;
	}

	return ret;
}

ey_member_t *ey_alloc_member(ey_engine_t *eng, ey_symbol_t *symbol, 
	unsigned int offset, unsigned short bit_start, unsigned short bit_size)
{
	ey_member_t *ret = (ey_member_t*)engine_zalloc(member_slab(eng));
	if(ret)
	{
		memset(ret, 0, sizeof(*ret));
		ret->member = symbol;
		ret->offset = offset;
		ret->bit_start = bit_start;
		ret->bit_size = bit_size;
	}

	return ret;
}

void ey_symbol_print(ey_engine_t *eng, ey_symbol_t *symbol, int tab)
{
	if(!symbol)
		return;
	/*TODO*/
}

void ey_symbol_list_print(ey_engine_t *eng, ey_symbol_list_t *head, int tab)
{
	if(!head)
		return;
	
	ey_symbol_t *symbol = NULL;
	TAILQ_FOREACH(symbol, head, list_next)
		ey_symbol_print(eng, symbol, tab);
}

void ey_member_list_print(ey_engine_t *eng, ey_member_list_t *head, int tab)
{
	if(!head)
		return;
	
	ey_member_t *member = NULL;
	TAILQ_FOREACH(member, head, member_next)
		ey_member_print(eng, member, tab);
}

void ey_member_print(ey_engine_t *eng, ey_member_t *member, int tab)
{
	if(!member)
		return;
	/*TODO*/
}

static int find_symbol_compare(void *k, void *v)
{
	ey_symbol_key_t *key = (ey_symbol_key_t*)k;
	ey_symbol_t *symbol = v;

	if(!key || !key->name || !symbol || !symbol->name)
		return 1;
	
	if(!strcmp(key->name, symbol->name))
		return 0;
	
	return 1;
}

ey_symbol_t *ey_find_symbol(ey_engine_t *eng, char *name, ey_symbol_table_type_t table)
{
	ey_symbol_key_t key={name, 0};
	ey_hash_t hash = NULL;
	if(table == SYMBOL_TABLE_IDENT)
		hash = ey_ident_hash(eng);
	else if(table == SYMBOL_TABLE_TAG)
		hash = ey_tag_hash(eng);
	else if(table == SYMBOL_TABLE_LABEL)
		hash = ey_label_hash(eng);
	
	if(hash)
		return ey_hash_find_ex(hash, &key, find_symbol_compare);
	return NULL;
}

static int find_level_symbol_compare(void *k, void *v)
{
	ey_symbol_key_t *key = (ey_symbol_key_t*)k;
	ey_symbol_t *symbol = v;

	if(!key || !key->name || !symbol || !symbol->name)
		return 1;
	
	if(!strcmp(key->name, symbol->name) && key->level<=symbol->level)
		return 0;
	
	return 1;
}

ey_symbol_t *ey_find_level_symbol(ey_engine_t *eng, char *name, unsigned int level, ey_symbol_table_type_t table)
{
	ey_symbol_key_t key={name, level};
	ey_hash_t hash = NULL;
	if(table == SYMBOL_TABLE_IDENT)
		hash = ey_ident_hash(eng);
	else if(table == SYMBOL_TABLE_TAG)
		hash = ey_tag_hash(eng);
	else if(table == SYMBOL_TABLE_LABEL)
		hash = ey_label_hash(eng);
	
	if(hash)
		return ey_hash_find_ex(hash, &key, find_level_symbol_compare);
	return NULL;
}

int ey_insert_symbol(ey_engine_t *eng, ey_symbol_t *symbol, ey_symbol_table_type_t table)
{
	if(!symbol || !symbol->name || symbol->hash)
		return EY_HASH_BAD_PARAM;
	
	ey_symbol_key_t key = {symbol->name, symbol->level};
	ey_hash_t hash = NULL;
	int ret = 0;

	if(table == SYMBOL_TABLE_IDENT)
		hash = ey_ident_hash(eng);
	else if(table == SYMBOL_TABLE_TAG)
		hash = ey_tag_hash(eng);
	else if(table == SYMBOL_TABLE_LABEL)
		hash = ey_label_hash(eng);
	
	if(!hash)
		return EY_HASH_BAD_PARAM;
	ret = ey_hash_insert(hash, &key, symbol);
	if(!ret)
		symbol->hash = hash;
	return ret;
}

int ey_remove_symbol(ey_engine_t *eng, ey_symbol_t *symbol)
{
	if(!symbol || !symbol->name || !symbol->hash)
		return EY_HASH_BAD_PARAM;

	ey_symbol_key_t key = {symbol->name, symbol->level};
	ey_hash_t hash = symbol->hash;
	int ret = 0;

	ret = ey_hash_remove(hash, &key, NULL);
	if(!ret)
		symbol->hash = NULL;
	
	return ret;
}

static int purge_symbol_compare(void *k, void *v)
{
	ey_symbol_key_t *key = (ey_symbol_key_t*)k;
	ey_symbol_t *symbol = (ey_symbol_t*)v;
	if(!key || !symbol)
		return 1;
	
	if(key->level <= symbol->level)
		return 0;
	
	return 1;
}

int ey_purge_symbol(ey_engine_t *eng, unsigned int level, ey_symbol_table_type_t table)
{
	ey_symbol_key_t key = {NULL, level};
	ey_hash_t hash = NULL;

	if(table == SYMBOL_TABLE_IDENT)
		hash = ey_ident_hash(eng);
	else if(table == SYMBOL_TABLE_TAG)
		hash = ey_tag_hash(eng);
	else if(table == SYMBOL_TABLE_LABEL)
		hash = ey_label_hash(eng);
	
	if(!hash)
		return EY_HASH_BAD_PARAM;
	
	return ey_hash_remove_all(hash, &key, purge_symbol_compare);
}

void* ey_alloc_symbol_value(ey_engine_t *eng, ey_type_t *type)
{
	if(!type || !type->size)
		return NULL;
	
	return engine_fzalloc(type->size, value_fslab(eng));
}

char *ey_alloc_symbol_name(ey_engine_t *eng, const char *name)
{
	if(!eng || !name)
		return NULL;
	
	char *ret = engine_fzalloc(strlen(name)+1, name_fslab(eng));
	if(ret)
		strcpy(ret, name);
	return ret;
}

void ey_free_symbol_value(ey_engine_t *eng, void *value)
{
	if(value)
		engine_fzfree(value_fslab(eng), value);
}

void ey_free_symbol_name(ey_engine_t *eng, char *name)
{
	if(name)
		engine_fzfree(name_fslab(eng), name);
}

unsigned long ey_convert_label(ey_engine_t *eng, ey_symbol_t *const_symbol)
{
	/*TODO: convert a case label into int value*/
	return 0;
}

int ey_symbol_is_zero(ey_engine_t *eng, ey_symbol_t *const_symbol)
{
	assert(const_symbol->type != NULL);
	assert(const_symbol->value != NULL);

	ey_type_t *type = const_symbol->type;
	if(!ey_type_is_scalar(type->type) && type->type!=TYPE_ARRAY)
		return 0;
	
	switch(type->type)
	{
		case TYPE_SIGNED_CHAR:
			return *(signed char*)(const_symbol->value)==0;
		case TYPE_UNSIGNED_CHAR:
			return *(unsigned char*)(const_symbol->value)==0;
		case TYPE_SIGNED_SHORT:
			return *(signed short*)(const_symbol->value)==0;
		case TYPE_UNSIGNED_SHORT:
			return *(unsigned short*)(const_symbol->value)==0;
		case TYPE_SIGNED_INT:
			return *(signed int*)(const_symbol->value)==0;
		case TYPE_UNSIGNED_INT:
			return *(unsigned int*)(const_symbol->value)==0;
		case TYPE_SIGNED_LONG:
			return *(signed long*)(const_symbol->value)==0;
		case TYPE_UNSIGNED_LONG:
			return *(unsigned long*)(const_symbol->value)==0;
		case TYPE_SIGNED_LONG_LONG:
			return *(signed long long*)(const_symbol->value)==0;
		case TYPE_UNSIGNED_LONG_LONG:
			return *(unsigned long long*)(const_symbol->value)==0;
		case TYPE_POINTER:
			return *(void**)(const_symbol->value)==NULL;
		case TYPE_ARRAY:
		default:
			return 0;
	}
	return 0;
}

int ey_symbol_set_storage_class(ey_engine_t *eng, ey_symbol_t *symbol, ey_type_t *type)
{
	assert(symbol!=NULL && symbol->type!=NULL);

	ey_type_t *symbol_type = symbol->type;
	unsigned int qualifier_class = type->qualifier_class;
	
	if(qualifier_class | TYPE_QUALIFIER_RESTRICT)
	{
		ey_parser_set_error(eng, &type->location, "bad type qualifier restrict\n");
		return -1;
	}

	if((symbol_type->qualifier_class | TYPE_QUALIFIER_RESTRICT) && symbol_type->type!=TYPE_POINTER)
	{
		ey_parser_set_error(eng, &type->location, "type qualifier restrict can only be used for pointer\n");
		return -1;
	}

	if(symbol_type->type==TYPE_FUNCTION)
	{
		if(qualifier_class & (TYPE_QUALIFIER_AUTO|TYPE_QUALIFIER_REGISTER))
		{
			ey_parser_set_error(eng, &type->location, "bad storage class for fucntion\n");
			return -1;
		}
	}
	else
	{
		if(qualifier_class & TYPE_QUALIFIER_INLINE)
		{
			ey_parser_set_error(eng, &type->location, "inline cannot be used for variable\n");
			return -1;
		}

		if(symbol->level==0)
		{
			if(qualifier_class & (TYPE_QUALIFIER_AUTO|TYPE_QUALIFIER_REGISTER))
			{
				ey_parser_set_error(eng, &type->location, "bad storage class for global variable\n");
				return -1;
			}
		}
		else if(symbol->class==SYMBOL_FORMAL)
		{
			if(qualifier_class & (TYPE_QUALIFIER_EXTERN|TYPE_QUALIFIER_STATIC|TYPE_QUALIFIER_TYPEDEF))
			{
				ey_parser_set_error(eng, &type->location, "static/extern/typedef storage class cannot be used for formal parameter\n");
				return -1;
			}
		}
	}

	if(qualifier_class | TYPE_QUALIFIER_TYPEDEF)
		symbol->storage_class = SYMBOL_STORAGE_TYPEDEF;
	else if(qualifier_class | TYPE_QUALIFIER_REGISTER)
		symbol->storage_class = SYMBOL_STORAGE_REGISTER;
	else if(qualifier_class | TYPE_QUALIFIER_AUTO)
		symbol->storage_class = SYMBOL_STORAGE_AUTO;
	else if(qualifier_class | TYPE_QUALIFIER_STATIC)
		symbol->storage_class = SYMBOL_STORAGE_STATIC;
	else 
		symbol->storage_class = SYMBOL_STORAGE_EXTERN;

	return 0;
}

int ey_symbol_set_class(ey_engine_t *eng, ey_symbol_t *symbol)
{
	assert(symbol!=NULL && symbol->type!=NULL);

	ey_type_t *symbol_type = symbol->type;
	if(symbol_type->type==TYPE_FUNCTION)
	{
		/*SYMBOL_COMPILED_FUNCTION should be checked while doing ey_symbol_declare*/
		symbol->class = SYMBOL_FUNCTION;
	}
	else
	{
		if(symbol->storage_class==SYMBOL_STORAGE_EXTERN)
			symbol->class = SYMBOL_GLOBAL;
		else if(symbol->storage_class==SYMBOL_STORAGE_TYPEDEF)
			symbol->class = SYMBOL_NAME;
		else
			symbol->class = (symbol->level>0)?(SYMBOL_LOCAL):(SYMBOL_GLOBAL);
	}
	return 0;
}

int ey_symbol_check_type(ey_engine_t *eng, ey_symbol_t *symbol)
{
	assert(symbol!=NULL && symbol->type!=NULL);

	ey_type_t *symbol_type = symbol->type;

	if(symbol_type->type==TYPE_FUNCTION)
	{
		ey_type_t *return_type = symbol_type->function_type.return_type;
		if(return_type->type==TYPE_ARRAY)
		{
			ey_parser_set_error(eng, &symbol->location, "array type cannot be as the return type of a function\n");
			return -1;
		}
		else if(!ey_type_is_declared(eng, return_type))
		{
			ey_parser_set_error(eng, &symbol->location, "function return type is not defined\n");
			return -1;
		}
	}
	else if(symbol_type->type==TYPE_VOID)
	{
		ey_parser_set_error(eng, &symbol->location, "ident type cannot be void\n");
		return -1;
	}
	else if(!ey_type_is_declared(eng, symbol_type))
	{
		if(symbol_type->type==TYPE_ARRAY)
		{
			if(!symbol_type->array_type.undefined)
			{
				ey_parser_set_error(eng, &symbol->location, "array type is not defined\n");
				return -1;
			}
			/*
			else if(!symbol->init_value)
			{
				ey_parser_set_error(eng, &symbol->location, "undefined array, but without initializer\n");
				return -1;
			}
			*/
		}
		else
		{
			ey_parser_set_error(eng, &symbol->location, "ident type is not defined\n");
			return -1;
		}
	}
	return 0;
}

int ey_symbol_is_declare(ey_engine_t *eng, ey_symbol_t *symbol)
{
	assert(symbol!=NULL && symbol->type!=NULL);
	
	switch(symbol->class)
	{
		case SYMBOL_FUNCTION:
		case SYMBOL_COMPILED_FUNCTION:
			return 1;
		case SYMBOL_NAME:
		case SYMBOL_LOCAL:
		case SYMBOL_FORMAL:
		case SYMBOL_COMPILED_GLOBAL:
			return 0;
		case SYMBOL_GLOBAL:
			return symbol->storage_class!=SYMBOL_STORAGE_EXTERN;
		default:
			*(int*)0 = 0;
	}
	return 0;
}

static int ey_symbol_declare_function(ey_engine_t *eng, ey_symbol_t *symbol)
{
	ey_symbol_t *find_symbol = ey_find_level_symbol(eng, symbol->name, 0, SYMBOL_TABLE_IDENT);
	if(!find_symbol)
	{
		symbol->value = ey_alloc_symbol_value(eng, ey_pvoid_type(eng));
		if(!symbol->value)
		{
			ey_parser_set_error(eng, &symbol->location, "alloc function value failed\n");
			return -1;
		}

		void *entry = dlsym(NULL, symbol->name);
		if(entry)
		{
			*(void**)(symbol->value) = entry;
			symbol->class = SYMBOL_COMPILED_FUNCTION;
			ey_symbol_set_flag(symbol, SYMBOL_FLAG_DEFINE);
		}

		if(!ey_insert_symbol(eng, symbol, SYMBOL_TABLE_IDENT))
		{
			ey_parser_set_error(eng, &symbol->location, "insert symbol %s failed\n", symbol->name);
			return -1;
		}

		if(!entry)
		{
			ey_symbol_set_flag(symbol, SYMBOL_FLAG_DECLARE);
			TAILQ_INSERT_TAIL(&ey_undefined_list(eng), symbol, undefined_next);
		}
	}
	else
	{
		if(symbol->storage_class!=find_symbol->storage_class)
		{
			ey_parser_set_error(eng, &symbol->location, "storage class mis-match\n");
			return -1;
		}
		
		if(find_symbol->class!=SYMBOL_FUNCTION && find_symbol->class!=SYMBOL_COMPILED_FUNCTION)
		{
			ey_parser_set_error(eng, &symbol->location, "%s is already defined as %s\n", 
				symbol->name, ey_symbol_class_name(symbol->class));
			return -1;
		}

		if(!ey_type_function_equal(eng, symbol->type, find_symbol->type, 1))
		{
			ey_parser_set_error(eng, &symbol->location, "function type is mis-match with declaration before\n");
			return -1;
		}

		if(find_symbol->class==SYMBOL_COMPILED_FUNCTION)
			symbol->class = SYMBOL_COMPILED_FUNCTION;
	}

	return 0;
}

static int ey_symbol_declare_variable(ey_engine_t *eng, ey_symbol_t *symbol)
{
	ey_symbol_t *find_symbol = ey_find_level_symbol(eng, symbol->name, 0, SYMBOL_TABLE_IDENT);
	if(!find_symbol)
	{
		if(!ey_insert_symbol(eng, symbol, SYMBOL_TABLE_IDENT))
		{
			ey_parser_set_error(eng, &symbol->location, "insert symbol %s failed\n", symbol->name);
			return -1;
		}

		ey_symbol_set_flag(symbol, SYMBOL_FLAG_DECLARE);
		TAILQ_INSERT_TAIL(&ey_undefined_list(eng), symbol, undefined_next);
	}
	else
	{
		if(symbol->storage_class!=find_symbol->storage_class)
		{
			ey_parser_set_error(eng, &symbol->location, "storage class mis-match\n");
			return -1;
		}
		
		if(find_symbol->class!=SYMBOL_GLOBAL && find_symbol->class!=SYMBOL_COMPILED_GLOBAL)
		{
			ey_parser_set_error(eng, &symbol->location, "%s is already defined as %s\n", 
				symbol->name, ey_symbol_class_name(symbol->class));
			return -1;
		}

		if(!ey_type_equal(eng, symbol->type, find_symbol->type, 1, 0))
		{
			ey_parser_set_error(eng, &symbol->location, "function type is mis-match with declaration before\n");
			return -1;
		}

		if(find_symbol->class==SYMBOL_COMPILED_GLOBAL)
			symbol->class = SYMBOL_COMPILED_GLOBAL;
	}

	return 0;
}

int ey_symbol_declare(ey_engine_t *eng, ey_symbol_t *symbol)
{
	assert(symbol!=NULL && symbol->type!=NULL && symbol->name!=NULL);
	assert((symbol->class==SYMBOL_GLOBAL && symbol->storage_class==SYMBOL_STORAGE_EXTERN) || symbol->class==SYMBOL_FUNCTION);

	if(symbol->level)
	{
		ey_parser_set_error(eng, &symbol->location, "variable or function declaration must be outside function\n");
		return -1;
	}
	
	switch(symbol->class)
	{
		case SYMBOL_GLOBAL:
		{
			if(symbol->storage_class!=SYMBOL_STORAGE_EXTERN)
			{
				ey_parser_set_error(eng, &symbol->location, "global declaration need extern storage class\n");
				return -1;
			}
			return ey_symbol_declare_variable(eng, symbol);
		}
		case SYMBOL_FUNCTION:
		{
			if(symbol->storage_class==SYMBOL_STORAGE_NONE)
				symbol->storage_class=SYMBOL_STORAGE_EXTERN;
			return ey_symbol_declare_function(eng, symbol);
		}
		default:
		{
			ey_parser_set_error(eng, &symbol->location, "bad symbol class %s\n", ey_symbol_class_name(symbol->class));
			return -1;
		}
	}
	return 0;
}

static int ey_symbol_define_function(ey_engine_t *eng, ey_symbol_t *symbol)
{
	if(!symbol->value)
	{
		symbol->value = ey_alloc_symbol_value(eng, ey_pvoid_type(eng));
		if(!symbol->value)
		{
			ey_parser_set_error(eng, &symbol->location, "alloc symbol value failed\n");
			return -1;
		}

		*(void**)(symbol->value) = NULL;
	}

	ey_symbol_t *find_symbol = ey_find_level_symbol(eng, symbol->name, 0, SYMBOL_TABLE_IDENT);
	if(!find_symbol)
	{
		if(!ey_insert_symbol(eng, symbol, SYMBOL_TABLE_IDENT))
		{
			ey_parser_set_error(eng, &symbol->location, "insert symbol %s failed\n", symbol->name);
			return -1;
		}
		ey_symbol_set_flag(symbol, SYMBOL_FLAG_DEFINE);
		ey_symbol_set_flag(symbol, SYMBOL_FLAG_DECLARE);
	}
	else
	{
		if(ey_symbol_check_flag(symbol, SYMBOL_FLAG_DEFINE))
		{
			ey_parser_set_error(eng, &symbol->location, "function re-defined, %s-%d:%d-%d:%d\n", print_location(&find_symbol->location));
			return -1;
		}

		if(symbol->storage_class!=find_symbol->storage_class)
		{
			ey_parser_set_error(eng, &symbol->location, "function storage class mis-match, %s-%d:%d-%d:%d\n", print_location(&find_symbol->location));
			return -1;
		}

		if(symbol->class!=find_symbol->class)
		{
			ey_parser_set_error(eng, &symbol->location, "function class mis-match, %s-%d:%d-%d:%d\n", print_location(&find_symbol->location));
			return -1;
		}
		
		if(!ey_type_equal(eng, symbol->type, find_symbol->type, 1, 0))
		{
			ey_parser_set_error(eng, &symbol->location, "function type mis-match, %s-%d:%d-%d:%d\n", print_location(&find_symbol->location));
			return -1;
		}

		ey_remove_symbol(eng, find_symbol);
		TAILQ_REMOVE(&ey_undefined_list(eng), find_symbol, undefined_next);

		if(!ey_insert_symbol(eng, symbol, SYMBOL_TABLE_IDENT))
		{
			ey_parser_set_error(eng, &symbol->location, "insert symbol %s failed\n", symbol->name);
			return -1;
		}
		ey_symbol_set_flag(symbol, SYMBOL_FLAG_DEFINE);
		ey_symbol_set_flag(symbol, SYMBOL_FLAG_DECLARE);
	}
	return 0;
}

static int ey_symbol_define_global_variable(ey_engine_t *eng, ey_symbol_t *symbol)
{
	ey_symbol_t *find_symbol = ey_find_level_symbol(eng, symbol->name, 0, SYMBOL_TABLE_IDENT);
	if(!find_symbol)
	{
		if(!ey_insert_symbol(eng, symbol, SYMBOL_TABLE_IDENT))
		{
			ey_parser_set_error(eng, &symbol->location, "insert symbol %s failed\n", symbol->name);
			return -1;
		}
		ey_symbol_set_flag(symbol, SYMBOL_FLAG_DEFINE);
		ey_symbol_set_flag(symbol, SYMBOL_FLAG_DECLARE);
	}
	else
	{
		if(ey_symbol_check_flag(symbol, SYMBOL_FLAG_DEFINE))
		{
			ey_parser_set_error(eng, &symbol->location, "function re-defined, %s-%d:%d-%d:%d\n", print_location(&find_symbol->location));
			return -1;
		}

		if(symbol->storage_class!=find_symbol->storage_class)
		{
			ey_parser_set_error(eng, &symbol->location, "function storage class mis-match, %s-%d:%d-%d:%d\n", print_location(&find_symbol->location));
			return -1;
		}

		if(symbol->class!=find_symbol->class)
		{
			ey_parser_set_error(eng, &symbol->location, "function class mis-match, %s-%d:%d-%d:%d\n", print_location(&find_symbol->location));
			return -1;
		}

		ey_remove_symbol(eng, find_symbol);
		TAILQ_REMOVE(&ey_undefined_list(eng), find_symbol, undefined_next);

		if(!ey_insert_symbol(eng, symbol, SYMBOL_TABLE_IDENT))
		{
			ey_parser_set_error(eng, &symbol->location, "insert symbol %s failed\n", symbol->name);
			return -1;
		}
		ey_symbol_set_flag(symbol, SYMBOL_FLAG_DEFINE);
		ey_symbol_set_flag(symbol, SYMBOL_FLAG_DECLARE);
	}
	if(symbol->class==SYMBOL_GLOBAL)
		ey_symbol_alloc_global_mem(eng, symbol);
	return 0;
}

static int ey_symbol_define_local_variable(ey_engine_t *eng, ey_symbol_t *symbol)
{
	ey_symbol_t *find_symbol = ey_find_level_symbol(eng, symbol->name, symbol->level, SYMBOL_TABLE_IDENT);
	if(!find_symbol)
	{
		if(!ey_insert_symbol(eng, symbol, SYMBOL_TABLE_IDENT))
		{
			ey_parser_set_error(eng, &symbol->location, "insert symbol %s failed\n", symbol->name);
			return -1;
		}
		ey_symbol_set_flag(symbol, SYMBOL_FLAG_DEFINE);
		ey_symbol_set_flag(symbol, SYMBOL_FLAG_DECLARE);
	}
	else
	{
		ey_parser_set_error(eng, &symbol->location, "local variable %s re-defined, %s-%d:%d-%d:%d\n", 
			symbol->name, print_location(&find_symbol->location));
		return -1;
	}
	if(symbol->class==SYMBOL_LOCAL)
	{
		if(symbol->storage_class!=SYMBOL_STORAGE_STATIC)
			ey_symbol_alloc_local_mem(eng, symbol);
		else
			ey_symbol_alloc_global_mem(eng, symbol);
	}
	return 0;
}

int ey_symbol_define(ey_engine_t *eng, ey_symbol_t *symbol)
{
	assert(symbol!=NULL && symbol->type!=NULL && symbol->name!=NULL);

	switch(symbol->class)
	{
		case SYMBOL_GLOBAL:
		case SYMBOL_COMPILED_GLOBAL:
		{
			if(symbol->storage_class==SYMBOL_STORAGE_EXTERN)
			{
				ey_parser_set_error(eng, &symbol->location, "global definition do not need extern storage class\n");
				return -1;
			}
			return ey_symbol_define_global_variable(eng, symbol);
		}
		case SYMBOL_FUNCTION:
		case SYMBOL_COMPILED_FUNCTION:
		{
			if(symbol->storage_class==SYMBOL_STORAGE_NONE)
				symbol->storage_class=SYMBOL_STORAGE_EXTERN;
			return ey_symbol_define_function(eng, symbol);
		}
		case SYMBOL_LOCAL:
		case SYMBOL_NAME:
		{
			return ey_symbol_define_local_variable(eng, symbol);
		}
		default:
		{
			ey_parser_set_error(eng, &symbol->location, "bad symbol class %s\n", ey_symbol_class_name(symbol->class));
			return -1;
		}
	}
	return 0;
}

void ey_symbol_alloc_global_mem(ey_engine_t *eng, ey_symbol_t *symbol)
{
	/*TODO:*/
}

void ey_symbol_alloc_local_mem(ey_engine_t *eng, ey_symbol_t *symbol)
{
	/*TODO:*/
}

void ey_symbol_alloc_formal_mem(ey_engine_t *eng, ey_symbol_t *symbol)
{
	/*TODO:*/
}

static int ey_symbol_init_simple(ey_engine_t *eng, ey_expr_t *left, ey_expr_t *right, 
	ey_expr_list_t *head, int check_const)
{
	assert(left!=NULL && right!=NULL && head!=NULL);
	assert(left->expr_type!=NULL && right->expr_type!=NULL);

	if(right->opcode==EXPR_OPCODE_INIT_COMPOUND)
	{
		ey_parser_set_error(eng, &right->location, "scalar ident cannot be initialized by list expr\n");
		return -1;
	}

	if(check_const && !right->const_value)
	{
		ey_parser_set_error(eng, &right->location, "const initial value is needed\n");
		return -1;
	}

	if(!ey_type_assignment_compatible(eng, left->expr_type, right->expr_type))
	{
		ey_parser_set_error(eng, &right->location, "assginment in-compatible\n");
		return -1;
	}

	ey_expr_t *expr = ey_alloc_binary_init_expr(eng, EXPR_OPCODE_ASGN, left, right, &right->location);
	if(!expr)
	{
		ey_parser_set_error(eng, &right->location, "alloc assignment expr failed\n");
		return -1;
	}

	TAILQ_INSERT_TAIL(head, expr, link);
	return 0;
}

static int ey_symbol_init_su(ey_engine_t *eng, ey_expr_t *left, ey_expr_t *right,
	ey_expr_list_t *head, int check_const)
{
	assert(left!=NULL && right!=NULL && head!=NULL);
	assert(left->expr_type!=NULL && ey_type_is_su(left->expr_type->type) && left->expr_type->tag_type.descriptor_type!=NULL);
	
	if(right->opcode!=EXPR_OPCODE_INIT_COMPOUND)
	{
		ey_parser_set_error(eng, &right->location, "struct/union type need list expr to init\n");
		return -1;
	}
	
	if(TAILQ_EMPTY(&right->list_expr.head))
		return 0;

	ey_type_t *su_type = left->expr_type->tag_type.descriptor_type;
	ey_member_t *member = NULL;
	ey_expr_t *right_it = TAILQ_FIRST(&right->list_expr.head);

	TAILQ_FOREACH(member, &su_type->su_type.member_list, member_next)
	{
		if(!right_it)
			break;
		
		ey_expr_t *member_expr = NULL;
		ey_expr_t *member_value = NULL;
		if(right_it->opcode==EXPR_OPCODE_INIT_ASGN)
		{
			if(right_it->binary_expr.left->opcode!=EXPR_OPCODE_MEMBER)
			{
				ey_parser_set_error(eng, &right_it->location, "initializer is not a member expr\n");
				return -1;
			}

			member_expr = ey_alloc_member_expr(eng, EXPR_OPCODE_MEMBER, left, 
				right_it->binary_expr.left->member_expr.member, &right_it->location);
			if(!member_expr)
				return -1;
			member_value = right_it->binary_expr.right;
			member = ey_type_find_member(eng, left->expr_type, right_it->binary_expr.left->member_expr.member);
		}
		else
		{
			member_expr = ey_alloc_member_expr(eng, EXPR_OPCODE_MEMBER, left, member->member, &right_it->location);
			if(!member_expr)
				return -1;
			member_value = right_it;
		}
		
		ey_type_t *member_type = member_expr->expr_type;
		assert(member_type !=NULL);
		switch(member_type->type)
		{
			case TYPE_ARRAY:
			{
				if(ey_symbol_init_array(eng, member_expr, member_value, head, check_const, 0))
					return -1;
				break;
			}
			case TYPE_STRUCT_TAG:
			case TYPE_UNION_TAG:
			{
				if(ey_symbol_init_su(eng, member_expr, member_value, head, check_const))
					return -1;
				break;
			}
			case TYPE_SIGNED_CHAR:
			case TYPE_UNSIGNED_CHAR:
			case TYPE_SIGNED_SHORT:
			case TYPE_UNSIGNED_SHORT:
			case TYPE_SIGNED_INT:
			case TYPE_UNSIGNED_INT:
			case TYPE_SIGNED_LONG:
			case TYPE_UNSIGNED_LONG:
			case TYPE_SIGNED_LONG_LONG:
			case TYPE_UNSIGNED_LONG_LONG:
			case TYPE_FLOAT:
			case TYPE_DOUBLE:
			case TYPE_ENUM_TAG:
			case TYPE_POINTER:
			{
				if(ey_symbol_init_simple(eng, member_expr, member_value, head, check_const))
					return -1;
				break;
			}
			default:
			{
				*(int*)0 = 0;
			}
		}
		
		right_it = TAILQ_NEXT(right_it, link);
	}
	return 0;
}

static int ey_symbol_init_array(ey_engine_t *eng, ey_expr_t *left, ey_expr_t *right,
	ey_expr_list_t *head, int check_const, int allow_undefined)
{
	assert(left!=NULL && right!=NULL && head!=NULL);
	assert(left->expr_type!=NULL && left->expr_type->type==TYPE_ARRAY && left->expr_type->array_type.base_type!=NULL);
	
	if(right->opcode!=EXPR_OPCODE_INIT_COMPOUND)
	{
		ey_parser_set_error(eng, &right->location, "array type need list expr to init\n");
		return -1;
	}
	
	ey_type_t *array_type = left->expr_type;
	int is_undefined = array_type->array_type.undefined;
	if(is_undefined && !allow_undefined)
	{
		ey_parser_set_error(eng, &right->location, "undefined array cannot be initialized\n");
		return -1;
	}

	if(TAILQ_EMPTY(&right->list_expr.head))
	{
		if(is_undefined)
		{
			ey_parser_set_error(eng, &right->location, "we cannot define array by empty init list");
			return -1;
		}
		return 0;
	}

	ey_expr_t *right_it = NULL;
	int index=0;

	TAILQ_FOREACH(right_it, &right->list_expr.head, link)
	{
		ey_expr_t *array_expr = NULL;
		ey_expr_t *array_value = NULL;
		if(right_it->opcode==EXPR_OPCODE_INIT_ASGN)
		{
			if(right_it->binary_expr.left->opcode!=EXPR_OPCODE_ARRAY_INDEX)
			{
				ey_parser_set_error(eng, &right_it->location, "initializer is not a array index expr\n");
				return -1;
			}
			
			index=ey_eval_const_expr_value(eng, right_it->binary_expr.left->binary_expr.right);
			if(index<0)
			{
				ey_parser_set_error(eng, &right_it->location, "index %d must be non-negative\n", index);
				return -1;
			}

			if(index+1>array_type->array_type.count)
			{
				if(!is_undefined)
				{
					ey_parser_set_error(eng, &right_it->location, "index %d is beyoung array length %d\n",
						index, array_type->array_type.count);
					return -1;
				}
				array_type->array_type.count = index+1;
				array_type->array_type.undefined = 0;
				array_type->size = array_type->array_type.base_type->size * (index+1);
			}

			array_expr = ey_alloc_binary_expr(eng, EXPR_OPCODE_ARRAY_INDEX, left, 
				right_it->binary_expr.left->binary_expr.right, &right_it->location);
			if(!array_expr)
				return -1;
			array_value = right_it->binary_expr.right;
		}
		else
		{
			if(index+1>array_type->array_type.count)
			{
				if(!is_undefined)
				{
					ey_parser_set_error(eng, &right_it->location, "index %d is beyoung array length %d\n",
						index, array_type->array_type.count);
					return -1;
				}
				array_type->array_type.count = index+1;
				array_type->array_type.undefined = 0;
				array_type->size = array_type->array_type.base_type->size * (index+1);
			}
			
			ey_symbol_t *index_symbol = ey_alloc_symbol(eng, NULL, ey_parser_level(eng), SYMBOL_CONST,
				SYMBOL_STORAGE_NONE, 0, ey_int_type(eng), NULL, NULL, &right_it->location);
			if(!index_symbol)
			{
				ey_parser_set_error(eng, &right_it->location, "alloc index symbol failed\n");
				return -1;
			}

			index_symbol->value = ey_alloc_symbol_value(eng, ey_int_type(eng));
			if(!index_symbol->value)
			{
				ey_parser_set_error(eng, &right_it->location, "alloc index symbol value failed\n");
				return -1;
			}
			*(int*)index_symbol->value = index;
			
			ey_expr_t *index_expr = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, index_symbol, &index_symbol->location);
			if(!index_expr)
			{
				ey_parser_set_error(eng, &right_it->location, "alloc index symbol expr failed\n");
				return -1;
			}

			array_expr = ey_alloc_binary_expr(eng, EXPR_OPCODE_ARRAY_INDEX, left, index_expr, &right_it->location);
			if(!array_expr)
				return -1;
			array_value = right_it;
		}
		
		ey_type_t *base_type = array_type->array_type.base_type;
		switch(base_type->type)
		{
			case TYPE_ARRAY:
			{
				if(ey_symbol_init_array(eng, array_expr, array_value, head, check_const, 0))
					return -1;
				break;
			}
			case TYPE_STRUCT_TAG:
			case TYPE_UNION_TAG:
			{
				if(ey_symbol_init_su(eng, array_expr, array_value, head, check_const))
					return -1;
				break;
			}
			case TYPE_SIGNED_CHAR:
			case TYPE_UNSIGNED_CHAR:
			case TYPE_SIGNED_SHORT:
			case TYPE_UNSIGNED_SHORT:
			case TYPE_SIGNED_INT:
			case TYPE_UNSIGNED_INT:
			case TYPE_SIGNED_LONG:
			case TYPE_UNSIGNED_LONG:
			case TYPE_SIGNED_LONG_LONG:
			case TYPE_UNSIGNED_LONG_LONG:
			case TYPE_FLOAT:
			case TYPE_DOUBLE:
			case TYPE_ENUM_TAG:
			case TYPE_POINTER:
			{
				if(ey_symbol_init_simple(eng, array_expr, array_value, head, check_const))
					return -1;
				break;
			}
			default:
			{
				*(int*)0 = 0;
			}
		}
		
		index++;
	}
	return 0;
}


ey_expr_t *ey_symbol_init_expr(ey_engine_t *eng, ey_symbol_t *symbol)
{
	assert(symbol!=NULL && symbol->type!=NULL && symbol->init_value!=NULL);
	assert(symbol->class==SYMBOL_GLOBAL || symbol->class==SYMBOL_LOCAL);

	int check_const = (symbol->class==SYMBOL_GLOBAL || symbol->storage_class==SYMBOL_STORAGE_STATIC);
	ey_type_t *symbol_type = symbol->type;

	ey_expr_t *symbol_expr = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, symbol, &symbol->location);
	if(!symbol_expr)
	{
		ey_parser_set_error(eng, &symbol->location, "alloc symbol expr failed\n");
		return NULL;
	}

	ey_expr_list_t expr_list;
	TAILQ_INIT(&expr_list);

	switch(symbol_type->type)
	{
		case TYPE_ARRAY:
		{
			if(ey_symbol_init_array(eng, symbol_expr, symbol->init_value, &expr_list, check_const, 1))
				return NULL;
			break;
		}
		case TYPE_STRUCT_TAG:
		case TYPE_UNION_TAG:
		{
			if(ey_symbol_init_su(eng, symbol_expr, symbol->init_value, &expr_list, check_const))
				return NULL;
			break;
		}
		case TYPE_SIGNED_CHAR:
		case TYPE_UNSIGNED_CHAR:
		case TYPE_SIGNED_SHORT:
		case TYPE_UNSIGNED_SHORT:
		case TYPE_SIGNED_INT:
		case TYPE_UNSIGNED_INT:
		case TYPE_SIGNED_LONG:
		case TYPE_UNSIGNED_LONG:
		case TYPE_SIGNED_LONG_LONG:
		case TYPE_UNSIGNED_LONG_LONG:
		case TYPE_FLOAT:
		case TYPE_DOUBLE:
		case TYPE_ENUM_TAG:
		case TYPE_POINTER:
		{
			if(ey_symbol_init_simple(eng, symbol_expr, symbol->init_value, &expr_list, check_const))
				return NULL;
			break;
		}
		default:
		{
			*(int*)0 = 0;
		}
	}

	ey_expr_t *return_expr = ey_alloc_list_expr(eng, EXPR_OPCODE_COMPOUND, &expr_list, &symbol->location);
	if(!return_expr)
		ey_parser_set_error(eng, &symbol->location, "alloc init return expr failed\n");

	return return_expr;
}
