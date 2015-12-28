#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "eng_priv.h"

int ey_expr_operand_check(ey_engine_t *eng, ey_expr_t *expr)
{
	ey_location_t *loc = &expr->location;
	ey_expr_opcode_t opcode = expr->opcode;
	ey_expr_t *operand1 = NULL;
	ey_type_t *type1 = NULL;
	ey_expr_t *operand2 = NULL;
	ey_type_t *type2 = NULL;
	ey_expr_t *operand3 = NULL;
	ey_type_t *type3 = NULL;
	switch(opcode)
	{
		/*
		 * Section 6.5.3.2 of ISO/IEC 9899:1999
		 *	The operand of the unary* operator shall have pointer type
		 * */
		case EXPR_OPCODE_DEREF:
		{
			operand1 = expr->unary_expr.operand;
			type1 = operand1->expr_type;
			if(!ey_type_is_pointer_array(type1->type))
			{
				ey_parser_set_error(eng, loc, "get type %s, expr should have pointer type\n", ey_type_type_name(type1->type));
				return -1;
			}

			type2 = ey_type_is_pointer(type1->type)?(type1->pointer_type.deref_type):(type1->array_type.base_type);
			if(!ey_type_is_declared(eng, type2))
			{
				ey_parser_set_error(eng, &type2->location, "deref type is not defined\n");
				return -1;
			}

			break;
		}
		
		/*
		 * Section 6.5.3.2 of ISO/IEC 9899:1999
		 *   The operand of the unary & operator shall be either a function designator, the result of a
		 *   []or unary * operator, or an lvalue that designates an object that is not a bit-field and is
		 *   not declared with the register storage-class specifier
		 * */
		case EXPR_OPCODE_ADDRESS:
		{
			operand1 = expr->unary_expr.operand;
			switch(operand1->opcode)
			{
				case EXPR_OPCODE_SYMBOL:
				{
					if(operand1->expr_type->type == TYPE_FUNCTION)
						break;

					ey_symbol_t *symbol = operand1->symbol_expr.symbol;
					if(!ey_symbol_is_variable(symbol->class))
					{
						ey_parser_set_error(eng, loc, "bad symbol class %s, variable is only supported\n", 
							ey_symbol_class_name(symbol->class));
						return -1;
					}

					if(symbol->storage_class == SYMBOL_STORAGE_REGISTER)
					{
						ey_parser_set_error(eng, loc, "register storage class variable cannot do & opcode\n");
						return -1;
					}
					break;
				}
				case EXPR_OPCODE_DEREF:
				case EXPR_OPCODE_ARRAY_INDEX:
				{
					break;
				}
				case EXPR_OPCODE_MEMBER:
				case EXPR_OPCODE_MEMBER_PTR:
				{
					ey_type_t *su_type = (operand1->opcode==EXPR_OPCODE_MEMBER)?
										 (operand1->member_expr.su->expr_type):
										 (operand1->member_expr.su->expr_type->pointer_type.deref_type);
					ey_member_t *member = ey_type_find_member(eng, su_type, operand1->member_expr.member);
					if(member->bit_size)
					{
						ey_parser_set_error(eng, loc, "bitwise member cannot do & opcode\n");
						return -1;
					}
				}
				default:
				{
					ey_parser_set_error(eng, loc, "opcode %s cannot do & opcode\n", ey_expr_opcode_name(operand1->opcode));
					return -1;
				}
			}
			break;
		}
		/*
		 * Section 6.5.3.3 of ISO/IEC 9899:1999
		 *  The operand of the unary + or - operator shall have arithmetic type; of the ˜ operator,
		 *  integer type; of the ! operator, scalar type.
		 *
		 *  The result of the unary + operator is the value of its (promoted) operand. The integer
		 *  promotions are performed on the operand, and the result has the promoted type.
		 *
		 *  The result of the unary - operator is the negative of its (promoted) operand. The integer
		 *  promotions are performed on the operand, and the result has the promoted type.
		 * */
		case EXPR_OPCODE_NEG:
		case EXPR_OPCODE_POS:
		{
			operand1 = expr->unary_expr.operand;
			type1 = operand1->expr_type;
			if(!ey_type_is_arithmetic(type1->type))
			{
				ey_parser_set_error(eng, loc, "opcode %s can only support operand with arithmetic type\n", ey_expr_opcode_name(expr->opcode));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.3.3 of ISO/IEC 9899:1999
		 *  The result of the ˜ operator is the bitwise complement of its (promoted) operand (that is,
		 *  each bit in the result is set if and only if the corresponding bit in the converted operand is
		 *  not set). The integer promotions are performed on the operand, and the result has the
		 *  promoted type. If the promoted type is an unsigned type, the expression˜E is equivalent
		 *  to the maximum value representable in that type minus E
		 * */
		case EXPR_OPCODE_BNEG:
		{
			operand1 = expr->unary_expr.operand;
			type1 = operand1->expr_type;
			if(!ey_type_is_integer(type1->type))
			{
				ey_parser_set_error(eng, loc, "opcode %s can only support operand with integer type\n", ey_expr_opcode_name(expr->opcode));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.3.3 of ISO/IEC 9899:1999
		 *  The result of the logical negation operator ! is 0 if the value of its operand compares
		 *  unequal to 0, 1 if the value of its operand compares equal to 0. The result has type "int".
		 *  The expression !E is equivalent to (0==E).
		 * */
		case EXPR_OPCODE_LNEG:
		{
			operand1 = expr->unary_expr.operand;
			type1 = operand1->expr_type;
			if(!ey_type_is_scalar(type1->type))
			{
				ey_parser_set_error(eng, loc, "opcode %s can only support operand with scalar type\n", ey_expr_opcode_name(expr->opcode));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.2.1 of ISO/IEC 9899:1999
		 *  One of the expressions shall have type "pointer to object type" the other expression shall
		 *  have integer type, and the result has type "type"
		 *
		 *  A postfix expression followed by an expression in square brackets [] is a subscripted
		 *  designation of an element of an array object. The definition of the subscript operator []
		 *  is that E1[E2] is identical to (*((E1)+(E2))). Because of the conversion rules that
		 *  apply to the binary + operator, if E1 is an array object (equivalently, a pointer to the
		 *  initial element of an array object) and E2is an integer, E1[E2] designates the E2-th
		 *  element of E1(counting from zero)
		 * */
		case EXPR_OPCODE_ARRAY_INDEX:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;

			if(!ey_type_is_integer(type2->type))
			{
				ey_parser_set_error(eng, &operand2->location, "index type %s is not integer type\n", ey_type_type_name(type2->type));
				return -1;
			}

			if(!ey_type_is_pointer_array(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "type %s is neither pointer nor array\n", ey_type_type_name(type1->type));
				return -1;
			}

			if(!ey_type_is_declared(eng, type1))
			{
				ey_parser_set_error(eng, &operand1->location, "type %s is not defined\n", ey_type_type_name(type1->type));
				return -1;
			}

			type3 = (ey_type_is_pointer(type1->type))?
					(type1->pointer_type.deref_type):
					(type1->array_type.base_type);
			if(!ey_type_is_declared(eng, type3))
			{
				ey_parser_set_error(eng, &operand1->location, "base type %s of %s is not defined\n", 
					ey_type_type_name(type3->type), ey_type_type_name(type1->type));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.2.3 of ISO/IEC 9899:1999
		 *  The first operand of the . operator shall have a qualified or unqualified structure or union
		 *  type, and the second operand shall name a member of that type
		 *
		 *  A postfix expression followed by the . operator and an identifier designates a member of
		 *  a structure or union object. The value is that of the named member, and is an lvalue if the
		 *  first expression is an lvalue.  If the first expression has qualified type, the result has the
		 *  so-qualified version of the type of the designated member.
		 */
		case EXPR_OPCODE_MEMBER:
		{
			operand1 = expr->member_expr.su;
			type1 = operand1->expr_type;
			if(!ey_type_is_su(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "expr type %s is not struct or union\n", ey_type_type_name(type1->type));
				return -1;
			}

			if(!ey_type_is_declared(eng, type1))
			{
				ey_parser_set_error(eng, &operand1->location, "struct or union type is not defined\n");
				return -1;
			}

			ey_member_t *member = ey_type_find_member(eng, type1, operand1->member_expr.member);
			if(!member)
			{
				ey_parser_set_error(eng, &operand1->member_expr.member->location, "%s is not a member of struct or union\n", 
					operand1->member_expr.member->name);
				return -1;
			}
			break;
		}
		/* 
		 * Section 6.5.2.3 of ISO/IEC 9899:1999
		 *  The first operand of the ->operator shall have type pointer to qualified or unqualified
		 *  structure or pointer to qualified or unqualified union, and the second operand shall 
		 *  name a member of the type pointed to.
		 *
		 *  A postfix expression followed by the->operator and an identifier designates a member
		 *  of a structure or union object. The value is that of the named member of the object to
		 *  which the first expression points, and is an lvalue. If the first expression is a pointer to
		 *  a qualified type, the result has the so-qualified version of the type of the designated
		 *  member.
		 * */
		case EXPR_OPCODE_MEMBER_PTR:
		{
			operand1 = expr->member_expr.su;
			type1 = operand1->expr_type;
			if(!ey_type_is_pointer(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "expr type %s is not pointer\n", ey_type_type_name(type1->type));
				return -1;
			}
			
			type2 = type1->pointer_type.deref_type;
			if(!ey_type_is_su(type2->type))
			{
				ey_parser_set_error(eng, &operand1->location, "expr type %s is not struct or union\n", ey_type_type_name(type2->type));
				return -1;
			}

			if(!ey_type_is_declared(eng, type2))
			{
				ey_parser_set_error(eng, &operand1->location, "struct or union type is not defined\n");
				return -1;
			}

			ey_member_t *member = ey_type_find_member(eng, type2, operand1->member_expr.member);
			if(!member)
			{
				ey_parser_set_error(eng, &operand1->member_expr.member->location, "%s is not a member of struct or union\n", 
					operand1->member_expr.member->name);
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.2.3 of ISO/IEC 9899:1999
		 *  The operand of the postfix increment or decrement operator shall have qualified or
		 *  unqualified real or pointer type and shall be a modifiable lvalue.
		 *
		 *  The result of the postfix ++ operator is the value of the operand. After the result is
		 *  obtained, the value of the operand is incremented. (That is, the value 1 of the appropriate
		 *  type is added to it.) See the discussions of additive operators and compound assignment
		 *  for information on constraints, types, and conversions and the effects of operations on
		 *  pointers. The side effect of updating the stored value of the operand shall occur between
		 *  the previous and the next sequence point.
		 *
		 *  The postfix -- operator is analogous to the postfix ++operator, except that the value of
		 *  the operand is decremented (that is, the value 1 of the appropriate type is subtracted from
		 *  it).
		 * */
		case EXPR_OPCODE_POST_INC:
		case EXPR_OPCODE_POST_DEC:
		{
			operand1 = expr->unary_expr.operand;
			type1 = operand1->expr_type;

			if(!ey_type_is_scalar(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "operand type %s is not scalar type\n", ey_type_type_name(type1->type));
				return -1;
			}

			if(!ey_type_is_lhs(type1))
			{
				ey_parser_set_error(eng, &operand1->location, "operand is not left-hand-side\n");
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.3.1 of ISO/IEC 9899:1999
		 * The operand of the prefix increment or decrement operator shall have qualified or
		 * unqualified real or pointer type and shall be a modifiable lvalue.
		 *
		 * The value of the operand of the prefix ++operator is incremented. The result is the new
		 * value of the operand after incrementation. The expression ++E is equivalent to (E+=1).
		 * See the discussions of additive operators and compound assignment for information on
		 * constraints, types, side effects, and conversions and the effects of operations on pointers.
		 *
		 * The prefix --operator is analogous to the prefix ++operator, except that the value of the
		 * operand is decremented.
		 * */
		case EXPR_OPCODE_PRE_DEC:
		case EXPR_OPCODE_PRE_INC:
		{
			operand1 = expr->unary_expr.operand;
			type1 = operand1->expr_type;

			if(!ey_type_is_scalar(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "operand type %s is not scalar type\n", ey_type_type_name(type1->type));
				return -1;
			}

			if(!ey_type_is_lhs(type1))
			{
				ey_parser_set_error(eng, &operand1->location, "operand is not left-hand-side\n");
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.4 of ISO/IEC 9899:1999
		 *  Unless the type name specifies a void type, the type name shall specify qualified or
		 *  unqualified scalar type and the operand shall have scalar type.
		 *
		 *  Conversions that involve pointers, other than where permitted by the constraints of
		 *  6.5.16.1, shall be specified by means of an explicit cast.
		 *
		 *  Preceding an expression by a parenthesized type name converts the value of the
		 *  expression to the named type. This construction is called acast. A cast that specifies
		 *  no conversion has no effect on the type or value of an expression.
		 * */
		case EXPR_OPCODE_CAST:
		{
			operand1 = expr->cast_expr.expr;
			type1 = operand1->expr_type;
			type2 = expr->cast_expr.type;

			if(!ey_type_is_scalar(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "operand type %s is not scalar\n", ey_type_type_name(type1->type));
				return -1;
			}

			if(!ey_type_is_scalar(type2->type))
			{
				ey_parser_set_error(eng, &type2->location, "cast type %s is not scalar\n", ey_type_type_name(type2->type));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.5 of ISO/IEC 9899:1999
		 *  Each of the operands shall have arithmetic type. The operands of the % operator shall
		 *  have integer type.
		 *
		 *  The usual arithmetic conversions are performed on the operands.
		 *
		 *  The result of the binary * operator is the product of the operands.
		 *
		 *  The result of the / operator is the quotient from the division of the first operand by the
		 *  second; the result of the % operator is the remainder. In both operations, if the value of
		 *  the second operand is zero, the behavior is undefined
		 *
		 *  When integers are divided, the result of the / operator is the algebraic quotient with any
		 *  fractional part discarded. If the quotient a/b is representable, the expression
		 *  (a/b)*b + a%b shall equal a
		 * */
		case EXPR_OPCODE_MOD:
		case EXPR_OPCODE_DIV:
		case EXPR_OPCODE_MULT:
		case EXPR_OPCODE_MOD_ASGN:
		case EXPR_OPCODE_DIV_ASGN:
		case EXPR_OPCODE_MULT_ASGN:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;
			
			if(ey_expr_is_assignment(opcode) && !ey_type_is_lhs(type1))
			{
				ey_parser_set_error(eng, &operand1->location, "left operand is not left-hand-side\n");
				return -1;
			}

			if(!ey_type_is_arithmetic(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "for opcode %s, left operand type %s must be arithmetic type\n", 
					ey_expr_opcode_name(opcode), ey_type_type_name(type1->type));
				return -1;
			}

			if(!ey_type_is_arithmetic(type2->type))
			{
				ey_parser_set_error(eng, &operand2->location, "for opcode %s, right operand type %s must be arithmetic type\n", 
					ey_expr_opcode_name(opcode), ey_type_type_name(type2->type));
				return -1;
			}

			if(opcode==EXPR_OPCODE_MOD)
			{
				if(!ey_type_is_integer(type1->type))
				{
					ey_parser_set_error(eng, &operand1->location, "for opcode %s, left operand type %s must be integer type\n", 
						ey_expr_opcode_name(opcode), ey_type_type_name(type1->type));
					return -1;
				}
				
				if(!ey_type_is_integer(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "for opcode %s, right operand type %s must be integer type\n", 
						ey_expr_opcode_name(opcode), ey_type_type_name(type2->type));
					return -1;
				}
			}

			if(ey_expr_is_div(opcode) && operand2->const_value && ey_symbol_is_zero(eng, operand2->const_value))
			{
				ey_parser_set_error(eng, &operand2->location, "divied number is zero\n");
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.6 of ISO/IEC 9899:1999
		 *  For addition:
		 *    both operands shall have arithmetic type.
		 *    one operand shall be a pointer to an object type and the other shall have integer type.
		 *
		 *  For subtraction, one of the following shall hold:
		 *    both operands have arithmetic type.
		 *    both operands are pointers to qualified or unqualified versions of compatible object types.
		 *    the left operand is a pointer to an object type and the right operand has integer type.
		 * */
		case EXPR_OPCODE_ADD:
		case EXPR_OPCODE_SUB:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;
			
			if(opcode==EXPR_OPCODE_ADD)
			{
				if(ey_type_is_integer(type1->type))
				{
					if(!ey_type_is_arithmetic(type2->type) && !ey_type_is_pointer_array(type2->type))
					{
						ey_parser_set_error(eng, &operand2->location, "right operand type %s is neither arithmetic nor pointer\n",
							ey_type_type_name(type2->type));
						return -1;
					}
				}
				else if(ey_type_is_real(type1->type))
				{
					if(!ey_type_is_arithmetic(type2->type))
					{
						ey_parser_set_error(eng, &operand2->location, "right operand type %s is not arithmetic\n", ey_type_type_name(type2->type));
						return -1;
					}
				}
				else if(ey_type_is_pointer_array(type1->type))
				{
					if(!ey_type_is_integer(type2->type))
					{
						ey_parser_set_error(eng, &operand2->location, "right operand type %s is not integer\n", ey_type_type_name(type2->type));
						return -1;
					}
				}
				else
				{
					ey_parser_set_error(eng, &operand2->location, "right operand type %s is neither arithmetic nor pointer\n", 
						ey_type_type_name(type2->type));
					return -1;
				}
			}
			else
			{
				if(ey_type_is_arithmetic(type1->type))
				{
					if(!ey_type_is_arithmetic(type1->type))
					{
						ey_parser_set_error(eng, &operand2->location, "right operand type %s is not arithmetic\n", ey_type_type_name(type2->type));
						return -1;
					}
				}
				else if(ey_type_is_pointer_array(type1->type))
				{
					if(!ey_type_is_arithmetic(type2->type) && !ey_type_is_pointer_array(type2->type))
					{
						ey_parser_set_error(eng, &operand2->location, "right operand type %s is neither arithmetic nor pointer\n",
							ey_type_type_name(type2->type));
						return -1;
					}

					if(ey_type_is_pointer_array(type2->type) && !ey_type_pointer_compatible(eng, type1, type2, 0))
					{
						ey_parser_set_error(eng, loc, "the two pointer types are not compatible\n");
						return -1;
					}
				}
				else
				{
					ey_parser_set_error(eng, &operand2->location, "left operand type %s is neither arithmetic nor pointer\n", 
						ey_type_type_name(type2->type));
					return -1;
				}
			}
			break;
		}
		/*
		 * Section 6.5.16.2 of ISO/IEC 9899:1999
		 *  For the operators+=and -=only, either the left operand shall be a pointer to an object
		 *  type and the right shall have integer type, or the left operand shall have qualified or
		 *  unqualified arithmetic type and the right shall have arithmetic type.
		 * */
		case EXPR_OPCODE_ADD_ASGN:
		case EXPR_OPCODE_SUB_ASGN:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;
			
			if(ey_expr_is_assignment(opcode) && !ey_type_is_lhs(type1))
			{
				ey_parser_set_error(eng, &operand1->location, "left operand is not left-hand-side\n");
				return -1;
			}
			
			if(ey_type_is_arithmetic(type1->type))
			{
				if(!ey_type_is_arithmetic(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "right operand type %s is not arithmetic\n", ey_type_type_name(type2->type));
					return -1;
				}
			}
			else if(ey_type_is_pointer(type1->type))
			{
				if(!ey_type_is_integer(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "right operand type %s is not integer\n", ey_type_type_name(type2->type));
					return -1;
				}
			}
			else
			{
				ey_parser_set_error(eng, &operand2->location, "left operand type %s is neither arithmetic nor pointer\n", 
					ey_type_type_name(type2->type));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.7 of ISO/IEC 9899:1999
		 *  Each of the operands shall have integer type.
		 *
		 *  The integer promotions are performed on each of the operands. The type of the result is
		 *  that of the promoted left operand. If the value of the right operand is negative or is
		 *  greater than or equal to the width of the promoted left operand, the behavior is undefined.
		 * */
		case EXPR_OPCODE_LSHIFT:
		case EXPR_OPCODE_RSHIFT:
		case EXPR_OPCODE_LSHIFT_ASGN:
		case EXPR_OPCODE_RSHIFT_ASGN:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;

			if(ey_expr_is_assignment(opcode) && !ey_type_is_lhs(type1))
			{
				ey_parser_set_error(eng, &operand1->location, "left operand is not left-hand-side\n");
				return -1;
			}

			if(!ey_type_is_integer(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "left operand type %s is not integer\n", ey_type_type_name(type1->type));
				return -1;
			}

			if(!ey_type_is_integer(type2->type))
			{
				ey_parser_set_error(eng, &operand1->location, "right operand type %s is not integer\n", ey_type_type_name(type2->type));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.8 of ISO/IEC 9899:1999
		 *  One of the following shall hold:
		 *    both operands have real type
		 *    both operands are pointers to qualified or unqualified versions of compatible object types
		 *    both operands are pointers to qualified or unqualified versions of compatible incomplete types
		 *
		 *  If both of the operands have arithmetic type, the usual arithmetic conversions are performed.
		 *
		 *  For the purposes of these operators, a pointer to a nonarray object behaves the same as a
		 *  pointer to the first element of an array of length one with the type of the object as its
		 *  element type
		 *
		 *  When two pointers are compared, the result depends on the relative locations in the
		 *  address space of the objects pointed to. If two pointers to object or incomplete types both
		 *  point to the same object, or both point one past the last element of the same array object,
		 *  they compare equal. If the objects pointed to are members of the same aggregate object,
		 *  pointers to structure members declared later compare greater than pointers to members
		 *  declared earlier in the structure, and pointers to array elements with larger subscript
		 *  values compare greater than pointers to elements of the same array with lower subscript
		 *  values.  All pointers to members of the same union object compare equal. If the
		 *  expression P points to an element of an array object and the expressionQ points to the
		 *  last element of the same array object, the pointer expression Q+1 compares greater than
		 *  P. In all other cases, the behavior is undefined
		 *
		 *  Each of the operators < (less than), > (greater than), <=(less than or equal to), and >=
		 *  (greater than or equal to) shall yield 1 if the specified relation is true and 0 if it is false.
		 *  The result has type int.
		 * */
		case EXPR_OPCODE_GE:
		case EXPR_OPCODE_GT:
		case EXPR_OPCODE_LE:
		case EXPR_OPCODE_LT:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;

			if(ey_type_is_arithmetic(type1->type))
			{
				if(!ey_type_is_arithmetic(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "right operand type %s is not arithmetic\n", ey_type_type_name(type2->type));
					return -1;
				}
			}
			else if(ey_type_is_pointer_array(type1->type))
			{
				if(!ey_type_is_pointer_array(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "right operand type %s is not pointer\n", ey_type_type_name(type2->type));
					return -1;
				}

				if(!ey_type_pointer_compatible(eng, type1, type2, 0))
				{
					ey_parser_set_error(eng, loc, "the two pointer type are not compatible\n");
					return -1;
				}
			}
			else
			{
				ey_parser_set_error(eng, &operand1->location, "left operand type %s is neither arithmetic nor pointer\n",
					ey_type_type_name(type1->type));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.9 of ISO/IEC 9899:1999
		 *  One of the following shall hold:
		 *    both operands have arithmetic type
		 *    both operands are pointers to qualified or unqualified versions of compatible types
		 *    one operand is a pointer to an object or incomplete type and the other is a pointer to a
		 *    qualified or unqualified version of void
		 *    one operand is a pointer and the other is a null pointer constant
		 *
		 *  The == (equal to) and != (not equal to) operators are analogous to the relational
		 *  operators except for their lower precedence. Each of the operators yields 1 if the
		 *  specified relation is true and 0 if it is false. The result has type "int" . For any pair of
		 *  operands, exactly one of the relations is true.
		 *
		 *  If both of the operands have arithmetic type, the usual arithmetic conversions are
		 *  performed.  Values of complex types are equal if and only if both their real parts are equal
		 *  and also their imaginary parts are equal. Any two values of arithmetic types from
		 *  different type domains are equal if and only if the results of their conversions to the
		 *  (complex) result type determined by the usual arithmetic conversions are equal.
		 *
		 *  Otherwise, at least one operand is a pointer. If one operand is a pointer and the other is a
		 *  null pointer constant, the null pointer constant is converted to the type of the pointer. If
		 *  one operand is a pointer to an object or incomplete type and the other is a pointer to a
		 *  qualified or unqualified version of void, the former is converted to the type of the latter.
		 *
		 *  Tw o pointers compare equal if and only if both are null pointers, both are pointers to the
		 *  same object (including a pointer to an object and a subobject at its beginning) or function,
		 *  both are pointers to one past the last element of the same array object, or one is a pointer
		 *  to one past the end of one array object and the other is a pointer to the start of a different
		 *  array object that happens to immediately follow the first array object in the address
		 *  space
		 * */
		case EXPR_OPCODE_EQ:
		case EXPR_OPCODE_NE:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;

			if(ey_type_is_arithmetic(type1->type))
			{
				if(!ey_type_is_arithmetic(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "right operand type %s is not arithmetic\n", ey_type_type_name(type2->type));
					return -1;
				}
			}
			else if(ey_type_is_pointer_array(type1->type))
			{
				if(!ey_type_is_pointer_array(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "right operand type %s is not pointer\n", ey_type_type_name(type2->type));
					return -1;
				}

				if(!ey_type_pointer_compatible(eng, type1, type2, 0))
				{
					ey_parser_set_error(eng, loc, "the two pointer type are not compatible\n");
					return -1;
				}
			}
			else
			{
				ey_parser_set_error(eng, &operand1->location, "left operand type %s is neither arithmetic nor pointer\n",
					ey_type_type_name(type1->type));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.10 of ISO/IEC 9899:1999
		 *  Each of the operands shall have integer type.
		 *
		 *  The usual arithmetic conversions are performed on the operands
		 *
		 *  The result of the binary & operator is the bitwise AND of the operands (that is, each bit in
		 *  the result is set if and only if each of the corresponding bits in the converted operands is
		 *  set)
		 * */
		case EXPR_OPCODE_BIT_AND:
		case EXPR_OPCODE_BIT_AND_ASGN:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;
			
			if(ey_expr_is_assignment(opcode) && !ey_type_is_lhs(type1))
			{
				ey_parser_set_error(eng, &operand1->location, "left operand is not left-hand-side\n");
				return -1;
			}

			if(!ey_type_is_integer(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "left operand type %s is not integer\n", ey_type_type_name(type1->type));
				return -1;
			}
			
			if(!ey_type_is_integer(type2->type))
			{
				ey_parser_set_error(eng, &operand2->location, "right operand type %s is not integer\n", ey_type_type_name(type2->type));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.11 of ISO/IEC 9899:1999
		 *  Each of the operands shall have integer type.
		 *
		 *  The usual arithmetic conversions are performed on the operands
		 *  
		 *  The result of the ˆ operator is the bitwise exclusiveORof the operands (that is, each bit
		 *  in the result is set if and only if exactly one of the corresponding bits in the converted
		 *  operands is set).
		 * */
		case EXPR_OPCODE_BIT_XOR:
		case EXPR_OPCODE_BIT_XOR_ASGN:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;
			
			if(ey_expr_is_assignment(opcode) && !ey_type_is_lhs(type1))
			{
				ey_parser_set_error(eng, &operand1->location, "left operand is not left-hand-side\n");
				return -1;
			}

			if(!ey_type_is_integer(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "left operand type %s is not integer\n", ey_type_type_name(type1->type));
				return -1;
			}
			
			if(!ey_type_is_integer(type2->type))
			{
				ey_parser_set_error(eng, &operand2->location, "right operand type %s is not integer\n", ey_type_type_name(type2->type));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.12 of ISO/IEC 9899:1999
		 *  Each of the operands shall have integer type.
		 *
		 *  The usual arithmetic conversions are performed on the operands
		 *  
		 *  The result of the | operator is the bitwise inclusive ORof the operands (that is, each bit in
		 *  the result is set if and only if at least one of the corresponding bits in the converted
		 *  operands is set)
		 * */
		case EXPR_OPCODE_BIT_OR:
		case EXPR_OPCODE_BIT_OR_ASGN:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;
			
			if(ey_expr_is_assignment(opcode) && !ey_type_is_lhs(type1))
			{
				ey_parser_set_error(eng, &operand1->location, "left operand is not left-hand-side\n");
				return -1;
			}

			if(!ey_type_is_integer(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "left operand type %s is not integer\n", ey_type_type_name(type1->type));
				return -1;
			}
			
			if(!ey_type_is_integer(type2->type))
			{
				ey_parser_set_error(eng, &operand2->location, "right operand type %s is not integer\n", ey_type_type_name(type1->type));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.13 of ISO/IEC 9899:1999
		 *  Each of the operands shall have scalar type.
		 *
		 *  The &&operator shall yield 1 if both of its operands compare unequal to 0; otherwise, it
		 *  yields 0. The result has type "int"
		 *
		 *  Unlike the bitwise binary & operator, the&&operator guarantees left-to-right evaluation;
		 *  there is a sequence point after the evaluation of the first operand. If the first operand
		 *  compares equal to 0, the second operand is not evaluated
		 * */
		case EXPR_OPCODE_LOGIC_AND:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;

			if(!ey_type_is_scalar(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "operand type %s is not scalar\n", ey_type_type_name(type1->type));
				return -1;
			}

			if(!ey_type_is_scalar(type2->type))
			{
				ey_parser_set_error(eng, &type2->location, "cast type %s is not scalar\n", ey_type_type_name(type2->type));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.14 of ISO/IEC 9899:1999
		 *  Each of the operands shall have scalar type.
		 *
		 *  The &&operator shall yield 1 if both of its operands compare unequal to 0; otherwise, it
		 *  yields 0. The result has type "int"
		 *
		 *  Unlike the bitwise | operator, the||operator guarantees left-to-right evaluation; there is
		 *  a sequence point after the evaluation of the first operand. If the first operand compares
		 *  unequal to 0, the second operand is not evaluated
		 * */
		case EXPR_OPCODE_LOGIC_OR:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;

			if(!ey_type_is_scalar(type1->type))
			{
				ey_parser_set_error(eng, &operand1->location, "operand type %s is not scalar\n", ey_type_type_name(type1->type));
				return -1;
			}

			if(!ey_type_is_scalar(type2->type))
			{
				ey_parser_set_error(eng, &type2->location, "cast type %s is not scalar\n", ey_type_type_name(type2->type));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.15 of ISO/IEC 9899:1999
		 *  The first operand shall have scalar type
		 *  One of the following shall hold for the second and third operands:
		 *    both operands have arithmetic type;
		 *    both operands have compatible structure or union types;
		 *    both operands have void type;
		 *    both operands are pointers to qualified or unqualified versions of compatible types;
		 *    one operand is a pointer and the other is a null pointer constant;
		 *    one operand is a pointer to an object or incomplete type and the other is a pointer to a
		 *    qualified or unqualified version of void
		 *
		 *  The first operand is evaluated; there is a sequence point after its evaluation.  The second
		 *  operand is evaluated only if the first compares unequal to 0; the third operand is evaluated
		 *  only if the first compares equal to 0; the result is the value of the second or third operand
		 *  (whichever is evaluated), converted to the type described below. If an attempt is made
		 *  to modify the result of a conditional operator or to access it after the next sequence point,
		 *  the behavior is undefined.
		 *
		 *  If both the second and third operands have arithmetic type, the result type that would be
		 *  determined by the usual arithmetic conversions, were they applied to those two operands,
		 *  is the type of the result. If both the operands have structure or union type, the result has
		 *  that type. If both operands have void type, the result has void type.
		 *
		 *  If both the second and third operands are pointers or one is a null pointer constant and the
		 *  other is a pointer, the result type is a pointer to a type qualified with all the type qualifiers
		 *  of the types pointed-to by both operands. Furthermore, if both operands are pointers to
		 *  compatible types or to differently qualified versions of compatible types, the result type is
		 *  a pointer to an appropriately qualified version of the composite type; if one operand is a
		 *  null pointer constant, the result has the type of the other operand; otherwise, one operand
		 *  is a pointer to void or a qualified version of void, in which case the result type is a 
		 *  pointer to an appropriately qualified version of void.
		 * */
		case EXPR_OPCODE_CONDITION:
		{
			operand1 = expr->condition_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->condition_expr.right;
			type2 = operand2->expr_type;
			operand3 = expr->condition_expr.condition;
			type3 = operand3->expr_type;

			if(!ey_type_is_scalar(type3->type))
			{
				ey_parser_set_error(eng, &operand3->location, "condition type %s is not scalar\n", ey_type_type_name(type3->type));
				return -1;
			}

			if(ey_type_is_arithmetic(type1->type))
			{
				if(!ey_type_is_arithmetic(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "second expr type %s is not arithmetic\n", ey_type_type_name(type2->type));
					return -1;
				}
			}
			else if(ey_type_is_su(type1->type))
			{
				if(!ey_type_is_su(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "second expr type %s is not struct/union\n", ey_type_type_name(type2->type));
					return -1;
				}

				if(!ey_type_tag_equal(eng, type1, type2, 0))
				{
					ey_parser_set_error(eng, ey_combine_location(eng, &operand1->location, &operand2->location),
						"the two expr are not compatible struct/union type\n");
					return -1;
				}
			}
			else if(ey_type_is_void(type1->type))
			{
				if(!ey_type_is_void(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "second expr type %s is not void\n", ey_type_type_name(type2->type));
					return -1;
				}
			}
			else if(ey_type_is_pointer_array(type1->type))
			{
				if(!ey_type_is_pointer_array(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "second expr type %s is not pointer/array\n", ey_type_type_name(type2->type));
					return -1;
				}

				if(!ey_type_pointer_compatible(eng, type1, type2, 0))
				{
					ey_parser_set_error(eng, &operand2->location, "sencond expr type is not pointer compatible\n");
					return -1;
				}
			}
			break;
		}
		/*
		 * Section 6.5.16 of ISO/IEC 9899:1999
		 *   An assignment operator shall have a modifiable lvalue as its left operand.
		 *
		 *   An assignment operator stores a value in the object designated by the left operand. An
		 *   assignment expression has the value of the left operand after the assignment, but is not an
		 *   lvalue.  The type of an assignment expression is the type of the left operand unless the
		 *   left operand has qualified type, in which case it is the unqualified version of the type of
		 *   the left operand. The side effect of updating the stored value of the left operand shall
		 *   occur between the previous and the next sequence point.
		 *
		 *   The order of evaluation of the operands is unspecified. If an attempt is made to modify
		 *   the result of an assignment operator or to access it after the next sequence point, the
		 *   behavior is "undefined".
		 * */
		/*
		 * Section 6.5.16.1 of ISO/IEC 9899:1999
		 *  One of the following shall hold:
		 *    the left operand has qualified or unqualified arithmetic type and the right has
		 *    arithmetic type;
		 *    the left operand has a qualified or unqualified version of a structure or union type
		 *    compatible with the type of the right;
		 *    both operands are pointers to qualified or unqualified versions of compatible types,
		 *    and the type pointed to by the left has all the qualifiers of the type pointed to by the
		 *    right;
		 *    one operand is a pointer to an object or incomplete type and the other is a pointer to a
		 *    qualified or unqualified version of void, and the type pointed to by the left has all
		 *    the qualifiers of the type pointed to by the right;
		 *    the left operand is a pointer and the right is a null pointer constant;
		 *    the left operand has type_Booland the right is a pointer;
		 *
		 *  Insimple assignment (=), the value of the right operand is converted to the type of the
		 *  assignment expression and replaces the value stored in the object designated by the left
		 *  operand;
		 *
		 *  If the value being stored in an object is read from another object that overlaps in any way
		 *  the storage of the first object, then the overlap shall be exact and the two objects shall
		 *  have qualified or unqualified versions of a compatible type; otherwise, the behavior is
		 *  undefined;
		 * */
		case EXPR_OPCODE_ASGN:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;

			if(!ey_type_is_lhs(type1))
			{
				ey_parser_set_error(eng, &operand1->location, "left operand is not left-hand-size\n");
				return -1;
			}

			if(ey_type_is_arithmetic(type1->type))
			{
				if(!ey_type_is_arithmetic(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "right operand type %s is not arithmetic\n", ey_type_type_name(type2->type));
					return -1;
				}
			}
			else if(ey_type_is_su(type1->type))
			{
				if(!ey_type_is_su(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "right operand type %s is not struct/union\n", ey_type_type_name(type2->type));
					return -1;
				}

				if(!ey_type_tag_equal(eng, type1, type2, 0))
				{
					ey_parser_set_error(eng, ey_combine_location(eng, &operand1->location, &operand2->location),
						"the two expr are not compatible struct/union type\n");
					return -1;
				}
			}
			else if(ey_type_is_pointer(type1->type))
			{
				if(!ey_type_is_pointer_array(type2->type))
				{
					ey_parser_set_error(eng, &operand2->location, "second expr type %s is not pointer/array\n", ey_type_type_name(type2->type));
					return -1;
				}

				if(!ey_type_pointer_compatible(eng, type1, type2, 1))
				{
					ey_parser_set_error(eng, &operand2->location, "sencond expr type is not pointer compatible\n");
					return -1;
				}
			}
			else
			{
				ey_parser_set_error(eng, &operand1->location, "left operand type %s is not supported\n", ey_type_type_name(type1->type));
				return -1;
			}
			break;
		}
		/*
		 * Section 6.5.17 of ISO/IEC 9899:1999
		 *  The left operand of a comma operator is evaluated as a void expression; there is a
		 *  sequence point after its evaluation.  Then the right operand is evaluated; the result has its
		 *  type and value. If an attempt is made to modify the result of a comma operator or to
		 *  access it after the next sequence point, the behavior is undefined.
		 * */
		case EXPR_OPCODE_COMPOUND:
		{
			break;
		}
		/*
		 * Section 6.5.2.2 of ISO/IEC 9899:1999
		 *  The expression that denotes the called function shall have type pointer to function
		 *  returning void or returning an object type other than an array type.
		 *
		 *  If the expression that denotes the called function has a type that includes a prototype, the
		 *  number of arguments shall agree with the number of parameters. Each argument shall
		 *  have a type such that its value may be assigned to an object with the unqualified version
		 *  of the type of its corresponding parameter.
		 *
		 *  A postfix expression followed by parentheses ()containing a possibly empty, 
		 *  comma-separated list of expressions is a function call. The postfix expression denotes the called
		 *  function.  The list of expressions specifies the arguments to the function.
		 *
		 *  An argument may be an expression of any object type. In preparing for the call to a
		 *  function, the arguments are evaluated, and each parameter is assigned the value of the
		 *  corresponding argument.
		 *
		 *  If the expression that denotes the called function has type pointer to function returning an
		 *  object type, the function call expression has the same type as that object type, and has the
		 *  value determined as specified in 6.8.6.4. Otherwise, the function call has type void.If
		 *  an attempt is made to modify the result of a function call or to access it after the next
		 *  sequence point, the behavior is undefined.
		 *
		 *  If the expression that denotes the called function has a type that does not include a
		 *  prototype, the integer promotions are performed on each argument, and arguments that
		 *  have type floatare promoted to double. These are called the default argument
		 *  promotions . If the number of arguments does not equal the number of parameters, the
		 *  behavior is undefined. If the function is defined with a type that includes a prototype, and
		 *  either the prototype ends with an ellipsis ( , ...) or the types of the arguments after
		 *  promotion are not compatible with the types of the parameters, the behavior is undefined.
		 *  If the function is defined with a type that does not include a prototype, and the types of
		 *  the arguments after promotion are not compatible with those of the parameters after
		 *  promotion, the behavior is undefined, except for the following cases:
		 *    one promoted type is a signed integer type, the other promoted type is the
		 *    corresponding unsigned integer type, and the value is representable in both types;
		 *    both types are pointers to qualified or unqualified versions of a character type or
		 *    void
		 *
		 *  If the expression that denotes the called function has a type that does include a prototype,
		 *  the arguments are implicitly converted, as if by assignment, to the types of the
		 *  corresponding parameters, taking the type of each parameter to be the unqualified version
		 *  of its declared type. The ellipsis notation in a function prototype declarator causes
		 *  argument type conversion to stop after the last declared parameter. The default argument
		 *  promotions are performed on trailing arguments.
		 *
		 *  No other conversions are performed implicitly; in particular, the number and types of
		 *  arguments are not compared with those of the parameters in a function definition that
		 *  does not include a function prototype declarator.
		 *
		 *  If the function is defined with a type that is not compatible with the type (of the
		 *  expression) pointed to by the expression that denotes the called function, the behavior is
		 *  undefined
		 *
		 *  The order of evaluation of the function designator, the actual arguments, and
		 *  subexpressions within the actual arguments is unspecified, but there is a sequence point
		 *  before the actual call
		 *
		 *  Recursive function calls shall be permitted, both directly and indirectly through any chain
		 *  of other functions
		 * */
		case EXPR_OPCODE_FUNCALL:
		{
			operand1 = expr->funcall_expr.function;
			type1 = operand1->expr_type;
			if(ey_type_is_pointer(type1->type))
				type1 =type1->pointer_type.deref_type;
			
			/*check function symbol*/
			if(type1->type != TYPE_FUNCTION)
			{
				ey_parser_set_error(eng, &operand1->location, "function expr is neither function nor pointer to function\n");
				return -1;
			}
			
			/*check parameter assignment compatible*/
			if(TAILQ_EMPTY(&type1->function_type.arg_list))
			{
				if(!TAILQ_EMPTY(&expr->funcall_expr.arg_list))
				{
					ey_parser_set_error(eng, &expr->location, "the prototype of function do not accept any parameter\n");
					return -1;
				}
			}
			else
			{
				ey_symbol_t *formal = NULL;
				ey_expr_t *arg = NULL;
				int index = 0;

				for(index=0, formal=TAILQ_FIRST(&type1->function_type.arg_list), arg=TAILQ_FIRST(&expr->funcall_expr.arg_list);
					formal && arg;
					index++, formal=TAILQ_NEXT(formal, list_next), arg=TAILQ_NEXT(arg, link))
				{
					type2 = formal->type;
					type3 = arg->expr_type;
					if(!ey_type_assignment_compatible(eng, type2, type3))
					{
						ey_parser_set_error(eng, &arg->location, "argument %d type %s is not assignment conpatible with prototype\n",
							index, ey_type_type_name(type3->type));
						return -1;
					}
				}

				if(formal)
				{
					ey_parser_set_error(eng, &expr->location, "passing less arguments than the prototype\n");
					return -1;
				}

				if(arg && !type1->function_type.var_args)
				{
					ey_parser_set_error(eng, &expr->location, "passing more arguments than the prototype\n");
					return -1;
				}
			}

			break;
		}
		default:
		{
			ey_parser_set_error(eng, loc, "bad opcode %d\n", opcode);
			return -1;
		}
	}

	return 0;
}

int ey_expr_calc_type(ey_engine_t *eng, ey_expr_t *expr)
{
	ey_location_t *loc = &expr->location;
	ey_expr_opcode_t opcode = expr->opcode;
	ey_expr_t *operand1 = NULL;
	ey_type_t *type1 = NULL;
	ey_expr_t *operand2 = NULL;
	ey_type_t *type2 = NULL;
	ey_type_t *type3 = NULL;
	switch(opcode)
	{
		case EXPR_OPCODE_DEREF:
		{
			operand1 = expr->unary_expr.operand;
			type1 = ey_type_is_pointer(operand1->expr_type->type)?
					(operand1->expr_type->pointer_type.deref_type):
					(operand1->expr_type->array_type.base_type);
			expr->promoted_type = expr->expr_type = ey_type_copy(eng, type1);
			if(!expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "copy deref type failed\n");
				return -1;
			}

			break;
		}
		case EXPR_OPCODE_ADDRESS:
		{
			operand1 = expr->unary_expr.operand;
			expr->promoted_type = expr->expr_type = ey_alloc_pointer_type(eng, TYPE_POINTER, TYPE_QUALIFIER_CONST, 
				sizeof(void*), alignment_of(void*), &expr->location, operand1->expr_type);
			if(!expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "alloc pointer type failed\n");
				return -1;
			}
			break;
		}
		case EXPR_OPCODE_NEG:
		case EXPR_OPCODE_POS:
		{
			operand1 = expr->unary_expr.operand;
			expr->promoted_type = expr->expr_type = ey_type_copy(eng, operand1->expr_type);
			if(expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "alloc expression type failed\n");
				return -1;
			}

			ey_type_integer_promotion(eng, expr->expr_type);
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_BNEG:
		{
			operand1 = expr->unary_expr.operand;
			expr->promoted_type = expr->expr_type = ey_type_copy(eng, operand1->expr_type);
			if(expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "alloc expression type failed\n");
				return -1;
			}

			ey_type_integer_promotion(eng, expr->expr_type);
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_LNEG:
		{
			operand1 = expr->unary_expr.operand;
			expr->promoted_type = expr->expr_type = ey_type_copy(eng, ey_int_type(eng));
			if(expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "alloc expression type failed\n");
				return -1;
			}

			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_ARRAY_INDEX:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;

			type2 = (type1->type==TYPE_POINTER)?
					(type1->pointer_type.deref_type):
					(type1->array_type.base_type);

			expr->promoted_type = expr->expr_type = ey_type_copy(eng, type2);
			if(!expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "alloc array index type failed\n");
				return -1;
			}
			break;
		}
		case EXPR_OPCODE_MEMBER:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			ey_member_t *member = ey_type_find_member(eng, type1, operand1->member_expr.member);
			
			expr->promoted_type = expr->expr_type = ey_type_copy(eng, member->member->type);
			if(!expr->expr_type)
			{
				ey_parser_set_error(eng, &operand1->location, "copy member type failed\n");
				return -1;
			}

			expr->expr_type->qualifier_class |= type1->qualifier_class;
			break;
		}
		case EXPR_OPCODE_MEMBER_PTR:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type->pointer_type.deref_type;
			ey_member_t *member = ey_type_find_member(eng, type1, operand1->member_expr.member);
			
			expr->promoted_type = expr->expr_type = ey_type_copy(eng, member->member->type);
			if(!expr->expr_type)
			{
				ey_parser_set_error(eng, &operand1->location, "copy member type failed\n");
				return -1;
			}

			expr->expr_type->qualifier_class |= type1->qualifier_class;
			break;
		}
		case EXPR_OPCODE_POST_INC:
		case EXPR_OPCODE_POST_DEC:
		{
			operand1 = expr->unary_expr.operand;
			type1 = operand1->expr_type;
			
			expr->promoted_type = expr->expr_type = ey_type_copy(eng, type1);
			if(!expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "copy %s type failed\n", ey_expr_opcode_name(opcode));
				return -1;
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_PRE_DEC:
		case EXPR_OPCODE_PRE_INC:
		{
			operand1 = expr->unary_expr.operand;
			type1 = operand1->expr_type;
			
			expr->promoted_type = expr->expr_type = ey_type_copy(eng, type1);
			if(!expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "copy %s type failed\n", ey_expr_opcode_name(opcode));
				return -1;
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_CAST:
		{
			expr->promoted_type = expr->expr_type = ey_type_copy(eng, expr->cast_expr.type);
			if(!expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "copt cast type failed\n");
				return -1;
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
		}
		case EXPR_OPCODE_MOD:
		case EXPR_OPCODE_DIV:
		case EXPR_OPCODE_MULT:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;
			
			expr->promoted_type = expr->expr_type = ey_type_promotion(eng, type1, type2);
			if(!expr->expr_type)
			{
				if(!ey_parser_isset_error(eng))
					ey_parser_set_error(eng, loc, "promote opcode %s expr failed\n", ey_expr_opcode_name(opcode));
				return -1;
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_LSHIFT:
		case EXPR_OPCODE_RSHIFT:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;
			
			expr->promoted_type = expr->expr_type = ey_type_promotion(eng, type1, type2);
			if(!expr->expr_type)
			{
				if(!ey_parser_isset_error(eng))
					ey_parser_set_error(eng, loc, "promote opcode %s expr failed\n", ey_expr_opcode_name(opcode));
				return -1;
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_ADD:
		case EXPR_OPCODE_SUB:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;
			
			if(opcode==EXPR_OPCODE_ADD)
			{
				if(ey_type_is_arithmetic(type1->type) && ey_type_is_arithmetic(type2->type))
				{
					expr->promoted_type = expr->expr_type = ey_type_promotion(eng, type1, type2);
					if(!expr->expr_type)
					{
						if(!ey_parser_isset_error(eng))
							ey_parser_set_error(eng, loc, "promote opcode %s expr failed\n", ey_expr_opcode_name(opcode));
						return -1;
					}
				}
				else
				{
					type3 = ey_type_is_pointer_array(type1->type)?type1:type2;
					if(!ey_type_is_pointer(type3->type))
					{
						type3 = ey_type_array2pointer(eng, type3);
						if(!type3)
						{
							ey_parser_set_error(eng, loc, "convert pointer type failed\n");
							return -1;
						}
					}

					expr->promoted_type = expr->expr_type = ey_type_copy(eng, type3);
					if(!expr->expr_type)
					{
						ey_parser_set_error(eng, loc, "copy opcode %s type failed\n", ey_expr_opcode_name(opcode));
						return -1;
					}
				}
			}
			else
			{
				if(ey_type_is_arithmetic(type1->type))
				{
					expr->promoted_type = expr->expr_type = ey_type_promotion(eng, type1, type2);
					if(!expr->expr_type)
					{
						if(!ey_parser_isset_error(eng))
							ey_parser_set_error(eng, loc, "promote opcode %s expr failed\n", ey_expr_opcode_name(opcode));
						return -1;
					}
				}
				else
				{
					if(ey_type_is_pointer_array(type2->type))
					{
						type3 = ey_ulong_type(eng);
					}
					else if(ey_type_is_pointer(type1->type))
					{
						type3 = type1;
					}
					else
					{
						type3 = ey_type_array2pointer(eng, type1);
						if(!type3)
						{
							ey_parser_set_error(eng, loc, "convert array type failed\n");
							return -1;
						}
					}
					expr->promoted_type = expr->expr_type = ey_type_copy(eng, type3);
					if(!expr->expr_type)
					{
						ey_parser_set_error(eng, loc, "copy opcode %s type failed\n", ey_expr_opcode_name(opcode));
						return -1;
					}
				}
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_GE:
		case EXPR_OPCODE_GT:
		case EXPR_OPCODE_LE:
		case EXPR_OPCODE_LT:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;

			if(ey_type_is_arithmetic(type1->type))
			{
				expr->promoted_type = ey_type_promotion(eng, type1, type2);
				if(!expr->promoted_type)
				{
					if(!ey_parser_isset_error(eng))
						ey_parser_set_error(eng, loc, "promote opcode %s expr failed\n", ey_expr_opcode_name(opcode));
					return -1;
				}
				ey_type_integer_promotion(eng, expr->promoted_type);
			}
			else
			{
				expr->promoted_type = ey_type_copy(eng, type1);
				if(!expr->promoted_type)
				{
					ey_parser_set_error(eng, loc, "copy opcode %s expr type failed\n", ey_expr_opcode_name(opcode));
					return -1;
				}
			}

			expr->expr_type = ey_type_copy(eng, ey_int_type(eng));
			if(!expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "copy int type failed\n");
				return -1;
			}
			break;
		}
		case EXPR_OPCODE_EQ:
		case EXPR_OPCODE_NE:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;

			if(ey_type_is_arithmetic(type1->type))
			{
				expr->promoted_type = ey_type_promotion(eng, type1, type2);
				if(!expr->promoted_type)
				{
					if(!ey_parser_isset_error(eng))
						ey_parser_set_error(eng, loc, "promote opcode %s expr failed\n", ey_expr_opcode_name(opcode));
					return -1;
				}
				ey_type_integer_promotion(eng, expr->promoted_type);
			}
			else
			{
				expr->promoted_type = ey_type_copy(eng, type1);
				if(!expr->promoted_type)
				{
					ey_parser_set_error(eng, loc, "copy opcode %s expr type failed\n", ey_expr_opcode_name(opcode));
					return -1;
				}
			}

			expr->expr_type = ey_type_copy(eng, ey_int_type(eng));
			if(!expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "copy int type failed\n");
				return -1;
			}
			break;
		}
		case EXPR_OPCODE_BIT_AND:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;
			
			expr->promoted_type = expr->expr_type = ey_type_promotion(eng, type1, type2);
			if(!expr->expr_type)
			{
				if(!ey_parser_isset_error(eng))
					ey_parser_set_error(eng, loc, "promote opcode %s expr failed\n", ey_expr_opcode_name(opcode));
				return -1;
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_BIT_XOR:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;
			
			expr->promoted_type = expr->expr_type = ey_type_promotion(eng, type1, type2);
			if(!expr->expr_type)
			{
				if(!ey_parser_isset_error(eng))
					ey_parser_set_error(eng, loc, "promote opcode %s expr failed\n", ey_expr_opcode_name(opcode));
				return -1;
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_BIT_OR:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;
			
			expr->promoted_type = expr->expr_type = ey_type_promotion(eng, type1, type2);
			if(!expr->expr_type)
			{
				if(!ey_parser_isset_error(eng))
					ey_parser_set_error(eng, loc, "promote opcode %s expr failed\n", ey_expr_opcode_name(opcode));
				return -1;
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_LOGIC_AND:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;

			expr->expr_type = ey_type_copy(eng, ey_int_type(eng));
			if(!expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "copy int type failed\n");
				return -1;
			}
			expr->promoted_type = NULL;
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_LOGIC_OR:
		{
			operand1 = expr->binary_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->binary_expr.right;
			type2 = operand2->expr_type;

			expr->expr_type = ey_type_copy(eng, ey_int_type(eng));
			if(!expr->expr_type)
			{
				ey_parser_set_error(eng, loc, "copy int type failed\n");
				return -1;
			}
			expr->promoted_type = NULL;
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_CONDITION:
		{
			operand1 = expr->condition_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->condition_expr.right;
			type2 = operand2->expr_type;
			
			if(ey_type_is_arithmetic(type1->type))
			{
				expr->promoted_type = expr->expr_type = ey_type_promotion(eng, type1, type2);
				if(!expr->expr_type)
				{
					if(!ey_parser_isset_error(eng))
						ey_parser_set_error(eng, loc, "promote opcode %s expr failed\n", ey_expr_opcode_name(opcode));
					return -1;
				}
			}
			else if(ey_type_is_su(type1->type))
			{
				expr->promoted_type = expr->expr_type = ey_type_copy(eng, type1);
				if(!expr->expr_type)
				{
					if(!ey_parser_isset_error(eng))
						ey_parser_set_error(eng, loc, "copy opcode %s expr failed\n", ey_expr_opcode_name(opcode));
					return -1;
				}
			}
			else if(ey_type_is_void(type1->type))
			{
				expr->promoted_type = expr->expr_type = ey_type_copy(eng, ey_void_type(eng));
				if(!expr->expr_type)
				{
					if(!ey_parser_isset_error(eng))
						ey_parser_set_error(eng, loc, "copy opcode %s expr failed\n", ey_expr_opcode_name(opcode));
					return -1;
				}
			}
			else
			{
				expr->promoted_type = expr->expr_type = ey_type_copy(eng, type1);
				if(!expr->expr_type)
				{
					if(!ey_parser_isset_error(eng))
						ey_parser_set_error(eng, loc, "copy opcode %s expr failed\n", ey_expr_opcode_name(opcode));
					return -1;
				}
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
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
		{
			operand1 = expr->condition_expr.left;
			type1 = operand1->expr_type;
			operand2 = expr->condition_expr.right;
			type2 = operand2->expr_type;

			expr->promoted_type = expr->expr_type = ey_type_copy(eng, type1);
			if(!expr->expr_type)
			{
				if(!ey_parser_isset_error(eng))
					ey_parser_set_error(eng, loc, "copy opcode %s expr failed\n", ey_expr_opcode_name(opcode));
				return -1;
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_COMPOUND:
		{
			operand1 = TAILQ_LAST(&expr->list_expr.head, ey_expr_list);
			type1 = operand1->expr_type;

			expr->promoted_type = expr->expr_type = ey_type_copy(eng, type1);
			if(!expr->expr_type)
			{
				if(!ey_parser_isset_error(eng))
					ey_parser_set_error(eng, loc, "copy opcode %s expr failed\n", ey_expr_opcode_name(opcode));
				return -1;
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		case EXPR_OPCODE_FUNCALL:
		{
			operand1 = expr->funcall_expr.function;
			type1 = operand1->expr_type;
			if(ey_type_is_pointer(type1->type))
				type1 =type1->pointer_type.deref_type;
			
			expr->promoted_type = expr->expr_type = ey_type_copy(eng, type1->function_type.return_type);
			if(!expr->expr_type)
			{
				if(!ey_parser_isset_error(eng))
					ey_parser_set_error(eng, loc, "copy opcode %s expr failed\n", ey_expr_opcode_name(opcode));
				return -1;
			}
			expr->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
			break;
		}
		default:
		{
			ey_parser_set_error(eng, &expr->location, "bad opcode %d\n", opcode);
			return -1;
		}
	}

	return 0;
}

int ey_expr_need_calc_value(ey_engine_t *eng, ey_expr_t *expr)
{
	ey_expr_opcode_t opcode = expr->opcode;
	ey_expr_t *operand1 = NULL;
	ey_expr_t *operand2 = NULL;
	switch(opcode)
	{
		case EXPR_OPCODE_NEG:
		case EXPR_OPCODE_POS:
		case EXPR_OPCODE_BNEG:
		case EXPR_OPCODE_LNEG:
		{
			operand1 = expr->unary_expr.operand;
			if(operand1->const_value)
				return 1;
			break;
		}
		case EXPR_OPCODE_CAST:
		{
			operand1 = expr->cast_expr.expr;
			if(operand1->const_value)
				return 1;
			break;
		}
		case EXPR_OPCODE_MOD:
		case EXPR_OPCODE_DIV:
		case EXPR_OPCODE_MULT:
		{
			operand1 = expr->binary_expr.left;
			operand2 = expr->binary_expr.right;
			if(operand1->const_value && operand2->const_value)
				return 1;
			break;
		}
		case EXPR_OPCODE_LSHIFT:
		case EXPR_OPCODE_RSHIFT:
		{
			operand1 = expr->binary_expr.left;
			operand2 = expr->binary_expr.right;
			if(operand1->const_value && operand2->const_value)
				return 1;
			break;
		}
		case EXPR_OPCODE_GE:
		case EXPR_OPCODE_GT:
		case EXPR_OPCODE_LE:
		case EXPR_OPCODE_LT:
		{
			operand1 = expr->binary_expr.left;
			operand2 = expr->binary_expr.right;
			if(operand1->const_value && operand2->const_value)
				return 1;
			break;
		}
		case EXPR_OPCODE_EQ:
		case EXPR_OPCODE_NE:
		{
			operand1 = expr->binary_expr.left;
			operand2 = expr->binary_expr.right;
			if(operand1->const_value && operand2->const_value)
				return 1;
			break;
		}
		case EXPR_OPCODE_BIT_AND:
		{
			operand1 = expr->binary_expr.left;
			operand2 = expr->binary_expr.right;
			if(operand1->const_value && operand2->const_value)
				return 1;
			break;
		}
		case EXPR_OPCODE_BIT_XOR:
		{
			operand1 = expr->binary_expr.left;
			operand2 = expr->binary_expr.right;
			if(operand1->const_value && operand2->const_value)
				return 1;
			break;
		}
		case EXPR_OPCODE_BIT_OR:
		{
			operand1 = expr->binary_expr.left;
			operand2 = expr->binary_expr.right;
			if(operand1->const_value && operand2->const_value)
				return 1;
			break;
		}
		case EXPR_OPCODE_LOGIC_AND:
		{
			operand1 = expr->binary_expr.left;
			operand2 = expr->binary_expr.right;
			if(operand1->const_value && operand2->const_value)
				return 1;
			break;
		}
		case EXPR_OPCODE_LOGIC_OR:
		{
			operand1 = expr->binary_expr.left;
			operand2 = expr->binary_expr.right;
			if(operand1->const_value && operand2->const_value)
				return 1;
			break;
		}
		case EXPR_OPCODE_ADD:
		case EXPR_OPCODE_SUB:
		{
			operand1 = expr->binary_expr.left;
			operand2 = expr->binary_expr.right;
			if(operand1->const_value && operand2->const_value &&
				ey_type_is_arithmetic(operand1->expr_type->type) &&
				ey_type_is_arithmetic(operand2->expr_type->type))
				return 1;
			break;
		}
		case EXPR_OPCODE_COMPOUND:
		{
			operand1 = TAILQ_LAST(&expr->list_expr.head, ey_expr_list);
			if(operand1->const_value)
				return 1;
			break;
		}
		case EXPR_OPCODE_DEREF:
		case EXPR_OPCODE_ADDRESS:
		case EXPR_OPCODE_ARRAY_INDEX:
		case EXPR_OPCODE_MEMBER_PTR:
		case EXPR_OPCODE_MEMBER:
		case EXPR_OPCODE_POST_INC:
		case EXPR_OPCODE_POST_DEC:
		case EXPR_OPCODE_PRE_DEC:
		case EXPR_OPCODE_PRE_INC:
		case EXPR_OPCODE_CONDITION:
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
		case EXPR_OPCODE_FUNCALL:
		default:
		{
			return 0;
		}
	}

	return 1;
}

int ey_expr_calc_value(ey_engine_t *eng, ey_expr_t *expr)
{
	ey_location_t *loc = &expr->location;
	ey_expr_opcode_t opcode = expr->opcode;
	void *value = NULL;
	ey_symbol_t *symbol = NULL;
	switch(opcode)
	{
		case EXPR_OPCODE_NEG:
		case EXPR_OPCODE_POS:
		case EXPR_OPCODE_BNEG:
		case EXPR_OPCODE_LNEG:
		case EXPR_OPCODE_MOD:
		case EXPR_OPCODE_DIV:
		case EXPR_OPCODE_MULT:
		case EXPR_OPCODE_CAST:
		case EXPR_OPCODE_ADD:
		case EXPR_OPCODE_SUB:
		case EXPR_OPCODE_LSHIFT:
		case EXPR_OPCODE_RSHIFT:
		case EXPR_OPCODE_GE:
		case EXPR_OPCODE_GT:
		case EXPR_OPCODE_LE:
		case EXPR_OPCODE_LT:
		case EXPR_OPCODE_EQ:
		case EXPR_OPCODE_NE:
		case EXPR_OPCODE_BIT_AND:
		case EXPR_OPCODE_BIT_XOR:
		case EXPR_OPCODE_BIT_OR:
		case EXPR_OPCODE_LOGIC_AND:
		case EXPR_OPCODE_LOGIC_OR:
		case EXPR_OPCODE_COMPOUND:
		{
			value = ey_alloc_symbol_value(eng, expr->expr_type);
			if(!value)
			{
				ey_parser_set_error(eng, loc, "alloc %s expression value failed\n", ey_expr_type_name(opcode));
				return -1;
			}

			symbol = ey_alloc_symbol(eng, NULL, ey_parser_level(eng), SYMBOL_CONST, SYMBOL_STORAGE_NONE,
				0, expr->expr_type, value, NULL, loc);
			if(!symbol)
			{
				ey_parser_set_error(eng, loc, "alloc %s value symbol failed\n", ey_expr_type_name(opcode));
				return -1;
			}

			ey_expr_eval_const_value(eng, symbol, expr);
			expr->const_value = symbol;
			break;
		}
		case EXPR_OPCODE_DEREF:
		case EXPR_OPCODE_ADDRESS:
		case EXPR_OPCODE_ARRAY_INDEX:
		case EXPR_OPCODE_MEMBER_PTR:
		case EXPR_OPCODE_MEMBER:
		case EXPR_OPCODE_POST_INC:
		case EXPR_OPCODE_POST_DEC:
		case EXPR_OPCODE_PRE_DEC:
		case EXPR_OPCODE_PRE_INC:
		case EXPR_OPCODE_CONDITION:
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
		case EXPR_OPCODE_FUNCALL:
		default:
		{
			ey_parser_set_error(eng, &expr->location, "bad opcode %d\n", opcode);
			return -1;
		}
	}
	ey_parser_set_error(eng, &expr->location, "bad opcode %d\n", opcode);
	return -1;
}

/*init/finit*/
int ey_expr_init(ey_engine_t *eng)
{
	if(!expr_slab(eng))
	{
		char slab_name[64];
		snprintf(slab_name, sizeof(slab_name)-1, "%s eyoung expression", eng->name);
		slab_name[63] = '\0';
		expr_slab(eng) = engine_zinit(slab_name, sizeof(ey_expr_t));
		if(!expr_slab(eng))
		{
			engine_init_error("init expression slab failed\n");
			return -1;
		}
	}

	ey_true_expr(eng) = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, ey_true_symbol(eng), NULL);
	if(!ey_true_expr(eng))
	{
		engine_init_error("init true expression failed\n");
		return -1;
	}

	ey_false_expr(eng) = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, ey_false_symbol(eng), NULL);
	if(!ey_false_expr(eng))
	{
		engine_init_error("init false expression failed\n");
		return -1;
	}

	ey_null_expr(eng) = ey_alloc_symbol_expr(eng, EXPR_OPCODE_SYMBOL, ey_null_symbol(eng), NULL);
	if(!ey_null_expr(eng))
	{
		engine_init_error("init null expreesion failed\n");
		return -1;
	}
	return 0;
}

void ey_expr_finit(ey_engine_t *eng)
{
	if(expr_slab(eng))
		engine_zclear(expr_slab(eng));
	ey_expr_id(eng) = 0;
	ey_true_expr(eng) = NULL;
	ey_false_expr(eng) = NULL;
	ey_null_expr(eng) = NULL;
}

static ey_expr_t *ey_alloc_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, ey_expr_type_t type, ey_location_t *location)
{
	ey_expr_t *ret = NULL;
	if(expr_slab(eng))
	{
		ret = engine_zalloc(expr_slab(eng));
		if(ret)
		{
			memset(ret, 0, sizeof(*ret));
			ret->expr_id = ++ey_expr_id(eng);
			ret->opcode = opcode;
			ret->type = type;
			ret->location = location?*location:default_location;
		}
	}

	return ret;
}

ey_expr_t *ey_alloc_list_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_expr_list_t *head, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_LIST, location);
	if(ret)
	{
		TAILQ_INIT(&ret->list_expr.head);
		if(head)
			TAILQ_CONCAT(&ret->list_expr.head, head, link);

		/*do type check*/
		if(ey_expr_operand_check(eng, ret))
			return NULL;
		
		/*get expr check*/
		if(ey_expr_calc_type(eng, ret))
			return NULL;
		
		/*calculate expr value*/
		if(ey_expr_need_calc_value(eng, ret) && ey_expr_calc_value(eng, ret))
			return NULL;
	}

	return ret;
}

ey_expr_t *ey_alloc_list_init_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_expr_list_t *head, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_LIST, location);
	if(ret)
	{
		TAILQ_INIT(&ret->list_expr.head);
		if(head)
			TAILQ_CONCAT(&ret->list_expr.head, head, link);
	}

	return ret;
}

ey_expr_t *ey_alloc_symbol_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_symbol_t *symbol, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_SYMBOL, location);
	if(ret)
	{
		ret->symbol_expr.symbol = symbol;
		ret->expr_type = ey_type_copy(eng, symbol->type);

		if(!ret->expr_type)
		{
			ey_parser_set_error(eng, &symbol->location, "copy symbol type failed\n");
			return NULL;
		}

		if(symbol->class == SYMBOL_CONST)
			ret->const_value = symbol;

		if(!ey_symbol_check_flag(symbol, SYMBOL_FLAG_LHS))
			ret->expr_type->qualifier_class |= TYPE_QUALIFIER_CONST;
	}

	return ret;
}

ey_expr_t *ey_alloc_symbol_init_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_symbol_t *symbol, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_SYMBOL, location);
	if(ret)
		ret->symbol_expr.symbol = symbol;

	return ret;
}

ey_expr_t *ey_alloc_unary_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *operand, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_UNARY, location);
	if(ret)
	{
		ret->unary_expr.operand = operand;

		/*do type check*/
		if(ey_expr_operand_check(eng, ret))
			return NULL;
		
		/*get expr check*/
		if(ey_expr_calc_type(eng, ret))
			return NULL;
		
		/*calculate expr value*/
		if(ey_expr_need_calc_value(eng, ret) && ey_expr_calc_value(eng, ret))
			return NULL;
	}

	return ret;
}

ey_expr_t *ey_alloc_unary_init_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *operand, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_UNARY, location);
	if(ret)
		ret->unary_expr.operand = operand;

	return ret;
}

ey_expr_t *ey_alloc_binary_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *left, ey_expr_t *right, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_BINARY, location);
	if(ret)
	{
		ret->binary_expr.left = left;
		ret->binary_expr.right = right;

		/*do type check*/
		if(ey_expr_operand_check(eng, ret))
			return NULL;
		
		/*get expr check*/
		if(ey_expr_calc_type(eng, ret))
			return NULL;
		
		/*calculate expr value*/
		if(ey_expr_need_calc_value(eng, ret) && ey_expr_calc_value(eng, ret))
			return NULL;
	}

	return ret;
}

ey_expr_t *ey_alloc_binary_init_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *left, ey_expr_t *right, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_BINARY, location);
	if(ret)
	{
		ret->binary_expr.left = left;
		ret->binary_expr.right = right;
	}

	return ret;
}

ey_expr_t *ey_alloc_condition_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *condition, ey_expr_t *left, ey_expr_t *right, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_CONDITION, location);
	if(ret)
	{
		ret->condition_expr.condition = condition;
		ret->condition_expr.left = left;
		ret->condition_expr.right = right;

		/*do type check*/
		if(ey_expr_operand_check(eng, ret))
			return NULL;
		
		/*get expr check*/
		if(ey_expr_calc_type(eng, ret))
			return NULL;
		
		/*calculate expr value*/
		if(ey_expr_need_calc_value(eng, ret) && ey_expr_calc_value(eng, ret))
			return NULL;
	}

	return ret;
}

ey_expr_t *ey_alloc_condition_init_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *condition, ey_expr_t *left, ey_expr_t *right, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_CONDITION, location);
	if(ret)
	{
		ret->condition_expr.condition = condition;
		ret->condition_expr.left = left;
		ret->condition_expr.right = right;
	}

	return ret;
}

ey_expr_t *ey_alloc_funcall_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *function, ey_expr_list_t *arg_list, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_FUNCALL, location);
	if(ret)
	{
		ret->funcall_expr.function = function;
		TAILQ_INIT(&ret->funcall_expr.arg_list);
		if(arg_list)
			TAILQ_CONCAT(&ret->funcall_expr.arg_list, arg_list, link);

		/*do type check*/
		if(ey_expr_operand_check(eng, ret))
			return NULL;
		
		/*get expr check*/
		if(ey_expr_calc_type(eng, ret))
			return NULL;
		
		/*calculate expr value*/
		if(ey_expr_need_calc_value(eng, ret) && ey_expr_calc_value(eng, ret))
			return NULL;
	}

	return ret;
}

ey_expr_t *ey_alloc_funcall_init_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *function, ey_expr_list_t *arg_list, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_FUNCALL, location);
	if(ret)
	{
		ret->funcall_expr.function = function;
		TAILQ_INIT(&ret->funcall_expr.arg_list);
		if(arg_list)
			TAILQ_CONCAT(&ret->funcall_expr.arg_list, arg_list, link);
	}

	return ret;
}

ey_expr_t *ey_alloc_member_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *su, ey_symbol_t *member, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_MEMBER, location);
	if(ret)
	{
		ret->member_expr.su = su;
		ret->member_expr.member = member;

		/*do type check*/
		if(ey_expr_operand_check(eng, ret))
			return NULL;
		
		/*get expr check*/
		if(ey_expr_calc_type(eng, ret))
			return NULL;
		
		/*calculate expr value*/
		if(ey_expr_need_calc_value(eng, ret) && ey_expr_calc_value(eng, ret))
			return NULL;
	}

	return ret;
}

ey_expr_t *ey_alloc_member_init_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_expr_t *su, ey_symbol_t *member, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_MEMBER, location);
	if(ret)
	{
		ret->member_expr.su = su;
		ret->member_expr.member = member;
	}

	return ret;
}

ey_expr_t *ey_alloc_cast_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, ey_type_t *type, ey_expr_t *expr, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_CAST, location);
	if(ret)
	{
		ret->cast_expr.type = type;
		ret->cast_expr.expr = expr;

		/*do type check*/
		if(ey_expr_operand_check(eng, ret))
			return NULL;
		
		/*get expr check*/
		if(ey_expr_calc_type(eng, ret))
			return NULL;
		
		/*calculate expr value*/
		if(ey_expr_need_calc_value(eng, ret) && ey_expr_calc_value(eng, ret))
			return NULL;
	}

	return ret;
}

ey_expr_t *ey_alloc_cast_init_expr(ey_engine_t *eng, ey_expr_opcode_t opcode, 
	ey_type_t *type, ey_expr_t *expr, ey_location_t *location)
{
	ey_expr_t *ret = ey_alloc_expr(eng, opcode, EXPR_TYPE_CAST, location);
	if(ret)
	{
		ret->cast_expr.type = type;
		ret->cast_expr.expr = expr;
	}

	return ret;
}


int ey_eval_const_expr_value(ey_engine_t *eng, ey_expr_t *expr)
{
	/*TODO*/
	return 0;
}

int ey_expr_is_meanless_const_value(ey_engine_t *eng, ey_expr_t *expr)
{
	/*TODO*/
	return 0;
}

int ey_expr_is_true(ey_engine_t *eng, ey_expr_t *expr)
{
	/*TODO*/
	return 0;
}

void ey_free_expr(ey_engine_t *eng, ey_expr_t *expr)
{
	if(!expr)
		return;
	
	switch(expr->type)
	{
		case EXPR_TYPE_LIST:
			ey_free_expr_list(eng, &expr->list_expr.head);
			break;
		case EXPR_TYPE_SYMBOL:
			ey_free_symbol(eng, expr->symbol_expr.symbol);
			break;
		case EXPR_TYPE_UNARY:
			ey_free_expr(eng, expr->unary_expr.operand);
			break;
		case EXPR_TYPE_BINARY:
			ey_free_expr(eng, expr->binary_expr.left);
			ey_free_expr(eng, expr->binary_expr.right);
			break;
		case EXPR_TYPE_FUNCALL:
			ey_free_expr(eng, expr->funcall_expr.function);
			ey_free_expr_list(eng, &expr->funcall_expr.arg_list);
			break;
		case EXPR_TYPE_MEMBER:
			ey_free_expr(eng, expr->member_expr.su);
			ey_free_symbol(eng, expr->member_expr.member);
			break;
		case EXPR_TYPE_CONDITION:
			ey_free_expr(eng, expr->condition_expr.condition);
			ey_free_expr(eng, expr->condition_expr.left);
			ey_free_expr(eng, expr->condition_expr.right);
			break;
		case EXPR_TYPE_CAST:
			ey_free_type(eng, expr->cast_expr.type);
			ey_free_expr(eng, expr->cast_expr.expr);
			break;
		default:
			*(int*)0 = 0;
			break;
	}
	engine_zfree(expr_slab(eng), expr);
}

void ey_free_expr_list(ey_engine_t *eng, ey_expr_list_t *expr_list)
{
	if(!expr_list)
		return;
	
	ey_expr_t *expr = NULL, *next = NULL;
	TAILQ_FOREACH_SAFE(expr, expr_list, link, next)
		ey_free_expr(eng, expr);
}

void ey_expr_print(ey_engine_t *eng, ey_expr_t *expr, int tab)
{
	/*TODO*/
	return;
}

int ey_expr_is_const_value(ey_engine_t *eng, ey_expr_t *expr)
{
	if(!expr->const_value || !expr->const_value->value)
		return 0;
	return 1;
}

int ey_expr_is_lhs(ey_engine_t *eng, ey_expr_t *expr)
{
	if(expr && expr->expr_type)
	{
		if(expr->const_value || (expr->expr_type->qualifier_class & TYPE_QUALIFIER_CONST))
			return 0;
		
		return 1;
	}

	return 0;
}

ey_symbol_t *ey_expr_eval_const_value(ey_engine_t *eng, ey_symbol_t *symbol, ey_expr_t *expr)
{
	/*TODO*/
	return symbol;
}
