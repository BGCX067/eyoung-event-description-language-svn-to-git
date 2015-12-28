#ifndef ENG_STMT_H
#define ENG_STMT_H 1

#include "eng_location.h"
typedef enum ey_stmt_class
{
	STMT_DEC,
	STMT_EXPR,
	STMT_LABEL,
	STMT_CASE,
	STMT_DEFAULT,
	STMT_BLOCK,

	STMT_BRANCH,
	STMT_GOTO,
	STMT_RETURN,
	STMT_WHILE,
	STMT_DO_WHILE,
	STMT_FOR,
	STMT_SWITCH,
	STMT_CONTINUE,
	STMT_BREAK,

	STMT_PESUDO,
	STMT_JUMP,
}ey_stmt_class_t;

static inline const char* ey_stmt_class_name(ey_stmt_class_t t)
{
	switch(t)
	{
		case STMT_DEC:
			return "DEC";
		case STMT_EXPR:
			return "EXPR";
		case STMT_GOTO:
			return "GOTO";
		case STMT_BRANCH:
			return "IF";
		case STMT_LABEL:
			return "LABEL";
		case STMT_CASE:
			return "CASE";
		case STMT_DEFAULT:
			return "DEFAULT";
		case STMT_RETURN:
			return "RETURN";
		case STMT_BLOCK:
			return "BLOCK";
		case STMT_WHILE:
			return "WHILE";
		case STMT_FOR:
			return "FOR";
		case STMT_DO_WHILE:
			return "DO_WHILE";
		case STMT_SWITCH:
			return "SWITCH";
		case STMT_BREAK:
			return "BREAK";
		case STMT_CONTINUE:
			return "CONTINUE";
		case STMT_PESUDO:
			return "PESUDO";
		case STMT_JUMP:
			return "JUMP";
		default:
			*(int*)0 = 0;
			return "Unknown";
	}
}

static inline int ey_stmt_is_label(ey_stmt_class_t class)
{
	switch(class)
	{
		case STMT_LABEL:
		case STMT_CASE:
		case STMT_DEFAULT:
			return 1;
		default:
			return 0;
	}
}

struct ey_symbol;
struct ey_symbol_list;
struct ey_expr;
struct ey_stmt;
typedef TAILQ_HEAD(ey_stmt_list, ey_stmt) ey_stmt_list_t;

typedef struct ey_dec_stmt
{
	struct ey_symbol_list symbol_list;
}ey_dec_stmt_t;

typedef struct ey_expr_stmt
{
	struct ey_expr *expr;
}ey_expr_stmt_t;

typedef struct ey_return_stmt
{
	struct ey_expr *expr;
	struct ey_stmt *target;
}ey_return_stmt_t;

typedef struct ey_goto_stmt
{
	struct ey_stmt *target;
}ey_goto_stmt_t;

typedef struct ey_branch_stmt
{
	struct ey_expr *expr;
	struct ey_stmt *true_target;
	struct ey_stmt *false_target;
}ey_branch_stmt_t;

typedef struct ey_block_stmt
{
	ey_stmt_list_t stmt_list;
}ey_block_stmt_t;

typedef struct ey_label_stmt
{
	struct ey_symbol *label;
	struct ey_stmt *stmt;
}ey_label_stmt_t;

typedef struct ey_case_stmt
{
	struct ey_symbol *label;
	struct ey_expr *expr;
	struct ey_stmt *stmt;
}ey_case_stmt_t;

typedef struct ey_default_stmt
{
	struct ey_symbol *label;
	struct ey_stmt *stmt;
}ey_default_stmt_t;

typedef struct ey_continue_stmt
{
	struct ey_stmt *target;
}ey_continue_stmt;

typedef struct ey_break_stmt
{
	struct ey_stmt *target;
}ey_break_stmt_t;

typedef struct ey_jump_stmt
{
	struct ey_stmt *target;
}ey_jump_stmt_t;

typedef struct ey_switch_stmt
{
	struct ey_expr *expr;
	struct ey_stmt *stmt;
	ey_stmt_list_t label_list;
}ey_switch_stmt_t;

typedef struct ey_while_stmt
{
	struct ey_expr *expr;
	struct ey_stmt *stmt;
}ey_while_stmt_t;

typedef struct ey_do_while_stmt
{
	struct ey_expr *expr;
	struct ey_stmt *stmt;
}ey_do_while_stmt_t;

typedef struct ey_for_stmt
{
	struct ey_expr *initial;
	struct ey_expr *condition;
	struct ey_expr *increase;
	struct ey_stmt *stmt;
}ey_for_stmt_t;

typedef struct ey_stmt
{
	ey_location_t location;
	ey_stmt_class_t class;
	unsigned int level;
	TAILQ_ENTRY(ey_stmt) block_link;
	TAILQ_ENTRY(ey_stmt) link;
	union
	{
		ey_dec_stmt_t dec_stmt;
		ey_expr_stmt_t expr_stmt;
		ey_return_stmt_t return_stmt;
		ey_goto_stmt_t goto_stmt;
		ey_branch_stmt_t branch_stmt;
		ey_block_stmt_t block_stmt;
		ey_label_stmt_t label_stmt;
		ey_case_stmt_t case_stmt;
		ey_default_stmt_t default_stmt;
		ey_continue_stmt continue_stmt;
		ey_break_stmt_t break_stmt;
		ey_switch_stmt_t switch_stmt;
		ey_while_stmt_t while_stmt;
		ey_do_while_stmt_t do_while_stmt;
		ey_for_stmt_t for_stmt;
		ey_jump_stmt_t jump_stmt;
	};
}ey_stmt_t;

typedef struct ey_saved_context
{
	unsigned int switch_level;
	unsigned int loop_level;
	ey_stmt_list_t label_list;
}ey_saved_context_t;

/*init/finit*/
struct ey_engine;
extern int ey_stmt_init(struct ey_engine *eng);
extern void ey_stmt_finit(struct ey_engine *eng);

/*alloc*/
extern ey_stmt_t *ey_alloc_dec_stmt(struct ey_engine *eng, struct ey_symbol_list *head, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_expr_stmt(struct ey_engine *eng, struct ey_expr *expr, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_return_stmt(struct ey_engine *eng, struct ey_expr *expr, ey_stmt_t *target, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_goto_stmt(struct ey_engine *eng, ey_stmt_t *target, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_branch_stmt(struct ey_engine *eng, struct ey_expr *expr, 
	ey_stmt_t *true_target, ey_stmt_t *false_target, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_label_stmt(struct ey_engine *eng, struct ey_symbol *label, ey_stmt_t *stmt, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_case_stmt(struct ey_engine *eng, struct ey_symbol *label, 
	struct ey_expr *expr, ey_stmt_t *stmt, unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_default_stmt(struct ey_engine *eng, struct ey_symbol *label, ey_stmt_t *stmt, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_block_stmt(struct ey_engine *eng, ey_stmt_list_t *block, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_continue_stmt(struct ey_engine *eng, ey_stmt_t *target, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_break_stmt(struct ey_engine *eng, ey_stmt_t *target, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_jump_stmt(struct ey_engine *eng, ey_stmt_t *target, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_switch_stmt(struct ey_engine *eng, struct ey_expr *expr, ey_stmt_t *stmt, ey_stmt_list_t *label_list,
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_while_stmt(struct ey_engine *eng, struct ey_expr *expr, ey_stmt_t *stmt, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_do_while_stmt(struct ey_engine *eng, struct ey_expr *expr, ey_stmt_t *stmt, 
	unsigned int level, ey_location_t *location);
extern ey_stmt_t *ey_alloc_for_stmt(struct ey_engine *eng, struct ey_expr *init, 
	struct ey_expr *condition, struct ey_expr *inc, ey_stmt_t *stmt, 
	unsigned int level, ey_location_t *location);

/*free*/
extern void ey_free_stmt(struct ey_engine *eng, ey_stmt_t *stmt);

/*print*/
extern void ey_stmt_print(struct ey_engine *eng, ey_stmt_t *stmt, int tab);

/*function prepare*/
extern int ey_function_prepare(struct ey_engine *eng, struct ey_symbol *function, ey_stmt_t *block_stmt);
#endif
