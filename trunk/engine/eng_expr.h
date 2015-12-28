#ifndef ENG_EXPR_H
#define ENG_EXPR_H 1

#include "ey_queue.h"
#include "eng_location.h"

typedef enum ey_expr_opcode
{
	/*
	 * C compatible unary expression
	 */
	EXPR_OPCODE_DEREF,			/*	*x				*///
	EXPR_OPCODE_SYMBOL,			/*	<var or const>	*///
	EXPR_OPCODE_FUNCALL,		/*	func(pm_list)	*/
	EXPR_OPCODE_ADDRESS,		/*	&x				*///

	/*
	 * C compatiable numeric unary expression
	 */
	EXPR_OPCODE_NEG,			/* -x				*///
	EXPR_OPCODE_LNEG,			/* !x				*///
	EXPR_OPCODE_BNEG,			/* ~y				*///
	EXPR_OPCODE_POST_DEC,		/* x--				*///
	EXPR_OPCODE_POST_INC,		/* x++				*///
	EXPR_OPCODE_POS,			/* +x				*///
	EXPR_OPCODE_PRE_DEC,		/* --x				*///
	EXPR_OPCODE_PRE_INC,		/* ++x				*///
	EXPR_OPCODE_SIZEOF,			/* sizeof			*///

	/*
	 * C compatiable bitwise binary expr
	 * */
	EXPR_OPCODE_BIT_AND,		/* x & y			*///
	EXPR_OPCODE_BIT_OR,			/* x | y			*///
	EXPR_OPCODE_BIT_XOR,		/* x ^ y			*///
	EXPR_OPCODE_LSHIFT,			/* x << y			*///
	EXPR_OPCODE_RSHIFT,			/* x >> y			*///

	/*
	 * C compatiable logical binary expr
	 * */
	EXPR_OPCODE_NE,				/* x != y			*///
	EXPR_OPCODE_LOGIC_OR,		/* x || y			*///
	EXPR_OPCODE_LOGIC_AND,		/* x && y			*///
	EXPR_OPCODE_EQ,				/* x == y			*///
	EXPR_OPCODE_GE,				/* x >= y			*///
	EXPR_OPCODE_GT,				/* x > y			*///
	EXPR_OPCODE_LE,				/* x <= y			*///
	EXPR_OPCODE_LT,				/* x < y			*///
	
	/*
	 * C compatiable math binary expr
	 * */
	EXPR_OPCODE_MOD,			/* x % y			*///
	EXPR_OPCODE_ADD,			/* x + y			*///
	EXPR_OPCODE_DIV,			/* x / y			*///
	EXPR_OPCODE_MULT,			/* x * y			*///
	EXPR_OPCODE_SUB,			/* x - y			*///
	
	/*
	 * C compatiable assignment binary expr
	 * */
	EXPR_OPCODE_ASGN,			/* x = y			*///
	EXPR_OPCODE_INIT_ASGN,		/* (for init)x = y	*///
	EXPR_OPCODE_MOD_ASGN,		/* x %= y			*///
	EXPR_OPCODE_ADD_ASGN,		/* x += y			*///
	EXPR_OPCODE_DIV_ASGN,		/* x /= y			*///
	EXPR_OPCODE_MULT_ASGN,		/* x *= y			*///
	EXPR_OPCODE_SUB_ASGN,		/* x -= y			*///
	EXPR_OPCODE_BIT_AND_ASGN,	/* x &= y			*///
	EXPR_OPCODE_BIT_OR_ASGN,	/* x |= y			*///
	EXPR_OPCODE_BIT_XOR_ASGN,	/* x ^= y			*///
	EXPR_OPCODE_LSHIFT_ASGN,	/* x <<= y			*///
	EXPR_OPCODE_RSHIFT_ASGN,	/* x >>= y			*///

	/*
	 * other C compatiable binary expr
	 * */
	EXPR_OPCODE_ARRAY_INDEX,	/* x[y]				*///
	EXPR_OPCODE_CAST,			/* (<type>) x		*///
	EXPR_OPCODE_COMPOUND,		/* x , y			*///
	EXPR_OPCODE_INIT_COMPOUND,	/* (fot init)x , y	*///
	EXPR_OPCODE_MEMBER,			/* x.y				*///
	EXPR_OPCODE_MEMBER_PTR,		/* x->y				*///

	/*
	 * C compatiable condition expression
	 * */
	EXPR_OPCODE_CONDITION,		/* x?y:z			*///

	/*
	 * eyoung extension
	 */
	EXPR_OPCODE_MATCH,			/* string match		*/
	EXPR_OPCODE_PCRE,			/* pcre match		*/
	EXPR_OPCODE_CMATCH,			/* cluster match	*/
	EXPR_OPCODE_CFUNC			/* cluster func		*/
}ey_expr_opcode_t;

static inline int ey_expr_is_assignment(ey_expr_opcode_t op)
{
	switch(op)
	{
		case EXPR_OPCODE_ASGN:
		case EXPR_OPCODE_MOD_ASGN:
		case EXPR_OPCODE_ADD_ASGN:
		case EXPR_OPCODE_DIV_ASGN:
		case EXPR_OPCODE_MULT_ASGN:
		case EXPR_OPCODE_SUB_ASGN:
		case EXPR_OPCODE_BIT_AND_ASGN:
		case EXPR_OPCODE_BIT_OR_ASGN:
		case EXPR_OPCODE_BIT_XOR_ASGN:
		case EXPR_OPCODE_LSHIFT_ASGN:
		case EXPR_OPCODE_RSHIFT_ASGN:
			return 1;
		default:
			return 0;
	}
}

static inline int ey_expr_is_div(ey_expr_opcode_t op)
{
	switch(op)
	{
		case EXPR_OPCODE_MOD_ASGN:
		case EXPR_OPCODE_DIV_ASGN:
		case EXPR_OPCODE_MOD:
		case EXPR_OPCODE_DIV:
			return 1;
		default:
			return 0;
	}
}

static inline const char* ey_expr_opcode_name(ey_expr_opcode_t type)
{
	switch(type)
	{
		case EXPR_OPCODE_DEREF:
			return "EXPR_OPCODE_DEREF";
		case EXPR_OPCODE_SYMBOL:
			return "EXPR_OPCODE_SYMBOL";
		case EXPR_OPCODE_FUNCALL:
			return "EXPR_OPCODE_FUNCALL";
		case EXPR_OPCODE_ADDRESS:
			return "EXPR_OPCODE_ADDRESS";
		case EXPR_OPCODE_NEG:
			return "EXPR_OPCODE_NEG";
		case EXPR_OPCODE_LNEG:
			return "EXPR_OPCODE_LNEG";
		case EXPR_OPCODE_BNEG:
			return "EXPR_OPCODE_BNEG";
		case EXPR_OPCODE_POST_DEC:
			return "EXPR_OPCODE_POST_DEC";
		case EXPR_OPCODE_POST_INC:
			return "EXPR_OPCODE_POST_INC";
		case EXPR_OPCODE_POS:
			return "EXPR_OPCODE_POS";
		case EXPR_OPCODE_PRE_DEC:
			return "EXPR_OPCODE_PRE_DEC";
		case EXPR_OPCODE_PRE_INC:
			return "EXPR_OPCODE_PRE_INC";
		case EXPR_OPCODE_SIZEOF:
			return "EXPR_OPCODE_SIZEOF";
		case EXPR_OPCODE_BIT_AND:
			return "EXPR_OPCODE_BIT_AND";
		case EXPR_OPCODE_BIT_OR:
			return "EXPR_OPCODE_BIT_OR";
		case EXPR_OPCODE_BIT_XOR:
			return "EXPR_OPCODE_BIT_XOR";
		case EXPR_OPCODE_LSHIFT:
			return "EXPR_OPCODE_LSHIFT";
		case EXPR_OPCODE_RSHIFT:
			return "EXPR_OPCODE_RSHIFT";
		case EXPR_OPCODE_NE:
			return "EXPR_OPCODE_NE";
		case EXPR_OPCODE_LOGIC_OR:
			return "EXPR_OPCODE_LOGIC_OR";
		case EXPR_OPCODE_LOGIC_AND:
			return "EXPR_OPCODE_LOGIC_AND";
		case EXPR_OPCODE_EQ:
			return "EXPR_OPCODE_EQ";
		case EXPR_OPCODE_GE:
			return "EXPR_OPCODE_GE";
		case EXPR_OPCODE_GT:
			return "EXPR_OPCODE_GT";
		case EXPR_OPCODE_LE:
			return "EXPR_OPCODE_LE";
		case EXPR_OPCODE_LT:
			return "EXPR_OPCODE_LT";
		case EXPR_OPCODE_MOD:
			return "EXPR_OPCODE_MOD";
		case EXPR_OPCODE_ADD:
			return "EXPR_OPCODE_ADD";
		case EXPR_OPCODE_DIV:
			return "EXPR_OPCODE_DIV";
		case EXPR_OPCODE_MULT:
			return "EXPR_OPCODE_MULT";
		case EXPR_OPCODE_SUB:
			return "EXPR_OPCODE_SUB";
		case EXPR_OPCODE_ASGN:
			return "EXPR_OPCODE_ASGN";
		case EXPR_OPCODE_INIT_ASGN:
			return "EXPR_OPCODE_INIT_ASGN";
		case EXPR_OPCODE_MOD_ASGN:
			return "EXPR_OPCODE_MOD_ASGN";
		case EXPR_OPCODE_ADD_ASGN:
			return "EXPR_OPCODE_ADD_ASGN";
		case EXPR_OPCODE_DIV_ASGN:
			return "EXPR_OPCODE_DIV_ASGN";
		case EXPR_OPCODE_MULT_ASGN:
			return "EXPR_OPCODE_MULT_ASGN";
		case EXPR_OPCODE_SUB_ASGN:
			return "EXPR_OPCODE_SUB_ASGN";
		case EXPR_OPCODE_BIT_AND_ASGN:
			return "EXPR_OPCODE_BIT_AND_ASGN";
		case EXPR_OPCODE_BIT_OR_ASGN:
			return "EXPR_OPCODE_BIT_OR_ASGN";
		case EXPR_OPCODE_BIT_XOR_ASGN:
			return "EXPR_OPCODE_BIT_XOR_ASGN";
		case EXPR_OPCODE_LSHIFT_ASGN:
			return "EXPR_OPCODE_LSHIFT_ASGN";
		case EXPR_OPCODE_RSHIFT_ASGN:
			return "EXPR_OPCODE_RSHIFT_ASGN";
		case EXPR_OPCODE_ARRAY_INDEX:
			return "EXPR_OPCODE_ARRAY_INDEX";
		case EXPR_OPCODE_CAST:
			return "EXPR_OPCODE_CAST";
		case EXPR_OPCODE_COMPOUND:
			return "EXPR_OPCODE_COMPOUND";
		case EXPR_OPCODE_INIT_COMPOUND:
			return "EXPR_OPCODE_INIT_COMPOUND";
		case EXPR_OPCODE_MEMBER:
			return "EXPR_OPCODE_MEMBER";
		case EXPR_OPCODE_MEMBER_PTR:
			return "EXPR_OPCODE_MEMBER_PTR";
		case EXPR_OPCODE_MATCH:
			return "EXPR_OPCODE_MATCH";
		case EXPR_OPCODE_PCRE:
			return "EXPR_OPCODE_PCRE";
		case EXPR_OPCODE_CMATCH:
			return "EXPR_OPCODE_CMATCH";
		case EXPR_OPCODE_CFUNC:
			return "EXPR_OPCODE_CFUNC";
		case EXPR_OPCODE_CONDITION:
			return "EXPR_OPCODE_CONDITION";
		default:
			*(int*)0 = 0;
			return "Unknown";
	}
}

typedef enum ey_expr_type
{
	EXPR_TYPE_LIST,
	EXPR_TYPE_SYMBOL,
	EXPR_TYPE_UNARY,
	EXPR_TYPE_BINARY,
	EXPR_TYPE_FUNCALL,
	EXPR_TYPE_MEMBER,
	EXPR_TYPE_CONDITION,
	EXPR_TYPE_CAST
}ey_expr_type_t;

static inline const char* ey_expr_type_name(ey_expr_type_t type)
{
	switch(type)
	{
		case EXPR_TYPE_LIST:
			return "LIST_EXPR";
		case EXPR_TYPE_SYMBOL:
			return "SYMBOL_EXPR";
		case EXPR_TYPE_UNARY:
			return "UNARY_EXPR";
		case EXPR_TYPE_BINARY:
			return "BINARY_EXPR";
		case EXPR_TYPE_FUNCALL:
			return "FUNCALL_EXPR";
		case EXPR_TYPE_MEMBER:
			return "MEMBER_EXPR";
		case EXPR_TYPE_CONDITION:
			return "CONDITION_EXPR";
		case EXPR_TYPE_CAST:
			return "CAST_EXPR";
		default:
			*(int*)0 = 0;
			return "Unknown";
	}
}

struct ey_expr;
struct ey_symbol;
struct ey_type;
typedef TAILQ_HEAD(ey_expr_list, ey_expr) ey_expr_list_t;

typedef struct ey_list_expr
{
	ey_expr_list_t head;
}ey_list_expr_t;

typedef struct ey_symbol_expr
{
	struct ey_symbol *symbol;
}ey_symbol_expr_t;

typedef struct ey_unary_expr
{
	struct ey_expr *operand;
}ey_unary_expr_t;

typedef struct ey_binary_expr
{
	struct ey_expr *left;
	struct ey_expr *right;
}ey_binary_expr_t;

typedef struct ey_condition_expr
{
	struct ey_expr *condition;
	struct ey_expr *left;
	struct ey_expr *right;
}ey_condition_expr_t;

typedef struct ey_funcall_expr
{
	struct ey_expr *function;
	ey_expr_list_t arg_list;
}ey_funcall_expr_t;

typedef struct ey_member_expr
{
	struct ey_expr *su;
	struct ey_symbol *member;
}ey_member_expr_t;

typedef struct ey_cast_expr
{
	struct ey_type *type;
	struct ey_expr *expr;
}ey_cast_expr_t;

typedef struct ey_expr
{
	ey_location_t location;
	TAILQ_ENTRY(ey_expr) link;
	ey_expr_opcode_t opcode;
	ey_expr_type_t type;
	unsigned int expr_id;
	struct ey_type *expr_type;
	struct ey_type *promoted_type;
	struct ey_symbol *const_value;
	union
	{
		ey_list_expr_t list_expr;
		ey_symbol_expr_t symbol_expr;
		ey_unary_expr_t unary_expr;
		ey_binary_expr_t binary_expr;
		ey_condition_expr_t condition_expr;
		ey_funcall_expr_t funcall_expr;
		ey_member_expr_t member_expr;
		ey_cast_expr_t cast_expr;
	};
}ey_expr_t;

/*true/false expr var*/
typedef struct ey_expr_const
{
	ey_expr_t *true_expr;
	ey_expr_t *false_expr;
	ey_expr_t *null_expr;
}ey_expr_const_t;

struct ey_engine;
/*init/finit*/
extern int ey_expr_init(struct ey_engine *eng);
extern void ey_expr_finit(struct ey_engine *eng);

/*alloc*/
extern ey_expr_t *ey_alloc_list_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	ey_expr_list_t *head, ey_location_t *location);
extern ey_expr_t *ey_alloc_symbol_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	struct ey_symbol *symbol, ey_location_t *location);
extern ey_expr_t *ey_alloc_unary_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *operand, ey_location_t *location);
extern ey_expr_t *ey_alloc_binary_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *left, ey_expr_t *right, ey_location_t *location);
extern ey_expr_t *ey_alloc_condition_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *condition, ey_expr_t *left, ey_expr_t *right, ey_location_t *location);
extern ey_expr_t *ey_alloc_funcall_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	struct ey_expr *function, ey_expr_list_t *arg_list, ey_location_t *location);
extern ey_expr_t *ey_alloc_member_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *su, struct ey_symbol *member, ey_location_t *location);
extern ey_expr_t *ey_alloc_cast_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	struct ey_type *type, ey_expr_t *expr, ey_location_t *location);
extern ey_expr_t *ey_alloc_list_init_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	ey_expr_list_t *head, ey_location_t *location);
extern ey_expr_t *ey_alloc_symbol_init_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	struct ey_symbol *symbol, ey_location_t *location);
extern ey_expr_t *ey_alloc_unary_init_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *operand, ey_location_t *location);
extern ey_expr_t *ey_alloc_binary_init_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *left, ey_expr_t *right, ey_location_t *location);
extern ey_expr_t *ey_alloc_condition_init_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *condition, ey_expr_t *left, ey_expr_t *right, ey_location_t *location);
extern ey_expr_t *ey_alloc_funcall_init_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	struct ey_expr *function, ey_expr_list_t *arg_list, ey_location_t *location);
extern ey_expr_t *ey_alloc_member_init_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *su, struct ey_symbol *member, ey_location_t *location);
extern ey_expr_t *ey_alloc_cast_init_expr(struct ey_engine *eng, ey_expr_opcode_t opcode, 
	struct ey_type *type, ey_expr_t *expr, ey_location_t *location);


/*type/value*/
extern int ey_eval_const_expr_value(struct ey_engine *eng, ey_expr_t *expr);
extern int ey_expr_is_const_value(struct ey_engine *eng, ey_expr_t *expr);
extern struct ey_symbol *ey_expr_eval_const_value(struct ey_engine *eng, struct ey_symbol *symbol, ey_expr_t *expr);
extern int ey_expr_is_meanless_const_value(struct ey_engine *eng, ey_expr_t *expr);
extern int ey_expr_is_true(struct ey_engine *eng, ey_expr_t *expr);

/*free*/
extern void ey_free_expr(struct ey_engine *eng, ey_expr_t *expr);
extern void ey_free_expr_list(struct ey_engine *eng, ey_expr_list_t *expr_list);

/*print for debug*/
extern void ey_expr_print(struct ey_engine *eng, ey_expr_t *expr, int tab);
#endif
