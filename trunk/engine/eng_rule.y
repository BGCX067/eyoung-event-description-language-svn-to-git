%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <assert.h>

#include "eng_priv.h"
#include "eng_rule_parser.h"
#include "eng_rule_lex.h"

#define YYMALLOC engine_malloc
#define YYFREE engine_free
#define EY_ABORT YYABORT

int eng_rule_error(ENG_RULE_LTYPE *loc, void *eng, const char *format, ...);

static inline void save_loop_context(ey_engine_t *eng, ey_saved_context_t *dst);
static inline void restore_loop_context(ey_engine_t *eng, ey_saved_context_t *src);
static inline void save_switch_context(ey_engine_t *eng, ey_saved_context_t *dst);
static inline void restore_switch_context(ey_engine_t *eng, ey_saved_context_t *src);
static inline void init_function_context(ey_engine_t *eng, ey_type_t *return_type);
static inline void init_enum_context(ey_engine_t *eng);
static inline void ey_parser_push_level(ey_engine_t *eng);
static inline void ey_parser_pop_level(ey_engine_t *eng);
%}

%debug
%verbose
%locations
%defines "eng_rule_parser.h"
%output "eng_rule_parser.c"
%define api.prefix eng_rule_
%define api.pure full
%define api.push-pull push
%parse-param {void *eng}

/*expression*/
%token TOKEN_MINUSARROW			/*	->		*/
%token TOKEN_DOT				/*	.		*/
%token TOKEN_COMMA				/*	,		*/
%token TOKEN_COLON				/*	:		*/
%token TOKEN_SEMI				/*	;		*/
%token TOKEN_QUEST				/*	?		*/
%token TOKEN_DPLUS				/*	++		*/
%token TOKEN_DMINUS				/*	--		*/
%token TOKEN_LPAREN				/*	(		*/
%token TOKEN_RPAREN				/*	)		*/
%token TOKEN_LBRACK				/*	[		*/
%token TOKEN_RBRACK				/*	]		*/
%token TOKEN_LBRACE				/*	{		*/
%token TOKEN_RBRACE				/*	}		*/
%token TOKEN_SIZEOF				/*	sizeof	*/
%token TOKEN_AMPER				/*	&		*/
%token TOKEN_DAMPER				/*	&&		*/
%token TOKEN_STAR				/*	*		*/
%token TOKEN_PLUS				/*	+		*/
%token TOKEN_MINUS				/*	-		*/
%token TOKEN_TILDE				/*	~		*/
%token TOKEN_EXCLAM				/*	!		*/
%token TOKEN_SLASH				/*	/		*/
%token TOKEN_PERCENT			/*	%		*/
%token TOKEN_DLARROW			/*	<<		*/
%token TOKEN_DRARROW			/*	>>		*/
%token TOKEN_LARROW				/*	<		*/
%token TOKEN_LARROWEQ			/*	<=		*/
%token TOKEN_RARROW				/*	>		*/
%token TOKEN_RARROWEQ			/*	>=		*/
%token TOKEN_DEQ				/*	==		*/
%token TOKEN_EXCLAMEQ			/*	!=		*/
%token TOKEN_UPARROW			/*	^		*/
%token TOKEN_BAR				/*	|		*/
%token TOKEN_DBAR				/*	||		*/
%token TOKEN_EQ					/*	=		*/
%token TOKEN_BAREQ				/*	|=		*/
%token TOKEN_UPARROWEQ			/*	^=		*/
%token TOKEN_AMPEREQ			/*	&=		*/
%token TOKEN_SLASHEQ			/*	/=		*/
%token TOKEN_PERCENTEQ			/*	%=		*/
%token TOKEN_DLARROWEQ			/*	<<=		*/
%token TOKEN_DRARROWEQ			/*	>>=		*/
%token TOKEN_PLUSEQ				/*	+=		*/
%token TOKEN_MINUSEQ			/*	-=		*/
%token TOKEN_STAREQ				/*	*=		*/

%token TOKEN_TYPEDEF			/*	typedef	*/
%token TOKEN_EXTERN				/*	extern	*/
%token TOKEN_STATIC				/*	static	*/
%token TOKEN_AUTO				/*	auto	*/
%token TOKEN_REGISTER			/*	register*/

%token TOKEN_CONST				/*	const	*/
%token TOKEN_RESTRICT			/*	restrict*/
%token TOKEN_VOLATILE			/*	volatile*/

%token TOKEN_INLINE				/*	inline	*/

%token TOKEN_ELLIPSIS			/*	...		*/

%token TOKEN_CONST_CHAR			/*	'a'		*/
%token TOKEN_CONST_INT			/*	123		*/
%token TOKEN_CONST_UINT			/*	123U	*/
%token TOKEN_CONST_LONG			/*	123L	*/
%token TOKEN_CONST_ULONG		/*	123UL	*/
%token TOKEN_CONST_LONGLONG		/*	123LL	*/
%token TOKEN_CONST_ULONGLONG	/*	123ULL	*/
%token TOKEN_CONST_FLOAT		/*	0.1f	*/
%token TOKEN_CONST_DOUBLE		/*	0.1lf	*/
%token TOKEN_CONST_STRING		/*	"eyoung"*/

%token TOKEN_IDENT				/*	<name>	*/
%token TOKEN_ENUM_IDENT			/*	enum const*/
%token TOKEN_TYPEDEF_IDENT		/*	typedef name*/

%token TOKEN_VOID				/*	void	*/
%token TOKEN_CHAR				/*	char	*/
%token TOKEN_SHORT				/*	short	*/
%token TOKEN_INT				/*	int		*/
%token TOKEN_LONG				/*	long	*/
%token TOKEN_FLOAT				/*	float	*/
%token TOKEN_DOUBLE				/*	double	*/
%token TOKEN_SIGNED				/*	signed	*/
%token TOKEN_UNSIGNED			/*	unsigned*/
%token TOKEN_BOOL				/*	_Bool	*/
%token TOKEN_COMPLEX			/*	_Complex*/
%token TOKEN_IMAGINARY			/*	_Imaginary*/

%token TOKEN_STRUCT				/*	struct	*/
%token TOKEN_UNION				/*	union	*/
%token TOKEN_ENUM				/*	enum	*/

%token TOKEN_CASE				/*	case	*/
%token TOKEN_DEFAULT			/*	default	*/
%token TOKEN_GOTO				/*	goto	*/
%token TOKEN_IF					/*	if		*/
%token TOKEN_ELSE				/*	else	*/
%token TOKEN_FOR				/*	for		*/
%token TOKEN_WHILE				/*	while	*/
%token TOKEN_DO					/*	do		*/
%token TOKEN_SWITCH				/*	switch	*/
%token TOKEN_RETURN				/*	return	*/
%token TOKEN_CONTINUE			/*	continue*/
%token TOKEN_BREAK				/*	break	*/

%nonassoc TOKEN_IF
%nonassoc TOKEN_ELSE

%union
{
	signed char signed_char;
	unsigned char unsigned_char;
	signed short signed_short;
	unsigned short unsigned_short;
	signed int signed_int;
	unsigned int unsigned_int;
	signed long signed_long;
	unsigned long unsigned_long;
	float float_real;
	double double_real;
	char *string;
	char *ident;
	char *typename;

	ey_saved_context_t	saved_context;
	ey_expr_opcode_t	expr_opcode;
	ey_stmt_t			*stmt;
	ey_symbol_t			*symbol;
	ey_expr_t			*expr;
	ey_type_t			*type;
	ey_member_t			*member;
	ey_expr_list_t		expr_list;
	ey_symbol_list_t	symbol_list;
	ey_stmt_list_t		stmt_list;
	ey_member_list_t	member_list;
	ey_arg_list_t		arg_list;
	unsigned int		type_qualifier;
	int					storage_class;
	int					inline_func;
	int					is_struct;
}

/*term type specification*/
%type <signed_char>		TOKEN_CONST_CHAR
%type <signed_int>		TOKEN_CONST_INT
%type <unsigned_int>	TOKEN_CONST_UINT
%type <signed_long>		TOKEN_CONST_LONG
%type <unsigned_long>	TOKEN_CONST_ULONG
%type <signed_long>		TOKEN_CONST_LONGLONG
%type <unsigned_long>	TOKEN_CONST_ULONGLONG
%type <float_real>		TOKEN_CONST_FLOAT
%type <double_real>		TOKEN_CONST_DOUBLE
%type <string>			TOKEN_CONST_STRING
%type <ident>			TOKEN_ENUM_IDENT
%type <ident>			TOKEN_IDENT
%type <typename>		TOKEN_TYPEDEF_IDENT

/*non-term type specification*/
%type <expr>			primary_expression
%type <expr>			expression
%type <expr>			postfix_expression
%type <expr>			unary_expression
%type <expr>			cast_expression
%type <expr>			multiplicative_expression
%type <expr>			additive_expression
%type <expr>			shift_expression
%type <expr>			relational_expression
%type <expr>			equality_expression
%type <expr>			and_expression
%type <expr>			exclusive_or_expression
%type <expr>			inclusive_or_expression
%type <expr>			logical_and_expression
%type <expr>			logical_or_expression
%type <expr>			conditional_expression
%type <expr>			assignment_expression
%type <expr>			assignment_expression_opt
%type <expr>			constant_expression
%type <expr>			expression_opt
%type <expr>			designator
%type <expr>			designator_list
%type <expr>			designation
%type <expr>			initializer
%type <expr>			initializer_opt
%type <expr>			initializer_list

%type <ident>			identifier

%type <storage_class>	storage_class_specifier

%type <type_qualifier>	type_qualifier
%type <type_qualifier>	type_qualifier_list_opt
%type <type_qualifier>	type_qualifier_list

%type <inline_func>		function_specifier

%type <type>			type_name
%type <type>			declaration_specifiers
%type <type>			type_specifier
%type <type>			struct_or_union_specifier
%type <type>			enum_specifier
%type <type>			typedef_name
%type <type>			specifier_qualifier_list
%type <type>			pointer
%type <type>			abstract_declarator
%type <type>			direct_abstract_declarator

%type <member>			struct_declarator

%type <member_list>		struct_declaration_list
%type <member_list>		struct_declarator_list
%type <member_list>		struct_declaration

%type <is_struct>		struct_or_union

%type <expr_opcode>		unary_operator
%type <expr_opcode>		assignment_operator

%type <expr_list>		argument_expression_list

%type <symbol>			constant
%type <symbol>			enumeration_constant
%type <symbol>			string_literal
%type <symbol>			init_declarator
%type <symbol>			enumerator
%type <symbol>			direct_declarator
%type <symbol>			parameter_declaration
%type <symbol>			declarator

%type <symbol_list>		declaration
%type <symbol_list>		declaration_list
%type <symbol_list>		init_declarator_list
%type <symbol_list>		enumerator_list
%type <symbol_list>		parameter_list
%type <symbol_list>		identifier_list

%type <arg_list>		parameter_type_list_opt

%type <stmt>			statement
%type <stmt>			labeled_statement
%type <stmt>			compound_statement
%type <stmt>			expression_statement
%type <stmt>			selection_statement
%type <stmt>			iteration_statement
%type <stmt>			jump_statement
%type <stmt>			block_item
%type <stmt>			simple_if

%type <stmt_list>		block_item_list

%start translation_unit
%%
/*external defines*/
translation_unit
	: external_declaration
	{
		/*
		 * Section 6.9 of ISO/IEC 9899:1999
		 *  The storage-class specifiers auto and register shall not appear in the declaration
		 *  specifiers in an external declaration.
		 *
		 *  There shall be no more than one external definition for each identifier declared with
		 *  internal linkage in a translation unit. Moreover, if an identifier declared with internal
		 *  linkage is used in an expression (other than as a part of the operand of a sizeof
		 *  operator whose result is an integer constant), there shall be exactly one external definition
		 *  for the identifier in the translation unit.
		 *
		 *  As discussed in 5.1.1.1, the unit of program text after preprocessing is a translation unit,
		 *  which consists of a sequence of external declarations. These are described as "external"
		 *  because they appear outside any function (and hence have file scope). As discussed in
		 *  6.7, a declaration that also causes storage to be reserved for an object or a function named
		 *  by the identifier is a definition.
		 *
		 *  Anexternal definition is an external declaration that is also a definition of a function
		 *  (other than an inline definition) or an object. If an identifier declared with external
		 *  linkage is used in an expression (other than as part of the operand of a sizeof operator
		 *  whose result is an integer constant), somewhere in the entire program there shall be
		 *  exactly one external definition for the identifier; otherwise, there shall be no more than
		 *  one.
		 * */
	}
	| translation_unit external_declaration
	{
	}
	;

external_declaration
	: function_definition
	{
		/*
		 * Section 6.9.1 of ISO/IEC 9899:1999
		 *  The identifier declared in a function definition (which is the name of the function) shall
		 *  have a function type, as specified by the declarator portion of the function definition.
		 *
		 *  The return type of a function shall be void or an object type other than array type.
		 *
		 *  The storage-class specifier, if any, in the declaration specifiers shall be either extern or
		 *  static.
		 *
		 *  If the declarator includes a parameter type list, the declaration of each parameter shall
		 *  include an identifier, except for the special case of a parameter list consisting of a single
		 *  parameter of type void, in which case there shall not be an identifier. No declaration list
		 *  shall follow.
		 *
		 *  If the declarator includes an identifier list, each declaration in the declaration list shall
		 *  have at least one declarator, those declarators shall declare only identifiers from the
		 *  identifier list, and every identifier in the identifier list shall be declared. An identifier
		 *  declared as a typedef name shall not be redeclared as a parameter. The declarations in the
		 *  declaration list shall contain no storage-class specifier other than register and no
		 *  initializations.
		 *
		 *  The declarator in a function definition specifies the name of the function being defined
		 *  and the identifiers of its parameters. If the declarator includes a parameter type list, the
		 *  list also specifies the types of all the parameters; such a declarator also serves as a
		 *  function prototype for later calls to the same function in the same translation unit. If the
		 *  declarator includes an identifier list, the types of the parameters shall be declared in a
		 *  following declaration list. In either case, the type of each parameter is adjusted as
		 *  described in 6.7.5.3 for a parameter type list; the resulting type shall be an object type.
		 *
		 *  If a function that accepts a variable number of arguments is defined without a parameter
		 *  type list that ends with the ellipsis notation, the behavior is undefined.
		 *
		 *  Each parameter has automatic storage duration. Its identifier is an lvalue, which is in
		 *  effect declared at the head of the compound statement that constitutes the function body
		 *  (and therefore cannot be redeclared in the function body except in an enclosed block).
		 *  The layout of the storage for parameters is unspecified.
		 *
		 *  On entry to the function, the size expressions of each variably modified parameter are
		 *  evaluated and the value of each argument expression is converted to the type of the
		 *  corresponding parameter as if by assignment. (Array expressions and function
		 *  designators as arguments were converted to pointers before the call.)
		 *
		 *  After all parameters have been assigned, the compound statement that constitutes the
		 *  body of the function definition is executed.
		 *
		 *  If the } that terminates a function is reached, and the value of the function call is used by
		 *  the caller, the behavior is undefined.
		 * */
	}
	| declaration
	{
		/*
		 * Section 6.9.1 of ISO/IEC 9899:1999
		 *  If the declaration of an identifier for an object has file scope and an initializer, the
		 *  declaration is an external definition for the identifier.
		 *
		 *  A declaration of an identifier for an object that has file scope without an initializer, and
		 *  without a storage-class specifier or with the storage-class specifier static, constitutes a
		 *  tentative definition. If a translation unit contains one or more tentative definitions for an
		 *  identifier, and the translation unit contains no external definition for that identifier, then
		 *  the behavior is exactly as if the translation unit contains a file scope declaration of that
		 *  identifier, with the composite type as of the end of the translation unit, with an initializer
		 *  equal to 0.
		 *
		 *  If the declaration of an identifier for an object is a tentative definition and has internal
		 *  linkage, the declared type shall not be an incomplete type.
		 * */
	}
	;

function_definition
	: declaration_specifiers declarator 
	{
		ey_symbol_t *func_symbol = $2;
		ey_type_t *func_type = $2->type;
		
		/*check function type*/
		if(func_type->type != TYPE_FUNCTION)
		{
			engine_parser_error("ident %s is not decleared as %s, not a function\n", 
				func_symbol->name, ey_type_type_name(func_type->type));
			EY_ABORT;
		}
		
		/*check return type*/
		ey_type_t *return_type = ey_type_normalize(eng, $1);
		if(!return_type)
		{
			engine_parser_error("simplify type %s failed\n", ey_type_type_name($1->type));
			EY_ABORT;
		}
		
		return_type = ey_type_merge(eng, func_type, return_type);
		if(!return_type)
		{
			engine_parser_error("merge type between %s and %s failed\n",
				func_type->function_type.return_type?
					ey_type_type_name(func_type->function_type.return_type->type):"NULL", 
				ey_type_type_name($1->type));
			EY_ABORT;
		}

		if(!ey_type_is_declared(eng, return_type))
		{
			engine_parser_error("function return type is not defined\n");
			EY_ABORT;
		}
		func_type->function_type.return_type = return_type;

		/*check parameter name and type*/
		ey_symbol_t *parameter = NULL;
		TAILQ_FOREACH(parameter, &func_type->function_type.arg_list, list_next)
		{
			if(!parameter->name)
			{
				engine_parser_error("parameter name is NULL\n");
				EY_ABORT;
			}

			if(!parameter->type || !ey_type_is_declared(eng, parameter->type))
			{
				engine_parser_error("parameter %s type is not declared\n", parameter->name);
				EY_ABORT;
			}
		}

		/*check redefined ident*/
		ey_symbol_t *symbol_hash = ey_find_level_symbol(eng, func_symbol->name, func_symbol->level, SYMBOL_TABLE_IDENT);
		if(symbol_hash)
		{
			if(ey_symbol_check_flag(symbol_hash, SYMBOL_FLAG_DEFINE))
			{
				engine_parser_error("function name is redefined in %s-%d:%d-%d:%d\n", print_location(&symbol_hash->location));
				EY_ABORT;
			}

			if(symbol_hash->type)
			{
				if(symbol_hash->type->type != TYPE_FUNCTION)
				{
					engine_parser_error("ident is already declared as %s\n", ey_type_type_name(symbol_hash->type->type));
					EY_ABORT;
				}

				if(!ey_type_function_equal(eng, symbol_hash->type, func_type, 1))
				{
					engine_parser_error("function type declared in %s-%d:%d-%d:%d is different from here\n",
						print_location(&symbol_hash->location));
					EY_ABORT;
				}
			}

			symbol_hash->type = func_type;
			symbol_hash->value = ey_alloc_symbol_value(eng, ey_pvoid_type(eng));
			if(!symbol_hash->value)
			{
				engine_parser_error("alloc function value failed\n");
				EY_ABORT;
			}
			*(ey_stmt_t**)(symbol_hash->value) = NULL;
			ey_symbol_set_flag(symbol_hash, SYMBOL_FLAG_DECLARE);
		}
		else
		{
			func_symbol->value = ey_alloc_symbol_value(eng, ey_pvoid_type(eng));
			if(!func_symbol->value)
			{
				engine_parser_error("alloc function value failed\n");
				EY_ABORT;
			}
			*(ey_stmt_t**)(func_symbol->value) = NULL;
			ey_symbol_set_flag(func_symbol, SYMBOL_FLAG_DECLARE);
			ey_insert_symbol(eng, func_symbol, SYMBOL_TABLE_IDENT);
		}

		/*NOTE: 
		 *	1, here we need insert all paramters into ident hash
		 *	2, but new type name like "int foo(struct s{int a;} *param)" is not supported
		 *	*/
		assert(ey_parser_level(eng) == 0);
		TAILQ_FOREACH(parameter, &func_type->function_type.arg_list, list_next)
		{
			assert(parameter->level == 1);
			ey_insert_symbol(eng, parameter, SYMBOL_TABLE_IDENT);
		}

		$<symbol>$ = symbol_hash;
		init_function_context(eng, return_type);
	}
	compound_statement
	{
		if(!TAILQ_EMPTY(&ey_func_undefined_label(eng)))
		{
			ey_symbol_t *label = NULL;
			TAILQ_FOREACH(label, &ey_func_undefined_label(eng), list_next)
				engine_parser_error("label %s is used but not defined\n", label->name);
			EY_ABORT;
		}
		ey_symbol_set_flag($<symbol>3, SYMBOL_FLAG_DEFINE);

		if(ey_function_prepare(eng, $<symbol>3, $4))
		{
			engine_parser_error("prepare function %s failed\n", $<symbol>3->name);
			EY_ABORT;
		}
	}
	| declaration_specifiers declarator declaration_list 
	{
		ey_symbol_t *func_symbol = $2;
		ey_type_t *func_type = $2->type;
		
		/*check function type*/
		if(func_type->type != TYPE_FUNCTION)
		{
			engine_parser_error("ident %s is not decleared as %s, not a function\n", 
				func_symbol->name, ey_type_type_name(func_type->type));
			EY_ABORT;
		}
		
		/*check return type*/
		ey_type_t *return_type = ey_type_normalize(eng, $1);
		if(!return_type)
		{
			engine_parser_error("simplify type %s failed\n", ey_type_type_name($1->type));
			EY_ABORT;
		}
		
		return_type = ey_type_merge(eng, func_type, return_type);
		if(!return_type)
		{
			engine_parser_error("merge type between %s and %s failed\n",
				func_type->function_type.return_type?
					ey_type_type_name(func_type->function_type.return_type->type):"NULL", 
				ey_type_type_name($1->type));
			EY_ABORT;
		}

		if(!ey_type_is_declared(eng, return_type))
		{
			engine_parser_error("function return type is not defined\n");
			EY_ABORT;
		}
		func_type->function_type.return_type = return_type;
		
		ey_symbol_t *p1 = TAILQ_FIRST(&func_type->function_type.arg_list);
		ey_symbol_t *p2 = TAILQ_FIRST(&$3);
		while(1)
		{
			if(!p1 || !p2)
				break;
			if(p1->type)
			{
				engine_parser_error("parameter %s is defined\n", p1->name);
				EY_ABORT;
			}
			
			if(strcmp(p1->name, p2->name))
			{
				engine_parser_error("parameter name mis-match, %s, %s\n", p1->name, p2->name);
				EY_ABORT;
			}

			if(!p2->type || !ey_type_is_declared(eng, p2->type))
			{
				engine_parser_error("parameter %s type is not declared\n", p2->name);
				EY_ABORT;
			}
			p1->type = p2->type;

			p1 = TAILQ_NEXT(p1, list_next);
			p2 = TAILQ_NEXT(p2, list_next);
		}
		if(p1 != p2)
		{
			engine_parser_error("parameter count mis-match\n");
			EY_ABORT;
		}

		/*check redefined ident*/
		ey_symbol_t *symbol_hash = ey_find_level_symbol(eng, func_symbol->name, func_symbol->level, SYMBOL_TABLE_IDENT);
		if(symbol_hash)
		{
			if(ey_symbol_check_flag(symbol_hash, SYMBOL_FLAG_DEFINE))
			{
				engine_parser_error("function name is redefined in %s-%d:%d-%d:%d\n", print_location(&symbol_hash->location));
				EY_ABORT;
			}

			if(symbol_hash->type)
			{
				if(symbol_hash->type->type != TYPE_FUNCTION)
				{
					engine_parser_error("ident is already declared as %s\n", ey_type_type_name(symbol_hash->type->type));
					EY_ABORT;
				}

				if(!ey_type_function_equal(eng, symbol_hash->type, func_type, 1))
				{
					engine_parser_error("function type declared in %s-%d:%d-%d:%d is different from here\n",
						print_location(&symbol_hash->location));
					EY_ABORT;
				}
			}

			symbol_hash->type = func_type;
			symbol_hash->value = ey_alloc_symbol_value(eng, ey_pvoid_type(eng));
			if(!symbol_hash->value)
			{
				engine_parser_error("alloc function value failed\n");
				EY_ABORT;
			}
			*(ey_stmt_t**)(symbol_hash->value) = NULL;
			ey_symbol_set_flag(symbol_hash, SYMBOL_FLAG_DECLARE);
		}
		else
		{
			func_symbol->value = ey_alloc_symbol_value(eng, ey_pvoid_type(eng));
			if(!func_symbol->value)
			{
				engine_parser_error("alloc function value failed\n");
				EY_ABORT;
			}
			*(ey_stmt_t**)(func_symbol->value) = NULL;
			ey_symbol_set_flag(func_symbol, SYMBOL_FLAG_DECLARE);
			ey_insert_symbol(eng, func_symbol, SYMBOL_TABLE_IDENT);
		}

		/*NOTE: 
		 *	1, here we need insert all paramters into ident hash
		 *	2, but new type name like "int foo(struct s{int a;} *param)" is not supported
		 *	*/
		assert(ey_parser_level(eng) == 0);
		TAILQ_FOREACH(p1, &func_type->function_type.arg_list, list_next)
		{
			assert(p1->level == 1);
			ey_insert_symbol(eng, p1, SYMBOL_TABLE_IDENT);
		}

		$<symbol>$ = symbol_hash;
		init_function_context(eng, return_type);
	}
	compound_statement
	{
		if(!TAILQ_EMPTY(&ey_func_undefined_label(eng)))
		{
			ey_symbol_t *label = NULL;
			TAILQ_FOREACH(label, &ey_func_undefined_label(eng), list_next)
				engine_parser_error("label %s is used but not defined\n", label->name);
			EY_ABORT;
		}
		ey_symbol_set_flag($<symbol>4, SYMBOL_FLAG_DEFINE);

		if(ey_function_prepare(eng, $<symbol>4, $5))
		{
			engine_parser_error("prepare function %s failed\n", $<symbol>4->name);
			EY_ABORT;
		}
	}
	;

declaration_list
	: declaration
	{
		if(TAILQ_EMPTY(&$1))
		{
			engine_parser_error("type specification is not supported here\n");
			EY_ABORT;
		}
	}
	| declaration_list declaration
	{
		if(TAILQ_EMPTY(&$2))
		{
			engine_parser_error("type specification is not supported here\n");
			EY_ABORT;
		}
		
		TAILQ_CONCAT(&$$, &$2, list_next);
	}
	;

/*expression*/
primary_expression
	: identifier
	{
		ey_expr_t *ret = NULL;
		ey_symbol_t *symbol = ey_find_symbol(eng, $1, SYMBOL_TABLE_IDENT);
		if(!symbol)
		{
			engine_parser_error("ident %s is not defined\n", $1);
			EY_ABORT;
		}

		ret = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, symbol, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc ident expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| constant
	{
		ey_expr_t *ret = NULL;
		ret = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, $1, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc const expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| string_literal
	{
		ey_expr_t *ret = NULL;
		ret = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, $1, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc const string expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_LPAREN expression TOKEN_RPAREN
	{
		$$ = $2;
	}
	;

string_literal
	: TOKEN_CONST_STRING
	{
		int str_len = strlen($1)+1;
		char *value = (char*)engine_fzalloc(str_len, value_fslab(eng));
		if(!value)
		{
			engine_parser_error("alloc const string value failed\n");
			EY_ABORT;
		}
		strcpy(value, $1);

		ey_type_t *array_type = ey_alloc_array_type(eng, TYPE_ARRAY, TYPE_QUALIFIER_CONST, 
			str_len, alignment_of(char), ey_parser_location(eng, &@1), ey_const_char_type(eng), str_len, 0);
		if(!array_type)
		{
			engine_parser_error("alloc const string array type failed\n");
			EY_ABORT;
		}
		
		ey_symbol_t *ret = ey_alloc_symbol(eng, NULL, ey_parser_level(eng), SYMBOL_CONST, SYMBOL_STORAGE_NONE, 
			SYMBOL_FLAG_DEFINE, array_type, value, NULL, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc const string array symbol failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

constant
	: TOKEN_CONST_CHAR
	{
		char *value = (char*)ey_alloc_symbol_value(eng, ey_char_type(eng));
		if(!value)
		{
			engine_parser_error("alloc symbol value for const char failed\n");
			EY_ABORT;
		}
		*value = $1;

		ey_symbol_t *ret = ey_alloc_symbol(eng, NULL, ey_parser_level(eng),
			SYMBOL_CONST, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, 
			ey_char_type(eng), value, NULL, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc symbol for const char failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_CONST_INT
	{
		int *value = (int*)ey_alloc_symbol_value(eng, ey_int_type(eng));
		if(!value)
		{
			engine_parser_error("alloc symbol value for const int failed\n");
			EY_ABORT;
		}
		*value = $1;

		ey_symbol_t *ret = ey_alloc_symbol(eng, NULL, ey_parser_level(eng),
			SYMBOL_CONST, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, 
			ey_int_type(eng), value, NULL, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc symbol for const int failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_CONST_UINT
	{
		unsigned int *value = (unsigned int*)ey_alloc_symbol_value(eng, ey_uint_type(eng));
		if(!value)
		{
			engine_parser_error("alloc symbol value for const unsigned int failed\n");
			EY_ABORT;
		}
		*value = $1;

		ey_symbol_t *ret = ey_alloc_symbol(eng, NULL, ey_parser_level(eng),
			SYMBOL_CONST, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, 
			ey_uint_type(eng), value, NULL, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc symbol for const unsigned int failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_CONST_LONG
	{
		long *value = (long*)ey_alloc_symbol_value(eng, ey_long_type(eng));
		if(!value)
		{
			engine_parser_error("alloc symbol value for const long failed\n");
			EY_ABORT;
		}
		*value = $1;

		ey_symbol_t *ret = ey_alloc_symbol(eng, NULL, ey_parser_level(eng),
			SYMBOL_CONST, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, 
			ey_long_type(eng), value, NULL, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc symbol for const long failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_CONST_ULONG
	{
		unsigned long *value = (unsigned long*)ey_alloc_symbol_value(eng, ey_ulong_type(eng));
		if(!value)
		{
			engine_parser_error("alloc symbol value for const unsigned long failed\n");
			EY_ABORT;
		}
		*value = $1;

		ey_symbol_t *ret = ey_alloc_symbol(eng, NULL, ey_parser_level(eng),
			SYMBOL_CONST, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, 
			ey_ulong_type(eng), value, NULL, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc symbol for const unsigned long failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_CONST_LONGLONG
	{
		long *value = (long*)ey_alloc_symbol_value(eng, ey_long_type(eng));
		if(!value)
		{
			engine_parser_error("alloc symbol value for const long long failed\n");
			EY_ABORT;
		}
		*value = $1;

		ey_symbol_t *ret = ey_alloc_symbol(eng, NULL, ey_parser_level(eng),
			SYMBOL_CONST, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, 
			ey_long_type(eng), value, NULL, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc symbol for const long long failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_CONST_ULONGLONG
	{
		unsigned long *value = (unsigned long*)ey_alloc_symbol_value(eng, ey_ulong_type(eng));
		if(!value)
		{
			engine_parser_error("alloc symbol value for const unsigned long long failed\n");
			EY_ABORT;
		}
		*value = $1;

		ey_symbol_t *ret = ey_alloc_symbol(eng, NULL, ey_parser_level(eng),
			SYMBOL_CONST, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, 
			ey_ulong_type(eng), value, NULL, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc symbol for const unsigned long long failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_CONST_FLOAT
	{
		float *value = (float*)ey_alloc_symbol_value(eng, ey_float_type(eng));
		if(!value)
		{
			engine_parser_error("alloc symbol value for const float failed\n");
			EY_ABORT;
		}
		*value = $1;

		ey_symbol_t *ret = ey_alloc_symbol(eng, NULL, ey_parser_level(eng),
			SYMBOL_CONST, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, 
			ey_float_type(eng), value, NULL, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc symbol for const float failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_CONST_DOUBLE
	{
		double *value = (double*)ey_alloc_symbol_value(eng, ey_double_type(eng));
		if(!value)
		{
			engine_parser_error("alloc symbol value for const double failed\n");
			EY_ABORT;
		}
		*value = $1;

		ey_symbol_t *ret = ey_alloc_symbol(eng, NULL, ey_parser_level(eng),
			SYMBOL_CONST, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, 
			ey_double_type(eng), value, NULL, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc symbol for const double failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| enumeration_constant
	{
		$$ = $1;
	}
	;

enumeration_constant
	: TOKEN_ENUM_IDENT
	{
		ey_symbol_t *ret = ey_find_symbol(eng, $1, SYMBOL_TABLE_IDENT);
		if(!ret || ret->class!=SYMBOL_ENUM_CONST || !ret->value)
		{
			engine_parser_error("enum const %s is not defined\n", $1);
			EY_ABORT;
		}
		$$ = ret;
	}
	;

identifier
	: TOKEN_IDENT
	{
		char *ret  = ey_alloc_symbol_name(eng, $1);
		if(!ret)
		{
			engine_parser_error("alloc symbol name failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

postfix_expression
	: primary_expression
	{
		$$ = $1;
	}
	| postfix_expression TOKEN_LBRACK expression TOKEN_RBRACK
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_ARRAY_INDEX, $1, $3, ey_combine_parser_location(eng, &@1, &@4));
		if(!ret)
		{
			engine_parser_error("alloc index expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| postfix_expression TOKEN_LPAREN argument_expression_list TOKEN_RPAREN
	{
		ey_expr_t *ret = ey_alloc_funcall_expr(eng, EXPR_OPCODE_FUNCALL, $1, &$3, ey_combine_parser_location(eng, &@1, &@4));
		if(!ret)
		{
			engine_parser_error("alloc funcall expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| postfix_expression TOKEN_LPAREN TOKEN_RPAREN
	{
		ey_expr_t *ret = ey_alloc_funcall_expr(eng, EXPR_OPCODE_FUNCALL, $1, NULL, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc funcall expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| postfix_expression TOKEN_DOT identifier
	{
		ey_symbol_t *member_symbol = ey_alloc_symbol(eng, $3, ey_parser_level(eng),
			SYMBOL_MEMBER, SYMBOL_STORAGE_NONE, 0, NULL, NULL, NULL, ey_parser_location(eng, &@3));
		if(!member_symbol)
		{
			engine_parser_error("alloc member symbol failed\n");
			EY_ABORT;
		}

		ey_expr_t *ret = ey_alloc_member_expr(eng, EXPR_OPCODE_MEMBER, $1, member_symbol, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc member expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| postfix_expression TOKEN_MINUSARROW identifier
	{
		ey_symbol_t *member_symbol = ey_alloc_symbol(eng, $3, ey_parser_level(eng),
			SYMBOL_MEMBER, SYMBOL_STORAGE_NONE, 0, NULL, NULL, NULL, ey_parser_location(eng, &@3));
		if(!member_symbol)
		{
			engine_parser_error("alloc member symbol failed\n");
			EY_ABORT;
		}

		ey_expr_t *ret = ey_alloc_member_expr(eng, EXPR_OPCODE_MEMBER_PTR, $1, 
			member_symbol, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc member ptr expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| postfix_expression TOKEN_DPLUS
	{
		ey_expr_t *ret = ey_alloc_unary_expr(eng, EXPR_OPCODE_POST_INC, $1, ey_combine_parser_location(eng, &@1, &@2));
		if(!ret)
		{
			engine_parser_error("alloc post-inc expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| postfix_expression TOKEN_DMINUS
	{
		ey_expr_t *ret = ey_alloc_unary_expr(eng, EXPR_OPCODE_POST_DEC, $1, ey_combine_parser_location(eng, &@1, &@2));
		if(!ret)
		{
			engine_parser_error("alloc post-dec expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_LPAREN type_name TOKEN_RPAREN TOKEN_LBRACE initializer_list comma_opt TOKEN_RBRACE
	{
		/*
		 * Section 6.5.2.5 of ISO/IEC 9899:1999
		 *  The type name shall specify an object type or an array of unknown size, but not a variable
		 *  length array type.
		 *
		 *  No initializer shall attempt to provide a value for an object not contained within the entire
		 *  unnamed object specified by the compound literal.
		 *
		 *  If the compound literal occurs outside the body of a function, the initializer list shall
		 *  consist of constant expressions.
		 *
		 *  A postfix expression that consists of a parenthesized type name followed by a braceenclosed
		 *  list of initializers is a compound literal. It provides an unnamed object whose
		 *  value is given by the initializer list.
		 *
		 *  If the type name specifies an array of unknown size, the size is determined by the
		 *  initializer list as specified in 6.7.8, and the type of the compound literal is that of the
		 *  completed array type. Otherwise (when the type name specifies an object type), the type
		 *  of the compound literal is that specified by the type name. In either case, the result is an
		 *  lvalue.
		 *
		 *  The value of the compound literal is that of an unnamed object initialized by the
		 *  initializer list. If the compound literal occurs outside the body of a function, the object
		 *  has static storage duration; otherwise, it has automatic storage duration associated with
		 *  the enclosing block.
		 *
		 *  All the semantic rules and constraints for initializer lists in 6.7.8 are applicable to
		 *  compound literals.
		 *
		 *  String literals, and compound literals with const-qualified types, need not designate
		 *  distinct objects.
		 * */
		ey_symbol_t *symbol = ey_alloc_symbol(eng, NULL, ey_parser_level(eng), 
			ey_parser_level(eng)==0?SYMBOL_GLOBAL:SYMBOL_LOCAL, SYMBOL_STORAGE_NONE, 
			SYMBOL_FLAG_DECLARE|SYMBOL_FLAG_DEFINE|SYMBOL_FLAG_ANONYMOUS, 
			$2, NULL, $5, ey_combine_parser_location(eng, &@1, &@7));
		if(!symbol)
		{
			engine_parser_error("alloc anonymous symbol failed\n");
			EY_ABORT;
		}

		if(ey_symbol_check_type(eng, symbol))
		{
			if(!ey_parser_isset_error(eng))
				engine_parser_error("symbol type check failed\n");
			EY_ABORT;
		}

		if(symbol->class==SYMBOL_GLOBAL)
			ey_symbol_alloc_global_mem(eng, symbol);
		else
			ey_symbol_alloc_local_mem(eng, symbol);

		symbol->init_value = ey_symbol_init_expr(eng, symbol);
		if(!symbol->init_value)
		{
			if(!ey_parser_isset_error(eng))
				engine_parser_error("failed to check init expr\n");
			EY_ABORT;
		}

		ey_expr_t *ret = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, symbol, &symbol->location);
		if(!ret)
		{
			engine_parser_error("failed to alloc anonymous symbol expr\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

argument_expression_list
	: assignment_expression
	{
		ey_expr_t *expr = $1;
		TAILQ_INIT(&$$);
		TAILQ_INSERT_TAIL(&$$, expr, link);
	}
	| argument_expression_list TOKEN_COMMA assignment_expression
	{
		TAILQ_INSERT_TAIL(&$$, $3, link);
	}
	;

unary_expression
	: postfix_expression
	{
		$$ = $1;
	}
	| TOKEN_DPLUS unary_expression
	{
		ey_expr_t *ret = ey_alloc_unary_expr(eng, EXPR_OPCODE_PRE_INC, $2, ey_combine_parser_location(eng, &@1, &@2));
		if(!ret)
		{
			engine_parser_error("alloc pre-inc expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_DMINUS unary_expression
	{
		ey_expr_t *ret = ey_alloc_unary_expr(eng, EXPR_OPCODE_PRE_DEC, $2, ey_combine_parser_location(eng, &@1, &@2));
		if(!ret)
		{
			engine_parser_error("alloc pre-dec expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| unary_operator cast_expression
	{
		ey_expr_t *ret = ey_alloc_unary_expr(eng, $1, $2, ey_combine_parser_location(eng, &@1, &@2));
		if(!ret)
		{
			engine_parser_error("alloc %s expr failed\n", ey_expr_opcode_name($1));
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_SIZEOF unary_expression
	{
		ey_type_t *expr_type = $2->expr_type;
		if(!expr_type || !ey_type_is_declared(eng, expr_type))
		{
			engine_parser_error("invalid expr type\n");
			EY_ABORT;
		}
		
		/*
		 * Section 6.5.3.4 of ISO/IEC 9899:1999
		 *  The sizeof operator shall not be applied to an expression that has function type or an
		 *  incomplete type, to the parenthesized name of such a type, or to an expression that
		 *  designates a bit-field member.
		 *
		 *  The sizeof operator yields the size (in bytes) of its operand, which may be an
		 *  expression or the parenthesized name of a type. The size is determined from the type of
		 *  the operand. The result is an integer. If the type of the operand is a variable length array
		 *  type, the operand is evaluated; otherwise, the operand is not evaluated and the result is an
		 *  integer constant
		 *
		 *  When applied to an operand that has type char, unsigned char ,or signed char ,
		 *  (or a qualified version thereof) the result is 1. When applied to an operand that has array
		 *  type, the result is the total number of bytes in the array.When applied to an operand
		 *  that has structure or union type, the result is the total number of bytes in such an object,
		 *  including internal and trailing padding
		 * */
		if(expr_type->type==TYPE_FUNCTION)
		{
			engine_parser_error("function type cannot support sizeof opcode\n");
			EY_ABORT;
		}
		
		if($2->opcode==EXPR_OPCODE_MEMBER || $2->opcode==EXPR_OPCODE_MEMBER_PTR)
		{
			ey_type_t *su_type = ($2->opcode==EXPR_OPCODE_MEMBER)?
								 ($2->member_expr.su->expr_type):
								 ($2->member_expr.su->expr_type->pointer_type.deref_type);
			ey_member_t *member = ey_type_find_member(eng, su_type, $2->member_expr.member);
			if(member->bit_size)
			{
				engine_parser_error("bitwise member cannot support sizeof opcode\n");
				EY_ABORT;
			}
		}

		void *value = ey_alloc_symbol_value(eng, ey_uint_type(eng));
		if(!value)
		{
			engine_parser_error("alloc sizeof value failed\n");
			EY_ABORT;
		}

		*(unsigned int*)value = expr_type->size;
		ey_symbol_t *symbol = ey_alloc_symbol(eng, NULL, ey_parser_level(eng), 
			SYMBOL_CONST, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, 
			ey_uint_type(eng), value, NULL, ey_combine_parser_location(eng, &@1, &@2));
		if(!symbol)
		{
			engine_parser_error("alloc sizeof symbol failed\n");
			EY_ABORT;
		}

		ey_expr_t *ret = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, symbol, ey_combine_parser_location(eng, &@1, &@2));
		if(!ret)
		{
			engine_parser_error("alloc sizeof expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_SIZEOF TOKEN_LPAREN type_name TOKEN_RPAREN
	{
		ey_type_t *expr_type = $3;
		if(!expr_type || !ey_type_is_declared(eng, expr_type))
		{
			engine_parser_error("invalid expr type\n");
			EY_ABORT;
		}
		
		if(expr_type->type==TYPE_FUNCTION)
		{
			engine_parser_error("function type cannot support sizeof opcode\n");
			EY_ABORT;
		}

		void *value = ey_alloc_symbol_value(eng, ey_uint_type(eng));
		if(!value)
		{
			engine_parser_error("alloc sizeof value failed\n");
			EY_ABORT;
		}

		*(unsigned int*)value = expr_type->size;
		ey_symbol_t *symbol = ey_alloc_symbol(eng, NULL, ey_parser_level(eng), 
			SYMBOL_CONST, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, 
			ey_uint_type(eng), value, NULL, ey_combine_parser_location(eng, &@1, &@4));
		if(!symbol)
		{
			engine_parser_error("alloc sizeof symbol failed\n");
			EY_ABORT;
		}

		ey_expr_t *ret = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, symbol, ey_combine_parser_location(eng, &@1, &@4));
		if(!ret)
		{
			engine_parser_error("alloc sizeof expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

unary_operator
	: TOKEN_AMPER
	{
		$$ = EXPR_OPCODE_ADDRESS;
	}
	| TOKEN_STAR
	{
		$$ = EXPR_OPCODE_DEREF;
	}
	| TOKEN_PLUS
	{
		$$ = EXPR_OPCODE_POS;
	}
	| TOKEN_MINUS
	{
		$$ = EXPR_OPCODE_NEG;
	}
	| TOKEN_TILDE
	{
		$$ = EXPR_OPCODE_BNEG;
	}
	| TOKEN_EXCLAM
	{
		$$ = EXPR_OPCODE_LNEG;
	}
	;

cast_expression
	: unary_expression
	{
		$$ = $1;
	}
	| TOKEN_LPAREN type_name TOKEN_RPAREN cast_expression
	{
		ey_expr_t *ret = ey_alloc_cast_expr(eng, EXPR_OPCODE_CAST, $2, $4, ey_combine_parser_location(eng, &@1, &@4));
		if(!ret)
		{
			engine_parser_error("alloc explicit cast expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

multiplicative_expression
	: cast_expression
	{
		$$ = $1;
	}
	| multiplicative_expression TOKEN_STAR cast_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_MULT, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc multiple expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| multiplicative_expression TOKEN_SLASH cast_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_DIV, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc division expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| multiplicative_expression TOKEN_PERCENT cast_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_MOD, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc mod expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

additive_expression
	: multiplicative_expression
	{
		$$ = $1;
	}
	| additive_expression TOKEN_PLUS multiplicative_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_ADD, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc add expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| additive_expression TOKEN_MINUS multiplicative_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_SUB, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc sub expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

shift_expression
	: additive_expression
	{
		$$ = $1;
	}
	| shift_expression TOKEN_DLARROW additive_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_LSHIFT, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc left shift expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| shift_expression TOKEN_DRARROW additive_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_RSHIFT, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc right shift expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

relational_expression
	: shift_expression
	{
		$$ = $1;
	}
	| relational_expression TOKEN_LARROW shift_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_LT, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc less-than expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| relational_expression TOKEN_RARROW shift_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_GT, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc greater-than expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| relational_expression TOKEN_LARROWEQ shift_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_LE, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc less-or-equal-than expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| relational_expression TOKEN_RARROWEQ shift_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_GE, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc greater-or-equal-than expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

equality_expression
	: relational_expression
	{
		$$ = $1;
	}
	| equality_expression TOKEN_DEQ relational_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_EQ, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc equal expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| equality_expression TOKEN_EXCLAMEQ relational_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_NE, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc not-equal expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

and_expression
	: equality_expression
	{
		$$ = $1;
	}
	| and_expression TOKEN_AMPER equality_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_BIT_AND, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc bitwise AND expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

exclusive_or_expression
	: and_expression
	{
		$$ = $1;
	}
	| exclusive_or_expression TOKEN_UPARROW and_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_BIT_XOR, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc bitwise exclusive OR expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

inclusive_or_expression
	: exclusive_or_expression
	{
		$$ = $1;
	}
	| inclusive_or_expression TOKEN_BAR exclusive_or_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_BIT_OR, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc bitwise or expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

logical_and_expression
	: inclusive_or_expression
	{
		$$ = $1;
	}
	| logical_and_expression TOKEN_DAMPER inclusive_or_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_LOGIC_AND, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc logical AND expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

logical_or_expression
	: logical_and_expression
	{
		$$ = $1;
	}
	| logical_or_expression TOKEN_DBAR logical_and_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, EXPR_OPCODE_LOGIC_OR, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc logical OR expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

conditional_expression
	: logical_or_expression
	{
		$$ = $1;
	}
	| logical_or_expression TOKEN_QUEST expression TOKEN_COLON conditional_expression
	{
		ey_expr_t *ret = ey_alloc_condition_expr(eng, EXPR_OPCODE_CONDITION, $1, $3, $5, ey_combine_parser_location(eng, &@1, &@5));
		if(!ret)
		{
			engine_parser_error("alloc condition expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

assignment_expression
	: conditional_expression
	{
		$$ = $1;
	}
	| unary_expression assignment_operator assignment_expression
	{
		ey_expr_t *ret = ey_alloc_binary_expr(eng, $2, $1, $3, ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc assignment %s expr failed\n", ey_expr_opcode_name($2));
			EY_ABORT;
		}
		$$ = ret;
	}
	;

assignment_operator
	: TOKEN_EQ
	{
		$$ = EXPR_OPCODE_ASGN;
	}
	| TOKEN_BAREQ
	{
		$$ = EXPR_OPCODE_BIT_OR_ASGN;
	}
	| TOKEN_UPARROWEQ
	{
		$$ = EXPR_OPCODE_BIT_XOR_ASGN;
	}
	| TOKEN_AMPEREQ
	{
		$$ = EXPR_OPCODE_BIT_AND_ASGN;
	}
	| TOKEN_SLASHEQ
	{
		$$ = EXPR_OPCODE_DIV_ASGN;
	}
	| TOKEN_PERCENTEQ
	{
		$$ = EXPR_OPCODE_MOD_ASGN;
	}
	| TOKEN_DLARROWEQ
	{
		$$ = EXPR_OPCODE_LSHIFT_ASGN;
	}
	| TOKEN_DRARROWEQ
	{
		$$ = EXPR_OPCODE_RSHIFT_ASGN;
	}
	| TOKEN_PLUSEQ
	{
		$$ = EXPR_OPCODE_ADD_ASGN;
	}
	| TOKEN_MINUSEQ
	{
		$$ = EXPR_OPCODE_SUB_ASGN;
	}
	| TOKEN_STAREQ
	{
		$$ = EXPR_OPCODE_MULT_ASGN;
	}
	;

expression
	: assignment_expression
	{
		$$ = $1;
	}
	| expression TOKEN_COMMA assignment_expression
	{
		if($1->opcode == EXPR_OPCODE_COMPOUND)
		{
			TAILQ_INSERT_TAIL(&$1->list_expr.head, $3, link);
			$1->location = *ey_combine_parser_location(eng, &@1, &@3);
			$$ = $1;
		}
		else
		{
			ey_expr_list_t head;
			TAILQ_INIT(&head);
			ey_expr_t *ret = ey_alloc_list_expr(eng, EXPR_OPCODE_COMPOUND, &head, ey_combine_parser_location(eng, &@1, &@3));
			if(!ret)
			{
				engine_parser_error("alloc compound expr failed\n");
				EY_ABORT;
			}

			TAILQ_INSERT_TAIL(&ret->list_expr.head, $1, link);
			TAILQ_INSERT_TAIL(&ret->list_expr.head, $3, link);
			$$ = ret;
		}
	}
	;

constant_expression
	: conditional_expression
	{
		$$ = $1;
	}
	;

/*declaration*/
declaration
	: declaration_specifiers TOKEN_SEMI
	{
		/*
		 * "Scopes of identifiers" Section 6.2.1 of ISO/IEC 9899:1999
		 *   An identifier can denote an object; a function; a tag or a member of a structure, union, or
		 *   enumeration; a typedef name; a label name; a macro name; or a macro parameter. The
		 *   same identifier can denote different entities at different points in the program. A member
		 *   of an enumeration is called an enumeration constant. Macro names and macro
		 *   parameters are not considered further here, because prior to the semantic phase of
		 *   program translation any occurrences of macro names in the source file are replaced by the
		 *   preprocessing token sequences that constitute their macro definitions.
		 *
		 *   For each different entity that an identifier designates, the identifier is visible (i.e., can be
		 *   used) only within a region of program text called its scope. Different entities designated
		 *   by the same identifier either have different scopes, or are in different name spaces. There
		 *   are four kinds of scopes: function, file, block, and function prototype. (A function
		 *   prototype is a declaration of a function that declares the types of its parameters.)
		 *
		 *   A label name is the only kind of identifier that has function scope. It can be used (in a
		 *   goto statement) anywhere in the function in which it appears, and is declared implicitly
		 *   by its syntactic appearance (followed by a : and a statement).
		 *
		 *   Every other identifier has scope determined by the placement of its declaration (in a
		 *   declarator or type specifier). If the declarator or type specifier that declares the identifier
		 *   appears outside of any block or list of parameters, the identifier has file scope, which
		 *   terminates at the end of the translation unit. If the declarator or type specifier that
		 *   declares the identifier appears inside a block or within the list of parameter declarations in
		 *   a function definition, the identifier has block scope, which terminates at the end of the
		 *   associated block. If the declarator or type specifier that declares the identifier appears
		 *   within the list of parameter declarations in a function prototype (not part of a function
		 *   definition), the identifier has function prototype scope, which terminates at the end of the
		 *   function declarator. If an identifier designates two different entities in the same name
		 *   space, the scopes might overlap. If so, the scope of one entity (the inner scope) will be a
		 *   strict subset of the scope of the other entity (the outer scope). Within the inner scope, the
		 *   identifier designates the entity declared in the inner scope; the entity declared in the outer
		 *   scope is hidden (and not visible) within the inner scope.
		 *
		 *   Unless explicitly stated otherwise, where this International Standard uses the term
		 *   identifier to refer to some entity (as opposed to the syntactic construct), it refers to the
		 *   entity in the relevant name space whose declaration is visible at the point the identifier
		 *   occurs.
		 *
		 *   Two identifiers have the same scope if and only if their scopes terminate at the same
		 *   point.
		 *
		 *	 Structure, union, and enumeration tags have scope that begins just after the appearance of
		 *	 the tag in a type specifier that declares the tag. Each enumeration constant has scope that
		 *	 begins just after the appearance of its defining enumerator in an enumerator list. Any
		 *	 other identifier has scope that begins just after the completion of its declarator.
		 * */

		/*
		 * "Linkages of identifiers" Section 6.2.2 of ISO/IEC 9899:1999
		 *  An identifier declared in different scopes or in the same scope more than once can be
		 *  made to refer to the same object or function by a process called linkage. There are
		 *  three kinds of linkage: external, internal, and none.
		 *
		 *  In the set of translation units and libraries that constitutes an entire program, each
		 *  declaration of a particular identifier with external linkage denotes the same object or
		 *  function. Within one translation unit, each declaration of an identifier with internal
		 *  linkage denotes the same object or function. Each declaration of an identifier with no
		 *  linkage denotes a unique entity.
		 *
		 *  If the declaration of a file scope identifier for an object or a function contains the storageclass
		 *  specifier static, the identifier has internal linkage
		 *
		 *  For an identifier declared with the storage-class specifier extern in a scope in which a
		 *  prior declaration of that identifier is visible, if the prior declaration specifies internal or
		 *  external linkage, the linkage of the identifier at the later declaration is the same as the
		 *  linkage specified at the prior declaration. If no prior declaration is visible, or if the prior
		 *  declaration specifies no linkage, then the identifier has external linkage.
		 *
		 *  If the declaration of an identifier for a function has no storage-class specifier, its linkage
		 *  is determined exactly as if it were declared with the storage-class specifier extern. If
		 *  the declaration of an identifier for an object has file scope and no storage-class specifier,
		 *  its linkage is external.
		 *
		 *  The following identifiers have no linkage: an identifier declared to be anything other than
		 *  an object or a function; an identifier declared to be a function parameter; a block scope
		 *  identifier for an object declared without the storage-class specifier extern.
		 *
		 *  If, within a translation unit, the same identifier appears with both internal and external
		 *  linkage, the behavior is undefined.
		 * */

		/*
		 * "Name spaces of identifiers" Section 6.2.3 of ISO/IEC 9899:1999
		 *  If more than one declaration of a particular identifier is visible at any point in a
		 *  translation unit, the syntactic context disambiguates uses that refer to different entities.
		 *  Thus, there are separate name spaces for various categories of identifiers, as follows:
		 *    --label names (disambiguated by the syntax of the label declaration and use);
		 *    --the tags of structures, unions, and enumerations (disambiguated by following any24)
		 *    of the keywords struct, union, or enum);
		 *    --the members of structures or unions; each structure or union has a separate name
		 *    space for its members (disambiguated by the type of the expression used to access the
		 *    member via the . or -> operator);
		 *    --all other identifiers, called ordinary identifiers (declared in ordinary declarators or as
		 *    enumeration constants).
		 * */
		TAILQ_INIT(&$$);
	}
	| declaration_specifiers 
	{
		ey_declaration_type(eng) = $1;
	} 
	init_declarator_list TOKEN_SEMI
	{
		ey_declaration_type(eng) = NULL;
		TAILQ_CONCAT(&$$, &$3, list_next);
	}
	;

declaration_specifiers
	: storage_class_specifier declaration_specifiers
	{
		/*
		 * Section 6.7.1 of ISO/IEC 9899:1999
		 *  At most, one storage-class specifier may be given in the declaration specifiers in a
		 *  declaration.
		 *
		 *  The typedef specifier is called a "storage-class specifier" for syntactic convenience
		 *  only; it is discussed in 6.7.7. The meanings of the various linkages and storage durations
		 *  were discussed in 6.2.2 and 6.2.4.
		 *
		 *  A declaration of an identifier for an object with storage-class specifier register
		 *  suggests that access to the object be as fast as possible. The extent to which such
		 *  suggestions are effective is implementation-defined.
		 *
		 *  The declaration of an identifier for a function that has block scope shall have no explicit
		 *  storage-class specifier other than extern.
		 *
		 *  If an aggregate or union object is declared with a storage-class specifier other than
		 *  typedef, the properties resulting from the storage-class specifier, except with respect to
		 *  linkage, also apply to the members of the object, and so on recursively for any aggregate
		 *  or union member objects.
		 * */
		if($2->qualifier_class | EY_STORAGE_CLASS_MASK)
		{
			engine_parser_error("at most, only one storage-class specifier may be given\n");
			EY_ABORT;
		}
		$2->qualifier_class |= $1;
		$$ = $2;
	}
	| storage_class_specifier 
	{
		ey_type_t *type = ey_alloc_simple_type(eng, TYPE_PESUDO, $1, 0, 0, ey_parser_location(eng, &@1));
		if(!type)
		{
			engine_parser_error("alloc inline pesudo type failed\n");
			EY_ABORT;
		}
		$$ = type;
	}
	| type_specifier declaration_specifiers
	{
		ey_type_t *type = ey_type_combine(eng, $1, $2);
		if(!type)
		{
			engine_parser_error("combine type between %s and %s failed\n",
				ey_type_type_name($1->type), ey_type_type_name($2->type));
			EY_ABORT;
		}
		$$ = type;
	}
	| type_specifier 
	{
		$$ = $1;
	}
	| type_qualifier declaration_specifiers
	{
		$2->qualifier_class |= $1;
		$$ = $2;
	}
	| type_qualifier 
	{
		ey_type_t *type = ey_alloc_simple_type(eng, TYPE_PESUDO, $1, 0, 0, ey_parser_location(eng, &@1));
		if(!type)
		{
			engine_parser_error("alloc inline pesudo type failed\n");
			EY_ABORT;
		}
		$$ = type;
	}
	| function_specifier declaration_specifiers
	{
		$2->qualifier_class |= $1;
		$$ = $2;
	}
	| function_specifier 
	{
		ey_type_t *type = ey_alloc_simple_type(eng, TYPE_PESUDO, TYPE_QUALIFIER_INLINE, 0, 0, ey_parser_location(eng, &@1));
		if(!type)
		{
			engine_parser_error("alloc inline pesudo type failed\n");
			EY_ABORT;
		}
		$$ = type;
	}
	;

init_declarator_list
	: init_declarator
	{
		ey_symbol_t *symbol = $1;
		TAILQ_INIT(&$$);
		TAILQ_INSERT_TAIL(&$$, symbol, list_next);
	}
	| init_declarator_list TOKEN_COMMA init_declarator
	{
		TAILQ_INSERT_TAIL(&$$, $3, list_next);
	}
	;

initializer_opt
	: 
	{
		$$ = NULL;
	}
	| TOKEN_EQ initializer
	{
		$$ = $2;
	}
	;

init_declarator
	: declarator 
	{
		/*
		 * Section 6.7.8 of ISO/IEC 9899:1999
		 *  No initializer shall attempt to provide a value for an object not contained within the entity
		 *  being initialized.
		 *
		 *  The type of the entity to be initialized shall be an array of unknown size or an object type
		 *  that is not a variable length array type.
		 *
		 *  All the expressions in an initializer for an object that has static storage duration shall be
		 *  constant expressions or string literals.
		 *
		 *  If the declaration of an identifier has block scope, and the identifier has external or
		 *  internal linkage, the declaration shall have no initializer for the identifier.
		 *
		 *  If a designator has the form
		 *      [ constant-expression ]
		 *  then the current object (defined below) shall have array type and the expression shall be
		 *  an integer constant expression. If the array is of unknown size, any nonnegative value is
		 *  valid.
		 *
		 *  If a designator has the form
		 *      . identifier
		 *  then the current object (defined below) shall have structure or union type and the
		 *  identifier shall be the name of a member of that type.
		 *
		 *  An initializer specifies the initial value stored in an object.
		 *
		 *  Except where explicitly stated otherwise, for the purposes of this subclause unnamed
		 *  members of objects of structure and union type do not participate in initialization.
		 *  Unnamed members of structure objects have indeterminate value even after initialization.
		 *
		 *  If an object that has automatic storage duration is not initialized explicitly, its value is
		 *  indeterminate. If an object that has static storage duration is not initialized explicitly,
		 *  then:
		 *    — if it has pointer type, it is initialized to a null pointer;
		 *    — if it has arithmetic type, it is initialized to (positive or unsigned) zero;
		 *    — if it is an aggregate, every member is initialized (recursively) according to these rules;
		 *    — if it is a union, the first named member is initialized (recursively) according to these
		 *       rules.
		 *  
		 *  The initializer for a scalar shall be a single expression, optionally enclosed in braces. The
		 *  initial value of the object is that of the expression (after conversion); the same type
		 *  constraints and conversions as for simple assignment apply, taking the type of the scalar
		 *  to be the unqualified version of its declared type.
		 *
		 *  The rest of this subclause deals with initializers for objects that have aggregate or union
		 *  type.
		 *
		 *  The initializer for a structure or union object that has automatic storage duration shall be
		 *  either an initializer list as described below, or a single expression that has compatible
		 *  structure or union type. In the latter case, the initial value of the object, including
		 *  unnamed members, is that of the expression.
		 *
		 *  An array of character type may be initialized by a character string literal, optionally
		 *  enclosed in braces. Successive characters of the character string literal (including the
		 *  terminating null character if there is room or if the array is of unknown size) initialize the
		 *  elements of the array.
		 *
		 *  An array with element type compatible with wchar_t may be initialized by a wide
		 *  string literal, optionally enclosed in braces. Successive wide characters of the wide string
		 *  literal (including the terminating null wide character if there is room or if the array is of
		 *  unknown size) initialize the elements of the array.
		 *
		 *  Otherwise, the initializer for an object that has aggregate or union type shall be a braceenclosed
		 *  list of initializers for the elements or named members.
		 *
		 *  Each brace-enclosed initializer list has an associated current object. When no
		 *  designations are present, subobjects of the current object are initialized in order according
		 *  to the type of the current object: array elements in increasing subscript order, structure
		 *  members in declaration order, and the first named member of a union. In contrast, a
		 *  designation causes the following initializer to begin initialization of the subobject
		 *  described by the designator. Initialization then continues forward in order, beginning
		 *  with the next subobject after that described by the designator.
		 *
		 *  Each designator list begins its description with the current object associated with the
		 *  closest surrounding brace pair. Each item in the designator list (in order) specifies a
		 *  particular member of its current object and changes the current object for the next
		 *  designator (if any) to be that member. The current object that results at the end of the
		 *  designator list is the subobject to be initialized by the following initializer.
		 *
		 *  The initialization shall occur in initializer list order, each initializer provided for a
		 *  particular subobject overriding any previously listed initializer for the same subobject; all
		 *  subobjects that are not initialized explicitly shall be initialized implicitly the same as
		 *  objects that have static storage duration.
		 *
		 *  If the aggregate or union contains elements or members that are aggregates or unions,
		 *  these rules apply recursively to the subaggregates or contained unions. If the initializer of
		 *  a subaggregate or contained union begins with a left brace, the initializers enclosed by
		 *  that brace and its matching right brace initialize the elements or members of the
		 *  subaggregate or the contained union. Otherwise, only enough initializers from the list are
		 *  taken to account for the elements or members of the subaggregate or the first member of
		 *  the contained union; any remaining initializers are left to initialize the next element or
		 *  member of the aggregate of which the current subaggregate or contained union is a part.
		 *
		 *  If there are fewer initializers in a brace-enclosed list than there are elements or members
		 *  of an aggregate, or fewer characters in a string literal used to initialize an array of known
		 *  size than there are elements in the array, the remainder of the aggregate shall be
		 *  initialized implicitly the same as objects that have static storage duration.
		 *
		 *  If an array of unknown size is initialized, its size is determined by the largest indexed
		 *  element with an explicit initializer. At the end of its initializer list, the array no longer
		 *  has incomplete type.
		 *
		 *  The order in which any side effects occur among the initialization list expressions is
		 *  unspecified.
		 * */
		$1->init_value = NULL;

		ey_symbol_t *symbol = $1;
		ey_type_t *type1 = ey_declaration_type(eng);
		ey_type_t *type2 = symbol->type;
		ey_type_t *symbol_type = ey_type_merge(eng, type2, type1);
		if(!symbol_type)
		{
			engine_parser_error("merge type failed\n");
			EY_ABORT;
		}
		symbol->type = symbol_type;

		if(ey_symbol_set_storage_class(eng, symbol, type1))
		{
			if(!ey_parser_isset_error(eng))
				engine_parser_error("illegal symbol %s storage class\n", symbol->name);
			EY_ABORT;
		}

		if(ey_symbol_set_class(eng, symbol))
		{
			if(!ey_parser_isset_error(eng))
				engine_parser_error("set symbol %s class failed\n", symbol->name);
			EY_ABORT;
		}

		if(ey_symbol_check_type(eng, symbol))
		{
			if(!ey_parser_isset_error(eng))
				engine_parser_error("symbol type check failed\n");
			EY_ABORT;
		}

		if(ey_symbol_is_declare(eng, symbol))
		{
			if(ey_symbol_declare(eng, symbol))
			{
				if(!ey_parser_isset_error(eng))
					engine_parser_error("failed to declare symbol %s\n", symbol->name);
				EY_ABORT;
			}
		}
		else
		{
			if(ey_symbol_define(eng, symbol))
			{
				if(!ey_parser_isset_error(eng))
					engine_parser_error("failed to define symbol %s\n", symbol->name);
				EY_ABORT;
			}
		}

		$<symbol>$ = $1;
	}
	initializer_opt
	{
		ey_symbol_t *symbol = $<symbol>2;
		symbol->init_value = $3;
		
		if(symbol->init_value)
		{
			if(ey_symbol_is_declare(eng, symbol))
			{
				engine_parser_error("declaration cannot follow init\n");
				EY_ABORT;
			}
			
			symbol->init_value = ey_symbol_init_expr(eng, symbol);
			if(!symbol->init_value)
			{
				if(!ey_parser_isset_error(eng))
					engine_parser_error("failed to check init expr\n");
				EY_ABORT;
			}
		}
		else
		{
			if(!ey_type_is_declared(eng, symbol->type))
			{
				engine_parser_error("undefined symbol type\n");
				EY_ABORT;
			}
		}
		$$ = symbol;
	}
	;

storage_class_specifier
	: TOKEN_TYPEDEF
	{
		/*
		 * Section 6.2.4 of ISO/IEC 9899:1999
		 *  An object has a storage duration that determines its lifetime. There are three storage
		 *  durations: static, automatic, and allocated. Allocated storage is described in 7.20.3.
		 *
		 *  The lifetime of an object is the portion of program execution during which storage is
		 *  guaranteed to be reserved for it. An object exists, has a constant address, and retains
		 *  its last-stored value throughout its lifetime. If an object is referred to outside of its
		 *  lifetime, the behavior is undefined. The value of a pointer becomes indeterminate when
		 *  the object it points to reaches the end of its lifetime.
		 *
		 *  An object whose identifier is declared with external or internal linkage, or with the
		 *  storage-class specifier static has static storage duration. Its lifetime is the entire
		 *  execution of the program and its stored value is initialized only once, prior to program
		 *  startup.
		 *
		 *  An object whose identifier is declared with no linkage and without the storage-class
		 *  specifier static has automatic storage duration.
		 *
		 *  For such an object that does not have a variable length array type, its lifetime extends
		 *  from entry into the block with which it is associated until execution of that block ends in
		 *  any way. (Entering an enclosed block or calling a function suspends, but does not end,
		 *  execution of the current block.) If the block is entered recursively, a new instance of the
		 *  object is created each time. The initial value of the object is indeterminate. If an
		 *  initialization is specified for the object, it is performed each time the declaration is
		 *  reached in the execution of the block; otherwise, the value becomes indeterminate each
		 *  time the declaration is reached.
		 *
		 *  For such an object that does have a variable length array type, its lifetime extends from
		 *  the declaration of the object until execution of the program leaves the scope of the
		 *  declaration. If the scope is entered recursively, a new instance of the object is created
		 *  each time. The initial value of the object is indeterminate.
		 * */
		$$ = TYPE_QUALIFIER_TYPEDEF;
	}
	| TOKEN_EXTERN
	{
		$$ = TYPE_QUALIFIER_EXTERN;
	}
	| TOKEN_STATIC
	{
		$$ = TYPE_QUALIFIER_STATIC;
	}
	| TOKEN_AUTO
	{
		$$ = TYPE_QUALIFIER_AUTO;
	}
	| TOKEN_REGISTER
	{
		$$ = TYPE_QUALIFIER_REGISTER;
	}
	;

type_specifier
	: TOKEN_VOID
	{
		ey_type_t *ret = ey_alloc_simple_type(eng, TYPE_VOID, TYPE_QUALIFIER_NORMAL, 0, 0, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc void type failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_CHAR
	{
		ey_type_t *ret = ey_alloc_simple_type(eng, TYPE_CHAR, TYPE_QUALIFIER_NORMAL, 
			sizeof(char), alignment_of(char), ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc signed char type failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_SHORT
	{
		ey_type_t *ret = ey_alloc_simple_type(eng, TYPE_SHORT, TYPE_QUALIFIER_NORMAL, 
			sizeof(short), alignment_of(short), ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc signed short type failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_INT
	{
		ey_type_t *ret = ey_alloc_simple_type(eng, TYPE_INT, TYPE_QUALIFIER_NORMAL, 
			sizeof(int), alignment_of(int), ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc signed int type failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_LONG
	{
		ey_type_t *ret = ey_alloc_simple_type(eng, TYPE_LONG, TYPE_QUALIFIER_NORMAL, 
			sizeof(long), alignment_of(long), ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc signed long type failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_FLOAT
	{
		ey_type_t *ret = ey_alloc_simple_type(eng, TYPE_FLOAT, TYPE_QUALIFIER_NORMAL, 
			sizeof(float), alignment_of(float), ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc float type failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_DOUBLE
	{
		ey_type_t *ret = ey_alloc_simple_type(eng, TYPE_DOUBLE, TYPE_QUALIFIER_NORMAL, 
			sizeof(double), alignment_of(double), ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc double type failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_SIGNED
	{
		ey_type_t *ret = ey_alloc_simple_type(eng, TYPE_SIGNED, TYPE_QUALIFIER_NORMAL, 
			sizeof(int), alignment_of(int), ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc signed type failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_UNSIGNED
	{
		ey_type_t *ret = ey_alloc_simple_type(eng, TYPE_UNSIGNED, TYPE_QUALIFIER_NORMAL, 
			sizeof(unsigned int), alignment_of(unsigned int), ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc unsigned type failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| struct_or_union_specifier
	{
		/*
		 * Section 6.7.2.1 of ISO/IEC 9899:1999
		 *  A structure or union shall not contain a member with incomplete or function type (hence,
		 *  a structure shall not contain an instance of itself, but may contain a pointer to an instance
		 *  of itself), except that the last member of a structure with more than one named member
		 *  may have incomplete array type; such a structure (and any union containing, possibly
		 *  recursively, a member that is such a structure) shall not be a member of a structure or an
		 *  element of an array.
		 *
		 *  The expression that specifies the width of a bit-field shall be an integer constant
		 *  expression that has nonnegative value that shall not exceed the number of bits in an object
		 *  of the type that is specified if the colon and expression are omitted. If the value is zero,
		 *  the declaration shall have no declarator.
		 *
		 *  A bit-field shall have a type that is a qualified or unqualified version of _Bool, signed
		 *  int , unsigned int, or some other implementation-defined type.
		 *
		 *  As discussed in 6.2.5, a structure is a type consisting of a sequence of members, whose
		 *  storage is allocated in an ordered sequence, and a union is a type consisting of a sequence
		 *  of members whose storage overlap.
		 *
		 *  The presence of a struct-declaration-list in a struct-or-union-specifier declares a new type,
		 *  within a translation unit. The struct-declaration-list is a sequence of declarations for the
		 *  members of the structure or union. If the struct-declaration-list contains no named
		 *  members, the behavior is undefined. The type is incomplete until after the } that
		 *  terminates the list.
		 *
		 *  A member of a structure or union may have any object type other than a variably
		 *  modified type. In addition, a member may be declared to consist of a specified
		 *  number of bits (including a sign bit, if any).  Such a member is called a bit-field;
		 *  its width is preceded by a colon.
		 *
		 *  A bit-field is interpreted as a signed or unsigned integer type consisting of the specified
		 *  number of bits.	If the value 0 or 1 is stored into a nonzero-width bit-field of type
		 *  _Bool, the value of the bit-field shall compare equal to the value stored.
		 *
		 *  An implementation may allocate any addressable storage unit large enough to hold a bit-field.  
		 *  If enough space remains, a bit-field that immediately follows another bit-field in a
		 *  structure shall be packed into adjacent bits of the same unit. If insufficient space remains,
		 *  whether a bit-field that does not fit is put into the next unit or overlaps adjacent units is
		 *  implementation-defined.  The order of allocation of bit-fields within a unit (high-order to
		 *  low-order or low-order to high-order) is implementation-defined. The alignment of the
		 *  addressable storage unit is unspecified.
		 *
		 *  A bit-field declaration with no declarator, but only a colon and a width, indicates an
		 *  unnamed bit-field. As a special case, a bit-field structure member with a width of 0
		 *  indicates that no further bit-field is to be packed into the unit in which the previous bit-
		 *  field, if any, was placed.
		 *
		 *  Each non-bit-field member of a structure or union object is aligned in an implementation-
		 *  defined manner appropriate to its type.
		 *
		 *  Within a structure object, the non-bit-field members and the units in which bit-fields
		 *  reside have addresses that increase in the order in which they are declared. A pointer to a
		 *  structure object, suitably converted, points to its initial member (or if that member is a
		 *  bit-field, then to the unit in which it resides), and vice versa.  There may be unnamed
		 *  padding within a structure object, but not at its beginning.
		 *
		 *  The size of a union is sufficient to contain the largest of its members. The value of at
		 *  most one of the members can be stored in a union object at any time.  A pointer to a
		 *  union object, suitably converted, points to each of its members (or if a member is a bit-field, 
		 *  then to the unit in which it resides), and vice versa.
		 *
		 *  There may be unnamed padding at the end of a structure or union.
		 *
		 *  As a special case, the last element of a structure with more than one named member may
		 *  have an incomplete array type; this is called a flexible array member . With two
		 *  exceptions, the flexible array member is ignored. First, the size of the structure shall be
		 *  equal to the offset of the last element of an otherwise identical structure that replaces the
		 *  flexible array member with an array of unspecified length. Second, when a . (or ->)
		 *  operator has a left operand that is (a pointer to) a structure with a flexible array member
		 *  and the right operand names that member, it behaves as if that member were replaced
		 *  with the longest array (with the same element type) that would not make the structure
		 *  larger than the object being accessed; the offset of the array shall remain that of the
		 *  flexible array member, even if this would differ from that of the replacement array. If this
		 *  array would have no elements, it behaves as if it had one element but the behavior is
		 *  undefined if any attempt is made to access that element or to generate a pointer one past
		 *  it.
		 * */
		$$ = $1;
	}
	| enum_specifier
	{
		/*
		 * Section 6.7.2.2 of ISO/IEC 9899:1999
		 *  The expression that defines the value of an enumeration constant shall be an integer
		 *  constant expression that has a value representable as an "int".
		 *
		 *  The identifiers in an enumerator list are declared as constants that have typeint and
		 *  may appear wherever such are permitted.	An enumerator with = defines its
		 *  enumeration constant as the value of the constant expression.  If the first enumerator has
		 *  no=, the value of its enumeration constant is 0. Each subsequent enumerator with no =
		 *  defines its enumeration constant as the value of the constant expression obtained by
		 *  adding 1 to the value of the previous enumeration constant. (The use of enumerators with
		 *  = may produce enumeration constants with values that duplicate other values in the same
		 *  enumeration.)  The enumerators of an enumeration are also known as its members.
		 *
		 *  Each enumerated type shall be compatible with char, a signed integer type, or an
		 *  unsigned integer type. The choice of type is implementation-defined, but shall be
		 *  capable of representing the values of all the members of the enumeration. The
		 *  enumerated type is incomplete until after the } that terminates the list of enumerator
		 *  declarations.
		 * */
		$$ = $1;
	}
	| typedef_name
	{
		$$ = $1;
	}
	| TOKEN_BOOL
	{
		engine_parser_error("BOOL type is not supported by eyoung\n");
		EY_ABORT;
	}
	| TOKEN_COMPLEX
	{
		engine_parser_error("_Complex type is not supported by eyoung\n");
		EY_ABORT;
	}
	| TOKEN_IMAGINARY
	{
		engine_parser_error("_Imaginary type is not supported by eyoung\n");
		EY_ABORT;
	}
	;

struct_or_union_specifier
	: struct_or_union identifier
	{
		/*
		 * Section 6.7.2.3 of ISO/IEC 9899:1999
		 *  A specific type shall have its content defined at most once.
		 *
		 *  A type specifier of the form
		 *		enum identifier
		 *  without an enumerator list shall only appear after the type it specifies is completed.
		 *
		 *  All declarations of structure, union, or enumerated types that have the same scope and
		 *  use the same tag declare the same type. The type is incomplete until the closing brace
		 *  of the list defining the content, and complete thereafter.
		 *
		 *  Tw o declarations of structure, union, or enumerated types which are in different scopes or
		 *  use different tags declare distinct types. Each declaration of a structure, union, or
		 *  enumerated type which does not include a tag declares a distinct type.
		 *
		 *  A type specifier of the form
		 *		struct-or-union identifier_opt { struct-declaration-list }
		 *  or
		 *		enum identifier { enumerator-list }
		 *  or
		 *		enum identifier { enumerator-list ,}
		 *  declares a structure, union, or enumerated type. The list defines the structure content ,
		 *  union content ,or enumeration content. If an identifier is provided, the type specifier
		 *  also declares the identifier to be the tag of that type.
		 *
		 *  A declaration of the form
		 *		struct-or-union identifier ;
		 *  specifies a structure or union type and declares the identifier as a tag of that type.
		 *
		 *  If a type specifier of the form
		 *		struct-or-union identifier
		 *  occurs other than as part of one of the ab ove  forms, and no other declaration of the
		 *  identifier as a tag is visible, then it declares an incomplete structure or union type, and
		 *  declares the identifier as the tag of that type.
		 *
		 *  If a type specifier of the form
		 *		struct-or-union identifier
		 *  or
		 *		enum identifier
		 *  occurs other than as part of one of the above  forms, and a declaration of the identifier as a
		 *  tag is visible, then it specifies the same type as that other declaration, and does not
		 *  redeclare the tag.
		 * */
		ey_symbol_t *su_tag_symbol = ey_find_symbol(eng, $2, SYMBOL_TABLE_TAG);
		if(su_tag_symbol)
		{
			if(!su_tag_symbol->type)
			{
				engine_parser_error("fatal error, null type for tag symbol %s, defined %s-%d:%d-%d:%d\n", 
					$2, print_location(&su_tag_symbol->location));
				EY_ABORT;
			}

			if(su_tag_symbol->type->type!=($1?TYPE_STRUCT_TAG:TYPE_UNION_TAG))
			{
				engine_parser_error("su tag %s is already define in %s-%d:%d-%d:%d\n", 
					$2, print_location(&su_tag_symbol->location));
				EY_ABORT;
			}

			$$ = su_tag_symbol->type;
		}
		else
		{
			ey_type_t *su_tag_type = ey_alloc_tag_type(eng, 
				$1?TYPE_STRUCT_TAG:TYPE_UNION_TAG, TYPE_QUALIFIER_NORMAL, 
				0, 0, ey_combine_parser_location(eng, &@1, &@2), NULL);
			if(!su_tag_type)
			{
				engine_parser_error("alloc su tag %s type failed\n", $2);
				EY_ABORT;
			}

			su_tag_symbol = ey_alloc_symbol(eng, $2, ey_parser_level(eng), 
				SYMBOL_NAME, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DECLARE,
				su_tag_type, NULL, NULL, ey_combine_parser_location(eng, &@1, &@2));
			if(!su_tag_symbol)
			{
				engine_parser_error("alloc su tag %s symbol failed\n", $2);
				EY_ABORT;
			}
			ey_insert_symbol(eng, su_tag_symbol, SYMBOL_TABLE_TAG);
			$$ = su_tag_type;
		}
	}
	| struct_or_union identifier TOKEN_LBRACE 
	{
		ey_symbol_t *su_tag_symbol = ey_find_level_symbol(eng, $2, ey_parser_level(eng), SYMBOL_TABLE_TAG);
		if(su_tag_symbol)
		{
			if(!su_tag_symbol->type)
			{
				engine_parser_error("fatal error, null type for tag symbol %s, defined %s-%d:%d-%d:%d\n", 
					$2, print_location(&su_tag_symbol->location));
				EY_ABORT;
			}

			if(su_tag_symbol->type->type!=($1?TYPE_STRUCT_TAG:TYPE_UNION_TAG))
			{
				engine_parser_error("su tag %s is already define in %s-%d:%d-%d:%d\n", 
					$2, print_location(&su_tag_symbol->location));
				EY_ABORT;
			}

			if(ey_symbol_check_flag(su_tag_symbol, SYMBOL_FLAG_DEFINE))
			{
				engine_parser_error("su tag %s is re-defined in %s-%d:%d-%d:%d\n", 
					$2, print_location(&su_tag_symbol->location));
				EY_ABORT;
			}

			$<symbol>$ = su_tag_symbol;
		}
		else
		{
			ey_type_t *su_tag_type = ey_alloc_tag_type(eng, 
				($1?TYPE_STRUCT_TAG:TYPE_UNION_TAG), TYPE_QUALIFIER_NORMAL, 
				0, 0, ey_combine_parser_location(eng, &@1, &@2), NULL);
			if(!su_tag_type)
			{
				engine_parser_error("alloc su tag %s type failed\n", $2);
				EY_ABORT;
			}

			su_tag_symbol = ey_alloc_symbol(eng, $2, ey_parser_level(eng), 
				SYMBOL_NAME, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DECLARE,
				su_tag_type, NULL, NULL, ey_combine_parser_location(eng, &@1, &@2));
			if(!su_tag_symbol)
			{
				engine_parser_error("alloc su tag %s symbol failed\n", $2);
				EY_ABORT;
			}
			ey_insert_symbol(eng, su_tag_symbol, SYMBOL_TABLE_TAG);
			$<symbol>$ = su_tag_symbol;
		}
		ey_parser_push_level(eng);
	}
	struct_declaration_list TOKEN_RBRACE
	{
		ey_parser_pop_level(eng);

		ey_symbol_t *su_tag_symbol = $<symbol>4;
		ey_type_t *su_tag_type = su_tag_symbol->type;
		ey_type_t *su_type = ey_alloc_su_type(eng, 
			$1?TYPE_STRUCT:TYPE_UNION, TYPE_QUALIFIER_NORMAL,
			0, 0, ey_combine_parser_location(eng, &@1, &@6),
			su_tag_symbol, &$5);
		if(!su_type)
		{
			engine_parser_error("alloc su %s type failed\n", $2);
			EY_ABORT;
		}
		
		if($1)
		{
			if(!ey_type_set_struct_type(eng, su_type))
			{
				if(!ey_parser_isset_error(eng))
					engine_parser_error("set struct type faile\n");
				EY_ABORT;
			}
		}
		else
		{
			if(!ey_type_set_union_type(eng, su_type))
			{
				if(!ey_parser_isset_error(eng))
					engine_parser_error("set union type faile\n");
				EY_ABORT;
			}
		}

		if(!ey_type_is_declared(eng, su_type))
		{
			engine_parser_error("su type is not complete\n");
			EY_ABORT;
		}

		ey_symbol_set_flag(su_tag_symbol, SYMBOL_FLAG_DEFINE);
		su_tag_type->tag_type.descriptor_type = su_type;
		su_tag_type->size = su_type->size;
		su_tag_type->alignment = su_type->alignment;
		$$ = su_tag_type;
	}
	| struct_or_union TOKEN_LBRACE
	{
		ey_symbol_t *su_tag_symbol = NULL;
		
		ey_type_t *su_tag_type = ey_alloc_tag_type(eng, 
			($1?TYPE_STRUCT_TAG:TYPE_UNION_TAG), TYPE_QUALIFIER_NORMAL, 
			0, 0, ey_combine_parser_location(eng, &@1, &@2), NULL);
		if(!su_tag_type)
		{
			engine_parser_error("alloc su tag %s type failed\n", "");
			EY_ABORT;
		}

		su_tag_symbol = ey_alloc_symbol(eng, NULL, ey_parser_level(eng), 
			SYMBOL_NAME, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DECLARE,
			su_tag_type, NULL, NULL, ey_combine_parser_location(eng, &@1, &@2));
		if(!su_tag_symbol)
		{
			engine_parser_error("alloc su tag %s symbol failed\n", "");
			EY_ABORT;
		}
		$<symbol>$ = su_tag_symbol;

		ey_parser_push_level(eng);
	}
	struct_declaration_list TOKEN_RBRACE
	{
		ey_parser_pop_level(eng);

		ey_symbol_t *su_tag_symbol = $<symbol>3;
		ey_type_t *su_tag_type = su_tag_symbol->type;
		ey_type_t *su_type = ey_alloc_su_type(eng, 
			$1?TYPE_STRUCT:TYPE_UNION, TYPE_QUALIFIER_NORMAL,
			0, 0, ey_combine_parser_location(eng, &@1, &@5),
			su_tag_symbol, &$4);
		if(!su_type)
		{
			engine_parser_error("alloc su %s type failed\n", "");
			EY_ABORT;
		}
		
		if($1)
		{
			if(!ey_type_set_struct_type(eng, su_type))
			{
				if(!ey_parser_isset_error(eng))
					engine_parser_error("set struct type faile\n");
				EY_ABORT;
			}
		}
		else
		{
			if(!ey_type_set_union_type(eng, su_type))
			{
				if(!ey_parser_isset_error(eng))
					engine_parser_error("set union type faile\n");
				EY_ABORT;
			}
		}
		if(!ey_type_is_declared(eng, su_type))
		{
			engine_parser_error("su type is not complete\n");
			EY_ABORT;
		}

		ey_symbol_set_flag(su_tag_symbol, SYMBOL_FLAG_DEFINE);
		su_tag_type->tag_type.descriptor_type = su_type;
		su_tag_type->size = su_type->size;
		su_tag_type->alignment = su_type->alignment;
		$$ = su_tag_type;
	}
	;

struct_or_union
	: TOKEN_STRUCT
	{
		$$ = 1;
	}
	| TOKEN_UNION
	{
		$$ = 0;
	}
	;

struct_declaration_list
	: struct_declaration
	{
		/*NOTE: here need do nothing, for $$ is just $1*/
	}
	| struct_declaration_list struct_declaration
	{
		ey_member_t *member1=NULL, *member2=NULL;
		TAILQ_FOREACH(member2, &$2, member_next)
		{
			TAILQ_FOREACH(member1, &$1, member_next)
			{
				if(!strcmp(member1->member->name, member2->member->name))
					break;
			}
			if(member1)
			{
				engine_parser_error("member %s redefined in %s-%d:%d-%d:%d\n",
					member1->member->name, print_location(&member1->member->location));
				EY_ABORT;
			}
		}
		TAILQ_CONCAT(&$$, &$2, member_next);
	}
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list TOKEN_SEMI
	{
		ey_type_type_t type_type = $1->type;
		ey_type_t *type = ey_type_normalize(eng, $1);
		if(!type)
		{
			engine_parser_error("simplify type %s failed\n", ey_type_type_name(type_type));
			EY_ABORT;
		}

		type_type = type->type;
		ey_member_t *member = NULL;
		TAILQ_FOREACH(member, &$2, member_next)
		{
			type = ey_type_merge(eng, member->member->type, type);
			if(!type)
			{
				engine_parser_error("merge member %s type between %s and %s failed\n", 
					member->member->name,
					member->member->type?ey_type_type_name(member->member->type->type):"NULL",
					ey_type_type_name(type_type));
				EY_ABORT;
			}
		}

		$$ = $2;
	}
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
	{
		ey_type_t *type = ey_type_combine(eng, $1, $2);
		if(!type)
		{
			engine_parser_error("combine type between %s and %s failed\n",
				ey_type_type_name($1->type), ey_type_type_name($2->type));
			EY_ABORT;
		}
		$$ = type;
	}
	| type_specifier 
	{
		$$ = $1;
	}
	| type_qualifier specifier_qualifier_list
	{
		$2->qualifier_class |= $1;
		$$ = $2;
	}
	| type_qualifier 
	{
		ey_type_t *type = ey_alloc_simple_type(eng, TYPE_PESUDO, $1, 0, 0, ey_parser_location(eng, &@1));
		if(!type)
		{
			engine_parser_error("alloc qualified type failed\n");
			EY_ABORT;
		}
		$$ = type;
	}
	;

struct_declarator_list
	: struct_declarator
	{
		ey_member_t *member = $1;
		TAILQ_INIT(&$$);
		TAILQ_INSERT_TAIL(&$$, member, member_next);
	}
	| struct_declarator_list TOKEN_COMMA struct_declarator
	{
		ey_member_t *member = NULL;
		TAILQ_FOREACH(member, &$1, member_next)
		{
			if(!strcmp(member->member->name, $3->member->name))
				break;
		}
		if(member)
		{
			engine_parser_error("member %s redefined in %s-%d:%d-%d:%d\n",
				$3->member->name, print_location(&member->member->location));
			EY_ABORT;
		}
		TAILQ_INSERT_TAIL(&$$, $3, member_next);
	}
	;

struct_declarator
	: declarator
	{
		ey_member_t *ret = ey_alloc_member(eng, $1, 0, 0, 0);
		if(!ret)
		{
			engine_parser_error("alloc member %s failed\n", $1->name);
			EY_ABORT;
		}
		$1->class = SYMBOL_MEMBER;
		$$ = ret;
	}
	| declarator TOKEN_COLON constant_expression
	{
		if(!ey_expr_is_const_value(eng, $3))
		{
			engine_parser_error("bitwise size need const value\n");
			EY_ABORT;
		}

		if(!ey_type_is_integer($3->expr_type->type))
		{
			engine_parser_error("bitwise size need const integer value\n");
			EY_ABORT;
		}
		
		unsigned short bit_size = ey_eval_const_expr_value(eng, $3);
		if(!bit_size)
		{
			engine_parser_error("bitwise size need positive const integer value\n");
			EY_ABORT;
		}

		ey_member_t *ret = ey_alloc_member(eng, $1, 0, 0, bit_size);
		if(!ret)
		{
			engine_parser_error("alloc member %s failed\n", $1->name);
			EY_ABORT;
		}
		$1->class = SYMBOL_MEMBER;
		$$ = ret;
	}
	| TOKEN_COLON constant_expression
	{
		engine_parser_error("anonymous bitwise member is not supported by eyoung\n");
		EY_ABORT;
	}
	;

enum_specifier
	: TOKEN_ENUM identifier TOKEN_LBRACE 
	{
		ey_symbol_t *enum_tag_symbol = ey_find_level_symbol(eng, $2, ey_parser_level(eng), SYMBOL_TABLE_TAG);
		if(enum_tag_symbol)
		{
			if(!enum_tag_symbol->type)
			{
				engine_parser_error("fatal error, null type for tag symbol %s, defined %s-%d:%d-%d:%d\n", $2, print_location(&enum_tag_symbol->location));
				EY_ABORT;
			}

			if(enum_tag_symbol->type->type!=TYPE_ENUM_TAG)
			{
				engine_parser_error("enum tag %s is already define in %s-%d:%d-%d:%d\n", $2, print_location(&enum_tag_symbol->location));
				EY_ABORT;
			}

			if(enum_tag_symbol->type->tag_type.descriptor_type)
			{
				engine_parser_error("enum tag %s is re-defined in %s-%d:%d-%d:%d\n", $2, print_location(&enum_tag_symbol->location));
				EY_ABORT;
			}

			$<symbol>$ = enum_tag_symbol;
		}
		else
		{
			ey_type_t *enum_tag_type = ey_alloc_tag_type(eng, TYPE_ENUM_TAG, TYPE_QUALIFIER_NORMAL, 
				0, 0, ey_combine_parser_location(eng, &@1, &@2), NULL);
			if(!enum_tag_type)
			{
				engine_parser_error("alloc enum tag %s type failed\n", $2);
				EY_ABORT;
			}

			enum_tag_symbol = ey_alloc_symbol(eng, $2, ey_parser_level(eng), 
				SYMBOL_NAME, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DECLARE,
				enum_tag_type, NULL, NULL, ey_combine_parser_location(eng, &@1, &@2));
			if(!enum_tag_symbol)
			{
				engine_parser_error("alloc enum tag %s symbol failed\n", $2);
				EY_ABORT;
			}
			ey_insert_symbol(eng, enum_tag_symbol, SYMBOL_TABLE_TAG);
			$<symbol>$ = enum_tag_symbol;
		}
		init_enum_context(eng);
	}
	enumerator_list comma_opt TOKEN_RBRACE
	{
		ey_symbol_t *enum_tag_symbol = $<symbol>4;
		ey_type_t *enum_tag_type = enum_tag_symbol->type;
		ey_type_t *enum_type = ey_alloc_enum_type(eng, TYPE_ENUM, TYPE_QUALIFIER_NORMAL,
			sizeof(int), alignment_of(int), ey_combine_parser_location(eng, &@1, &@7),
			enum_tag_symbol, &$5);
		if(!enum_type)
		{
			engine_parser_error("alloc enum %s type failed\n", $2);
			EY_ABORT;
		}
		ey_symbol_set_flag(enum_tag_symbol, SYMBOL_FLAG_DEFINE);
		enum_tag_type->tag_type.descriptor_type = enum_type;
		enum_tag_type->size = sizeof(int);
		enum_tag_type->alignment = alignment_of(int);
		$$ = enum_tag_type;
	}
	| TOKEN_ENUM TOKEN_LBRACE
	{
		init_enum_context(eng);
	}
	enumerator_list comma_opt TOKEN_RBRACE
	{
		ey_type_t *enum_tag_type = ey_alloc_tag_type(eng, TYPE_ENUM_TAG, TYPE_QUALIFIER_NORMAL,
			sizeof(int), alignment_of(int), ey_combine_parser_location(eng, &@1, &@6), NULL);
		if(!enum_tag_type)
		{
			engine_parser_error("alloc anonymous enum tag type failed\n");
			EY_ABORT;
		}
		
		ey_symbol_t *enum_tag_symbol = ey_alloc_symbol(eng, NULL, ey_parser_level(eng),
			SYMBOL_NAME, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE,
			enum_tag_type, NULL, NULL, ey_combine_parser_location(eng, &@1, &@6));
		if(!enum_tag_symbol)
		{
			engine_parser_error("alloc anonymous enum tag symbol failed\n");
			EY_ABORT;
		}

		ey_type_t *enum_type = ey_alloc_enum_type(eng, TYPE_ENUM, TYPE_QUALIFIER_NORMAL, 
			sizeof(int), alignment_of(int), ey_combine_parser_location(eng, &@1, &@6),
			enum_tag_symbol, &$4);
		if(!enum_type)
		{
			engine_parser_error("alloc anonymous enum type failed\n");
			EY_ABORT;
		}
		enum_tag_type->tag_type.descriptor_type = enum_type;
		$$ = enum_tag_type;
	}
	| TOKEN_ENUM identifier
	{
		ey_symbol_t *enum_tag_symbol = ey_find_symbol(eng, $2, SYMBOL_TABLE_TAG);
		if(enum_tag_symbol)
		{
			if(!enum_tag_symbol->type)
			{
				engine_parser_error("fatal error, null type for tag symbol %s, defined %s-%d:%d-%d:%d\n", 
					$2, print_location(&enum_tag_symbol->location));
				EY_ABORT;
			}

			if(enum_tag_symbol->type->type!=TYPE_ENUM_TAG)
			{
				engine_parser_error("enum tag %s is already define in %s-%d:%d-%d:%d\n", 
					$2, print_location(&enum_tag_symbol->location));
				EY_ABORT;
			}

			$$ = enum_tag_symbol->type;
		}
		else
		{
			ey_type_t *enum_tag_type = ey_alloc_tag_type(eng, TYPE_ENUM_TAG, TYPE_QUALIFIER_NORMAL, 
				0, 0, ey_combine_parser_location(eng, &@1, &@2), NULL);
			if(!enum_tag_type)
			{
				engine_parser_error("alloc enum tag %s type failed\n", $2);
				EY_ABORT;
			}

			enum_tag_symbol = ey_alloc_symbol(eng, $2, ey_parser_level(eng), 
				SYMBOL_NAME, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DECLARE,
				enum_tag_type, NULL, NULL, ey_combine_parser_location(eng, &@1, &@2));
			if(!enum_tag_symbol)
			{
				engine_parser_error("alloc enum tag %s symbol failed\n", $2);
				EY_ABORT;
			}
			ey_insert_symbol(eng, enum_tag_symbol, SYMBOL_TABLE_TAG);
			$$ = enum_tag_type;
		}
	}
	;

comma_opt
	:
	| TOKEN_COMMA
	;

enumerator_list
	: enumerator
	{
		ey_symbol_t *symbol = $1;
		TAILQ_INIT(&$$);
		TAILQ_INSERT_TAIL(&$$, symbol, list_next);
	}
	| enumerator_list TOKEN_COMMA enumerator
	{
		TAILQ_INSERT_TAIL(&$$, $3, list_next);
	}
	;

enumerator
	: identifier
	{
		ey_symbol_t *enum_symbol = ey_find_level_symbol(eng, $1, ey_parser_level(eng), SYMBOL_TABLE_IDENT);
		if(enum_symbol)
		{
			engine_parser_error("ident redefined, %s is already defined %s-%d:%d-%d:%d\n",
				$1, print_location(&enum_symbol->location));
			EY_ABORT;
		}

		void *enum_symbol_value = ey_alloc_symbol_value(eng, ey_enum_const_type(eng));
		if(!enum_symbol_value)
		{
			engine_parser_error("alloc enum const symbol %s value failed\n", $1);
			EY_ABORT;
		}
		*(int*)enum_symbol_value = ey_enum_value(eng);
		
		enum_symbol = ey_alloc_symbol(eng, $1, ey_parser_level(eng), SYMBOL_ENUM_CONST, SYMBOL_STORAGE_NONE,
			SYMBOL_FLAG_DEFINE, ey_enum_const_type(eng), enum_symbol_value, NULL, ey_parser_location(eng, &@1));
		if(!enum_symbol)
		{
			engine_parser_error("alloc enum const %s symbol failed\n", $1);
			EY_ABORT;
		}
		ey_insert_symbol(eng, enum_symbol, SYMBOL_TABLE_IDENT);
		ey_enum_value(eng)++;
		$$ = enum_symbol;
	}
	| identifier TOKEN_EQ constant_expression
	{
		ey_symbol_t *enum_symbol = ey_find_level_symbol(eng, $1, ey_parser_level(eng), SYMBOL_TABLE_IDENT);
		if(enum_symbol)
		{
			engine_parser_error("ident redefined, %s is already defined %s-%d:%d-%d:%d\n",
				$1, print_location(&enum_symbol->location));
			EY_ABORT;
		}
		
		if(!$3->expr_type || !ey_type_is_integer($3->expr_type->type))
		{
			engine_parser_error("enumerator value for %s is not integer type\n", $1);
			EY_ABORT;
		}

		if(!ey_expr_is_const_value(eng, $3))
		{
			engine_parser_error("enumerator value for %s is not and integer constant\n", $1);
			EY_ABORT;
		}

		void *enum_symbol_value = ey_alloc_symbol_value(eng, ey_enum_const_type(eng));
		if(!enum_symbol_value)
		{
			engine_parser_error("alloc enum const symbol %s value failed\n", $1);
			EY_ABORT;
		}
		*(int*)enum_symbol_value = ey_enum_value(eng) = ey_eval_const_expr_value(eng, $3);
		
		enum_symbol = ey_alloc_symbol(eng, $1, ey_parser_level(eng), SYMBOL_ENUM_CONST, SYMBOL_STORAGE_NONE,
			SYMBOL_FLAG_DEFINE, ey_enum_const_type(eng), enum_symbol_value, NULL, ey_parser_location(eng, &@1));
		if(!enum_symbol)
		{
			engine_parser_error("alloc enum const %s symbol failed\n", $1);
			EY_ABORT;
		}
		ey_insert_symbol(eng, enum_symbol, SYMBOL_TABLE_IDENT);
		ey_enum_value(eng)++;
		$$ = enum_symbol;
	}
	;

type_qualifier
	: TOKEN_CONST
	{
		/*
		 * Section 6.7.3 of ISO/IEC 9899:1999
		 *  Types other than pointer types derived from object or incomplete types shall not be
		 *  restrict-qualified.
		 *
		 *  The properties associated with qualified types are meaningful only for expressions that
		 *  are lvalues.
		 *
		 *  If the same qualifier appears more than once in the samespecifier-qualifier-list , either
		 *  directly or via one or more typedefs, the behavior is the same as if it appeared only
		 *  once.
		 *
		 *  If an attempt is made to modify an object defined with a const-qualified type through use
		 *  of an lvalue with non-const-qualified type, the behavior is undefined. If an attempt is
		 *  made to refer to an object defined with a volatile-qualified type through use of an lvalue
		 *  with non-volatile-qualified type, the behavior is undefined.
		 *
		 *  An object that has volatile-qualified type may be modified in ways unknown to the
		 *  implementation or have other unknown side effects.  Therefore any expression referring
		 *  to such an object shall be evaluated strictly according to the rules of the abstract machine,
		 *  as described in 5.1.2.3. Furthermore, at every sequence point the value last stored in the
		 *  object shall agree with that prescribed by the abstract machine, except as modified by the
		 *  unknown factors mentioned previously. What constitutes an access to an object that
		 *  has volatile-qualified type is implementation-defined.
		 *
		 *  An object that is accessed through a restrict-qualified pointer has a special association
		 *  with that pointer. This association, defined in 6.7.3.1 below, requires that all accesses to
		 *  that object use, directly or indirectly, the value of that particular pointer. The intended
		 *  use of the restrict qualifier (like the register storage class) is to promote
		 *  optimization, and deleting all instances of the qualifier from all preprocessing translation
		 *  units composing a conforming program does not change its meaning (i.e., observable
		 *  behavior).
		 *
		 *  If the specification of an array type includes any type qualifiers, the element type is 
		 *  so-qualified, not the array type. If the specification of a function type includes any type
		 *  qualifiers, the behavior is undefined.
		 *
		 *  For two qualified types to be compatible, both shall have the identically qualified version
		 *  of a compatible type; the order of type qualifiers within a list of specifiers or qualifiers
		 *  does not affect the specified type.
		 * */
		$$ |= TYPE_QUALIFIER_CONST;
	}
	| TOKEN_RESTRICT
	{
		/*NOTE: keyword "restrict" is not implement*/
		$$ |= TYPE_QUALIFIER_RESTRICT;
	}
	| TOKEN_VOLATILE
	{
		$$ = TYPE_QUALIFIER_VOLATILE;
	}
	;

function_specifier
	: TOKEN_INLINE
	{
		/*NOTE: keyword "inline" is not implement*/
		/*
		 * Section 6.7.4 of ISO/IEC 9899:1999
		 *  Function specifiers shall be used only in the declaration of an identifier for a function.
		 *
		 *  An inline definition of a function with external linkage shall not contain a definition of a
		 *  modifiable object with static storage duration, and shall not contain a reference to an
		 *  identifier with internal linkage.
		 *
		 *  In a hosted environment, the inline function specifier shall not appear in a declaration
		 *  of main.
		 *
		 *  A function declared with an inline function specifier is an inline function. The
		 *  function specifier may appear more than once; the behavior is the same as if it appeared
		 *  only once. Making a function an inline function suggests that calls to the function be as
		 *  fast as possible. The extent to which such suggestions are effective is
		 *  implementation-defined.
		 *  
		 *  Any function with internal linkage can be an inline function. For a function with external
		 *  linkage, the following restrictions apply: If a function is declared with an inline
		 *  function specifier, then it shall also be defined in the same translation unit. If all of the
		 *  file scope declarations for a function in a translation unit include the inline function
		 *  specifier without extern, then the definition in that translation unit is an inline
		 *  definition . An inline definition does not provide an external definition for the function,
		 *  and does not forbid an external definition in another translation unit. An inline definition
		 *  provides an alternative to an external definition, which a translator may use to implement
		 *  any call to the function in the same translation unit. It is unspecified whether a call to the
		 *  function uses the inline definition or the external definition.
		 * */
		$$ = TYPE_QUALIFIER_INLINE;
	}
	;

declarator
	: pointer direct_declarator
	{
		ey_symbol_t *symbol = $2;
		ey_type_t *type = ey_type_merge(eng, symbol->type, $1);
		if(!type)
		{
			engine_parser_error("merge type between %s and %s failed\n", $1->type, symbol->type?ey_type_type_name(symbol->type->type):"NULL");
			EY_ABORT;
		}

		symbol->type = type;
		$$ = symbol;
	}
	| direct_declarator
	{
		$$ = $1;
	}
	;

direct_declarator
	: identifier
	{
		/*NOTE: 
		 * here we do not find ident symbol table but just create new symbol, 
		 * because C Language permit ident declaration but not definition.
		 * */
		ey_symbol_t *symbol = ey_alloc_symbol(eng, $1, ey_parser_level(eng), 
			SYMBOL_LOCAL, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DECLARE, NULL, NULL, NULL, ey_parser_location(eng, &@1));
		if(!symbol)
		{
			engine_parser_error("alloc ident symbol failed\n");
			EY_ABORT;
		}
		$$ = symbol;
	}
	| TOKEN_LPAREN declarator TOKEN_RPAREN
	{
		$$ = $2;
	}
	| direct_declarator TOKEN_LBRACK type_qualifier_list_opt assignment_expression_opt TOKEN_RBRACK
	{
		ey_symbol_t *symbol = $1;
		ey_type_t *symbol_type = symbol->type;

		if($3)
		{
			engine_parser_error("type qualified array length is not supported\n");
			EY_ABORT;
		}
		
		unsigned int count = 0;
		if($4)
		{
			if(!ey_expr_is_const_value(eng, $4))
			{
				engine_parser_error("array length must be const value\n");
				EY_ABORT;
			}

			if(!ey_type_is_integer($4->expr_type->type))
			{
				engine_parser_error("array length must be const integer\n");
				EY_ABORT;
			}

			count = ey_eval_const_expr_value(eng, $4);
		}
		else
		{
			if($1->type->type == TYPE_ARRAY && $1->type->array_type.undefined)
			{
				engine_parser_error("array base type is not defined\n");
				EY_ABORT;
			}
		}

		ey_type_t *array_type = ey_alloc_array_type(eng, TYPE_ARRAY, TYPE_QUALIFIER_NORMAL,
			0, 0, ey_combine_parser_location(eng, &@1, &@5), NULL, count, $4==NULL);
		if(!array_type)
		{
			engine_parser_error("alloc array type failed\n");
			EY_ABORT;
		}

		symbol_type = ey_type_merge(eng, symbol_type, array_type);
		if(!symbol_type)
		{
			engine_parser_error("merge type between %s and %s failed\n",
				symbol_type?ey_type_type_name(symbol_type->type):"NULL",
				ey_type_type_name(array_type->type));
			EY_ABORT;
		}
		symbol->type = symbol_type;
		$$ = symbol;
	}
	| direct_declarator TOKEN_LPAREN 
	{
		ey_parser_push_level(eng);
	}
	parameter_type_list_opt TOKEN_RPAREN
	{
		ey_parser_pop_level(eng);

		ey_symbol_t *symbol = $1;
		ey_type_t *symbol_type = symbol->type;

		ey_type_t *func_type = ey_alloc_function_type(eng, TYPE_FUNCTION, TYPE_QUALIFIER_NORMAL,
			0, 0, ey_combine_parser_location(eng, &@1, &@5), NULL, $4.ellipsis, &$4.arg_list);
		if(!func_type)
		{
			engine_parser_error("alloc function type failed\n");
			EY_ABORT;
		}
		
		symbol_type = ey_type_merge(eng, symbol_type, func_type);
		if(!symbol_type)
		{
			engine_parser_error("merge type between %s and %s failed\n",
				symbol_type?ey_type_type_name(symbol_type->type):"NULL",
				ey_type_type_name(func_type->type));
			EY_ABORT;
		}
		symbol->type = symbol_type;
		$$ = symbol;
	}
	| direct_declarator TOKEN_LPAREN identifier_list TOKEN_RPAREN
	{
		ey_symbol_t *symbol = $1;
		ey_type_t *symbol_type = symbol->type;

		ey_type_t *func_type = ey_alloc_function_type(eng, TYPE_FUNCTION, TYPE_QUALIFIER_NORMAL,
			0, 0, ey_combine_parser_location(eng, &@1, &@4), NULL, 0, &$3);
		if(!func_type)
		{
			engine_parser_error("alloc function type failed\n");
			EY_ABORT;
		}
		
		symbol_type = ey_type_merge(eng, symbol_type, func_type);
		if(!symbol_type)
		{
			engine_parser_error("merge type between %s and %s failed\n",
				symbol_type?ey_type_type_name(symbol_type->type):"NULL",
				ey_type_type_name(func_type->type));
			EY_ABORT;
		}
		symbol->type = symbol_type;
		$$ = symbol;
	}
	| direct_declarator TOKEN_LBRACK TOKEN_STATIC type_qualifier_list_opt assignment_expression TOKEN_RBRACK
	{
		engine_parser_error("keyword static is not supported in array declaration\n");
		EY_ABORT;
	}
	| direct_declarator TOKEN_LBRACK type_qualifier_list TOKEN_STATIC assignment_expression TOKEN_RBRACK
	{
		engine_parser_error("keyword static is not supported in array declaration\n");
		EY_ABORT;
	}
	| direct_declarator TOKEN_LBRACK type_qualifier_list_opt TOKEN_STAR TOKEN_RBRACK
	{
		engine_parser_error("variable length array is not supported\n");
		EY_ABORT;
	}
	;

assignment_expression_opt
	:
	{
		$$ = NULL;
	}
	| assignment_expression
	{
		$$ = $1;
	}
	;

pointer
	: TOKEN_STAR type_qualifier_list_opt
	{
		ey_type_t *ret = ey_alloc_pointer_type(eng, TYPE_POINTER, $2, sizeof(void*), alignment_of(void*),
			ey_combine_parser_location(eng, &@1, &@2), NULL);
		if(!ret)
		{
			engine_parser_error("alloc pointer failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_STAR type_qualifier_list_opt pointer
	{
		ey_type_t *ret = ey_alloc_pointer_type(eng, TYPE_POINTER, $2, sizeof(void*), alignment_of(void*),
			ey_combine_parser_location(eng, &@1, &@3), $3);
		if(!ret)
		{
			engine_parser_error("alloc pointer failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

type_qualifier_list_opt
	: 
	{
		$$ = TYPE_QUALIFIER_NORMAL;
	}
	| type_qualifier_list
	{
		$$ = $1;
	}
	;

type_qualifier_list
	: type_qualifier
	{
		$$ = $1;
	}
	| type_qualifier_list type_qualifier
	{
		$$ = ($1|$2);
	}
	;

parameter_type_list_opt
	:
	{
		$$.ellipsis = 0;
		TAILQ_INIT(&$$.arg_list);
	}
	| parameter_list
	{
		$$.ellipsis = 0;
		$$.arg_list = $1;
	}
	| parameter_list TOKEN_COMMA TOKEN_ELLIPSIS
	{
		$$.ellipsis = 1;
		$$.arg_list = $1;
	}
	;

parameter_list
	: parameter_declaration
	{
		ey_symbol_t *symbol = $1;
		TAILQ_INIT(&$$);
		TAILQ_INSERT_TAIL(&$$, symbol, list_next);
	}
	| parameter_list TOKEN_COMMA parameter_declaration
	{
		if($3->name)
		{
			ey_symbol_t *symbol = NULL;
			TAILQ_FOREACH(symbol, &$1, list_next)
			{
				if(symbol->name && !strcmp(symbol->name, $3->name))
					break;
			}
			if(symbol)
			{
				engine_parser_error("parameter %s redeclared in %s-%d:%d-%d:%d\n", symbol->name, print_location(&symbol->location));
				EY_ABORT;
			}
		}
		TAILQ_INSERT_TAIL(&$$, $3, list_next);
	}
	;

parameter_declaration
	: declaration_specifiers declarator
	{
		ey_type_t *type1 = ey_type_normalize(eng, $1);
		if(!type1)
		{
			engine_parser_error("simplify type %s failed\n", ey_type_type_name($1->type));
			EY_ABORT;
		}

		ey_symbol_t *symbol = $2;
		ey_type_t *type2 = symbol->type;
		ey_type_t *type3 = ey_type_merge(eng, type2, type1);
		if(!type3)
		{
			engine_parser_error("merge parameter type between %s and %s failed\n",
				ey_type_type_name(type1->type),
				type2?ey_type_type_name(type2->type):"NULL");
			EY_ABORT;
		}

		if(!ey_type_is_declared(eng, type3))
		{
			if(type3->type!=TYPE_ARRAY || !type3->array_type.undefined)
			{
				engine_parser_error("parameter type %s is not declared\n", ey_type_type_name(type3->type));
				EY_ABORT;
			}
		}

		symbol->type = type3;
		symbol->class = SYMBOL_FORMAL;
		symbol->level = 1;
		ey_symbol_set_flag(symbol, SYMBOL_FLAG_DECLARE);
		$$ = symbol;
	}
	| declaration_specifiers abstract_declarator
	{
		ey_type_t *type1 = ey_type_normalize(eng, $1);
		if(!type1)
		{
			engine_parser_error("simplify type %s failed\n", ey_type_type_name($1->type));
			EY_ABORT;
		}

		ey_type_t *type2 = $2;
		ey_type_t *type3 = ey_type_merge(eng, type2, type1);
		if(!type3)
		{
			engine_parser_error("merge parameter type failed\n");
			EY_ABORT;
		}

		if(!ey_type_is_declared(eng, type3))
		{
			if(type3->type!=TYPE_ARRAY || !type3->array_type.undefined)
			{
				engine_parser_error("parameter type %s is not declared\n", ey_type_type_name(type3->type));
				EY_ABORT;
			}
		}

		ey_symbol_t *symbol = ey_alloc_symbol(eng, NULL, 1, SYMBOL_FORMAL, SYMBOL_STORAGE_NONE,
			SYMBOL_FLAG_DECLARE, type3, NULL, NULL, ey_combine_parser_location(eng, &@1, &@2));
		if(!symbol)
		{
			engine_parser_error("alloc parameter symbol failed\n");
			EY_ABORT;
		}
		$$ = symbol;
	}
	| declaration_specifiers
	{
		ey_type_t *type = ey_type_normalize(eng, $1);
		if(!type)
		{
			engine_parser_error("simplify type %s failed\n", ey_type_type_name($1->type));
			EY_ABORT;
		}

		if(!ey_type_is_declared(eng, type))
		{
			engine_parser_error("parameter type %s is not declared\n", ey_type_type_name(type->type));
			EY_ABORT;
		}

		ey_symbol_t *symbol = ey_alloc_symbol(eng, NULL, 1, SYMBOL_FORMAL, SYMBOL_STORAGE_NONE,
			SYMBOL_FLAG_DECLARE, $1, NULL, NULL, ey_parser_location(eng, &@1));
		if(!symbol)
		{
			engine_parser_error("alloc parameter symbol failed\n");
			EY_ABORT;
		}
		$$ = symbol;
	}
	;

identifier_list
	: identifier
	{
		ey_symbol_t *symbol = ey_alloc_symbol(eng, $1, 1, 
			SYMBOL_FORMAL, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DECLARE, NULL, NULL, NULL,
			ey_parser_location(eng, &@1));
		if(!symbol)
		{
			engine_parser_error("alloc formal parameter %s failed\n", $1);
			EY_ABORT;
		}

		TAILQ_INIT(&$$);
		TAILQ_INSERT_TAIL(&$$, symbol, list_next);
	}
	| identifier_list TOKEN_COMMA identifier
	{
		ey_symbol_t *symbol = NULL;
		TAILQ_FOREACH(symbol, &$1, list_next)
		{
			if(symbol->name && !strcmp(symbol->name, $3))
				break;
		}
		if(symbol)
		{
			engine_parser_error("formal parameter %s redefined in %s-%d:%d-%d:%d\n", $3, print_location(&symbol->location));
			EY_ABORT;
		}

		symbol = ey_alloc_symbol(eng, $3, 1, 
			SYMBOL_FORMAL, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DECLARE, NULL, NULL, NULL,
			ey_parser_location(eng, &@1));
		if(!symbol)
		{
			engine_parser_error("alloc formal parameter %s failed\n", $1);
			EY_ABORT;
		}

		TAILQ_INSERT_TAIL(&$$, symbol, list_next);
	}
	;

type_name
	: specifier_qualifier_list abstract_declarator
	{
		ey_type_type_t type_type = $1->type;
		ey_type_t *type = ey_type_normalize(eng, $1);
		if(!type)
		{
			engine_parser_error("simplify type %s failed\n", ey_type_type_name(type_type));
			EY_ABORT;
		}
		
		type_type = type->type;
		type = ey_type_merge(eng, $2, type);
		if(!type)
		{
			engine_parser_error("meger type between %s and %s failed\n", 
				ey_type_type_name(type_type), ey_type_type_name($2->type));
			EY_ABORT;
		}
		$$ = type;
	}
	| specifier_qualifier_list 
	{
		ey_type_type_t type_type = $1->type;
		ey_type_t *type = ey_type_normalize(eng, $1);
		if(!type)
		{
			engine_parser_error("simplify type %s failed\n", ey_type_type_name(type_type));
			EY_ABORT;
		}
		$$ = type;
	}
	;

abstract_declarator
	: pointer
	{
		$$ = $1;
	}
	| pointer direct_abstract_declarator
	{
		ey_type_t *type = ey_type_merge(eng, $2, $1);
		if(!type)
		{
			engine_parser_error("cannot merge types between %s and %s\n", 
				ey_type_type_name($1->type), ey_type_type_name($2->type));
			EY_ABORT;
		}
	}
	| direct_abstract_declarator
	{
		$$ = $1;
	}
	;

direct_abstract_declarator
	: TOKEN_LPAREN abstract_declarator TOKEN_RPAREN
	{
		$$ = $2;
	}
	| direct_abstract_declarator TOKEN_LBRACK assignment_expression_opt TOKEN_RBRACK
	{
		unsigned int count = 0;
		if($3)
		{
			if(!ey_expr_is_const_value(eng, $3))
			{
				engine_parser_error("array length must be const value\n");
				EY_ABORT;
			}

			if(!ey_type_is_integer($3->expr_type->type))
			{
				engine_parser_error("array length must be const integer\n");
				EY_ABORT;
			}

			count = ey_eval_const_expr_value(eng, $3);
		}
		else
		{
			if($1->type==TYPE_POINTER && $1->array_type.undefined)
			{
				engine_parser_error("array base type is not defined\n");
				EY_ABORT;
			}
		}

		ey_type_t *array_type = ey_alloc_array_type(eng, TYPE_ARRAY, TYPE_QUALIFIER_NORMAL,
			0, 0, ey_combine_parser_location(eng, &@1, &@4), NULL, count, $3==NULL);
		if(!array_type)
		{
			engine_parser_error("alloc array type failed\n");
			EY_ABORT;
		}

		ey_type_t *ret = ey_type_merge(eng, $1, array_type);
		if(!ret)
		{
			engine_parser_error("merge type between %s and %s failed\n", 
				ey_type_type_name($1->type), ey_type_type_name(array_type->type));
			EY_ABORT;
		}
		$$ = ret;
	}
	| direct_abstract_declarator TOKEN_LPAREN parameter_type_list_opt TOKEN_RPAREN
	{
		ey_type_t *func_type = ey_alloc_function_type(eng, TYPE_FUNCTION, TYPE_QUALIFIER_NORMAL, 0, 0,
			ey_combine_parser_location(eng, &@1, &@4), NULL, $3.ellipsis, &$3.arg_list);
		if(!func_type)
		{
			engine_parser_error("alloc function type failed\n");
			EY_ABORT;
		}

		ey_type_t *ret = ey_type_merge(eng, $1, func_type);
		if(!ret)
		{
			engine_parser_error("merge type between %s and %s failed\n", 
				ey_type_type_name($1->type), ey_type_type_name(func_type->type));
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_LPAREN parameter_type_list_opt TOKEN_RPAREN
	{
		ey_type_t *func_type = ey_alloc_function_type(eng, TYPE_FUNCTION, TYPE_QUALIFIER_NORMAL, 0, 0,
			ey_combine_parser_location(eng, &@1, &@3), NULL, $2.ellipsis, &$2.arg_list);
		if(!func_type)
		{
			engine_parser_error("alloc function type failed\n");
			EY_ABORT;
		}

		$$ = func_type;
	}
	| TOKEN_LBRACK assignment_expression_opt TOKEN_RBRACK
	{
		unsigned int count = 0;
		if($2)
		{
			if(!ey_expr_is_const_value(eng, $2))
			{
				engine_parser_error("array length must be const value\n");
				EY_ABORT;
			}

			if(!ey_type_is_integer($2->expr_type->type))
			{
				engine_parser_error("array length must be const integer\n");
				EY_ABORT;
			}

			count = ey_eval_const_expr_value(eng, $2);
		}

		ey_type_t *array_type = ey_alloc_array_type(eng, TYPE_ARRAY, TYPE_QUALIFIER_NORMAL,
			0, 0, ey_combine_parser_location(eng, &@1, &@3), NULL, count, $2==NULL);
		if(!array_type)
		{
			engine_parser_error("alloc array type failed\n");
			EY_ABORT;
		}

		$$ = array_type;
	}
	| direct_abstract_declarator TOKEN_LBRACK TOKEN_STAR TOKEN_RBRACK
	{
		engine_parser_error("variable length array is not supported\n");
		EY_ABORT;
	}
	| TOKEN_LBRACK TOKEN_STAR TOKEN_RBRACK
	{
		engine_parser_error("variable length array is not supported\n");
		EY_ABORT;
	}
	;

typedef_name
	: TOKEN_TYPEDEF_IDENT
	{
		ey_symbol_t *symbol = ey_find_symbol(eng, $1, SYMBOL_TABLE_IDENT);
		if(!symbol)
		{
			engine_parser_error("cannot find type name %s\n", $1);
			EY_ABORT;
		}

		if(symbol->class!=SYMBOL_NAME)
		{
			engine_parser_error("class of type name %s is %s\n", $1, ey_symbol_class_name(symbol->class));
			EY_ABORT;
		}

		if(!symbol->type)
		{
			engine_parser_error("type of the type name %s is null\n", $1);
			EY_ABORT;
		}

		ey_type_t *ret = symbol->type;
		while(ret && ret->class==TYPE_CLASS_TYPEDEF)
			ret = ret->typedef_type.descriptor_type;

		if(!ret)
		{
			engine_parser_error("invalid typedef type\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

initializer
	: assignment_expression
	{
		$$ = $1;
	}
	| TOKEN_LBRACE initializer_list TOKEN_RBRACE
	{
		$$ = $2;
	}
	| TOKEN_LBRACE initializer_list TOKEN_COMMA TOKEN_RBRACE
	{
		$$ = $2;
	}
	;

initializer_list
	: initializer
	{
		ey_expr_t *ret = ey_alloc_list_init_expr(eng, EXPR_OPCODE_INIT_COMPOUND, NULL, ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc initializer_list failed\n");
			EY_ABORT;
		}
		TAILQ_INSERT_TAIL(&ret->list_expr.head, $1, link);
		$$ = ret;
	}
	| initializer_list TOKEN_COMMA initializer
	{
		TAILQ_INSERT_TAIL(&$$->list_expr.head, $3, link);
	}
	| designation initializer
	{
		ey_expr_t *assign_expr = ey_alloc_binary_init_expr(eng, EXPR_OPCODE_INIT_ASGN, 
			$1, $2, ey_combine_parser_location(eng, &@1, &@2));
		if(!assign_expr)
		{
			engine_parser_error("alloc assignment expression failed\n");
			EY_ABORT;
		}

		ey_expr_t *ret = ey_alloc_list_init_expr(eng, EXPR_OPCODE_INIT_COMPOUND, NULL, ey_combine_parser_location(eng, &@1, &@2));
		if(!ret)
		{
			engine_parser_error("alloc initializer_list failed\n");
			EY_ABORT;
		}
		TAILQ_INSERT_TAIL(&ret->list_expr.head, assign_expr, link);
		$$ = ret;
	}
	| initializer_list TOKEN_COMMA designation initializer
	{
		ey_expr_t *assign_expr = ey_alloc_binary_init_expr(eng, EXPR_OPCODE_INIT_ASGN, 
			$3, $4, ey_combine_parser_location(eng, &@1, &@4));
		if(!assign_expr)
		{
			engine_parser_error("alloc assignment expression failed\n");
			EY_ABORT;
		}

		TAILQ_INSERT_TAIL(&$$->list_expr.head, assign_expr, link);
	}
	;

designation
	: designator_list TOKEN_EQ
	{
		$$ = $1;
	}
	;

designator_list
	: designator
	{
		$$ = $1;
	}
	| designator_list designator
	{
		engine_parser_error("designator list is not supported by eyoung\n");
		EY_ABORT;
		/*
		switch($2->opcode)
		{
			case EXPR_OPCODE_ARRAY_INDEX:
				$2->binary_expr.left = $1;
				break;
			case EXPR_OPCODE_MEMBER:
				$2->member_expr.su = $1;
				break;
			default:
				engine_parser_error("fatal error expr type cannot be %s here\n", ey_expr_opcode_name($2->opcode));
				EY_ABORT;
		}
		$$ = $2;
		*/
	}
	;

designator
	: TOKEN_LBRACK constant_expression TOKEN_RBRACK
	{
		/*NOTE: for more safety, we limit that the constant_expression have a constant integer value*/
		if(!ey_type_is_integer($2->expr_type->type))
		{
			engine_parser_error("case array index should be integer type\n");
			EY_ABORT;
		}

		if(!ey_expr_is_const_value(eng, $2))
		{
			engine_parser_error("case label expr should be const integer value\n");
			EY_ABORT;
		}
		
		ey_expr_t *index_expr = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, $2->const_value, 
			ey_combine_parser_location(eng, &@1, &@3));
		if(!index_expr)
		{
			engine_parser_error("alloc index symbol expr failed\n");
			EY_ABORT;
		}

		ey_expr_t *ret = ey_alloc_binary_init_expr(eng, EXPR_OPCODE_ARRAY_INDEX, NULL, index_expr,
			ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc array index expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_DOT identifier
	{
		ey_symbol_t *member = ey_alloc_symbol(eng, $2, ey_parser_level(eng), 
			SYMBOL_MEMBER, SYMBOL_STORAGE_NONE, 0, NULL, NULL, NULL, ey_combine_parser_location(eng, &@1, &@2));
		if(!member)
		{
			engine_parser_error("alloc member symbol failed\n");
			EY_ABORT;
		}

		ey_expr_t *ret = ey_alloc_member_init_expr(eng, EXPR_OPCODE_MEMBER, NULL, member, ey_combine_parser_location(eng, &@1, &@2));
		if(!ret)
		{
			engine_parser_error("alloc member expr failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

/*statement*/
/*
 * Section 6.8 of ISO/IEC 9899:1999
 *  Astatement specifies an action to be performed. Except as indicated, statements are
 *  executed in sequence.
 *
 *  Ablock allows a set of declarations and statements to be grouped into one syntactic unit.
 *  The initializers of objects that have automatic storage duration, and the variable length
 *  array declarators of ordinary identifiers with block scope, are evaluated and the values are
 *  stored in the objects (including storing an indeterminate value in objects without an
 *  initializer) each time the declaration is reached in the order of execution, as if it were a
 *  statement, and within each declaration in the order that declarators appear.
 *
 *  Afull expression is an expression that is not part of another expression or of a declarator.
 *  Each of the following is a full expression: an initializer; the expression in an expression
 *  statement; the controlling expression of a selection statement (if or switch); the
 *  controlling expression of a while or do statement; each of the (optional) expressions of
 *  a for statement; the (optional) expression in a return statement. The end of a full
 *  expression is a sequence point.
 * */
statement
	: labeled_statement
	{
		/*
		 *  Section 6.8.1 of ISO/IEC 9899:1999
		 *   A case or default label shall appear only in a switch statement. Further
		 *   constraints on such labels are discussed under the switch statement.
		 *
		 *   Label names shall be unique within a function.
		 *
		 *   Any statement may be preceded by a prefix that declares an identifier as a label name.
		 *   Labels in themselves do not alter the flow of control, which continues unimpeded across
		 *   them.
		 * */
		$$ = $1;
	}
	| compound_statement
	{
		/*
		 *  Section 6.8.2 of ISO/IEC 9899:1999
		 *   Acompound statement is a block.
		 * */
		$$ = $1;
	}
	| expression_statement
	{
		/*
		 *  Section 6.8.3 of ISO/IEC 9899:1999
		 *   The expression in an expression statement is evaluated as a void expression for its side
		 *   effects.
		 *
		 *   Anull statement (consisting of just a semicolon) performs no operations.
		 * */
		$$ = $1;
	}
	| selection_statement
	{
		/*
		 *  Section 6.8.4 of ISO/IEC 9899:1999
		 *   A selection statement selects among a set of statements depending on the value of a
		 *   controlling expression.
		 *
		 *   A selection statement is a block whose scope is a strict subset of the scope of its
		 *   enclosing block. Each associated substatement is also a block whose scope is a strict
		 *   subset of the scope of the selection statement.
		 * */
		$$ = $1;
	}
	| iteration_statement
	{
		/*
		 * Section 6.8.5 of ISO/IEC 9899:1999
		 *  The controlling expression of an iteration statement shall have scalar type.
		 *
		 *  The declaration part of a for statement shall only declare identifiers for objects having
		 *  storage class auto or register.
		 *
		 *  An iteration statement causes a statement called the loop body to be executed repeatedly
		 *  until the controlling expression compares equal to 0.
		 *
		 *  An iteration statement is a block whose scope is a strict subset of the scope of its
		 *  enclosing block. The loop body is also a block whose scope is a strict subset of the scope
		 *  of the iteration statement.
		 * */
		$$ = $1;
	}
	| jump_statement
	{
		$$ = $1;
	}
	;

labeled_statement
	: identifier TOKEN_COLON statement
	{
		void *label_value = NULL;
		ey_symbol_t *label_symbol = ey_find_symbol(eng, $1, SYMBOL_TABLE_LABEL);
		if(label_symbol && ey_symbol_check_flag(label_symbol,SYMBOL_FLAG_DEFINE))
		{
			engine_parser_error("label redefined, label %s is already define in %s-%d:%d-%d:%d\n", 
				$1, print_location(&label_symbol->location));
			EY_ABORT;
		}
		
		if(!label_symbol)
		{
			label_value = ey_alloc_symbol_value(eng, ey_pvoid_type(eng));
			if(!label_value)
			{
				engine_parser_error("alloc label %s value failed\n", $1);
				EY_ABORT;
			}

			label_symbol = ey_alloc_symbol(eng, $1, 1, SYMBOL_LABEL, SYMBOL_STORAGE_NONE,
				SYMBOL_FLAG_DEFINE, ey_pvoid_type(eng), label_value, NULL, ey_combine_parser_location(eng, &@1, &@3));
			if(!label_symbol)
			{
				engine_parser_error("alloc label %s symbol failed\n", $1);
				EY_ABORT;
			}

			ey_stmt_t *ret = ey_alloc_label_stmt(eng, label_symbol, $3, 
				ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@3));
			if(!ret)
			{
				engine_parser_error("alloc label %s stmt failed\n", $1);
				EY_ABORT;
			}
			*(ey_stmt_t**)label_value = ret;
			$$ = ret;
		}
		else
		{
			label_value = label_symbol->value;
			ey_stmt_t *ret = *(ey_stmt_t**)label_value;
			ret->level = ey_parser_level(eng);
			ret->location = *ey_combine_parser_location(eng, &@1, &@3);
			ey_symbol_set_flag(label_symbol, SYMBOL_FLAG_DEFINE);
			TAILQ_REMOVE(&ey_func_undefined_label(eng), label_symbol, list_next);
			$$ = ret;
		}
	}
	| TOKEN_CASE constant_expression TOKEN_COLON statement
	{
		if(switch_level(eng) == 0)
		{
			engine_parser_error("case stmt should be in a switch stmt\n");
			EY_ABORT;
		}

		if(!ey_type_is_integer($2->expr_type->type))
		{
			engine_parser_error("case label should be integer type\n");
			EY_ABORT;
		}

		if(!ey_expr_is_const_value(eng, $2))
		{
			engine_parser_error("case label expr should be const integer value\n");
			EY_ABORT;
		}

		unsigned long label_number = ey_convert_label(eng, $2->const_value);
		char label_name_buf[64] = {0};
		snprintf(label_name_buf, sizeof(label_name_buf)-1, "%ld", label_number);
		char *label_name = ey_alloc_symbol_name(eng, label_name_buf);
		if(!label_name)
		{
			engine_parser_error("alloc case label %s failed\n", label_name_buf);
			EY_ABORT;
		}
		
		void *label_value = ey_alloc_symbol_value(eng, ey_pvoid_type(eng));
		if(!label_value)
		{
			engine_parser_error("alloc case label %s value failed\n", label_name);
			EY_ABORT;
		}

		ey_symbol_t *label_symbol = ey_find_level_symbol(eng, label_name, switch_level(eng), SYMBOL_TABLE_LABEL);
		if(label_symbol)
		{
			engine_parser_error("case label %s re-defined\n", label_name_buf);
			EY_ABORT;
		}

		label_symbol = ey_alloc_symbol(eng, label_name, switch_level(eng),
			SYMBOL_LABEL, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, ey_pvoid_type(eng), label_value, 
			NULL, ey_combine_parser_location(eng, &@1, &@4));
		if(!label_symbol)
		{
			engine_parser_error("alloc case label %s symbol failed\n", label_name);
			EY_ABORT;
		}
		ey_insert_symbol(eng, label_symbol, SYMBOL_TABLE_LABEL);

		ey_stmt_t *ret = ey_alloc_case_stmt(eng, label_symbol, $2, $4, ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@4));
		if(!ret)
		{
			engine_parser_error("alloc case label %s stmt failed\n", label_name);
			EY_ABORT;
		}
		*(ey_stmt_t**)label_value = ret;

		TAILQ_INSERT_TAIL(&ey_label_list(eng), ret, link);
		$$ = ret;
	}
	| TOKEN_DEFAULT TOKEN_COLON statement
	{
		if(switch_level(eng) == 0)
		{
			engine_parser_error("default stmt should be in a switch stmt\n");
			EY_ABORT;
		}

		char *label_name = ey_alloc_symbol_name(eng, "default");
		if(!label_name)
		{
			engine_parser_error("alloc default label default failed\n");
			EY_ABORT;
		}
		
		void *label_value = ey_alloc_symbol_value(eng, ey_pvoid_type(eng));
		if(!label_value)
		{
			engine_parser_error("alloc default label %s value failed\n", label_name);
			EY_ABORT;
		}

		ey_symbol_t *label_symbol = ey_find_level_symbol(eng, label_name, switch_level(eng), SYMBOL_TABLE_LABEL);
		if(label_symbol && label_symbol->level>=switch_level(eng))
		{
			engine_parser_error("default label %s re-defined\n", label_name);
			EY_ABORT;
		}

		label_symbol = ey_alloc_symbol(eng, label_name, switch_level(eng),
			SYMBOL_LABEL, SYMBOL_STORAGE_NONE, SYMBOL_FLAG_DEFINE, ey_pvoid_type(eng), label_value, 
			NULL, ey_combine_parser_location(eng, &@1, &@3));
		if(!label_symbol)
		{
			engine_parser_error("alloc default label %s symbol failed\n", label_name);
			EY_ABORT;
		}
		ey_insert_symbol(eng, label_symbol, SYMBOL_TABLE_LABEL);

		ey_stmt_t *ret = ey_alloc_default_stmt(eng, label_symbol, $3, ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc default label %s stmt failed\n", label_name);
			EY_ABORT;
		}
		*(ey_stmt_t**)label_value = ret;

		TAILQ_INSERT_TAIL(&ey_label_list(eng), ret, link);
		$$ = ret;
	}
	;

compound_statement
	: 
	{
		ey_parser_push_level(eng);
	}
	TOKEN_LBRACE block_item_list TOKEN_RBRACE
	{
		ey_parser_pop_level(eng);

		ey_stmt_t *ret = ey_alloc_block_stmt(eng, &$3, ey_parser_level(eng), ey_combine_parser_location(eng, &@2, &@4));
		if(!ret)
		{
			engine_parser_error("alloc block stmt failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

block_item_list
	: 
	{
		TAILQ_INIT(&$$);
	}
	| block_item_list block_item
	{
		TAILQ_INSERT_TAIL(&$$, $2, block_link);
	}
	;

block_item
	: declaration
	{
		ey_stmt_t *ret = ey_alloc_dec_stmt(eng, &$1, ey_parser_level(eng), ey_parser_location(eng, &@1));
		if(!ret)
		{
			engine_parser_error("alloc dec stmt failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| statement
	{
		$$ = $1;
	}
	;

expression_statement
	: expression_opt TOKEN_SEMI
	{
		ey_stmt_t *ret = ey_alloc_expr_stmt(eng, $1, ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@2));
		if(!ret)
		{
			engine_parser_error("alloc expr stmt failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

expression_opt
	:
	{
		$$ = ey_true_expr(eng);
	}
	| expression
	{
		$$ = $1;
	}
	;

selection_statement
	: simple_if %prec TOKEN_IF
	{
		/*
		 * Section 6.8.4.1 of ISO/IEC 9899:1999
		 *  The controlling expression of an if statement shall have scalar type.
		 *
		 *  In both forms, the first substatement is executed if the expression compares unequal to 0.
		 *  In the else form, the second substatement is executed if the expression compares equal
		 *  to 0. If the first substatement is reached via a label, the second substatement is not
		 *  executed.
		 *
		 *  An else is associated with the lexically nearest preceding if that is allowed by the
		 *  syntax.
		 * */
		$$ = $1;
	}
	| simple_if TOKEN_ELSE statement
	{
		$1->branch_stmt.false_target = $3;
		$1->location = *ey_combine_parser_location(eng, &@1, &@3);
		$$ = $1;
	}
	| TOKEN_SWITCH TOKEN_LPAREN expression TOKEN_RPAREN 
	{
		/*
		 * Section 6.8.4.2 of ISO/IEC 9899:1999
		 *  The controlling expression of a switch statement shall have integer type.
		 *
		 *  If a switch statement has an associated case or default label within the scope of an
		 *  identifier with a variably modified type, the entire switch statement shall be within the
		 *  scope of that identifier.
		 *
		 *  The expression of each case label shall be an integer constant expression and no two of
		 *  the case constant expressions in the same switch statement shall have the same value
		 *  after conversion. There may be at most one default label in a switch statement.
		 *  (Any enclosed switch statement may have a default label or case constant
		 *  expressions with values that duplicate case constant expressions in the enclosing
		 *  switch statement.)
		 *
		 *  Aswitch statement causes control to jump to, into, or past the statement that is the
		 *  switch body, depending on the value of a controlling expression, and on the presence of a
		 *  default label and the values of any case labels on or in the switch body. A case or
		 *  default label is accessible only within the closest enclosing switch statement.
		 *
		 *  The integer promotions are performed on the controlling expression. The constant
		 *  expression in each case label is converted to the promoted type of the controlling
		 *  expression. If a converted value matches that of the promoted controlling expression,
		 *  control jumps to the statement following the matched case label. Otherwise, if there is
		 *  a default label, control jumps to the labeled statement. If no converted case constant
		 *  expression matches and there is no default label, no part of the switch body is
		 *  executed.
		 *
		 *  As discussed in 5.2.4.1, the implementation may limit the number of case values in a
		 *  switch statement.
		 * */
		if(!ey_type_is_integer($3->expr_type->type))
		{
			engine_parser_error("switch condition type %s is not integer\n");
			EY_ABORT;
		}
		save_switch_context(eng, &$<saved_context>$);
	} 
	statement
	{
		ey_stmt_t *ret = ey_alloc_switch_stmt(eng, $3, $6, &ey_label_list(eng),
			ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@6));
		if(!ret)
		{
			engine_parser_error("alloc switch stmt failed\n");
			EY_ABORT;
		}
		restore_switch_context(eng, &$<saved_context>5);
		$$ = ret;
	}
	;

simple_if
	: TOKEN_IF TOKEN_LPAREN expression TOKEN_RPAREN statement
	{
		ey_stmt_t *ret = ey_alloc_branch_stmt(eng, $3, $5, NULL, ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@5));
		if(!ret)
		{
			engine_parser_error("alloc simple_if stmt failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;

iteration_statement
	: TOKEN_WHILE TOKEN_LPAREN expression TOKEN_RPAREN 
	{
		/*
		 * Section 6.8.5.1 of ISO/IEC 9899:1999
		 *  The evaluation of the controlling expression takes place before each execution of the loop
		 *  body.
		 * */
		if(!ey_type_is_scalar($3->expr_type->type))
		{
			engine_parser_error("while condition type %s is not scalar\n");
			EY_ABORT;
		}
		save_loop_context(eng, &$<saved_context>$);
	}
	statement
	{
		ey_stmt_t *ret = ey_alloc_while_stmt(eng, $3, $6,
			ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@6));
		if(!ret)
		{
			engine_parser_error("alloc while stmt failed\n");
			EY_ABORT;
		}

		restore_loop_context(eng, &$<saved_context>5);
		$$ = ret;
	}
	| TOKEN_DO 
	{
		/*
		 * Section 6.8.5.2 of ISO/IEC 9899:1999
		 *  The evaluation of the controlling expression takes place after each execution of the loop
		 *  body.
		 * */
		save_loop_context(eng, &$<saved_context>$);
	}
	statement TOKEN_WHILE TOKEN_LPAREN expression TOKEN_RPAREN
	{
		if(!ey_type_is_scalar($6->expr_type->type))
		{
			engine_parser_error("do-while condition type %s is not integer\n");
			EY_ABORT;
		}
		ey_stmt_t *ret = ey_alloc_do_while_stmt(eng, $6, $3,
			ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@7));
		if(!ret)
		{
			engine_parser_error("alloc do-while stmt failed\n");
			EY_ABORT;
		}
		restore_loop_context(eng, &$<saved_context>2);
		$$ = ret;
	}
	| TOKEN_FOR TOKEN_LPAREN expression_opt TOKEN_SEMI expression_opt TOKEN_SEMI expression_opt TOKEN_RPAREN
	{
		/*
		 * Section 6.8.5.3 of ISO/IEC 9899:1999
		 *  The statement
		 *		for ( clause-1 ; expression-2 ; expression-3 ) statement
		 *  behaves as follows: The expression expression-2 is the controlling expression that is
		 *  evaluated before each execution of the loop body. The expression expression-3 is
		 *  evaluated as a void expression after each execution of the loop body. If clause-1 is a
		 *  declaration, the scope of any variables it declares is the remainder of the declaration and
		 *  the entire loop, including the other two expressions; it is reached in the order of execution
		 *  before the first evaluation of the controlling expression. If clause-1 is an expression, it is
		 *  evaluated as a void expression before the first evaluation of the controlling expression.
		 *
		 *  Both clause-1 and expression-3 can be omitted. An omitted expression-2 is replaced by a
		 *  nonzero constant.
		 * */
		if(!ey_type_is_scalar($5->expr_type->type))
		{
			engine_parser_error("for condition type %s is not integer\n");
			EY_ABORT;
		}
		save_loop_context(eng, &$<saved_context>$);
	}
	statement
	{
		ey_stmt_t *ret = ey_alloc_for_stmt(eng, $3, $5, $7, $10,
			ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@10));
		if(!ret)
		{
			engine_parser_error("alloc for stmt failed\n");
			EY_ABORT;
		}
		restore_loop_context(eng, &$<saved_context>9);
		$$ = ret;
	}
	| TOKEN_FOR TOKEN_LPAREN declaration
	{
		engine_parser_error("declaration in init part of FOR stmt is not supported by eyoung\n");
		EY_ABORT;
	}
	expression_opt TOKEN_SEMI expression_opt TOKEN_RPAREN statement
	{
		$$ = NULL;
	}
	;

jump_statement
	: TOKEN_GOTO identifier TOKEN_SEMI
	{
		/*
		 * Section 6.8.6.1 of ISO/IEC 9899:1999
		 *  The identifier in a goto statement shall name a label located somewhere in the enclosing
		 *  function. A goto statement shall not jump from outside the scope of an identifier having
		 *  a variably modified type to inside the scope of that identifier.
		 *
		 *  Agoto statement causes an unconditional jump to the statement prefixed by the named
		 *  label in the enclosing function.
		 * */
		ey_symbol_t *label_symbol = ey_find_symbol(eng, $2, SYMBOL_TABLE_LABEL);
		if(!label_symbol)
		{
			void *label_value = ey_alloc_symbol_value(eng, ey_pvoid_type(eng));
			if(!label_value)
			{
				engine_parser_error("alloc undefined label value failed\n");
				EY_ABORT;
			}

			label_symbol = ey_alloc_symbol(eng, $2, 1, SYMBOL_LABEL, SYMBOL_STORAGE_NONE,
				SYMBOL_FLAG_DECLARE, ey_pvoid_type(eng), label_value, NULL, ey_combine_parser_location(eng, &@1, &@3));
			if(!label_symbol)
			{
				engine_parser_error("alloc undefined label symbol failed\n");
				EY_ABORT;
			}

			ey_stmt_t *label_stmt = ey_alloc_label_stmt(eng, label_symbol, NULL, 
				ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@3));
			if(!label_stmt)
			{
				engine_parser_error("alloc undefined label stmt failed\n");
				EY_ABORT;
			}

			*(ey_stmt_t**)label_value = label_stmt;
			ey_insert_symbol(eng, label_symbol, SYMBOL_TABLE_LABEL);
			TAILQ_INSERT_TAIL(&ey_func_undefined_label(eng), label_symbol, list_next);
		}

		ey_stmt_t *ret = ey_alloc_goto_stmt(eng, *(ey_stmt_t**)label_symbol->value, 
			ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc goto stmt failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_CONTINUE TOKEN_SEMI
	{
		/*
		 * Section 6.8.6.2 of ISO/IEC 9899:1999
		 *  Acontinue statement shall appear only in or as a loop body.
		 *
		 *  Acontinue statement causes a jump to the loop-continuation portion of the smallest
		 *  enclosing iteration statement
		 * */
		if(!loop_level(eng))
		{
			engine_parser_error("continue stmt must be in a loop stmt\n");
			EY_ABORT;
		}
		
		ey_stmt_t *ret = ey_alloc_continue_stmt(eng, NULL, ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@2));
		if(!ret)
		{
			engine_parser_error("alloc continue stmt failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_BREAK TOKEN_SEMI
	{
		/*
		 * Section 6.8.6.3 of ISO/IEC 9899:1999
		 *  Abreak statement shall appear only in or as a switch body or loop body.
		 *
		 *  Abreak statement terminates execution of the smallest enclosing switch or iteration
		 *  statement.
		 * */
		if(!loop_level(eng) && !switch_level(eng))
		{
			engine_parser_error("break stmt must be either in switch stmt or in loop stmt\n");
			EY_ABORT;
		}

		ey_stmt_t *ret = ey_alloc_break_stmt(eng, NULL, ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@2));
		if(!ret)
		{
			engine_parser_error("alloc continue stmt failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_RETURN expression TOKEN_SEMI
	{
		/*
		 * Section 6.8.6.4 of ISO/IEC 9899:1999
		 *  Areturn statement with an expression shall not appear in a function whose return type
		 *  is void. A return statement without an expression shall only appear in a function
		 *  whose return type is void.
		 *
		 *  Areturn statement terminates execution of the current function and returns control to
		 *  its caller. A function may have any number of return statements.
		 *
		 *  If a return statement with an expression is executed, the value of the expression is
		 *  returned to the caller as the value of the function call expression. If the expression has a
		 *  type different from the return type of the function in which it appears, the value is
		 *  converted as if by assignment to an object having the return type of the function.
		 * */
		ey_type_t *expr_type = $2->expr_type;
		ey_type_t *return_type = ey_return_type(eng);
		
		if(!expr_type)
		{
			engine_parser_error("fatal error null expr type");
			EY_ABORT;
		}

		if(!return_type || !ey_type_is_declared(eng, return_type))
		{
			engine_parser_error("function do not need return any value\n");
			EY_ABORT;
		}

		if(!ey_type_assignment_compatible(eng, return_type, expr_type))
		{
			engine_parser_error("return value assignment incompatible\n");
			EY_ABORT;
		}

		ey_stmt_t *ret = ey_alloc_return_stmt(eng, $2, NULL, ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@3));
		if(!ret)
		{
			engine_parser_error("alloc return stmt failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	| TOKEN_RETURN TOKEN_SEMI
	{
		ey_type_t *return_type = ey_return_type(eng);
		
		if(return_type && return_type->size)
		{
			engine_parser_error("function need return value\n");
			EY_ABORT;
		}

		ey_stmt_t *ret = ey_alloc_return_stmt(eng, NULL, NULL, ey_parser_level(eng), ey_combine_parser_location(eng, &@1, &@2));
		if(!ret)
		{
			engine_parser_error("alloc return stmt failed\n");
			EY_ABORT;
		}
		$$ = ret;
	}
	;
%%
int eng_rule_error(ENG_RULE_LTYPE *loc, void *eng, const char *format, ...)
{
	ey_engine_t *engine = (ey_engine_t*)eng;
	ey_parser_t *parser = SLIST_FIRST(&engine->parser_stack);
	if(!ey_parser_isset_error(engine))
	{
		va_list args;
		va_start(args, format);
		parser->error_location = *ey_parser_location(eng,loc);
		vsnprintf(parser->error_reason, MAX_ERROR_REASON, format, args);
		parser->error_reason[MAX_ERROR_REASON-1] = '\0';
		va_end(args);
	}
	engine_parser_error("%s %d:%d-%d:%d error: %s\n", print_location(&parser->error_location), parser->error_reason);
	return 0;
}

static inline void save_loop_context(ey_engine_t *eng, ey_saved_context_t *dst)
{
	dst->loop_level = loop_level(eng);
	loop_level(eng) = ey_parser_level(eng)+1;
}

static inline void restore_loop_context(ey_engine_t *eng, ey_saved_context_t *src)
{
	loop_level(eng) = src->loop_level;
}

static inline void save_switch_context(ey_engine_t *eng, ey_saved_context_t *dst)
{
	dst->switch_level = switch_level(eng);
	switch_level(eng) = ey_parser_level(eng)+1;

	TAILQ_INIT(&dst->label_list);
	TAILQ_CONCAT(&dst->label_list, &ey_label_list(eng), link);
}

static inline void restore_switch_context(ey_engine_t *eng, ey_saved_context_t *src)
{
	switch_level(eng) = src->switch_level;

	TAILQ_INIT(&ey_label_list(eng));
	TAILQ_CONCAT(&ey_label_list(eng), &src->label_list, link);
}

static inline void init_function_context(ey_engine_t *eng, ey_type_t *return_type)
{
	TAILQ_INIT(&ey_func_undefined_label(eng));
	ey_return_type(eng) = return_type;
}

static inline void init_enum_context(ey_engine_t *eng)
{
	ey_enum_value(eng) = 0;
}

static inline void ey_parser_push_level(ey_engine_t *eng)
{
	ey_parser_level(eng)++;
}

static inline void ey_parser_pop_level(ey_engine_t *eng)
{
	ey_purge_symbol(eng, ey_parser_level(eng), SYMBOL_TABLE_IDENT);
	ey_purge_symbol(eng, ey_parser_level(eng), SYMBOL_TABLE_TAG);
	ey_purge_symbol(eng, ey_parser_level(eng), SYMBOL_TABLE_LABEL);

	ey_parser_level(eng)--;
}
