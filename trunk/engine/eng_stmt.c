#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "eng_priv.h"

static int ey_stmt_prepare(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point);

static ey_stmt_t* ey_alloc_stmt(struct ey_engine *eng, unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = NULL;
	if(stmt_slab(eng))
	{
		ret = engine_zalloc(stmt_slab(eng));
		if(ret)
		{
			memset(ret, 0, sizeof(*ret));
			ret->level = level;
			ret->location = location?*location:default_location;
		}
	}
	return ret;
}

int ey_stmt_init(struct ey_engine *eng)
{
	if(!stmt_slab(eng))
	{
		char slab_name[64];
		snprintf(slab_name, 63, "%s eyoung stmt slab", eng->name);
		slab_name[63] = '\0';
		stmt_slab(eng) = engine_zinit(slab_name, sizeof(ey_stmt_t));
		if(!stmt_slab(eng))
			return -1;
	}
	return 0;
}

void ey_stmt_finit(struct ey_engine *eng)
{
	if(stmt_slab(eng))
		engine_zclear(stmt_slab(eng));
}

ey_stmt_t *ey_alloc_dec_stmt(struct ey_engine *eng, ey_symbol_list_t *head, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_DEC;
		TAILQ_INIT(&ret->dec_stmt.symbol_list);
		if(head)
			TAILQ_CONCAT(&ret->dec_stmt.symbol_list, head, list_next);
	}
	return ret;
}

ey_stmt_t *ey_alloc_pesudo_stmt(struct ey_engine *eng, unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
		ret->class = STMT_PESUDO;
	return ret;
}

ey_stmt_t *ey_alloc_expr_stmt(struct ey_engine *eng, ey_expr_t *expr, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_EXPR;
		ret->expr_stmt.expr = expr;
	}
	return ret;
}

ey_stmt_t *ey_alloc_return_stmt(struct ey_engine *eng, ey_expr_t *expr, ey_stmt_t *target, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_RETURN;
		ret->return_stmt.expr = expr;
		ret->return_stmt.target = target;
	}
	return ret;
}

ey_stmt_t *ey_alloc_goto_stmt(struct ey_engine *eng, ey_stmt_t *target, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_GOTO;
		ret->goto_stmt.target = target;
	}
	return ret;
}

ey_stmt_t *ey_alloc_branch_stmt(struct ey_engine *eng, ey_expr_t *expr, 
	ey_stmt_t *true_target, ey_stmt_t *false_target, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_BRANCH;
		ret->branch_stmt.expr = expr;
		ret->branch_stmt.true_target = true_target;
		ret->branch_stmt.false_target = false_target;
	}
	return ret;
}

ey_stmt_t *ey_alloc_label_stmt(struct ey_engine *eng, ey_symbol_t *label, ey_stmt_t *stmt, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_LABEL;
		ret->label_stmt.label = label;
		ret->label_stmt.stmt = stmt;
	}
	return ret;
}

ey_stmt_t *ey_alloc_case_stmt(struct ey_engine *eng, ey_symbol_t *label, ey_expr_t *expr, ey_stmt_t *stmt, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_CASE;
		ret->case_stmt.label = label;
		ret->case_stmt.expr = expr;
		ret->case_stmt.stmt = stmt;
	}
	return ret;
}

ey_stmt_t *ey_alloc_default_stmt(struct ey_engine *eng, ey_symbol_t *label, ey_stmt_t *stmt, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_DEFAULT;
		ret->default_stmt.label = label;
		ret->default_stmt.stmt = stmt;
	}
	return ret;
}

ey_stmt_t *ey_alloc_block_stmt(struct ey_engine *eng, ey_stmt_list_t *block, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_BLOCK;
		TAILQ_INIT(&ret->block_stmt.stmt_list);
		if(block)
			TAILQ_CONCAT(&ret->block_stmt.stmt_list, block, block_link);
	}
	return ret;
}

ey_stmt_t *ey_alloc_continue_stmt(ey_engine_t *eng, ey_stmt_t *target, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_CONTINUE;
		ret->continue_stmt.target = target;
	}
	return ret;
}

ey_stmt_t *ey_alloc_break_stmt(ey_engine_t *eng, ey_stmt_t *target, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_BREAK;
		ret->break_stmt.target = target;
	}
	return ret;
}

ey_stmt_t *ey_alloc_jump_stmt(ey_engine_t *eng, ey_stmt_t *target, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_JUMP;
		ret->jump_stmt.target = target;
	}
	return ret;
}

ey_stmt_t *ey_alloc_switch_stmt(ey_engine_t *eng, ey_expr_t *expr, ey_stmt_t *stmt, ey_stmt_list_t *label_list, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_SWITCH;
		ret->switch_stmt.expr = expr;
		ret->switch_stmt.stmt = stmt;
		TAILQ_INIT(&ret->switch_stmt.label_list);
		if(label_list)
			TAILQ_CONCAT(&ret->switch_stmt.label_list, label_list, link);
	}
	return ret;
}

ey_stmt_t *ey_alloc_while_stmt(ey_engine_t *eng, ey_expr_t *expr, ey_stmt_t *stmt, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_WHILE;
		ret->while_stmt.expr = expr;
		ret->while_stmt.stmt = stmt;
	}
	return ret;
}

ey_stmt_t *ey_alloc_do_while_stmt(ey_engine_t *eng, ey_expr_t *expr, ey_stmt_t *stmt, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_DO_WHILE;
		ret->do_while_stmt.expr = expr;
		ret->do_while_stmt.stmt = stmt;
	}
	return ret;
}

ey_stmt_t *ey_alloc_for_stmt(ey_engine_t *eng, 
	ey_expr_t *init, ey_expr_t *condition, ey_expr_t *inc, ey_stmt_t *stmt, 
	unsigned int level, ey_location_t *location)
{
	ey_stmt_t *ret = ey_alloc_stmt(eng, level, location);
	if(ret)
	{
		ret->class = STMT_FOR;
		ret->for_stmt.initial = init;
		ret->for_stmt.condition = condition;
		ret->for_stmt.increase = inc;
		ret->for_stmt.stmt = stmt;
	}
	return ret;
}

void ey_free_stmt(struct ey_engine *eng, ey_stmt_t *stmt)
{
	if(!stmt)
		return;
	switch(stmt->class)
	{
		case STMT_DEC:
			ey_free_symbol_list(eng, &stmt->dec_stmt.symbol_list);
			break;
		case STMT_EXPR:
			ey_free_expr(eng, stmt->expr_stmt.expr);
			break;
		case STMT_RETURN:
			ey_free_expr(eng, stmt->return_stmt.expr);
			break;
		case STMT_BRANCH:
			ey_free_expr(eng, stmt->branch_stmt.expr);
			break;
		case STMT_WHILE:
			ey_free_expr(eng, stmt->while_stmt.expr);
			break;
		case STMT_DO_WHILE:
			ey_free_expr(eng, stmt->do_while_stmt.expr);
			break;
		case STMT_SWITCH:
			ey_free_expr(eng, stmt->switch_stmt.expr);
			break;
		case STMT_FOR:
			ey_free_expr(eng, stmt->for_stmt.initial);
			ey_free_expr(eng, stmt->for_stmt.condition);
			ey_free_expr(eng, stmt->for_stmt.increase);
			break;
		case STMT_BLOCK:
		case STMT_GOTO:
		case STMT_LABEL:
		case STMT_CONTINUE:
		case STMT_BREAK:
			break;
		default:
			*(int*)0 = 0;
	}
	engine_zfree(stmt_slab(eng), stmt);
}

void ey_stmt_print(struct ey_engine *eng, ey_stmt_t *stmt, int tab)
{
	/*TODO*/
	return;
}

static int ey_stmt_prepare_dec(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class==STMT_DEC && !TAILQ_EMPTY(&stmt->dec_stmt.symbol_list) && new_list!=NULL);
	
	ey_symbol_t *symbol = NULL;
	ey_stmt_t *init_stmt = NULL;
	TAILQ_FOREACH(symbol, &stmt->dec_stmt.symbol_list, list_next)
	{
		if(symbol->init_value)
		{
			init_stmt = ey_alloc_expr_stmt(eng, symbol->init_value, stmt->level, &symbol->init_value->location);
			if(!init_stmt)
			{
				ey_parser_set_error(eng, &symbol->init_value->location, "alloc init stmt failed\n");
				return -1;
			}

			TAILQ_INSERT_TAIL(new_list, init_stmt, link);
		}
	}
	return 0;
}

static int ey_stmt_prepare_expr(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class==STMT_EXPR && stmt->expr_stmt.expr!=NULL && new_list!=NULL);
	
	TAILQ_INSERT_TAIL(new_list, stmt, link);
	return 0;
}

static int ey_stmt_prepare_label(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class==STMT_LABEL && new_list!=NULL);
	assert(stmt->label_stmt.label!=NULL && stmt->label_stmt.label->class==SYMBOL_LABEL);
	assert(ey_symbol_check_flag(stmt->label_stmt.label, SYMBOL_FLAG_DEFINE));
	
	TAILQ_INSERT_TAIL(new_list, stmt, link);

	ey_stmt_list_t target;
	TAILQ_INIT(&target);
	int ret = ey_stmt_prepare(eng, stmt->label_stmt.stmt, &target, break_point, continue_point, return_point);
	if(ret)
	{
		if(ey_parser_isset_error(eng))
			ey_parser_set_error(eng, &stmt->label_stmt.stmt->location, "prepare label target failed\n");
		return -1;
	}
	stmt->label_stmt.stmt = TAILQ_FIRST(&target);
	TAILQ_CONCAT(new_list, &target, link);
	return 0;
}

static int ey_stmt_prepare_case(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class==STMT_CASE && new_list!=NULL);
	assert(stmt->case_stmt.label!=NULL && stmt->case_stmt.label->class==SYMBOL_LABEL && stmt->case_stmt.stmt!=NULL);
	assert(ey_symbol_check_flag(stmt->case_stmt.label, SYMBOL_FLAG_DEFINE));
	
	TAILQ_INSERT_TAIL(new_list, stmt, link);

	ey_stmt_list_t target;
	TAILQ_INIT(&target);
	int ret = ey_stmt_prepare(eng, stmt->case_stmt.stmt, &target, break_point, continue_point, return_point);
	if(ret)
	{
		if(ey_parser_isset_error(eng))
			ey_parser_set_error(eng, &stmt->case_stmt.stmt->location, "prepare case target failed\n");
		return -1;
	}
	stmt->case_stmt.stmt = TAILQ_FIRST(&target);;
	TAILQ_CONCAT(new_list, &target, link);
	return 0;
}

static int ey_stmt_prepare_default(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class==STMT_DEFAULT && new_list!=NULL);
	assert(stmt->default_stmt.label!=NULL && stmt->default_stmt.label->class==SYMBOL_LABEL && stmt->default_stmt.stmt!=NULL);
	assert(ey_symbol_check_flag(stmt->default_stmt.label, SYMBOL_FLAG_DEFINE));
	
	TAILQ_INSERT_TAIL(new_list, stmt, link);

	ey_stmt_list_t target;
	TAILQ_INIT(&target);
	int ret = ey_stmt_prepare(eng, stmt->default_stmt.stmt, &target, break_point, continue_point, return_point);
	if(ret)
	{
		if(ey_parser_isset_error(eng))
			ey_parser_set_error(eng, &stmt->default_stmt.stmt->location, "prepare default target failed\n");
		return -1;
	}
	stmt->default_stmt.stmt = TAILQ_FIRST(&target);
	TAILQ_CONCAT(new_list, &target, link);
	return 0;
}

static int ey_stmt_prepare_block(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class==STMT_BLOCK && !TAILQ_EMPTY(&stmt->block_stmt.stmt_list) && new_list!=NULL);
	
	ey_stmt_list_t list;
	TAILQ_INIT(&list);

	ey_stmt_t *s=NULL, *t=NULL;
	TAILQ_FOREACH_SAFE(s, &stmt->block_stmt.stmt_list, block_link, t)
	{
		TAILQ_REMOVE(&stmt->block_stmt.stmt_list, s, block_link);
		if(ey_stmt_prepare(eng, s, &list, break_point, continue_point, return_point))
		{
			if(!ey_parser_isset_error(eng))
				ey_parser_set_error(eng, &s->location, "prepare stmt failed\n");
			return -1;
		}
	}

	TAILQ_CONCAT(new_list, &list, link);
	return 0;
}

static int ey_stmt_contain_label(ey_engine_t *eng, ey_stmt_t *stmt)
{
	if(ey_stmt_is_label(stmt->class))
		return 1;
	
	if(stmt->class==STMT_BLOCK)
	{
		ey_stmt_t *s = NULL;
		TAILQ_FOREACH(s, &stmt->block_stmt.stmt_list, block_link)
		{
			if(ey_stmt_contain_label(eng, s))
				return 1;
		}
	}
	return 0;
}

static int ey_stmt_prepare_if(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class==STMT_BRANCH && new_list!=NULL);
	assert(stmt->branch_stmt.expr!=NULL);
	assert(stmt->branch_stmt.true_target!=NULL);
	
	ey_expr_t *cond = stmt->branch_stmt.expr;
	if(ey_expr_is_const_value(eng, cond) && 
		!ey_stmt_contain_label(eng, stmt->branch_stmt.true_target) &&
		!ey_stmt_contain_label(eng, stmt->branch_stmt.false_target))
	{
		if(!ey_expr_is_meanless_const_value(eng, cond))
		{
			ey_stmt_t *cond_stmt = ey_alloc_expr_stmt(eng, cond, stmt->level, &cond->location);
			if(!cond_stmt)
			{
				ey_parser_set_error(eng, &cond->location, "alloc cond stmt failed\n");
				return -1;
			}
			TAILQ_INSERT_TAIL(new_list, cond_stmt, link);
		}

		if(ey_expr_is_true(eng, cond))
		{
			return ey_stmt_prepare(eng, stmt->branch_stmt.true_target, new_list, 
				break_point, continue_point, return_point);
		}
		else if(stmt->branch_stmt.false_target)
		{
			return ey_stmt_prepare(eng, stmt->branch_stmt.false_target, new_list, 
				break_point, continue_point, return_point);
		}
	}
	else
	{
		ey_stmt_list_t true_list;
		TAILQ_INIT(&true_list);
		if(ey_stmt_prepare(eng, stmt->branch_stmt.true_target, &true_list, 
			break_point, continue_point, return_point))
		{
			if(!ey_parser_isset_error(eng))
				ey_parser_set_error(eng, &stmt->location, "prepare true branch failed\n");
			return -1;
		}

		ey_stmt_list_t false_list;
		TAILQ_INIT(&false_list);
		if(stmt->branch_stmt.false_target && 
			ey_stmt_prepare(eng, stmt->branch_stmt.false_target, &false_list, 
				break_point, continue_point, return_point))
		{
			if(!ey_parser_isset_error(eng))
				ey_parser_set_error(eng, &stmt->location, "prepare false branch failed\n");
			return -1;
		}
		
		if(TAILQ_EMPTY(&true_list))
		{
			if(TAILQ_EMPTY(&false_list))
			{
				/**************************************/
				/*             |                      */
				/*             V                      */
				/*      +--------------+              */
				/*      |    cond      |     (EXPR)   */
				/*      +--------------+              */
				/*             |                      */
				/*             V                      */
				/**************************************/
				ey_stmt_t *cond_stmt = ey_alloc_expr_stmt(eng, stmt->branch_stmt.expr, 
					stmt->level, &stmt->branch_stmt.expr->location);
				if(!cond_stmt)
				{
					ey_parser_set_error(eng, &stmt->branch_stmt.expr->location, "alloc condition stmt failed\n");
					return -1;
				}
				TAILQ_INSERT_TAIL(new_list, cond_stmt, link);
			}
			else
			{
				/**************************************/
				/*             |                      */
				/*             V                      */
				/*      +--------------+              */
				/*      |     cond     |--+ F (BRANCH)*/
				/*      +--------------+  |           */
				/*             | T        |           */
				/*             V          |           */
				/*      +--------------+  |           */
				/*      |  true_tail   |  |    (JUMP) */
				/*      +--------------+  |           */
				/*             |          |           */
				/*   +---------+          |           */
				/*   |                    |           */
				/*   |  +--------------+  |           */
				/*   |  |  false body  |<-+           */
				/*   |  +--------------+              */
				/*   |         |                      */
				/*   |         V                      */
				/*   |  +--------------+              */
				/*   +->|  if_tail     |     (PESUDO) */
				/*      +--------------+              */
				/*             |                      */
				/*             V                      */
				/**************************************/

				ey_stmt_t *if_tail = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
				if(!if_tail)
				{
					ey_parser_set_error(eng, &stmt->location, "alloc pesudo tail stmt failed\n");
					return -1;
				}

				ey_stmt_t *true_tail = ey_alloc_jump_stmt(eng, if_tail, stmt->level, &stmt->location);
				if(!true_tail)
				{
					ey_parser_set_error(eng, &stmt->location, "alloc true tail jump stmt failed\n");
					return -1;
				}

				stmt->branch_stmt.true_target = true_tail;
				stmt->branch_stmt.false_target = TAILQ_FIRST(&false_list);

				TAILQ_INSERT_TAIL(new_list, stmt, link);
				TAILQ_INSERT_TAIL(new_list, true_tail, link);
				TAILQ_CONCAT(new_list, &false_list, link);
				TAILQ_INSERT_TAIL(new_list, if_tail, link);
			}
		}
		else
		{
			ey_stmt_t *if_tail = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
			if(!if_tail)
			{
				ey_parser_set_error(eng, &stmt->location, "alloc pesudo tail stmt failed\n");
				return -1;
			}

			if(TAILQ_EMPTY(&false_list))
			{
				/**************************************/
				/*             |                      */
				/*             V                      */
				/*      +--------------+              */
				/*      |     cond     |--+ F (BRANCH)*/
				/*      +--------------+  |           */
				/*             | T        |           */
				/*             V          |           */
				/*      +--------------+  |           */
				/*      |  true body   |  |           */
				/*      +--------------+  |           */
				/*             |          |           */
				/*             V          |           */
				/*      +--------------+  |           */
				/*      |  if_tail     |<-+  (PESUDO) */
				/*      +--------------+              */
				/*             |                      */
				/*             V                      */
				/**************************************/

				stmt->branch_stmt.false_target = if_tail;
				stmt->branch_stmt.true_target = TAILQ_FIRST(&true_list);
				
				TAILQ_INSERT_TAIL(new_list, stmt, link);
				TAILQ_CONCAT(new_list, &true_list, link);
				TAILQ_INSERT_TAIL(new_list, if_tail, link);
			}
			else
			{
				/**************************************/
				/*             |                      */
				/*             V                      */
				/*      +--------------+              */
				/*      |     cond     |--+ F (BRANCH)*/
				/*      +--------------+  |           */
				/*             | T        |           */
				/*             V          |           */
				/*      +--------------+  |           */
				/*      |  true body   |  |           */
				/*      +--------------+  |           */
				/*             |          |           */
				/*             V          |           */
				/*      +--------------+  |           */
				/*      |  true_tail   |  |    (JUMP) */
				/*      +--------------+  |           */
				/*             |          |           */
				/*   +---------+          |           */
				/*   |                    |           */
				/*   |  +--------------+  |           */
				/*   |  |  false body  |<-+           */
				/*   |  +--------------+              */
				/*   |         |                      */
				/*   |         V                      */
				/*   |  +--------------+              */
				/*   +->|  if_tail     |     (PESUDO) */
				/*      +--------------+              */
				/*             |                      */
				/*             V                      */
				/**************************************/

				ey_stmt_t *true_tail = ey_alloc_jump_stmt(eng, if_tail, stmt->level, &stmt->location);
				if(!true_tail)
				{
					ey_parser_set_error(eng, &stmt->location, "alloc true tail jump stmt failed\n");
					return -1;
				}
				
				stmt->branch_stmt.true_target = TAILQ_FIRST(&true_list);
				stmt->branch_stmt.false_target = TAILQ_FIRST(&false_list);

				TAILQ_INSERT_TAIL(new_list, stmt, link);
				TAILQ_CONCAT(new_list, &true_list, link);
				TAILQ_INSERT_TAIL(new_list, true_tail, link);
				TAILQ_CONCAT(new_list, &false_list, link);
				TAILQ_INSERT_TAIL(new_list, if_tail, link);
			}
		}
	}
	return 0;
}

static int ey_stmt_prepare_goto(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class==STMT_GOTO && stmt->goto_stmt.target!=NULL && new_list!=NULL);
	assert(stmt->goto_stmt.target->class==STMT_LABEL);
	
	TAILQ_INSERT_TAIL(new_list, stmt, link);
	return 0;
}

static int ey_stmt_prepare_return(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && return_point!=NULL && stmt->class==STMT_RETURN && stmt->return_stmt.target==NULL && new_list!=NULL);
	
	stmt->return_stmt.target = return_point;
	TAILQ_INSERT_TAIL(new_list, stmt, link);
	return 0;
}

static int ey_stmt_prepare_while(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class==STMT_WHILE && new_list!=NULL);
	assert(stmt->while_stmt.expr!=NULL && stmt->while_stmt.stmt!=NULL);
	
	ey_expr_t *cond = stmt->while_stmt.expr;
	if(ey_expr_is_const_value(eng, cond) && !ey_stmt_contain_label(eng, stmt->while_stmt.stmt))
	{
		if(!ey_expr_is_true(eng, cond))
		{
			if(ey_expr_is_meanless_const_value(eng, cond))
				return 0;

			/**************************************/
			/*             |                      */
			/*             V                      */
			/*      +--------------+              */
			/*      |    cond      |     (EXPR)   */
			/*      +--------------+              */
			/*             |                      */
			/*             V                      */
			/**************************************/
			ey_stmt_t *cond_stmt = ey_alloc_expr_stmt(eng, cond, stmt->level, &cond->location);
			if(!cond_stmt)
			{
				ey_parser_set_error(eng, &cond->location, "alloc cond stmt failed\n");
				return -1;
			}
			TAILQ_INSERT_TAIL(new_list, cond_stmt, link);
		}
		else
		{
			/****************************************/
			/*             |                        */
			/*             V                        */
			/*      +--------------+                */
			/*   +->| CONTINUE_PT  |       (PESUDO) */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   |         V                        */
			/*   |  +--------------+                */
			/*   |  | (OPT): cond  |       (EXPR)   */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   |         V                        */
			/*   |  +--------------+                */
			/*   |  |  loop body   |                */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   |         V                        */
			/*   |  +--------------+                */
			/*   |  |  body tail   |       (JUMP)   */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   +---------+                        */
			/*                                      */
			/*      +--------------+                */
			/*      |  BREAK_PT    |       (PESODU) */
			/*      +--------------+                */
			/*             |                        */
			/*             V                        */
			/****************************************/
			ey_stmt_t *cp = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
			if(!cp)
			{
				ey_parser_set_error(eng, &stmt->location, "alloc while continue point failed\n");
				return -1;
			}
			TAILQ_INSERT_TAIL(new_list, cp, link);
			
			if(!ey_expr_is_meanless_const_value(eng, cond))
			{
				ey_stmt_t *cond_stmt = ey_alloc_expr_stmt(eng, cond, stmt->level, &cond->location);
				if(!cond_stmt)
				{
					ey_parser_set_error(eng, &cond->location, "alloc cond stmt failed\n");
					return -1;
				}
				TAILQ_INSERT_TAIL(new_list, cond_stmt, link);
			}

			ey_stmt_t *bp = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
			if(!bp)
			{
				ey_parser_set_error(eng, &stmt->location, "alloc while break point failed\n");
				return -1;
			}

			if(ey_stmt_prepare(eng, stmt->while_stmt.stmt, new_list, bp, cp, return_point))
			{
				if(!ey_parser_isset_error(eng))
					ey_parser_set_error(eng, &stmt->location, "prepare while body failed\n");
				return -1;
			}

			ey_stmt_t *body_tail = ey_alloc_jump_stmt(eng, cp, stmt->level, &stmt->location);
			if(!body_tail)
			{
				ey_parser_set_error(eng, &stmt->location, "alloc body jump stmt failed\n");
				return -1;
			}
			TAILQ_INSERT_TAIL(new_list, body_tail, link);
			TAILQ_INSERT_TAIL(new_list, bp, link);
		}
	}
	else
	{
		/****************************************/
		/*             |                        */
		/*             V                        */
		/*      +--------------+                */
		/*   +->| CONTINUE_PT  |       (PESUDO) */
		/*   |  +--------------+                */
		/*   |         |                        */
		/*   |         V                        */
		/*   |  +--------------+ False          */
		/*   |  | branch: cond |--+    (BRANCH) */
		/*   |  +--------------+  |             */
		/*   |         | True     |             */
		/*   |         V          |             */
		/*   |  +--------------+  |             */
		/*   |  |  loop body   |  |             */
		/*   |  +--------------+  |             */
		/*   |         |          |             */
		/*   |         V          |             */
		/*   |  +--------------+  |             */
		/*   |  |  body tail   |  |    (JUMP)   */
		/*   |  +--------------+  |             */
		/*   |         |          |             */
		/*   +---------+          |             */
		/*                        |             */
		/*      +--------------+  |             */
		/*      |  BREAK_PT    |<-+    (PESODU) */
		/*      +--------------+                */
		/*             |                        */
		/*             V                        */
		/****************************************/
		ey_stmt_t *cp = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
		if(!cp)
		{
			ey_parser_set_error(eng, &stmt->location, "alloc while continue point failed\n");
			return -1;
		}
		TAILQ_INSERT_TAIL(new_list, cp, link);
		
		ey_stmt_t *branch_stmt = ey_alloc_branch_stmt(eng, stmt->while_stmt.expr, NULL, NULL, 
			stmt->level, &stmt->while_stmt.expr->location);
		if(!branch_stmt)
		{
			ey_parser_set_error(eng, &stmt->while_stmt.expr->location, "alloc while branch stmt failed\n");
			return -1;
		}
		TAILQ_INSERT_TAIL(new_list, branch_stmt, link);

		ey_stmt_t *bp = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
		if(!bp)
		{
			ey_parser_set_error(eng, &stmt->location, "alloc while break point failed\n");
			return -1;
		}

		ey_stmt_list_t while_body;
		TAILQ_INIT(&while_body);
		if(ey_stmt_prepare(eng, stmt->while_stmt.stmt, &while_body, bp, cp, return_point))
		{
			if(!ey_parser_isset_error(eng))
				ey_parser_set_error(eng, &stmt->location, "prepare while body failed\n");
			return -1;
		}
		branch_stmt->branch_stmt.true_target = TAILQ_FIRST(&while_body);
		branch_stmt->branch_stmt.false_target = bp;
		TAILQ_CONCAT(new_list, &while_body, link);

		ey_stmt_t *body_tail = ey_alloc_jump_stmt(eng, cp, stmt->level, &stmt->location);
		if(!body_tail)
		{
			ey_parser_set_error(eng, &stmt->location, "alloc while body jump stmt failed\n");
			return -1;
		}
		TAILQ_INSERT_TAIL(new_list, body_tail, link);
		TAILQ_INSERT_TAIL(new_list, bp, link);
	}
	return 0;
}

static int ey_stmt_prepare_do_while(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class==STMT_DO_WHILE && new_list!=NULL);
	assert(stmt->do_while_stmt.expr!=NULL && stmt->do_while_stmt.stmt!=NULL);
	
	ey_stmt_t *cp = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
	if(!cp)
	{
		ey_parser_set_error(eng, &stmt->location, "alloc do-while continue point failed\n");
		return -1;
	}

	ey_stmt_t *bp = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
	if(!bp)
	{
		ey_parser_set_error(eng, &stmt->location, "alloc do-while break point failed\n");
		return -1;
	}

	ey_stmt_list_t do_while_body;
	TAILQ_INIT(&do_while_body);
	if(ey_stmt_prepare(eng, stmt->do_while_stmt.stmt, &do_while_body, bp, cp, return_point))
	{
		if(!ey_parser_isset_error(eng))
			ey_parser_set_error(eng, &stmt->location, "prepare do-while body failed\n");
		return -1;
	}
	
	ey_expr_t *cond = stmt->do_while_stmt.expr;
	if(ey_expr_is_const_value(eng, cond) && !ey_stmt_contain_label(eng, stmt->do_while_stmt.stmt))
	{
		if(!ey_expr_is_true(eng, cond))
		{
			/****************************************/
			/*             |                        */
			/*             V                        */
			/*      +--------------+                */
			/*      |   loop body  |                */
			/*      +--------------+                */
			/*             |                        */
			/*             V                        */
			/*      +--------------+                */
			/*      |  CONTINUE_PT |       (PESUDO) */
			/*      +--------------+                */
			/*             |                        */
			/*             V                        */
			/*      +--------------+                */
			/*      |  (OPT): cond |       (EXPR)   */
			/*      +--------------+                */
			/*             |                        */
			/*             V                        */
			/*      +--------------+                */
			/*      |   BREAK_PT   |       (PESODU) */
			/*      +--------------+                */
			/*             |                        */
			/*             V                        */
			/****************************************/
			TAILQ_CONCAT(new_list, &do_while_body, link);
			TAILQ_INSERT_TAIL(new_list, cp, link);

			if(!ey_expr_is_meanless_const_value(eng, cond))
			{
				ey_stmt_t *cond_stmt = ey_alloc_expr_stmt(eng, cond, stmt->level, &cond->location);
				if(!cond_stmt)
				{
					ey_parser_set_error(eng, &cond->location, "alloc cond stmt failed\n");
					return -1;
				}
				TAILQ_INSERT_TAIL(new_list, cond_stmt, link);
			}

			TAILQ_INSERT_TAIL(new_list, bp, link);
		}
		else
		{
			/****************************************/
			/*             |                        */
			/*             V                        */
			/*      +--------------+                */
			/*   +->|   body_head  |       (PESUDO) */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   |         V                        */
			/*   |  +--------------+                */
			/*   |  |  loop body   |                */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   |         V                        */
			/*   |  +--------------+                */
			/*   |  |  CONTINUE_PT |       (PESUDO) */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   |         V                        */
			/*   |  +--------------+                */
			/*   |  |  (OPT): cond |       (EXPR)   */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   |         V                        */
			/*   |  +--------------+                */
			/*   |  |  body tail   |       (JUMP)   */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   +---------+                        */
			/*                                      */
			/*      +--------------+                */
			/*      |  BREAK_PT    |       (PESODU) */
			/*      +--------------+                */
			/*             |                        */
			/*             V                        */
			/****************************************/
			ey_stmt_t *body_head = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
			if(!body_head)
			{
				ey_parser_set_error(eng, &stmt->location, "alloc do-while body start failed\n");
				return -1;
			}

			TAILQ_INSERT_TAIL(new_list, body_head, link);
			TAILQ_CONCAT(new_list, &do_while_body, link);
			TAILQ_INSERT_TAIL(new_list, cp, link);
			
			if(!ey_expr_is_meanless_const_value(eng, cond))
			{
				ey_stmt_t *cond_stmt = ey_alloc_expr_stmt(eng, cond, stmt->level, &cond->location);
				if(!cond_stmt)
				{
					ey_parser_set_error(eng, &cond->location, "alloc do-while cond stmt failed\n");
					return -1;
				}
				TAILQ_INSERT_TAIL(new_list, cond_stmt, link);
			}

			ey_stmt_t *body_tail = ey_alloc_jump_stmt(eng, body_head, stmt->level, &stmt->location);
			if(!body_tail)
			{
				ey_parser_set_error(eng, &stmt->location, "alloc do-while body tail failed\n");
				return -1;
			}

			TAILQ_INSERT_TAIL(new_list, body_tail, link);
			TAILQ_INSERT_TAIL(new_list, bp, link);
		}
	}
	else
	{
		/****************************************/
		/*             |                        */
		/*             V                        */
		/*      +--------------+                */
		/*   +->|   body_head  |       (PESUDO) */
		/*   |  +--------------+                */
		/*   |         |                        */
		/*   |         V                        */
		/*   |  +--------------+                */
		/*   |  |  loop body   |                */
		/*   |  +--------------+                */
		/*   |         |                        */
		/*   |         V                        */
		/*   |  +--------------+                */
		/*   |  |  CONTINUE_PT |       (PESUDO) */
		/*   |  +--------------+                */
		/*   |         |                        */
		/*   |         V                        */
		/*   |  +--------------+ False          */
		/*   |  |  branch stmt |---+   (BRANCH) */
		/*   |  +--------------+   |            */
		/*   |         |True       |            */
		/*   +---------+           |            */
		/*                         |            */
		/*      +--------------+   |            */
		/*      |  BREAK_PT    |<--+   (PESODU) */
		/*      +--------------+                */
		/*             |                        */
		/*             V                        */
		/****************************************/
		ey_stmt_t *body_head = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
		if(!body_head)
		{
			ey_parser_set_error(eng, &stmt->location, "alloc do-while body start failed\n");
			return -1;
		}
		
		TAILQ_INSERT_TAIL(new_list, body_head, link);
		TAILQ_CONCAT(new_list, &do_while_body, link);
		TAILQ_INSERT_TAIL(new_list, cp, link);

		ey_stmt_t *branch_stmt = ey_alloc_branch_stmt(eng, cond, body_head, bp, stmt->level, &stmt->location);
		if(!branch_stmt)
		{
			ey_parser_set_error(eng, &stmt->location, "alloc do-while branch faile\n");
			return -1;
		}
		
		TAILQ_INSERT_TAIL(new_list, branch_stmt, link);
		TAILQ_INSERT_TAIL(new_list, bp, link);
	}
	return 0;
}

static int ey_stmt_prepare_for(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class==STMT_FOR && new_list!=NULL);
	assert(stmt->for_stmt.initial!=NULL);
	assert(stmt->for_stmt.condition!=NULL);
	assert(stmt->for_stmt.increase!=NULL);
	assert(stmt->for_stmt.stmt!=NULL);
	
	ey_expr_t *init = stmt->for_stmt.initial;
	ey_expr_t *cond = stmt->for_stmt.condition;
	ey_expr_t *inc = stmt->for_stmt.increase;
	ey_stmt_t *body = stmt->for_stmt.stmt;

	if(!ey_expr_is_meanless_const_value(eng, init))
	{
		ey_stmt_t *init_stmt = ey_alloc_expr_stmt(eng, init, stmt->level, &init->location);
		if(!init_stmt)
		{
			ey_parser_set_error(eng, &init->location, "alloc init stmt failed\n");
			return -1;
		}
		TAILQ_INSERT_TAIL(new_list, init_stmt, link);
	}

	if(ey_expr_is_const_value(eng, cond) && !ey_stmt_contain_label(eng, body))
	{
		if(!ey_expr_is_true(eng, cond))
		{
			/**************************************/
			/*             |                      */
			/*             V                      */
			/*      +--------------+              */
			/*      |  (OPT) init  |     (EXPR)   */
			/*      +--------------+              */
			/*             |                      */
			/*             V                      */
			/*      +--------------+              */
			/*      |    cond      |     (EXPR)   */
			/*      +--------------+              */
			/*             |                      */
			/*             V                      */
			/**************************************/
			if(ey_expr_is_meanless_const_value(eng, cond))
				return 0;
			
			ey_stmt_t *cond_stmt = ey_alloc_expr_stmt(eng, cond, stmt->level, &cond->location);
			if(!cond_stmt)
			{
				ey_parser_set_error(eng, &cond->location, "alloc cond stmt failed\n");
				return -1;
			}
			TAILQ_INSERT_TAIL(new_list, cond_stmt, link);
		}
		else
		{
			/****************************************/
			/*             |                        */
			/*             V                        */
			/*      +--------------+                */
			/*      |  (OPT) init  |       (EXPR)   */
			/*      +--------------+                */
			/*             |                        */
			/*             V                        */
			/*      +--------------+                */
			/*   +->|   body_head  |       (PESUDO) */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   |         V                        */
			/*   |  +--------------+                */
			/*   |  |  (OPT) cond  |       (EXPR)   */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   |         V                        */
			/*   |  +--------------+                */
			/*   |  |  loop body   |                */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   |         V                        */
			/*   |  +--------------+                */
			/*   |  |  cotinue_pt  |       (PESUDO) */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   |         V                        */
			/*   |  +--------------+                */
			/*   |  |  (OPT) inc   |       (EXPR)   */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   |         V                        */
			/*   |  +--------------+                */
			/*   |  |  body tail   |       (JUMP)   */
			/*   |  +--------------+                */
			/*   |         |                        */
			/*   +---------+                        */
			/*                                      */
			/*      +--------------+                */
			/*      |  BREAK_PT    |       (PESODU) */
			/*      +--------------+                */
			/*             |                        */
			/*             V                        */
			/****************************************/
			ey_stmt_t *body_head = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
			if(!body_head)
			{
				ey_parser_set_error(eng, &stmt->location, "alloc for body head failed\n");
				return -1;
			}
			TAILQ_INSERT_TAIL(new_list, body_head, link);

			ey_stmt_t *cp = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
			if(!cp)
			{
				ey_parser_set_error(eng, &body->location, "alloc for continue point failed\n");
				return -1;
			}
			
			if(!ey_expr_is_meanless_const_value(eng, cond))
			{
				ey_stmt_t *cond_stmt = ey_alloc_expr_stmt(eng, cond, stmt->level, &cond->location);
				if(!cond_stmt)
				{
					ey_parser_set_error(eng, &cond->location, "alloc cond stmt failed\n");
					return -1;
				}
				TAILQ_INSERT_TAIL(new_list, cond_stmt, link);
			}

			ey_stmt_t *bp = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
			if(!bp)
			{
				ey_parser_set_error(eng, &body->location, "alloc bor break point failed\n");
				return -1;
			}

			if(ey_stmt_prepare(eng, body, new_list, bp, cp, return_point))
			{
				if(!ey_parser_isset_error(eng))
					ey_parser_set_error(eng, &body->location, "prepare for body failed\n");
				return -1;
			}
			
			TAILQ_INSERT_TAIL(new_list, cp, link);
			if(!ey_expr_is_meanless_const_value(eng, inc))
			{
				ey_stmt_t *inc_stmt = ey_alloc_expr_stmt(eng, inc, stmt->level, &inc->location);
				if(!inc_stmt)
				{
					ey_parser_set_error(eng, &inc->location, "alloc for inc stmt failed\n");
					return -1;
				}
				TAILQ_INSERT_TAIL(new_list, inc_stmt, link);
			}

			ey_stmt_t *body_tail = ey_alloc_jump_stmt(eng, body_head, stmt->level, &body->location);
			if(!body_tail)
			{
				ey_parser_set_error(eng, &body->location, "alloc body jump stmt failed\n");
				return -1;
			}
			TAILQ_INSERT_TAIL(new_list, body_tail, link);
			TAILQ_INSERT_TAIL(new_list, bp, link);
		}
	}
	else
	{
		/****************************************/
		/*             |                        */
		/*             V                        */
		/*      +--------------+                */
		/*      |  (OPT) init  |       (EXPR)   */
		/*      +--------------+                */
		/*             |                        */
		/*             V                        */
		/*      +--------------+                */
		/*   +->|   body_head  |       (PESUDO) */
		/*   |  +--------------+                */
		/*   |         |                        */
		/*   |         V                        */
		/*   |  +--------------+ False          */
		/*   |  |     cond     |--+    (BRANCH) */
		/*   |  +--------------+  |             */
		/*   |         | True     |             */
		/*   |         V          |             */
		/*   |  +--------------+  |             */
		/*   |  |  loop body   |  |             */
		/*   |  +--------------+  |             */
		/*   |         |          |             */
		/*   |         V          |             */
		/*   |  +--------------+  |             */
		/*   |  |  cotinue_pt  |  |    (PESUDO) */
		/*   |  +--------------+  |             */
		/*   |         |          |             */
		/*   |         V          |             */
		/*   |  +--------------+  |             */
		/*   |  |  (OPT) inc   |  |    (EXPR)   */
		/*   |  +--------------+  |             */
		/*   |         |          |             */
		/*   |         V          |             */
		/*   |  +--------------+  |             */
		/*   |  |  body tail   |  |    (JUMP)   */
		/*   |  +--------------+  |             */
		/*   |         |          |             */
		/*   +---------+          |             */
		/*                        |             */
		/*      +--------------+  |             */
		/*      |  BREAK_PT    |<-+    (PESODU) */
		/*      +--------------+                */
		/*             |                        */
		/*             V                        */
		/****************************************/
		ey_stmt_t *body_head = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
		if(!body_head)
		{
			ey_parser_set_error(eng, &stmt->location, "alloc for body head failed\n");
			return -1;
		}
		TAILQ_INSERT_TAIL(new_list, body_head, link);

		ey_stmt_t *cp = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
		if(!cp)
		{
			ey_parser_set_error(eng, &stmt->location, "alloc for continue point failed\n");
			return -1;
		}
		
		ey_stmt_t *branch_stmt = ey_alloc_branch_stmt(eng, cond, NULL, NULL, 
			stmt->level, &cond->location);
		if(!branch_stmt)
		{
			ey_parser_set_error(eng, &cond->location, "alloc for branch stmt failed\n");
			return -1;
		}
		TAILQ_INSERT_TAIL(new_list, branch_stmt, link);

		ey_stmt_t *bp = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
		if(!bp)
		{
			ey_parser_set_error(eng, &body->location, "alloc for break point failed\n");
			return -1;
		}

		ey_stmt_list_t for_body;
		TAILQ_INIT(&for_body);
		if(ey_stmt_prepare(eng, body, &for_body, bp, cp, return_point))
		{
			if(!ey_parser_isset_error(eng))
				ey_parser_set_error(eng, &body->location, "prepare for body failed\n");
			return -1;
		}
		branch_stmt->branch_stmt.true_target = TAILQ_FIRST(&for_body);
		branch_stmt->branch_stmt.false_target = bp;
		TAILQ_CONCAT(new_list, &for_body, link);

		TAILQ_INSERT_TAIL(new_list, cp, link);
		if(!ey_expr_is_meanless_const_value(eng, inc))
		{
			ey_stmt_t *inc_stmt = ey_alloc_expr_stmt(eng, inc, stmt->level, &inc->location);
			if(!inc_stmt)
			{
				ey_parser_set_error(eng, &inc->location, "alloc for inc stmt failed\n");
				return -1;
			}
			TAILQ_INSERT_TAIL(new_list, inc_stmt, link);
		}

		ey_stmt_t *body_tail = ey_alloc_jump_stmt(eng, body_head, stmt->level, &stmt->location);
		if(!body_tail)
		{
			ey_parser_set_error(eng, &stmt->location, "alloc for body jump stmt failed\n");
			return -1;
		}
		TAILQ_INSERT_TAIL(new_list, body_tail, link);
		TAILQ_INSERT_TAIL(new_list, bp, link);
	}
	return 0;
}

static int ey_stmt_prepare_switch(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class==STMT_SWITCH && new_list!=NULL);
	assert(stmt->switch_stmt.expr!=NULL && stmt->switch_stmt.stmt!=NULL);

	/**************************************/
	/*             |                      */
	/*             V                      */
	/*      +--------------+              */
	/*      | selector asgn|      (EXPR)  */
	/*      +--------------+              */
	/*             |                      */
	/*             V                      */
	/*      +--------------+              */
	/*      | case 1 cond  |--+ F (BRANCH)*/
	/*      +--------------+  |           */
	/*             | T        |           */
	/*             V          |           */
	/*      +--------------+  |           */
	/*   +--|  goto case 1 |  |    (JUMP) */
	/*   |  +--------------+  |           */
	/*   |                    |           */
	/*   |                    |           */
	/*   |  +--------------+<-+           */
	/*   |  |  case 2 cond |--+ F (BRANCH)*/
	/*   |  +--------------+  |           */
	/*   V          | T       |           */
	/* (TO:case 1)  V         |           */
	/*      +--------------+  |           */
	/*   +--|  goto case 2 |  |   (JUMP)  */
	/*   |  +--------------+  |           */
	/*   |                    |           */
	/*   |                    |           */
	/*   |  +--------------+  |           */
	/*   |  | goto default |<-+   (JUMP)  */
	/*   |  +--------------+              */
	/*   V          |                     */
	/* (TO:case 2)  V                     */
	/*       (TO:default/break)           */
	/*                                    */
	/*      +--------------+              */
	/*      |  Switch Body |              */
	/*      +--------------+              */
	/*             |                      */
	/*             V                      */
	/*      +--------------+              */
	/*      |  BREAK_PT    |      (PESODU)*/
	/*      +--------------+              */
	/*             |                      */
	/*             V                      */
	/**************************************/

	/*
	 *[EXAMPLE1]
	 *	switch(a)							unsigned long <nil_symbol>;
	 *	{                                   <nil_symbol>=a;
	 *		case C1:                        if(<nil_symbol>==C1)
	 *			stmt_C1;     ===>				goto stmt_C1;
	 *		case C2:                        else if(<nil_symbol==C2>)
	 *			stmt_C2;                    	goto stmt_C2;
	 *		default:                        else
	 *			stmt_default:               	goto stmt_default;
	 *	}                                    <alloc prepared switch body>
	 *	<BREAK_PT>
	 *
	 *
	 *[EXAMPLE2]
	 *	switch(a)							unsigned long <nil_symbol>;
	 *	{                                   <nil_symbol>=a;
	 *		case C1:         ===>           if(<nil_symbol>==C1)
	 *			stmt_C1;                    	goto stmt_C1;
	 *		case C2:                        else if(<nil_symbol==C2>)
	 *			stmt_C2;                    	goto stmt_C2;
	 *	}                                   goto BREAK_POINT;
	 *  <BREAK_PT>                          <alloc prepared switch body>
	 *
	 *
	 *[EXAMPLE3]
	 *	switch(a)							unsigned long <nil_symbol>;
	 *	{                                   <nil_symbol>=a;
	 *		default:         ===>           goto stmt_default;
	 *			stmt_default;               <alloc prepared switch body>
	 *	}
	 *	<BREAK_PT>
	 *
	 *[EXAMPLE4]
	 *	switch(a)							unsigned long <nil_symbol>;
	 *	{                                   <nil_symbol>=a;
	 *		no_label_stmt;   ===>           goto BREAK_POINT;
	 *	}                                   <alloc prepared no_label body>
	 *	<BREAK_PT>
	 * */

	ey_expr_t *cond = stmt->switch_stmt.expr;
	ey_stmt_t *body = stmt->switch_stmt.stmt;
	
	ey_symbol_t *selector = ey_alloc_symbol(eng, NULL, stmt->level, 
								SYMBOL_LOCAL, SYMBOL_STORAGE_NONE,
								SYMBOL_FLAG_DECLARE|SYMBOL_FLAG_DEFINE, ey_ulong_type(eng),
								NULL, NULL, &cond->location);
	if(!selector)
	{
		ey_parser_set_error(eng, &cond->location, "alloc selector symbol failed\n");
		return -1;
	}
	ey_symbol_alloc_local_mem(eng, selector);

	ey_expr_t *selector_expr = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, selector, &selector->location);
	if(!selector_expr)
	{
		ey_parser_set_error(eng, &selector->location, "alloc selector expr failed\n");
		return -1;
	}

	ey_expr_t *selector_assign = ey_alloc_binary_expr(eng, EXPR_OPCODE_ASGN, selector_expr, cond, &selector->location);
	if(!selector_assign)
	{
		ey_parser_set_error(eng, &selector->location, "alloc selector assginment expr failed\n");
		return -1;
	}

	ey_stmt_t *selector_assign_stmt = ey_alloc_expr_stmt(eng, selector_assign, selector->level, &selector->location);
	if(!selector_assign_stmt)
	{
		ey_parser_set_error(eng, &selector->location, "alloc selector assginment stmt failed\n");
		return -1;
	}
	TAILQ_INSERT_TAIL(new_list, selector_assign_stmt, link);
	
	if(!ey_stmt_contain_label(eng, body))
		return 0;

	ey_stmt_t *bp = ey_alloc_pesudo_stmt(eng, stmt->level, &stmt->location);
	if(!bp)
	{
		ey_parser_set_error(eng, &stmt->location, "alloc swithc break point failed\n");
		return -1;
	}

	ey_stmt_t *branch_root=NULL;
	ey_stmt_t **else_branch=&branch_root;
	ey_stmt_t *label=NULL, *tmp=NULL;
	ey_stmt_t *default_label=bp;

	TAILQ_FOREACH_SAFE(label, &stmt->switch_stmt.label_list, link, tmp)
	{
		TAILQ_REMOVE(&stmt->switch_stmt.label_list, label, link);
		if(label->class==STMT_DEFAULT)
		{
			default_label = label;
			continue;
		}

		ey_expr_t *branch_cond = ey_alloc_binary_expr(eng, EXPR_OPCODE_EQ, 
			selector_expr, label->case_stmt.expr, &label->location);
		if(!branch_cond)
		{
			ey_parser_set_error(eng, &label->location, "alloc branch stmt failed\n");
			return -1;
		}

		ey_stmt_t *jump_stmt = ey_alloc_jump_stmt(eng, label, stmt->level, &stmt->location);
		if(!jump_stmt)
		{
			ey_parser_set_error(eng, &label->location, "alloc case jump stmt failed\n");
			return -1;
		}

		ey_stmt_t *branch_stmt = ey_alloc_branch_stmt(eng, branch_cond, jump_stmt, NULL, stmt->level, &stmt->location);
		if(!branch_stmt)
		{
			ey_parser_set_error(eng, &label->location, "alloc case branch stmt failed\n");
			return -1;
		}

		*else_branch = branch_stmt;
		else_branch = &branch_stmt->branch_stmt.false_target;

		TAILQ_INSERT_TAIL(new_list, branch_stmt, link);
		TAILQ_INSERT_TAIL(new_list, jump_stmt, link);
	}

	*else_branch = ey_alloc_jump_stmt(eng, default_label, stmt->level, &stmt->location);
	if(!*else_branch)
	{
		ey_parser_set_error(eng, &default_label->location, "alloc default jump stmt failed\n");
		return -1;
	}
	TAILQ_INSERT_TAIL(new_list, *else_branch, link);

	if(ey_stmt_prepare(eng, body, new_list, bp, continue_point, return_point))
	{
		if(!ey_parser_isset_error(eng))
			ey_parser_set_error(eng, &body->location, "prepare switch body failed\n");
		return -1;
	}
	
	TAILQ_INSERT_TAIL(new_list, bp, link);
	return 0;
}

static int ey_stmt_prepare_break(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && break_point!=NULL && stmt->class==STMT_BREAK && stmt->break_stmt.target==NULL && new_list!=NULL);
	
	stmt->break_stmt.target = break_point;
	TAILQ_INSERT_TAIL(new_list, stmt, link);
	return 0;
}

static int ey_stmt_prepare_continue(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && continue_point!=NULL && stmt->class==STMT_CONTINUE && stmt->continue_stmt.target==NULL && new_list!=NULL);
	
	stmt->continue_stmt.target = continue_point;
	TAILQ_INSERT_TAIL(new_list, stmt, link);
	return 0;
}

static int ey_stmt_prepare(ey_engine_t *eng, ey_stmt_t *stmt, ey_stmt_list_t *new_list, 
	ey_stmt_t *break_point, ey_stmt_t *continue_point, ey_stmt_t *return_point)
{
	assert(stmt!=NULL && stmt->class!=STMT_PESUDO && stmt->class!=STMT_JUMP && new_list!=NULL);
	
	int ret = 0;
	switch(stmt->class)
	{
		case STMT_DEC:
		{
			ret = ey_stmt_prepare_dec(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_EXPR:
		{
			ret = ey_stmt_prepare_expr(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_LABEL:
		{
			ret = ey_stmt_prepare_label(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_CASE:
		{
			ret = ey_stmt_prepare_case(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_DEFAULT:
		{
			ret = ey_stmt_prepare_default(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_BLOCK:
		{
			ret = ey_stmt_prepare_block(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_BRANCH:
		{
			ret = ey_stmt_prepare_if(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_GOTO:
		{
			ret = ey_stmt_prepare_goto(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_RETURN:
		{
			ret = ey_stmt_prepare_return(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_WHILE:
		{
			ret = ey_stmt_prepare_while(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_DO_WHILE:
		{
			ret = ey_stmt_prepare_do_while(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_FOR:
		{
			ret = ey_stmt_prepare_for(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_SWITCH:
		{
			ret = ey_stmt_prepare_switch(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_BREAK:
		{
			ret = ey_stmt_prepare_break(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_CONTINUE:
		{
			ret = ey_stmt_prepare_continue(eng, stmt, new_list, break_point, continue_point, return_point);
			break;
		}
		case STMT_PESUDO:
		case STMT_JUMP:
		default:
		{
			*(int*)0 = 0;
		}
	}
	return ret;
}

int ey_function_prepare(ey_engine_t *eng, ey_symbol_t *function, ey_stmt_t *block_stmt)
{
	assert(function!=NULL && function->type!=NULL && function->type->type==TYPE_FUNCTION && function->class==SYMBOL_FUNCTION);
	assert(block_stmt!=NULL && block_stmt->class==STMT_BLOCK);
	
	ey_stmt_t *return_point = ey_alloc_pesudo_stmt(eng, 1, &function->location);
	if(!return_point)
	{
		ey_parser_set_error(eng, &function->location, "alloc return point failed\n");
		return -1;
	}
	
	ey_stmt_list_t new_list;
	TAILQ_INIT(&new_list);

	ey_parser_clear_error(eng);
	if(ey_stmt_prepare_block(eng, block_stmt, &new_list, NULL, NULL, return_point))
	{
		if(!ey_parser_isset_error(eng))
			ey_parser_set_error(eng, &block_stmt->location, "prepare stmt failed\n");
		return -1;
	}

	assert(TAILQ_EMPTY(&block_stmt->block_stmt.stmt_list));
	TAILQ_CONCAT(&block_stmt->block_stmt.stmt_list, &new_list, link);
	return 0;
}
