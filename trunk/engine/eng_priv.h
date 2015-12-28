#ifndef ENG_PRIV_H
#define ENG_PRIV_H 1

#include <stdio.h>

#include "ey_memory.h"
#include "eng_location.h"
#include "eng_expr.h"
#include "eng_sym.h"
#include "eng_type.h"
#include "eng_stmt.h"
#include "eng_parser.h"
#include "eng_mem.h"
#include "eng_util.h"
#include "eng_rule_parser.h"

typedef struct ey_engine
{
	char name[64];
	/*
	 * for enum
	 * */
	int enum_value;
	/*
	 * for ey_expr
	 * */
	int expr_id;
	ey_slab_t expr_slab;
	ey_expr_const_t expr_const;

	/*
	 * for ey_stmt
	 * */
	ey_slab_t stmt_slab;

	/*
	 * for ey_type
	 * */
	ey_slab_t type_slab;
	ey_type_const_t type_const;

	/*
	 * for ey_sym
	 * */
	ey_slab_t symbol_slab;
	ey_slab_t member_slab;
	ey_fslab_t value_fslab;
	ey_fslab_t name_fslab;
	ey_symbol_const_t symbol_const;
	ey_symbol_list_t undefined_list;

	/*parser*/
	unsigned int switch_level;
	unsigned int loop_level;
	ey_stmt_list_t label_list;
	ey_fslab_t filename_fslab;
	ey_hash_t filename_hash;
	ey_parser_stack_t parser_stack;

	/*for function*/
	ey_symbol_list_t func_undefined_label;
	ey_type_t *return_type;
	ey_type_t *declaration_type;

	/*
	 * IDENT SYMBOL TABLE:
	 * 1, global var/function/enum-const
	 * 2, local var/enum-const while parsing
	 *	but all local var/enum-const will be cleared after parsing
	 * */
	ey_hash_t ident_hash;

	/*
	 * TYPE NAME SYMBOL TABLE:
	 * 1, global struct/union/enum name
	 * 2, local/nested struct/union/enum tag name,
	 *	but all local/nestd struct/union/enum will be cleared after parsing current BLOCK
	 * */
	ey_hash_t tag_hash;

	/*
	 * LABEL NAME SYMBOL TABLE:
	 *   label name symbol will be inserted into this hash table, 
	 *   but after leaving currently parsing function, 
	 *   all label symbol must be cleared from this table.
	 *   LABEL HAS FUNCTION SCOPE
	 * */
	ey_hash_t label_hash;
}ey_engine_t;

/*
 * expr access
 * */
#define expr_slab(eng) (((ey_engine_t*)(eng))->expr_slab)
#define ey_expr_id(eng) (((ey_engine_t*)(eng))->expr_id)
#define ey_true_expr(eng) (((ey_engine_t*)(eng))->expr_const.true_expr)
#define ey_false_expr(eng) (((ey_engine_t*)(eng))->expr_const.false_expr)
#define ey_null_expr(eng) (((ey_engine_t*)(eng))->expr_const.null_expr)

/*
 * stmt access
 * */
#define stmt_slab(eng) (((ey_engine_t*)(eng))->stmt_slab)

/*
 * type access
 * */
#define type_slab(eng) (((ey_engine_t*)(eng))->type_slab)
#define ey_char_type(eng) (((ey_engine_t*)(eng))->type_const.char_type)
#define ey_uchar_type(eng) (((ey_engine_t*)(eng))->type_const.uchar_type)
#define ey_short_type(eng) (((ey_engine_t*)(eng))->type_const.short_type)
#define ey_ushort_type(eng) (((ey_engine_t*)(eng))->type_const.ushort_type)
#define ey_int_type(eng) (((ey_engine_t*)(eng))->type_const.int_type)
#define ey_uint_type(eng) (((ey_engine_t*)(eng))->type_const.uint_type)
#define ey_long_type(eng) (((ey_engine_t*)(eng))->type_const.long_type)
#define ey_ulong_type(eng) (((ey_engine_t*)(eng))->type_const.ulong_type)
#define ey_float_type(eng) (((ey_engine_t*)(eng))->type_const.float_type)
#define ey_double_type(eng) (((ey_engine_t*)(eng))->type_const.double_type)
#define ey_void_type(eng) (((ey_engine_t*)(eng))->type_const.void_type)
#define ey_pvoid_type(eng) (((ey_engine_t*)(eng))->type_const.pvoid_type)
#define ey_pchar_type(eng) (((ey_engine_t*)(eng))->type_const.pchar_type)
#define ey_const_char_type(eng) (((ey_engine_t*)(eng))->type_const.const_char_type)
#define ey_enum_const_type(eng) (((ey_engine_t*)(eng))->type_const.enum_const_type)

/*
 * symbol access
 * */
#define symbol_slab(eng) (((ey_engine_t*)(eng))->symbol_slab)
#define member_slab(eng) (((ey_engine_t*)(eng))->member_slab)
#define value_fslab(eng) (((ey_engine_t*)(eng))->value_fslab)
#define name_fslab(eng) (((ey_engine_t*)(eng))->name_fslab)
#define switch_level(eng) (((ey_engine_t*)(eng))->switch_level)
#define loop_level(eng) (((ey_engine_t*)(eng))->loop_level)
#define ey_label_list(eng) (((ey_engine_t*)(eng))->label_list)
#define ey_true_symbol(eng) (((ey_engine_t*)(eng))->symbol_const.true_symbol)
#define ey_false_symbol(eng) (((ey_engine_t*)(eng))->symbol_const.false_symbol)
#define ey_null_symbol(eng) (((ey_engine_t*)(eng))->symbol_const.null_symbol)
#define ey_undefined_list(eng) (((ey_engine_t*)(eng))->undefined_list)

/*
 * symbol table
 * */
#define ey_ident_hash(eng) (((ey_engine_t*)(eng))->ident_hash)
#define ey_tag_hash(eng) (((ey_engine_t*)(eng))->tag_hash)
#define ey_label_hash(eng) (((ey_engine_t*)(eng))->label_hash)

/*
 * parser
 * */
#define ey_filename_fslab(eng) (((ey_engine_t*)(eng))->filename_fslab)
#define ey_filename_hash(eng) (((ey_engine_t*)(eng))->filename_hash)
#define ey_parser_stack(eng) (((ey_engine_t*)(eng))->parser_stack)
#define ey_parser_level(eng) (SLIST_FIRST(&ey_parser_stack(eng))->level)
#define ey_func_undefined_label(eng) (((ey_engine_t*)(eng))->func_undefined_label)
#define ey_return_type(eng) (((ey_engine_t*)(eng))->return_type)
#define ey_declaration_type(eng) (((ey_engine_t*)(eng))->declaration_type)
#define ey_enum_value(eng) (((ey_engine_t*)(eng))->enum_value)

/*
 * for parser error
 * */
#define ey_current_parser(eng)											\
	SLIST_FIRST(&ey_parser_stack(eng))

#define ey_parser_set_error(eng, location, fmt...)						\
	do																	\
	{																	\
		ey_parser_t *par = ey_current_parser(eng);						\
		(par)->error_location = *location;								\
		snprintf((par)->error_reason, sizeof(MAX_ERROR_REASON), fmt);	\
		(par)->error_reason[MAX_ERROR_REASON-1] = '\0';					\
	}while(0)

#define ey_parser_clear_error(eng)										\
	do																	\
	{																	\
		ey_parser_t *par = ey_current_parser(eng);						\
		(par)->error_reason[0] = '\0';									\
	}while(0)

#define ey_parser_isset_error(eng)										\
	(ey_current_parser((ey_engine_t*)(eng))->error_reason[0]!='\0')
#endif
