#ifndef ENG_SYM_H
#define ENG_SYM_H 1

#include "ey_queue.h"
#include "ey_memory.h"
#include "eng_location.h"
struct ey_type;
struct ey_expr;

typedef enum ey_symbol_class
{
	SYMBOL_CONST,				/*123,'a',"eyoung"*/
	SYMBOL_ENUM_CONST,			/*enum const*/
	SYMBOL_FUNCTION,			/*function definition*/
	SYMBOL_COMPILED_FUNCTION,	/*compiled function*/
	SYMBOL_GLOBAL,				/*global var*/
	SYMBOL_COMPILED_GLOBAL,		/*global var from elf*/
	SYMBOL_LOCAL,				/*local var*/
	SYMBOL_FORMAL,				/*formal parameter of function*/
	SYMBOL_RESULT,				/*function return value*/
	SYMBOL_TYPE,				/*struct/union/enum type*/
	SYMBOL_MEMBER,				/*struct/union member*/
	SYMBOL_NAME,				/*struct/union/enum type name, typedef name*/
	SYMBOL_LABEL,				/*label*/
}ey_symbol_class_t;

static inline int ey_symbol_is_variable(ey_symbol_class_t t)
{
	switch(t)
	{
		case SYMBOL_GLOBAL:
		case SYMBOL_COMPILED_GLOBAL:
		case SYMBOL_LOCAL:
		case SYMBOL_FORMAL:
			return 1;
		default:
			return 0;
	}
}

static inline const char *ey_symbol_class_name(ey_symbol_class_t t)
{
	switch(t)
	{
		case SYMBOL_CONST:
			return "CONST";
		case SYMBOL_ENUM_CONST:
			return "ENUM_CONST";
		case SYMBOL_FUNCTION:
			return "FUNCTION";
		case SYMBOL_COMPILED_FUNCTION:
			return "COMPILED_FUNCTION";
		case SYMBOL_COMPILED_GLOBAL:
			return "SYMBOL_COMPILED_GLOBAL";
		case SYMBOL_GLOBAL:
			return "GLOBAL";
		case SYMBOL_LOCAL:
			return "LOCAL";
		case SYMBOL_FORMAL:
			return "FORMAL";
		case SYMBOL_RESULT:
			return "RESULT";
		case SYMBOL_TYPE:
			return "TYPE";
		case SYMBOL_MEMBER:
			return "MEMBER";
		case SYMBOL_NAME:
			return "NAME";
		case SYMBOL_LABEL:
			return "LABEL";
		default:
			*(int*)0 = 0;
			return "Unknown";
	}
}

typedef enum ey_symbol_storage_class
{
	SYMBOL_STORAGE_NONE=0,
	SYMBOL_STORAGE_TYPEDEF,
	SYMBOL_STORAGE_EXTERN,
	SYMBOL_STORAGE_STATIC,
	SYMBOL_STORAGE_AUTO,
	SYMBOL_STORAGE_REGISTER
}ey_symbol_storage_class_t;

static inline const char *ey_symbol_storage_class_name(ey_symbol_storage_class_t t)
{
	switch(t)
	{
		case SYMBOL_STORAGE_NONE:
			return "NONE";
		case SYMBOL_STORAGE_EXTERN:
			return "EXTERN";
		case SYMBOL_STORAGE_STATIC:
			return "STATIC";
		case SYMBOL_STORAGE_AUTO:
			return "AUTO";
		case SYMBOL_STORAGE_REGISTER:
			return "REGISTER";
		case SYMBOL_STORAGE_TYPEDEF:
			return "TYPEDEF";
		default:
			*(int*)0 = 0;
			return "Unknown";
	}
}

typedef enum ey_symbol_flag
{
	SYMBOL_FLAG_DEFINE		=0x00000001,
	SYMBOL_FLAG_LHS			=0x00000002,
	SYMBOL_FLAG_DECLARE		=0x00000004,
	SYMBOL_FLAG_ANONYMOUS	=0x00000008,
}ey_symbol_flag_t;

typedef enum ey_symbol_table_type
{
	SYMBOL_TABLE_IDENT		=0x00000001,
	SYMBOL_TABLE_TAG		=0x00000002,
	SYMBOL_TABLE_LABEL		=0x00000004,
}ey_symbol_table_type_t;

typedef struct ey_symbol
{
	/*for symbol list*/
	TAILQ_ENTRY(ey_symbol) list_next;
	TAILQ_ENTRY(ey_symbol) undefined_next;
	
	/*(name, level) are combined keyword in a symbol table of a given namespace*/
	char *name;
	unsigned int level;
	int offset;
	ey_location_t location;

	ey_symbol_class_t class;
	ey_symbol_storage_class_t storage_class;

	unsigned int flag;
	struct ey_type *type;
	void *value;
	struct ey_expr *init_value;
	ey_hash_t hash;	/*in which hash*/
}ey_symbol_t;
typedef TAILQ_HEAD(ey_symbol_list, ey_symbol) ey_symbol_list_t;

typedef struct ey_arg_list
{
	ey_symbol_list_t arg_list;
	int ellipsis;
}ey_arg_list_t;

typedef struct ey_symbol_key
{
	char *name;
	unsigned int level;
}ey_symbol_key_t;

static inline int ey_symbol_check_flag(ey_symbol_t *symbol, ey_symbol_flag_t flag)
{
	return (symbol->flag & flag) ? 1 : 0;
}

static inline void ey_symbol_set_flag(ey_symbol_t *symbol, ey_symbol_flag_t flag)
{
	symbol->flag |= flag;
}

static inline void ey_symbol_unset_flag(ey_symbol_t *symbol, ey_symbol_flag_t flag)
{
	symbol->flag &= (~flag);
}

typedef struct ey_member
{
	TAILQ_ENTRY(ey_member) member_next;
	ey_symbol_t *member;
	unsigned int offset;
	unsigned short bit_start;
	unsigned short bit_size;
}ey_member_t;
typedef TAILQ_HEAD(ey_member_list, ey_member) ey_member_list_t;

typedef struct ey_symbol_const
{
	/*true/false/null symbol*/
	ey_symbol_t *true_symbol;
	ey_symbol_t *false_symbol;
	ey_symbol_t *null_symbol;
}ey_symbol_const_t;

/*free*/
struct ey_engine;
extern void ey_free_symbol(struct ey_engine *eng, ey_symbol_t *symbol);
extern void ey_free_symbol_list(struct ey_engine *eng, ey_symbol_list_t *symbol_list);
extern void ey_free_member_list(struct ey_engine *eng, ey_member_list_t *member_list);

/*alloc*/
extern ey_symbol_t *ey_alloc_symbol(struct ey_engine *eng, char *name, unsigned int level, 
	ey_symbol_class_t class, ey_symbol_storage_class_t storage_class,
	unsigned int flag, struct ey_type *type, void *value, struct ey_expr *init_value, ey_location_t *location);
extern ey_member_t *ey_alloc_member(struct ey_engine *eng, ey_symbol_t *symbol, 
	unsigned int offset, unsigned short bit_start, unsigned short bit_size);

/*symbol table*/
extern ey_symbol_t *ey_find_symbol(struct ey_engine *eng, char *name, ey_symbol_table_type_t table);
extern ey_symbol_t *ey_find_level_symbol(struct ey_engine *eng, char *name, unsigned int level, ey_symbol_table_type_t table);
extern int ey_insert_symbol(struct ey_engine *eng, ey_symbol_t *symbol, ey_symbol_table_type_t table);
extern int ey_remove_symbol(struct ey_engine *eng, ey_symbol_t *symbol);
extern int ey_purge_symbol(struct ey_engine *eng, unsigned int level, ey_symbol_table_type_t table);

/*symbol value*/
struct ey_type;
extern void* ey_alloc_symbol_value(struct ey_engine *eng, struct ey_type *type);
extern void ey_free_symbol_value(struct ey_engine *eng, void *value);
extern unsigned long ey_convert_label(struct ey_engine *eng, ey_symbol_t *const_symbol);
extern int ey_symbol_is_zero(struct ey_engine *eng, ey_symbol_t *const_symbol);

/*symbol name*/
extern char *ey_alloc_symbol_name(struct ey_engine *eng, const char *name);
extern void ey_free_symbol_name(struct ey_engine *eng, char *name);

/*symbol debug*/
extern void ey_symbol_print(struct ey_engine *eng, ey_symbol_t *symbol, int tab);
extern void ey_symbol_list_print(struct ey_engine *eng, ey_symbol_list_t *head, int tab);
extern void ey_member_print(struct ey_engine *eng, ey_member_t *member, int tab);
extern void ey_member_list_print(struct ey_engine *eng, ey_member_list_t *head, int tab);

/*init/finit*/
extern int ey_symbol_init(struct ey_engine *eng);
extern void ey_symbol_finit(struct ey_engine *eng);

/*check*/
struct ey_expr;
extern int ey_symbol_set_storage_class(struct ey_engine *eng, ey_symbol_t *symbol, struct ey_type *type);
extern int ey_symbol_set_class(struct ey_engine *eng, ey_symbol_t *symbol);
extern int ey_symbol_check_type(struct ey_engine *eng, ey_symbol_t *symbol);
extern int ey_symbol_is_declare(struct ey_engine *eng, ey_symbol_t *symbol);
extern int ey_symbol_declare(struct ey_engine *eng, ey_symbol_t *symbol);
extern int ey_symbol_define(struct ey_engine *eng, ey_symbol_t *symbol);
extern struct ey_expr *ey_symbol_init_expr(struct ey_engine *eng, ey_symbol_t *symbol);
extern void ey_symbol_alloc_global_mem(struct ey_engine *eng, ey_symbol_t *symbol);
extern void ey_symbol_alloc_local_mem(struct ey_engine *eng, ey_symbol_t *symbol);
extern void ey_symbol_alloc_formal_mem(struct ey_engine *eng, ey_symbol_t *symbol);
#endif
