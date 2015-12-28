#ifndef ENG_TYPE_H
#define ENG_TYPE_H 1

#include "eng_location.h"

typedef enum ey_type_type
{
	/*basic type*/
	TYPE_SIGNED_CHAR,				/*	(signed) char			*/
	TYPE_UNSIGNED_CHAR,				/*	unsigned char			*/
	TYPE_SIGNED_SHORT,				/*	(signed) short			*/
	TYPE_UNSIGNED_SHORT,			/*	unsigned short			*/
	TYPE_SIGNED_INT,				/*	(signed) int			*/
	TYPE_UNSIGNED_INT,				/*	unsigned int			*/
	TYPE_SIGNED_LONG,				/*	(signed) long			*/
	TYPE_UNSIGNED_LONG,				/*	unsigned long			*/
	TYPE_SIGNED_LONG_LONG,			/*	(signed) long long		*/
	TYPE_UNSIGNED_LONG_LONG,		/*	unsigned long long		*/
	TYPE_ENUM_CONST,				/*	enum const				*/
	TYPE_FLOAT,						/*	float					*/
	TYPE_DOUBLE,					/*	(long) double			*/
	
	/*un-supported type*/
	TYPE_COMPLEX,					/*	complex					*/
	TYPE_IMAGINARY,					/*	imaginary				*/
	TYPE_BOOL,						/*	bool					*/
	TYPE_LONG_DOUBLE,				/*	long double				*/

	/*temp type*/
	TYPE_VOID,						/*	void					*/
	TYPE_PESUDO,					/*	pesudo type				*/
	TYPE_SIGNED,					/*	signed					*/
	TYPE_UNSIGNED,					/*	unsigned				*/
	TYPE_CHAR,						/*	char					*/
	TYPE_SHORT,						/*	short					*/
	TYPE_SHORT_INT,					/*	short int				*/
	TYPE_SIGNED_SHORT_INT,			/*	signed short int		*/
	TYPE_UNSIGNED_SHORT_INT,		/*	unsigned short int		*/
	TYPE_INT,						/*	int						*/
	TYPE_LONG,						/*	long					*/
	TYPE_LONG_INT,					/*	long int				*/
	TYPE_SIGNED_LONG_INT,			/*	signed long int			*/
	TYPE_UNSIGNED_LONG_INT,			/*	unsigned long int		*/
	TYPE_LONG_LONG,					/*	long long				*/
	TYPE_LONG_LONG_INT,				/*	long long int			*/
	TYPE_SIGNED_LONG_LONG_INT,		/*	signed long long int	*/
	TYPE_UNSIGNED_LONG_LONG_INT,	/*	unsigned long long int	*/

	/*struct/union/enum*/
	TYPE_STRUCT,					/*	struct define			*/
	TYPE_STRUCT_TAG,				/*	struct name				*/
	TYPE_UNION,						/*	union define			*/
	TYPE_UNION_TAG,					/*	union name				*/
	TYPE_ENUM,						/*	enum define				*/
	TYPE_ENUM_TAG,					/*	enum name				*/
	
	/*complex*/
	TYPE_FUNCTION,					/*	function				*/
	TYPE_ARRAY,						/*	array					*/
	TYPE_POINTER,					/*	pointer					*/
	TYPE_NAME,						/*	typedef name			*/
}ey_type_type_t;

static inline const char* ey_type_type_name(ey_type_type_t c)
{
	switch(c)
	{
		case TYPE_SIGNED_CHAR:
			return "signed char";
		case TYPE_UNSIGNED_CHAR:
			return "unsigned char";
		case TYPE_SIGNED_SHORT:
			return "signed short";
		case TYPE_UNSIGNED_SHORT:
			return "unsigned short";
		case TYPE_SIGNED_INT:
			return "signed int";
		case TYPE_UNSIGNED_INT:
			return "unsigned int";
		case TYPE_SIGNED_LONG:
			return "signed long";
		case TYPE_UNSIGNED_LONG:
			return "unsigned long";
		case TYPE_SIGNED_LONG_LONG:
			return "signed long long";
		case TYPE_UNSIGNED_LONG_LONG:
			return "unsigned long long";
		case TYPE_UNSIGNED:
			return "unsigned";
		case TYPE_SIGNED:
			return "signed";
		case TYPE_CHAR:
			return "char";
		case TYPE_SHORT:
			return "short";
		case TYPE_SHORT_INT:
			return "short int";
		case TYPE_SIGNED_SHORT_INT:
			return "signed short int";
		case TYPE_UNSIGNED_SHORT_INT:
			return "unsigned short int";
		case TYPE_INT:
			return "int";
		case TYPE_LONG:
			return "long";
		case TYPE_LONG_INT:
			return "long int";
		case TYPE_SIGNED_LONG_INT:
			return "signed long int";
		case TYPE_UNSIGNED_LONG_INT:
			return "unsigned long int";
		case TYPE_LONG_LONG:
			return "long long";
		case TYPE_LONG_LONG_INT:
			return "long long int";
		case TYPE_SIGNED_LONG_LONG_INT:
			return "signed long long int";
		case TYPE_UNSIGNED_LONG_LONG_INT:
			return "unsigned long long int";
		case TYPE_ENUM_CONST:
			return "<enum const>";
		case TYPE_FLOAT:
			return "float";
		case TYPE_DOUBLE:
			return "double";
		case TYPE_COMPLEX:
			return "_Complex";
		case TYPE_IMAGINARY:
			return "_Imaginary";
		case TYPE_BOOL:
			return "_Bool";
		case TYPE_STRUCT:
			return "struct";
		case TYPE_STRUCT_TAG:
			return "<struct name>";
		case TYPE_UNION:
			return "union";
		case TYPE_UNION_TAG:
			return "<union name>";
		case TYPE_ENUM:
			return "enum";
		case TYPE_ENUM_TAG:
			return "<enum name>";
		case TYPE_FUNCTION:
			return "function";
		case TYPE_ARRAY:
			return "array";
		case TYPE_POINTER:
			return "pointer";
		case TYPE_NAME:
			return "typedef name";
		default:
			*(int*)0 = 0;
			return "Unknown";
	}
}

static inline int ey_type_is_integer(ey_type_type_t c)
{
	switch(c)
	{
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
		case TYPE_ENUM_CONST:
			return 1;
		default:
			return 0;
	}
}

static inline int ey_type_is_signed_integer(ey_type_type_t c)
{
	switch(c)
	{
		case TYPE_SIGNED_CHAR:
		case TYPE_SIGNED_SHORT:
		case TYPE_SIGNED_INT:
		case TYPE_SIGNED_LONG:
		case TYPE_SIGNED_LONG_LONG:
		case TYPE_ENUM_CONST:
			return 1;
		default:
			return 0;
	}
}

static inline int ey_type_is_unsigned_integer(ey_type_type_t c)
{
	switch(c)
	{
		case TYPE_UNSIGNED_CHAR:
		case TYPE_UNSIGNED_SHORT:
		case TYPE_UNSIGNED_INT:
		case TYPE_UNSIGNED_LONG:
		case TYPE_UNSIGNED_LONG_LONG:
			return 1;
		default:
			return 0;
	}
}

static inline int ey_type_is_real(ey_type_type_t c)
{
	switch(c)
	{
		case TYPE_FLOAT:
		case TYPE_DOUBLE:
			return 1;
		default:
			return 0;
	}
}

static inline int ey_type_is_complex(ey_type_type_t c)
{
	switch(c)
	{
		case TYPE_COMPLEX:
		case TYPE_IMAGINARY:
			return 1;
		default:
			return 0;
	}
}

static inline int ey_type_is_arithmetic(ey_type_type_t c)
{
	return ey_type_is_integer(c) || ey_type_is_real(c);
}

static inline int ey_type_is_pointer(ey_type_type_t c)
{
	switch(c)
	{
		case TYPE_POINTER:
			return 1;
		default:
			return 0;
	}
}

static inline int ey_type_is_pointer_array(ey_type_type_t c)
{
	switch(c)
	{
		case TYPE_POINTER:
		case TYPE_ARRAY:
			return 1;
		default:
			return 0;
	}
}

static inline int ey_type_is_scalar(ey_type_type_t c)
{
	return (ey_type_is_arithmetic(c) || ey_type_is_pointer(c));
}

static inline int ey_type_is_aggregate(ey_type_type_t c)
{
	switch(c)
	{
		case TYPE_ARRAY:
		case TYPE_STRUCT_TAG:
		case TYPE_UNION_TAG:
			return 1;
		default:
			return 0;
	}
}

static inline int ey_type_is_su(ey_type_type_t c)
{
	switch(c)
	{
		case TYPE_STRUCT_TAG:
		case TYPE_UNION_TAG:
			return 1;
		default:
			return 0;
	}
}

static inline int ey_type_is_tag(ey_type_type_t c)
{
	switch(c)
	{
		case TYPE_STRUCT_TAG:
		case TYPE_UNION_TAG:
		case TYPE_ENUM_TAG:
			return 1;
		default: 
			return 0;
	}
}

static inline int ey_type_is_derived(ey_type_type_t c)
{
	switch(c)
	{
		case TYPE_ARRAY:
		case TYPE_FUNCTION:
		case TYPE_POINTER:
			return 1;
		default:
			return 0;
	}
}

static inline int ey_type_is_void(ey_type_type_t c)
{
	switch(c)
	{
		case TYPE_VOID:
			return 1;
		default:
			return 0;
	}
}

typedef enum ey_type_qualifier_class
{
	TYPE_QUALIFIER_NORMAL		=0x00000000,
	TYPE_QUALIFIER_CONST		=0x00000001,
	TYPE_QUALIFIER_RESTRICT		=0x00000002,
	TYPE_QUALIFIER_VOLATILE		=0x00000004,

	/*FOR TYPE_PESUDO*/
	TYPE_QUALIFIER_TYPEDEF		=0x00000008,
	TYPE_QUALIFIER_EXTERN		=0x00000010,
	TYPE_QUALIFIER_STATIC		=0x00000020,
	TYPE_QUALIFIER_AUTO			=0x00000040,
	TYPE_QUALIFIER_REGISTER		=0x00000080,
	#define EY_STORAGE_CLASS_MASK		\
		(TYPE_QUALIFIER_TYPEDEF			\
		|TYPE_QUALIFIER_EXTERN			\
		|TYPE_QUALIFIER_STATIC			\
		|TYPE_QUALIFIER_AUTO			\
		|TYPE_QUALIFIER_REGISTER)
	TYPE_QUALIFIER_INLINE		=0x00000100,
}ey_type_qualifier_class_t;

static inline const char *ey_type_qualifier_class_name(int t)
{
	switch(t&(TYPE_QUALIFIER_CONST|TYPE_QUALIFIER_RESTRICT|TYPE_QUALIFIER_VOLATILE))
	{
		case TYPE_QUALIFIER_NORMAL:
			return "NORMAL";
		case TYPE_QUALIFIER_CONST:
			return "CONST";
		case TYPE_QUALIFIER_RESTRICT:
			return "RESTRICT";
		case TYPE_QUALIFIER_VOLATILE:
			return "VOLATILE";
		case TYPE_QUALIFIER_CONST|TYPE_QUALIFIER_RESTRICT:
			return "CONST RESTRICT";
		case TYPE_QUALIFIER_CONST|TYPE_QUALIFIER_VOLATILE:
			return "CONST VOLATILE";
		case TYPE_QUALIFIER_RESTRICT|TYPE_QUALIFIER_VOLATILE:
			return "RESTRICT VOLATILE";
		case TYPE_QUALIFIER_CONST|TYPE_QUALIFIER_RESTRICT|TYPE_QUALIFIER_VOLATILE:
			return "CONST RESTRICT VOLATILE";
		default:
			*(int*)0 = 0;
			return "Unknown";
	}
}

typedef enum ey_type_class
{
	TYPE_CLASS_SIMPLE,
	TYPE_CLASS_ARRAY,
	TYPE_CLASS_POINTER,
	TYPE_CLASS_ENUM,
	TYPE_CLASS_FUNCTION,
	TYPE_CLASS_SU,
	TYPE_CLASS_TAG,
	TYPE_CLASS_TYPEDEF
}ey_type_class_t;

static inline const char* ey_type_class_name(ey_type_class_t class)
{
	switch(class)
	{
		case TYPE_CLASS_SIMPLE:
			return "simple";
		case TYPE_CLASS_ARRAY:
			return "array";
		case TYPE_CLASS_POINTER:
			return "pointer";
		case TYPE_CLASS_ENUM:
			return "enum";
		case TYPE_CLASS_FUNCTION:
			return "function";
		case TYPE_CLASS_SU:
			return "struct-union";
		case TYPE_CLASS_TAG:
			return "tag";
		case TYPE_CLASS_TYPEDEF:
			return "typedef";
		default:
			*(int*)0 = 0;
			return "Unknown";
	}
}

#include "eng_sym.h"
struct ey_type;
typedef struct array_type
{
	struct ey_type *base_type;		/*base type*/
	unsigned int count;
	unsigned int undefined;
}array_type_t;

typedef struct enum_type
{
	ey_symbol_t *tag;
	ey_symbol_list_t enum_const_list;
}enum_type_t;

typedef struct function_type
{
	int var_args;
	struct ey_type *return_type;
	ey_symbol_list_t arg_list;
}function_type_t;

typedef struct pointer_type
{
	struct ey_type *deref_type;
}pointer_type_t;

typedef struct su_type
{
	ey_symbol_t *tag;
	ey_member_list_t member_list;
}su_type_t;

typedef struct tag_type
{
	struct ey_type *descriptor_type;
}tag_type_t;

typedef struct typedef_type
{
	struct ey_type *descriptor_type;
}typedef_type_t;

typedef struct ey_type
{
	ey_location_t location;
	ey_type_type_t type;
	ey_type_class_t class;
	int qualifier_class;
	unsigned int size;
	unsigned int alignment;
	union
	{
		array_type_t array_type;
		pointer_type_t pointer_type;
		enum_type_t enum_type;
		function_type_t function_type;
		su_type_t su_type;
		tag_type_t tag_type;
		typedef_type_t typedef_type;
	};
}ey_type_t;

/*predefined type*/
typedef struct ey_type_const
{
	ey_type_t *char_type;
	ey_type_t *uchar_type;
	ey_type_t *short_type;
	ey_type_t *ushort_type;
	ey_type_t *int_type;
	ey_type_t *uint_type;
	ey_type_t *long_type;
	ey_type_t *ulong_type;
	ey_type_t *float_type;
	ey_type_t *double_type;
	ey_type_t *void_type;
	ey_type_t *pvoid_type;
	ey_type_t *pchar_type;
	ey_type_t *const_char_type;
	ey_type_t *enum_const_type;
}ey_type_const_t;

static inline int ey_type_is_lhs(ey_type_t *type)
{
	return ((type->type!=TYPE_ENUM_CONST) && !(type->qualifier_class & TYPE_QUALIFIER_CONST));
}

/*init/finit*/
struct ey_engine;
extern int ey_type_init(struct ey_engine *eng);
extern void ey_type_finit(struct ey_engine *eng);

/*free*/
extern void ey_free_type(struct ey_engine *eng, ey_type_t *type);

/*alloc*/
extern ey_type_t *ey_alloc_simple_type(struct ey_engine *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location);
extern ey_type_t *ey_alloc_array_type(struct ey_engine *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_type_t *base_type, size_t count, int undefined);
extern ey_type_t *ey_alloc_enum_type(struct ey_engine *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_symbol_t *tag, ey_symbol_list_t *const_list);
extern ey_type_t *ey_alloc_function_type(struct ey_engine *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_type_t *return_type, int var_args, ey_symbol_list_t *arg_list);
extern ey_type_t *ey_alloc_pointer_type(struct ey_engine *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_type_t *deref_type);
extern ey_type_t *ey_alloc_su_type(struct ey_engine *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_symbol_t *tag, ey_member_list_t *member_list);
extern ey_type_t *ey_alloc_tag_type(struct ey_engine *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_type_t *descriptor);
extern ey_type_t *ey_alloc_typedef_type(struct ey_engine *eng, ey_type_type_t type, int qualifier_class, 
	size_t size, size_t alignment, ey_location_t *location, ey_type_t *descriptor);

/*type check*/
extern int ey_type_assignment_compatible(struct ey_engine *eng, ey_type_t *dst, ey_type_t *src);
extern int ey_type_pointer_compatible(struct ey_engine *eng, ey_type_t *dst, ey_type_t *src, int compare_qulifier);
extern int ey_type_function_equal(struct ey_engine *eng, ey_type_t *dst, ey_type_t *src, int compare_qulifier);
extern int ey_type_tag_equal(struct ey_engine *eng, ey_type_t *dst, ey_type_t *src, int compare_qulifier);
extern int ey_type_array_equal(struct ey_engine *eng, ey_type_t *dst, ey_type_t *src, int compare_qulifier, int compare_count);
extern int ey_type_pointer_equal(struct ey_engine *eng, ey_type_t *dst, ey_type_t *src, int compare_qulifier);
extern int ey_type_equal(struct ey_engine *eng, ey_type_t *dst, ey_type_t *src, int compare_qulifier, int compare_count);
extern int ey_type_is_declared(struct ey_engine *eng, ey_type_t *type);

/*two type merge*/
extern ey_type_t *ey_type_merge(struct ey_engine *eng, ey_type_t *dst, ey_type_t *src);
extern ey_type_t *ey_type_normalize(struct ey_engine *eng, ey_type_t *dst);
extern ey_type_t *ey_type_combine(struct ey_engine *eng, ey_type_t *dst, ey_type_t *src);
extern ey_type_t *ey_type_promotion(struct ey_engine *eng, ey_type_t *dst, ey_type_t *src);
extern ey_type_t *ey_type_integer_promotion(struct ey_engine *eng, ey_type_t *dst);
extern int ey_type_has_bitwise_member(struct ey_engine *eng, ey_type_t *su_type);
extern ey_type_t *ey_type_set_struct_type(struct ey_engine *eng, ey_type_t *su_type);
extern ey_type_t *ey_type_set_union_type(struct ey_engine *eng, ey_type_t *su_type);
extern ey_type_t *ey_type_copy(struct ey_engine *eng, ey_type_t *type);
extern ey_type_t *ey_type_array2pointer(struct ey_engine *eng, ey_type_t *type);

/*print*/
extern void ey_type_print(struct ey_engine *eng, ey_type_t *type, int tab);

struct ey_member;
struct ey_symbol;
struct ey_member *ey_type_find_member(struct ey_engine *eng, ey_type_t *su_tag, struct ey_symbol *member_symbol);
#endif
