/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 2 "yacc_sql.y"


  /**
    * @file yacc_sql.y
    * @brief SQL语法解析器的yacc文件
    * 
    * 注意：vscode插件对.y静态语法分析文件的支持不太好，编程时要谨慎
    *
    * @version 1.0
    */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "common/log/log.h"
#include "common/lang/string.h"
#include "sql/parser/parse_defs.h"
#include "sql/parser/yacc_sql.hpp"
#include "sql/parser/lex_sql.h"

using namespace std;

string token_name(const char *sql_string, YYLTYPE *llocp)
{
  return string(sql_string + llocp->first_column, llocp->last_column - llocp->first_column + 1);
}

int yyerror(YYLTYPE *llocp, const char *sql_string, ParsedSqlResult *sql_result, yyscan_t scanner, const char *msg)
{
  std::unique_ptr<ParsedSqlNode> error_sql_node = std::make_unique<ParsedSqlNode>(SCF_ERROR);
  error_sql_node->error.error_msg = msg;
  error_sql_node->error.line = llocp->first_line;
  error_sql_node->error.column = llocp->first_column;
  sql_result->add_sql_node(std::move(error_sql_node));
  return 0;
}


#line 112 "yacc_sql.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "yacc_sql.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SEMICOLON = 3,                  /* SEMICOLON  */
  YYSYMBOL_CREATE = 4,                     /* CREATE  */
  YYSYMBOL_DROP = 5,                       /* DROP  */
  YYSYMBOL_TABLE = 6,                      /* TABLE  */
  YYSYMBOL_TABLES = 7,                     /* TABLES  */
  YYSYMBOL_INDEX = 8,                      /* INDEX  */
  YYSYMBOL_CALC = 9,                       /* CALC  */
  YYSYMBOL_SELECT = 10,                    /* SELECT  */
  YYSYMBOL_DESC = 11,                      /* DESC  */
  YYSYMBOL_ASC = 12,                       /* ASC  */
  YYSYMBOL_SHOW = 13,                      /* SHOW  */
  YYSYMBOL_SYNC = 14,                      /* SYNC  */
  YYSYMBOL_INSERT = 15,                    /* INSERT  */
  YYSYMBOL_DELETE = 16,                    /* DELETE  */
  YYSYMBOL_UPDATE = 17,                    /* UPDATE  */
  YYSYMBOL_LBRACE = 18,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 19,                    /* RBRACE  */
  YYSYMBOL_COMMA = 20,                     /* COMMA  */
  YYSYMBOL_TRX_BEGIN = 21,                 /* TRX_BEGIN  */
  YYSYMBOL_TRX_COMMIT = 22,                /* TRX_COMMIT  */
  YYSYMBOL_TRX_ROLLBACK = 23,              /* TRX_ROLLBACK  */
  YYSYMBOL_INT_T = 24,                     /* INT_T  */
  YYSYMBOL_STRING_T = 25,                  /* STRING_T  */
  YYSYMBOL_FLOAT_T = 26,                   /* FLOAT_T  */
  YYSYMBOL_DATE_T = 27,                    /* DATE_T  */
  YYSYMBOL_TEXT_T = 28,                    /* TEXT_T  */
  YYSYMBOL_THE_NULL = 29,                  /* THE_NULL  */
  YYSYMBOL_HELP = 30,                      /* HELP  */
  YYSYMBOL_EXIT = 31,                      /* EXIT  */
  YYSYMBOL_DOT = 32,                       /* DOT  */
  YYSYMBOL_INTO = 33,                      /* INTO  */
  YYSYMBOL_VALUES = 34,                    /* VALUES  */
  YYSYMBOL_FROM = 35,                      /* FROM  */
  YYSYMBOL_WHERE = 36,                     /* WHERE  */
  YYSYMBOL_SET = 37,                       /* SET  */
  YYSYMBOL_ON = 38,                        /* ON  */
  YYSYMBOL_LOAD = 39,                      /* LOAD  */
  YYSYMBOL_DATA = 40,                      /* DATA  */
  YYSYMBOL_INFILE = 41,                    /* INFILE  */
  YYSYMBOL_EXPLAIN = 42,                   /* EXPLAIN  */
  YYSYMBOL_GROUP = 43,                     /* GROUP  */
  YYSYMBOL_BY = 44,                        /* BY  */
  YYSYMBOL_ORDER = 45,                     /* ORDER  */
  YYSYMBOL_HAVING = 46,                    /* HAVING  */
  YYSYMBOL_AS = 47,                        /* AS  */
  YYSYMBOL_JOIN = 48,                      /* JOIN  */
  YYSYMBOL_EXISTS = 49,                    /* EXISTS  */
  YYSYMBOL_IN = 50,                        /* IN  */
  YYSYMBOL_INNER = 51,                     /* INNER  */
  YYSYMBOL_NUMBER = 52,                    /* NUMBER  */
  YYSYMBOL_FLOAT = 53,                     /* FLOAT  */
  YYSYMBOL_ID = 54,                        /* ID  */
  YYSYMBOL_SSS = 55,                       /* SSS  */
  YYSYMBOL_IS = 56,                        /* IS  */
  YYSYMBOL_LIKE = 57,                      /* LIKE  */
  YYSYMBOL_OR = 58,                        /* OR  */
  YYSYMBOL_AND = 59,                       /* AND  */
  YYSYMBOL_NOT = 60,                       /* NOT  */
  YYSYMBOL_LT = 61,                        /* LT  */
  YYSYMBOL_GT = 62,                        /* GT  */
  YYSYMBOL_LE = 63,                        /* LE  */
  YYSYMBOL_GE = 64,                        /* GE  */
  YYSYMBOL_EQ = 65,                        /* EQ  */
  YYSYMBOL_NE = 66,                        /* NE  */
  YYSYMBOL_ADD = 67,                       /* ADD  */
  YYSYMBOL_SUB = 68,                       /* SUB  */
  YYSYMBOL_STAR = 69,                      /* STAR  */
  YYSYMBOL_DIV = 70,                       /* DIV  */
  YYSYMBOL_UMINUS = 71,                    /* UMINUS  */
  YYSYMBOL_YYACCEPT = 72,                  /* $accept  */
  YYSYMBOL_commands = 73,                  /* commands  */
  YYSYMBOL_command_wrapper = 74,           /* command_wrapper  */
  YYSYMBOL_exit_stmt = 75,                 /* exit_stmt  */
  YYSYMBOL_help_stmt = 76,                 /* help_stmt  */
  YYSYMBOL_sync_stmt = 77,                 /* sync_stmt  */
  YYSYMBOL_begin_stmt = 78,                /* begin_stmt  */
  YYSYMBOL_commit_stmt = 79,               /* commit_stmt  */
  YYSYMBOL_rollback_stmt = 80,             /* rollback_stmt  */
  YYSYMBOL_drop_table_stmt = 81,           /* drop_table_stmt  */
  YYSYMBOL_show_tables_stmt = 82,          /* show_tables_stmt  */
  YYSYMBOL_desc_table_stmt = 83,           /* desc_table_stmt  */
  YYSYMBOL_create_index_stmt = 84,         /* create_index_stmt  */
  YYSYMBOL_index_col_list = 85,            /* index_col_list  */
  YYSYMBOL_drop_index_stmt = 86,           /* drop_index_stmt  */
  YYSYMBOL_create_table_stmt = 87,         /* create_table_stmt  */
  YYSYMBOL_attr_def_list = 88,             /* attr_def_list  */
  YYSYMBOL_attr_def = 89,                  /* attr_def  */
  YYSYMBOL_number = 90,                    /* number  */
  YYSYMBOL_type = 91,                      /* type  */
  YYSYMBOL_insert_stmt = 92,               /* insert_stmt  */
  YYSYMBOL_value_list = 93,                /* value_list  */
  YYSYMBOL_value = 94,                     /* value  */
  YYSYMBOL_delete_stmt = 95,               /* delete_stmt  */
  YYSYMBOL_update_stmt = 96,               /* update_stmt  */
  YYSYMBOL_update_asgn_factor = 97,        /* update_asgn_factor  */
  YYSYMBOL_update_asgn_list = 98,          /* update_asgn_list  */
  YYSYMBOL_select_stmt = 99,               /* select_stmt  */
  YYSYMBOL_select_with_parenthesis = 100,  /* select_with_parenthesis  */
  YYSYMBOL_expression_list = 101,          /* expression_list  */
  YYSYMBOL_expression = 102,               /* expression  */
  YYSYMBOL_rel_attr = 103,                 /* rel_attr  */
  YYSYMBOL_opt_where = 104,                /* opt_where  */
  YYSYMBOL_table_factor = 105,             /* table_factor  */
  YYSYMBOL_opt_table_refs = 106,           /* opt_table_refs  */
  YYSYMBOL_table_ref = 107,                /* table_ref  */
  YYSYMBOL_opt_join_condition = 108,       /* opt_join_condition  */
  YYSYMBOL_table_ref_list = 109,           /* table_ref_list  */
  YYSYMBOL_expression_with_order = 110,    /* expression_with_order  */
  YYSYMBOL_expression_with_order_list = 111, /* expression_with_order_list  */
  YYSYMBOL_query_expression = 112,         /* query_expression  */
  YYSYMBOL_query_expression_list = 113,    /* query_expression_list  */
  YYSYMBOL_opt_order_by = 114,             /* opt_order_by  */
  YYSYMBOL_opt_group_by = 115,             /* opt_group_by  */
  YYSYMBOL_opt_order_type = 116,           /* opt_order_type  */
  YYSYMBOL_opt_having = 117,               /* opt_having  */
  YYSYMBOL_load_data_stmt = 118,           /* load_data_stmt  */
  YYSYMBOL_explain_stmt = 119,             /* explain_stmt  */
  YYSYMBOL_set_variable_stmt = 120,        /* set_variable_stmt  */
  YYSYMBOL_opt_inner = 121,                /* opt_inner  */
  YYSYMBOL_opt_semicolon = 122,            /* opt_semicolon  */
  YYSYMBOL_opt_not = 123,                  /* opt_not  */
  YYSYMBOL_opt_alias = 124,                /* opt_alias  */
  YYSYMBOL_opt_as = 125,                   /* opt_as  */
  YYSYMBOL_opt_nullable = 126              /* opt_nullable  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  69
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   296

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  72
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  55
/* YYNRULES -- Number of rules.  */
#define YYNRULES  133
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  236

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   326


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   207,   207,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   237,   243,   248,   254,   260,   266,   272,   279,
     285,   293,   306,   311,   322,   333,   354,   357,   370,   379,
     391,   395,   396,   397,   398,   399,   403,   420,   423,   435,
     439,   443,   449,   456,   468,   481,   491,   496,   509,   535,
     545,   552,   557,   569,   578,   587,   596,   605,   610,   621,
     630,   639,   648,   657,   666,   675,   684,   693,   696,   714,
     721,   728,   744,   759,   773,   781,   789,   800,   806,   813,
     818,   829,   832,   837,   844,   853,   856,   861,   869,   885,
     888,   894,   899,   909,   918,   923,   933,   945,   950,   961,
     964,   971,   974,   981,   984,   988,   995,   998,  1004,  1017,
    1025,  1035,  1036,  1039,  1040,  1044,  1045,  1049,  1050,  1055,
    1056,  1059,  1060,  1061
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "SEMICOLON", "CREATE",
  "DROP", "TABLE", "TABLES", "INDEX", "CALC", "SELECT", "DESC", "ASC",
  "SHOW", "SYNC", "INSERT", "DELETE", "UPDATE", "LBRACE", "RBRACE",
  "COMMA", "TRX_BEGIN", "TRX_COMMIT", "TRX_ROLLBACK", "INT_T", "STRING_T",
  "FLOAT_T", "DATE_T", "TEXT_T", "THE_NULL", "HELP", "EXIT", "DOT", "INTO",
  "VALUES", "FROM", "WHERE", "SET", "ON", "LOAD", "DATA", "INFILE",
  "EXPLAIN", "GROUP", "BY", "ORDER", "HAVING", "AS", "JOIN", "EXISTS",
  "IN", "INNER", "NUMBER", "FLOAT", "ID", "SSS", "IS", "LIKE", "OR", "AND",
  "NOT", "LT", "GT", "LE", "GE", "EQ", "NE", "ADD", "SUB", "STAR", "DIV",
  "UMINUS", "$accept", "commands", "command_wrapper", "exit_stmt",
  "help_stmt", "sync_stmt", "begin_stmt", "commit_stmt", "rollback_stmt",
  "drop_table_stmt", "show_tables_stmt", "desc_table_stmt",
  "create_index_stmt", "index_col_list", "drop_index_stmt",
  "create_table_stmt", "attr_def_list", "attr_def", "number", "type",
  "insert_stmt", "value_list", "value", "delete_stmt", "update_stmt",
  "update_asgn_factor", "update_asgn_list", "select_stmt",
  "select_with_parenthesis", "expression_list", "expression", "rel_attr",
  "opt_where", "table_factor", "opt_table_refs", "table_ref",
  "opt_join_condition", "table_ref_list", "expression_with_order",
  "expression_with_order_list", "query_expression",
  "query_expression_list", "opt_order_by", "opt_group_by",
  "opt_order_type", "opt_having", "load_data_stmt", "explain_stmt",
  "set_variable_stmt", "opt_inner", "opt_semicolon", "opt_not",
  "opt_alias", "opt_as", "opt_nullable", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-145)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-130)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     165,    16,    22,    80,    80,   -25,    53,  -145,    28,    29,
       9,  -145,  -145,  -145,  -145,  -145,    11,    45,   165,    97,
      98,  -145,  -145,  -145,  -145,  -145,  -145,  -145,  -145,  -145,
    -145,  -145,  -145,  -145,  -145,  -145,  -145,  -145,  -145,  -145,
      46,    54,    57,    58,    -2,  -145,    92,  -145,  -145,     2,
    -145,    80,    80,  -145,  -145,  -145,   171,  -145,    93,  -145,
      95,  -145,  -145,    73,    74,    99,    66,    96,  -145,  -145,
    -145,  -145,   123,   104,  -145,   105,   126,    12,    26,  -145,
      80,   -37,   203,  -145,  -145,    86,    80,    80,  -145,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,   -32,
    -145,   111,    80,   -12,   131,   113,   131,   114,     4,   116,
     118,   119,   129,  -145,  -145,   158,    94,  -145,  -145,   155,
      56,   214,    37,    37,    37,    37,    37,    37,   -31,   -31,
    -145,  -145,   179,   134,  -145,  -145,    26,  -145,   -28,   -11,
    -145,    80,   156,   180,  -145,   135,   181,   131,  -145,   170,
     166,   185,   188,  -145,  -145,    80,  -145,    -2,  -145,  -145,
     189,  -145,   -12,  -145,   161,   192,   167,   164,     4,    80,
     114,  -145,   206,  -145,  -145,  -145,  -145,  -145,    -6,   118,
     194,   160,  -145,   196,  -145,  -145,   -12,    80,    80,   172,
     199,   192,  -145,   162,   191,  -145,   193,  -145,   185,  -145,
     204,   225,  -145,   -28,  -145,   192,   201,  -145,     4,   227,
    -145,  -145,   228,  -145,  -145,   160,  -145,   247,    33,   199,
    -145,   -14,  -145,    80,  -145,  -145,  -145,   266,  -145,    80,
    -145,  -145,   192,    33,   192,  -145
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,    24,     0,     0,
       0,    25,    26,    27,    23,    22,     0,     0,     0,     0,
     123,    21,    20,    13,    14,    15,    16,     8,     9,    10,
      11,    12,     7,     4,     6,     5,     3,    17,    18,    19,
       0,     0,     0,     0,     0,    52,     0,    49,    50,    87,
      51,     0,     0,    90,    85,    67,   127,    86,   107,    59,
      95,    30,    29,     0,     0,     0,     0,     0,   119,     1,
     124,     2,     0,     0,    28,     0,     0,   125,     0,    80,
       0,     0,    79,    68,   130,   125,     0,     0,   126,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     106,     0,     0,     0,    91,     0,    91,     0,     0,     0,
       0,     0,     0,    60,    77,     0,    61,    88,    89,     0,
      76,    75,    70,    71,    72,    73,    69,    74,    63,    64,
      65,    66,     0,     0,   128,   108,     0,    93,   127,   101,
      96,     0,   111,     0,    53,     0,    56,    91,   120,     0,
       0,    36,     0,    34,    84,     0,    83,     0,    82,    78,
       0,    97,     0,   122,     0,    92,     0,   116,     0,     0,
       0,    54,     0,    41,    42,    43,    45,    44,   131,     0,
       0,     0,    62,     0,    94,   102,     0,     0,     0,   109,
      47,    55,    57,     0,     0,   133,     0,    39,    36,    35,
      32,     0,    81,   127,   112,   117,     0,    58,     0,     0,
     118,    40,     0,   132,    37,     0,    31,    99,   113,    47,
      46,   131,    33,     0,    98,   115,   114,   104,   110,     0,
      48,    38,   100,   113,   103,   105
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -145,  -145,   205,  -145,  -145,  -145,  -145,  -145,  -145,  -145,
    -145,  -145,  -145,    72,  -145,  -145,    90,   110,  -145,  -145,
    -145,    71,  -106,  -145,  -145,  -145,   121,     3,   -36,  -144,
      -3,  -145,   -92,   106,  -145,  -145,  -145,   132,  -145,    60,
    -145,     1,  -145,  -145,  -145,  -145,  -145,  -145,  -145,  -145,
    -145,   210,  -134,  -145,    75
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,   201,    31,    32,   180,   151,   212,   178,
      33,   209,    54,    34,    35,   146,   147,    76,    55,   115,
     116,    57,   142,   138,   104,   139,   224,   140,   227,   228,
      58,    59,   207,   167,   229,   189,    37,    38,    39,   164,
      71,    99,   100,   101,   197
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      56,    56,   148,    36,   161,    60,   136,     3,     4,   162,
      79,   182,   194,   183,   144,   195,    44,   117,   132,    84,
      80,    36,    40,   195,    41,   133,  -129,    45,    42,    61,
      43,   114,   118,    45,    81,     3,     4,  -121,    97,    98,
     163,    77,   137,   204,   225,   226,   196,    46,    82,    83,
      47,    48,    49,    50,   196,   171,    47,    48,    51,    50,
      62,    63,   190,    65,    64,    66,    52,    53,    85,   217,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,   120,   121,    67,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   158,    69,    44,    56,
      72,    70,   219,   135,    95,    96,    97,    98,    73,    45,
      78,    74,    75,   102,   155,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   105,   106,    46,
     103,   108,    47,    48,    49,    50,   107,   109,   165,   160,
      51,   110,   111,   112,  -125,   113,    88,   143,    52,    53,
      85,  -125,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,   134,   191,   141,   145,     1,
       2,   149,   150,   152,     3,     4,     5,   154,     6,     7,
       8,     9,    10,   153,   156,   205,    11,    12,    13,   159,
     173,   174,   175,   176,   177,    14,    15,   157,   168,   166,
     169,   170,    16,   172,    17,   179,   181,    18,   184,   186,
     188,   187,   193,   199,   200,   202,   210,   206,    84,   208,
     232,  -125,   213,    68,   215,  -129,   234,    85,  -125,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,  -125,   211,   216,   218,   220,   221,    85,  -125,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,  -130,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,   223,   233,   222,   214,   198,
     230,   192,   203,   235,   185,   119,   231
};

static const yytype_uint8 yycheck[] =
{
       3,     4,   108,     0,   138,     4,    18,     9,    10,    20,
      46,   155,    18,   157,   106,    29,    18,    54,    50,    47,
      18,    18,     6,    29,     8,    57,    54,    29,     6,    54,
       8,    19,    69,    29,    32,     9,    10,    48,    69,    70,
      51,    44,    54,   187,    11,    12,    60,    49,    51,    52,
      52,    53,    54,    55,    60,   147,    52,    53,    60,    55,
       7,    33,   168,    54,    35,    54,    68,    69,    56,   203,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    86,    87,    40,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,   132,     0,    18,   102,
      54,     3,   208,   102,    67,    68,    69,    70,    54,    29,
      18,    54,    54,    20,    20,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    54,    54,    49,
      35,    65,    52,    53,    54,    55,    37,    41,   141,   136,
      60,    18,    38,    38,    50,    19,    60,    34,    68,    69,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    54,   169,    36,    54,     4,
       5,    55,    54,    54,     9,    10,    11,    19,    13,    14,
      15,    16,    17,    54,    29,   188,    21,    22,    23,    55,
      24,    25,    26,    27,    28,    30,    31,    18,    18,    43,
      65,    20,    37,    33,    39,    20,    18,    42,    19,    48,
      46,    44,     6,    19,    54,    19,    54,    45,    47,    20,
     223,    50,    29,    18,    20,    54,   229,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    50,    52,    19,    44,    19,    19,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    38,    20,   215,   198,   179,
     219,   170,   186,   233,   162,    85,   221
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     4,     5,     9,    10,    11,    13,    14,    15,    16,
      17,    21,    22,    23,    30,    31,    37,    39,    42,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    86,    87,    92,    95,    96,    99,   118,   119,   120,
       6,     8,     6,     8,    18,    29,    49,    52,    53,    54,
      55,    60,    68,    69,    94,   100,   102,   103,   112,   113,
     113,    54,     7,    33,    35,    54,    54,    40,    74,     0,
       3,   122,    54,    54,    54,    54,    99,   102,    18,   100,
      18,    32,   102,   102,    47,    56,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,   123,
     124,   125,    20,    35,   106,    54,    54,    37,    65,    41,
      18,    38,    38,    19,    19,   101,   102,    54,    69,   123,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,    50,    57,    54,   113,    18,    54,   105,   107,
     109,    36,   104,    34,   104,    54,    97,    98,    94,    55,
      54,    89,    54,    54,    19,    20,    29,    18,   100,    55,
      99,   124,    20,    51,   121,   102,    43,   115,    18,    65,
      20,   104,    33,    24,    25,    26,    27,    28,    91,    20,
      88,    18,   101,   101,    19,   109,    48,    44,    46,   117,
      94,   102,    98,     6,    18,    29,    60,   126,    89,    19,
      54,    85,    19,   105,   101,   102,    45,   114,    20,    93,
      54,    52,    90,    29,    88,    20,    19,   124,    44,    94,
      19,    19,    85,    38,   108,    11,    12,   110,   111,   116,
      93,   126,   102,    20,   102,   111
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    72,    73,    74,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    85,    86,    87,    88,    88,    89,    89,
      90,    91,    91,    91,    91,    91,    92,    93,    93,    94,
      94,    94,    94,    95,    96,    97,    98,    98,    99,    99,
     100,   101,   101,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   103,   103,   103,
     103,   104,   104,   105,   105,   106,   106,   107,   107,   108,
     108,   109,   109,   110,   111,   111,   112,   113,   113,   114,
     114,   115,   115,   116,   116,   116,   117,   117,   118,   119,
     120,   121,   121,   122,   122,   123,   123,   124,   124,   125,
     125,   126,   126,   126
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       2,     8,     1,     3,     5,     7,     0,     3,     6,     3,
       1,     1,     1,     1,     1,     1,     8,     0,     3,     1,
       1,     1,     1,     4,     5,     3,     1,     3,     7,     2,
       3,     1,     3,     3,     3,     3,     3,     1,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     4,     2,
       2,     6,     4,     4,     4,     1,     1,     1,     3,     3,
       1,     0,     2,     1,     3,     0,     2,     2,     6,     0,
       2,     1,     3,     2,     1,     3,     2,     1,     3,     0,
       3,     0,     3,     0,     1,     1,     0,     2,     7,     2,
       4,     0,     1,     0,     1,     0,     1,     0,     2,     0,
       1,     0,     2,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (&yylloc, sql_string, sql_result, scanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, sql_string, sql_result, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (sql_string);
  YY_USE (sql_result);
  YY_USE (scanner);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, sql_string, sql_result, scanner);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), sql_string, sql_result, scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, sql_string, sql_result, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (sql_string);
  YY_USE (sql_result);
  YY_USE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, &yylloc, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* commands: command_wrapper opt_semicolon  */
#line 208 "yacc_sql.y"
  {
    std::unique_ptr<ParsedSqlNode> sql_node = std::unique_ptr<ParsedSqlNode>((yyvsp[-1].sql_node));
    sql_result->add_sql_node(std::move(sql_node));
  }
#line 1820 "yacc_sql.cpp"
    break;

  case 22: /* exit_stmt: EXIT  */
#line 237 "yacc_sql.y"
         {
      (void)yynerrs;  // 这么写为了消除yynerrs未使用的告警。如果你有更好的方法欢迎提PR
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXIT);
    }
#line 1829 "yacc_sql.cpp"
    break;

  case 23: /* help_stmt: HELP  */
#line 243 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_HELP);
    }
#line 1837 "yacc_sql.cpp"
    break;

  case 24: /* sync_stmt: SYNC  */
#line 248 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SYNC);
    }
#line 1845 "yacc_sql.cpp"
    break;

  case 25: /* begin_stmt: TRX_BEGIN  */
#line 254 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_BEGIN);
    }
#line 1853 "yacc_sql.cpp"
    break;

  case 26: /* commit_stmt: TRX_COMMIT  */
#line 260 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_COMMIT);
    }
#line 1861 "yacc_sql.cpp"
    break;

  case 27: /* rollback_stmt: TRX_ROLLBACK  */
#line 266 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ROLLBACK);
    }
#line 1869 "yacc_sql.cpp"
    break;

  case 28: /* drop_table_stmt: DROP TABLE ID  */
#line 272 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_TABLE);
      (yyval.sql_node)->drop_table.relation_name = (yyvsp[0].string);
      free((yyvsp[0].string));
    }
#line 1879 "yacc_sql.cpp"
    break;

  case 29: /* show_tables_stmt: SHOW TABLES  */
#line 279 "yacc_sql.y"
                {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SHOW_TABLES);
    }
#line 1887 "yacc_sql.cpp"
    break;

  case 30: /* desc_table_stmt: DESC ID  */
#line 285 "yacc_sql.y"
             {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DESC_TABLE);
      (yyval.sql_node)->desc_table.relation_name = (yyvsp[0].string);
      free((yyvsp[0].string));
    }
#line 1897 "yacc_sql.cpp"
    break;

  case 31: /* create_index_stmt: CREATE INDEX ID ON ID LBRACE index_col_list RBRACE  */
#line 294 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = (yyval.sql_node)->create_index;
      create_index.index_name = (yyvsp[-5].string);
      create_index.relation_name = (yyvsp[-3].string);
      create_index.attribute_names.swap(*(yyvsp[-1].string_list));
      free((yyvsp[-5].string));
      free((yyvsp[-3].string));
      delete (yyvsp[-1].string_list);
    }
#line 1912 "yacc_sql.cpp"
    break;

  case 32: /* index_col_list: ID  */
#line 306 "yacc_sql.y"
                   {
      (yyval.string_list) = new std::vector<std::string>;
      (yyval.string_list)->push_back((yyvsp[0].string));
      free((yyvsp[0].string));
    }
#line 1922 "yacc_sql.cpp"
    break;

  case 33: /* index_col_list: ID COMMA index_col_list  */
#line 311 "yacc_sql.y"
                                {
      if ((yyvsp[0].string_list) != nullptr) {
        (yyval.string_list) = (yyvsp[0].string_list);
      } else {
        (yyval.string_list) = new std::vector<std::string>;
      }
      (yyval.string_list)->insert((yyval.string_list)->begin(), (yyvsp[-2].string));
      free((yyvsp[-2].string));
    }
#line 1936 "yacc_sql.cpp"
    break;

  case 34: /* drop_index_stmt: DROP INDEX ID ON ID  */
#line 323 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_INDEX);
      (yyval.sql_node)->drop_index.index_name = (yyvsp[-2].string);
      (yyval.sql_node)->drop_index.relation_name = (yyvsp[0].string);
      free((yyvsp[-2].string));
      free((yyvsp[0].string));
    }
#line 1948 "yacc_sql.cpp"
    break;

  case 35: /* create_table_stmt: CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE  */
#line 334 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = (yyval.sql_node)->create_table;
      create_table.relation_name = (yyvsp[-4].string);
      free((yyvsp[-4].string));

      std::vector<AttrInfoSqlNode> *src_attrs = (yyvsp[-1].attr_infos);

      if (src_attrs != nullptr) {
        create_table.attr_infos.swap(*src_attrs);
        delete src_attrs;
      }
      create_table.attr_infos.emplace_back(*(yyvsp[-2].attr_info));
      std::reverse(create_table.attr_infos.begin(), create_table.attr_infos.end());
      delete (yyvsp[-2].attr_info);
    }
#line 1969 "yacc_sql.cpp"
    break;

  case 36: /* attr_def_list: %empty  */
#line 354 "yacc_sql.y"
    {
      (yyval.attr_infos) = nullptr;
    }
#line 1977 "yacc_sql.cpp"
    break;

  case 37: /* attr_def_list: COMMA attr_def attr_def_list  */
#line 358 "yacc_sql.y"
    {
      if ((yyvsp[0].attr_infos) != nullptr) {
        (yyval.attr_infos) = (yyvsp[0].attr_infos);
      } else {
        (yyval.attr_infos) = new std::vector<AttrInfoSqlNode>;
      }
      (yyval.attr_infos)->emplace_back(*(yyvsp[-1].attr_info));
      delete (yyvsp[-1].attr_info);
    }
#line 1991 "yacc_sql.cpp"
    break;

  case 38: /* attr_def: ID type LBRACE number RBRACE opt_nullable  */
#line 371 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-4].number);
      (yyval.attr_info)->name = (yyvsp[-5].string);
      (yyval.attr_info)->length = (yyvsp[-2].number);
      (yyval.attr_info)->nullable = (yyvsp[0].booleans);
      free((yyvsp[-5].string));
    }
#line 2004 "yacc_sql.cpp"
    break;

  case 39: /* attr_def: ID type opt_nullable  */
#line 380 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-1].number);
      (yyval.attr_info)->name = (yyvsp[-2].string);
      (yyval.attr_info)->length = 4;   /*length 默认为4, 这一点和Mysql不一样*/
      (yyval.attr_info)->nullable = (yyvsp[0].booleans);
      free((yyvsp[-2].string));
    }
#line 2017 "yacc_sql.cpp"
    break;

  case 40: /* number: NUMBER  */
#line 391 "yacc_sql.y"
           {(yyval.number) = (yyvsp[0].number);}
#line 2023 "yacc_sql.cpp"
    break;

  case 41: /* type: INT_T  */
#line 395 "yacc_sql.y"
               { (yyval.number)=INTS; }
#line 2029 "yacc_sql.cpp"
    break;

  case 42: /* type: STRING_T  */
#line 396 "yacc_sql.y"
               { (yyval.number)=CHARS; }
#line 2035 "yacc_sql.cpp"
    break;

  case 43: /* type: FLOAT_T  */
#line 397 "yacc_sql.y"
               { (yyval.number)=FLOATS; }
#line 2041 "yacc_sql.cpp"
    break;

  case 44: /* type: TEXT_T  */
#line 398 "yacc_sql.y"
               { (yyval.number)=TEXTS; }
#line 2047 "yacc_sql.cpp"
    break;

  case 45: /* type: DATE_T  */
#line 399 "yacc_sql.y"
               { (yyval.number)=DATES; }
#line 2053 "yacc_sql.cpp"
    break;

  case 46: /* insert_stmt: INSERT INTO ID VALUES LBRACE value value_list RBRACE  */
#line 404 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_INSERT);
      (yyval.sql_node)->insertion.relation_name = (yyvsp[-5].string);
      if ((yyvsp[-1].value_list) != nullptr) {
        (yyval.sql_node)->insertion.values.swap(*(yyvsp[-1].value_list));
        delete (yyvsp[-1].value_list);
      }
      (yyval.sql_node)->insertion.values.emplace_back(*(yyvsp[-2].value));
      std::reverse((yyval.sql_node)->insertion.values.begin(), (yyval.sql_node)->insertion.values.end());
      delete (yyvsp[-2].value);
      free((yyvsp[-5].string));
    }
#line 2070 "yacc_sql.cpp"
    break;

  case 47: /* value_list: %empty  */
#line 420 "yacc_sql.y"
    {
      (yyval.value_list) = nullptr;
    }
#line 2078 "yacc_sql.cpp"
    break;

  case 48: /* value_list: COMMA value value_list  */
#line 423 "yacc_sql.y"
                              { 
      if ((yyvsp[0].value_list) != nullptr) {
        (yyval.value_list) = (yyvsp[0].value_list);
      } else {
        (yyval.value_list) = new std::vector<Value>;
      }
      (yyval.value_list)->emplace_back(*(yyvsp[-1].value));
      delete (yyvsp[-1].value);
    }
#line 2092 "yacc_sql.cpp"
    break;

  case 49: /* value: NUMBER  */
#line 435 "yacc_sql.y"
           {
      (yyval.value) = new Value((int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2101 "yacc_sql.cpp"
    break;

  case 50: /* value: FLOAT  */
#line 439 "yacc_sql.y"
           {
      (yyval.value) = new Value((float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2110 "yacc_sql.cpp"
    break;

  case 51: /* value: SSS  */
#line 443 "yacc_sql.y"
         {
      char *tmp = common::substr((yyvsp[0].string),1,strlen((yyvsp[0].string))-2);
      (yyval.value) = new Value(tmp);
      free(tmp);
      free((yyvsp[0].string));
    }
#line 2121 "yacc_sql.cpp"
    break;

  case 52: /* value: THE_NULL  */
#line 449 "yacc_sql.y"
              {
      (yyval.value) = new Value();
      (yyval.value)->set_null();
    }
#line 2130 "yacc_sql.cpp"
    break;

  case 53: /* delete_stmt: DELETE FROM ID opt_where  */
#line 457 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DELETE);
      (yyval.sql_node)->deletion.relation_name = (yyvsp[-1].string);
      if ((yyvsp[0].expression) != nullptr) {
        (yyval.sql_node)->deletion.condition = (yyvsp[0].expression);
      }
      free((yyvsp[-1].string));
    }
#line 2143 "yacc_sql.cpp"
    break;

  case 54: /* update_stmt: UPDATE ID SET update_asgn_list opt_where  */
#line 469 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_UPDATE);
      (yyval.sql_node)->update.relation_name = (yyvsp[-3].string);
      (yyval.sql_node)->update.assignments.swap(*(yyvsp[-1].update_asgn_list));
      if ((yyvsp[0].expression) != nullptr) {
        (yyval.sql_node)->update.condition = (yyvsp[0].expression);
      }
      free((yyvsp[-3].string));
    }
#line 2157 "yacc_sql.cpp"
    break;

  case 55: /* update_asgn_factor: ID EQ expression  */
#line 482 "yacc_sql.y"
    {
      UpdateAssignmentSqlNode *tmp = new UpdateAssignmentSqlNode;
      tmp->attribute_name = (yyvsp[-2].string);
      tmp->value = (yyvsp[0].expression);

      (yyval.update_asgn_factor) = tmp;
    }
#line 2169 "yacc_sql.cpp"
    break;

  case 56: /* update_asgn_list: update_asgn_factor  */
#line 492 "yacc_sql.y"
    {
      (yyval.update_asgn_list) = new std::vector<UpdateAssignmentSqlNode *>;
      (yyval.update_asgn_list)->push_back((yyvsp[0].update_asgn_factor));
    }
#line 2178 "yacc_sql.cpp"
    break;

  case 57: /* update_asgn_list: update_asgn_factor COMMA update_asgn_list  */
#line 497 "yacc_sql.y"
    {
      if ((yyvsp[0].update_asgn_list) != nullptr) {
        (yyval.update_asgn_list) = (yyvsp[0].update_asgn_list);
      } else {
        (yyval.update_asgn_list) = new std::vector<UpdateAssignmentSqlNode *>;
      }
      (yyval.update_asgn_list)->insert((yyval.update_asgn_list)->begin(), (yyvsp[-2].update_asgn_factor));
    }
#line 2191 "yacc_sql.cpp"
    break;

  case 58: /* select_stmt: SELECT query_expression_list opt_table_refs opt_where opt_group_by opt_having opt_order_by  */
#line 510 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-5].expression_with_alias_list) != nullptr) {
        (yyval.sql_node)->selection.attributes.swap(*(yyvsp[-5].expression_with_alias_list));
        delete (yyvsp[-5].expression_with_alias_list);
      }
      if ((yyvsp[-4].table_reference_list) != nullptr) {
        (yyval.sql_node)->selection.relations.swap(*(yyvsp[-4].table_reference_list));
        delete (yyvsp[-4].table_reference_list);
      }
      if ((yyvsp[-3].expression) != nullptr) {
        (yyval.sql_node)->selection.condition = (yyvsp[-3].expression);
      }
      if ((yyvsp[-2].expression_list) != nullptr) {
        (yyval.sql_node)->selection.group_by.swap(*(yyvsp[-2].expression_list));
        delete (yyvsp[-2].expression_list);
      }
      if ((yyvsp[-1].expression) != nullptr) {
        (yyval.sql_node)->selection.having = (yyvsp[-1].expression);
      }
      if ((yyvsp[0].order_by_list) != nullptr) {
        (yyval.sql_node)->selection.order_by.swap(*(yyvsp[0].order_by_list));
        delete (yyvsp[0].order_by_list);
      }
    }
#line 2221 "yacc_sql.cpp"
    break;

  case 59: /* select_stmt: CALC query_expression_list  */
#line 536 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[0].expression_with_alias_list) != nullptr) {
        (yyval.sql_node)->selection.attributes.swap(*(yyvsp[0].expression_with_alias_list));
        delete (yyvsp[0].expression_with_alias_list);
      }
    }
#line 2233 "yacc_sql.cpp"
    break;

  case 60: /* select_with_parenthesis: LBRACE select_stmt RBRACE  */
#line 546 "yacc_sql.y"
    {
      (yyval.subquery) = new SubqueryExpressionSqlNode;
      (yyval.subquery)->subquery = (yyvsp[-1].sql_node);
    }
#line 2242 "yacc_sql.cpp"
    break;

  case 61: /* expression_list: expression  */
#line 553 "yacc_sql.y"
    {
      (yyval.expression_list) = new std::vector<ExpressionSqlNode *>;
      (yyval.expression_list)->emplace_back((yyvsp[0].expression));
    }
#line 2251 "yacc_sql.cpp"
    break;

  case 62: /* expression_list: expression COMMA expression_list  */
#line 558 "yacc_sql.y"
    {
      if ((yyvsp[0].expression_list) != nullptr) {
        (yyval.expression_list) = (yyvsp[0].expression_list);
      } else {
        (yyval.expression_list) = new std::vector<ExpressionSqlNode *>;
      }
      (yyval.expression_list)->insert((yyval.expression_list)->begin(), (yyvsp[-2].expression));
    }
#line 2264 "yacc_sql.cpp"
    break;

  case 63: /* expression: expression ADD expression  */
#line 569 "yacc_sql.y"
                              {
      ArithmeticExpressionSqlNode* tmp = new ArithmeticExpressionSqlNode;
      tmp->left = (yyvsp[-2].expression);
      tmp->right = (yyvsp[0].expression);
      tmp->arithmetic_type = ArithmeticType::ADD;

      (yyval.expression) = tmp;
      (yyval.expression)->name = token_name(sql_string, &(yyloc));
    }
#line 2278 "yacc_sql.cpp"
    break;

  case 64: /* expression: expression SUB expression  */
#line 578 "yacc_sql.y"
                                {
      ArithmeticExpressionSqlNode* tmp = new ArithmeticExpressionSqlNode;
      tmp->left = (yyvsp[-2].expression);
      tmp->right = (yyvsp[0].expression);
      tmp->arithmetic_type = ArithmeticType::SUB;

      (yyval.expression) = tmp;
      (yyval.expression)->name = token_name(sql_string, &(yyloc));
    }
#line 2292 "yacc_sql.cpp"
    break;

  case 65: /* expression: expression STAR expression  */
#line 587 "yacc_sql.y"
                                 {
      ArithmeticExpressionSqlNode* tmp = new ArithmeticExpressionSqlNode;
      tmp->left = (yyvsp[-2].expression);
      tmp->right = (yyvsp[0].expression);
      tmp->arithmetic_type = ArithmeticType::MUL;

      (yyval.expression) = tmp;
      (yyval.expression)->name = token_name(sql_string, &(yyloc));
    }
#line 2306 "yacc_sql.cpp"
    break;

  case 66: /* expression: expression DIV expression  */
#line 596 "yacc_sql.y"
                                {
      ArithmeticExpressionSqlNode* tmp = new ArithmeticExpressionSqlNode;
      tmp->left = (yyvsp[-2].expression);
      tmp->right = (yyvsp[0].expression);
      tmp->arithmetic_type = ArithmeticType::DIV;

      (yyval.expression) = tmp;
      (yyval.expression)->name = token_name(sql_string, &(yyloc));
    }
#line 2320 "yacc_sql.cpp"
    break;

  case 67: /* expression: select_with_parenthesis  */
#line 605 "yacc_sql.y"
                              {
      (yyval.expression) = (yyvsp[0].subquery);

      (yyval.expression)->name = token_name(sql_string, &(yyloc));
    }
#line 2330 "yacc_sql.cpp"
    break;

  case 68: /* expression: SUB expression  */
#line 610 "yacc_sql.y"
                                  {
      ArithmeticExpressionSqlNode* tmp = new ArithmeticExpressionSqlNode;
      tmp->right = (yyvsp[0].expression);
      tmp->left = new ValueExpressionSqlNode;
      tmp->arithmetic_type = ArithmeticType::SUB;

      ((ValueExpressionSqlNode *)(tmp->left))->value = Value(0);

      (yyval.expression) = tmp;
      (yyval.expression)->name = token_name(sql_string, &(yyloc));
    }
#line 2346 "yacc_sql.cpp"
    break;

  case 69: /* expression: expression EQ expression  */
#line 621 "yacc_sql.y"
                               {
      ComparisonExpressionSqlNode *tmp = new ComparisonExpressionSqlNode;
      tmp->comp_op = EQUAL_TO;
      tmp->left = (yyvsp[-2].expression);
      tmp->right = (yyvsp[0].expression);

      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2360 "yacc_sql.cpp"
    break;

  case 70: /* expression: expression LT expression  */
#line 630 "yacc_sql.y"
                               {
      ComparisonExpressionSqlNode *tmp = new ComparisonExpressionSqlNode;
      tmp->comp_op = LESS_THAN;
      tmp->left = (yyvsp[-2].expression);
      tmp->right = (yyvsp[0].expression);

      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2374 "yacc_sql.cpp"
    break;

  case 71: /* expression: expression GT expression  */
#line 639 "yacc_sql.y"
                               {
      ComparisonExpressionSqlNode *tmp = new ComparisonExpressionSqlNode;
      tmp->comp_op = GREAT_THAN;
      tmp->left = (yyvsp[-2].expression);
      tmp->right = (yyvsp[0].expression);

      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2388 "yacc_sql.cpp"
    break;

  case 72: /* expression: expression LE expression  */
#line 648 "yacc_sql.y"
                               {
      ComparisonExpressionSqlNode *tmp = new ComparisonExpressionSqlNode;
      tmp->comp_op = LESS_EQUAL;
      tmp->left = (yyvsp[-2].expression);
      tmp->right = (yyvsp[0].expression);

      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2402 "yacc_sql.cpp"
    break;

  case 73: /* expression: expression GE expression  */
#line 657 "yacc_sql.y"
                               {
      ComparisonExpressionSqlNode *tmp = new ComparisonExpressionSqlNode;
      tmp->comp_op = GREAT_EQUAL;
      tmp->left = (yyvsp[-2].expression);
      tmp->right = (yyvsp[0].expression);

      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2416 "yacc_sql.cpp"
    break;

  case 74: /* expression: expression NE expression  */
#line 666 "yacc_sql.y"
                               {
      ComparisonExpressionSqlNode *tmp = new ComparisonExpressionSqlNode;
      tmp->comp_op = NOT_EQUAL;
      tmp->left = (yyvsp[-2].expression);
      tmp->right = (yyvsp[0].expression);

      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2430 "yacc_sql.cpp"
    break;

  case 75: /* expression: expression AND expression  */
#line 675 "yacc_sql.y"
                                {
      ConjunctionExpressionSqlNode *tmp = new ConjunctionExpressionSqlNode;
      tmp->left = (yyvsp[-2].expression);
      tmp->right = (yyvsp[0].expression);
      tmp->conjunction_type = ConjunctionType::AND;
      
      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2444 "yacc_sql.cpp"
    break;

  case 76: /* expression: expression OR expression  */
#line 684 "yacc_sql.y"
                               {
      ConjunctionExpressionSqlNode *tmp = new ConjunctionExpressionSqlNode;
      tmp->left = (yyvsp[-2].expression);
      tmp->right = (yyvsp[0].expression);
      tmp->conjunction_type = ConjunctionType::OR;
      
      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2458 "yacc_sql.cpp"
    break;

  case 77: /* expression: LBRACE expression RBRACE  */
#line 693 "yacc_sql.y"
                               {
      (yyval.expression) = (yyvsp[-1].expression);
    }
#line 2466 "yacc_sql.cpp"
    break;

  case 78: /* expression: expression opt_not LIKE SSS  */
#line 696 "yacc_sql.y"
                                  {
      LikeExpressionSqlNode *tmp = new LikeExpressionSqlNode;
      tmp->child = (yyvsp[-3].expression);
      char *dupped_str = common::substr((yyvsp[0].string),1,strlen((yyvsp[0].string))-2);
      tmp->pattern = dupped_str;
      free(dupped_str);
      free((yyvsp[0].string));

      (yyval.expression) = tmp;

      if ((yyvsp[-2].booleans)) {
        NotExpressionSqlNode* not_p = new NotExpressionSqlNode;
        not_p->child = (yyval.expression);
        (yyval.expression) = not_p;
      }

      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2489 "yacc_sql.cpp"
    break;

  case 79: /* expression: NOT expression  */
#line 714 "yacc_sql.y"
                     {
      NotExpressionSqlNode *tmp = new NotExpressionSqlNode;
      tmp->child = (yyvsp[0].expression);

      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2501 "yacc_sql.cpp"
    break;

  case 80: /* expression: EXISTS select_with_parenthesis  */
#line 721 "yacc_sql.y"
                                     {
      ExistsExpressionSqlNode *tmp = new ExistsExpressionSqlNode;
      tmp->subquery = (yyvsp[0].subquery);

      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2513 "yacc_sql.cpp"
    break;

  case 81: /* expression: expression opt_not IN LBRACE expression_list RBRACE  */
#line 728 "yacc_sql.y"
                                                          {
      InExpressionSqlNode *tmp = new InExpressionSqlNode;
      tmp->child = (yyvsp[-5].expression);
      tmp->value_list.swap(*(yyvsp[-1].expression_list));
      delete (yyvsp[-1].expression_list);

      (yyval.expression) = tmp;

      if ((yyvsp[-4].booleans)) {
        NotExpressionSqlNode* not_p = new NotExpressionSqlNode;
        not_p->child = (yyval.expression);
        (yyval.expression) = not_p;
      }

      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2534 "yacc_sql.cpp"
    break;

  case 82: /* expression: expression opt_not IN select_with_parenthesis  */
#line 744 "yacc_sql.y"
                                                    {
      InExpressionSqlNode *tmp = new InExpressionSqlNode;
      tmp->child = (yyvsp[-3].expression);
      tmp->subquery = (yyvsp[0].subquery);

      (yyval.expression) = tmp;

      if ((yyvsp[-2].booleans)) {
        NotExpressionSqlNode* not_p = new NotExpressionSqlNode;
        not_p->child = (yyval.expression);
        (yyval.expression) = not_p;
      }

      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2554 "yacc_sql.cpp"
    break;

  case 83: /* expression: expression IS opt_not THE_NULL  */
#line 759 "yacc_sql.y"
                                     {
      IsNullExpressionSqlNode *tmp = new IsNullExpressionSqlNode;
      tmp->child = (yyvsp[-3].expression);

      (yyval.expression) = tmp;

      if ((yyvsp[-1].booleans)) {
        NotExpressionSqlNode* not_p = new NotExpressionSqlNode;
        not_p->child = (yyval.expression);
        (yyval.expression) = not_p;
      }

      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2573 "yacc_sql.cpp"
    break;

  case 84: /* expression: ID LBRACE expression_list RBRACE  */
#line 773 "yacc_sql.y"
                                       {
      FunctionExpressionSqlNode *tmp = new FunctionExpressionSqlNode;
      tmp->param_exprs.swap(*(yyvsp[-1].expression_list));
      tmp->function_name = (yyvsp[-3].string);

      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2586 "yacc_sql.cpp"
    break;

  case 85: /* expression: value  */
#line 781 "yacc_sql.y"
            {
      ValueExpressionSqlNode *tmp = new ValueExpressionSqlNode;
      tmp->value = *(yyvsp[0].value);
      delete (yyvsp[0].value);

      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2599 "yacc_sql.cpp"
    break;

  case 86: /* expression: rel_attr  */
#line 789 "yacc_sql.y"
               {
      FieldExpressionSqlNode *tmp = new FieldExpressionSqlNode;
      tmp->field = *(yyvsp[0].rel_attr);
      delete (yyvsp[0].rel_attr);

      (yyval.expression) = tmp;
      (yyval.expression)->name = (token_name(sql_string, &(yyloc)));
    }
#line 2612 "yacc_sql.cpp"
    break;

  case 87: /* rel_attr: ID  */
#line 800 "yacc_sql.y"
       {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name = "";
      (yyval.rel_attr)->attribute_name = (yyvsp[0].string);
      free((yyvsp[0].string));
    }
#line 2623 "yacc_sql.cpp"
    break;

  case 88: /* rel_attr: ID DOT ID  */
#line 806 "yacc_sql.y"
                {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name  = (yyvsp[-2].string);
      (yyval.rel_attr)->attribute_name = (yyvsp[0].string);
      free((yyvsp[-2].string));
      free((yyvsp[0].string));
    }
#line 2635 "yacc_sql.cpp"
    break;

  case 89: /* rel_attr: ID DOT STAR  */
#line 813 "yacc_sql.y"
                  {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name  = (yyvsp[-2].string);
      (yyval.rel_attr)->attribute_name = "*";
    }
#line 2645 "yacc_sql.cpp"
    break;

  case 90: /* rel_attr: STAR  */
#line 818 "yacc_sql.y"
           {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name  = "";
      (yyval.rel_attr)->attribute_name = "*";
    }
#line 2655 "yacc_sql.cpp"
    break;

  case 91: /* opt_where: %empty  */
#line 829 "yacc_sql.y"
    {
      (yyval.expression) = nullptr;
    }
#line 2663 "yacc_sql.cpp"
    break;

  case 92: /* opt_where: WHERE expression  */
#line 832 "yacc_sql.y"
                       {
      (yyval.expression) = (yyvsp[0].expression);  
    }
#line 2671 "yacc_sql.cpp"
    break;

  case 93: /* table_factor: ID  */
#line 838 "yacc_sql.y"
    {
      TablePrimarySqlNode* tmp = new TablePrimarySqlNode;
      tmp->relation_name = (yyvsp[0].string);

      (yyval.table_factor_node) = tmp;
    }
#line 2682 "yacc_sql.cpp"
    break;

  case 94: /* table_factor: LBRACE select_stmt RBRACE  */
#line 845 "yacc_sql.y"
    {
      TableSubquerySqlNode* tmp = new TableSubquerySqlNode;
      tmp->subquery = (yyvsp[-1].sql_node)->selection;

      (yyval.table_factor_node) = tmp;
    }
#line 2693 "yacc_sql.cpp"
    break;

  case 95: /* opt_table_refs: %empty  */
#line 853 "yacc_sql.y"
    {
      (yyval.table_reference_list) = nullptr;
    }
#line 2701 "yacc_sql.cpp"
    break;

  case 96: /* opt_table_refs: FROM table_ref_list  */
#line 857 "yacc_sql.y"
    {
      (yyval.table_reference_list) = (yyvsp[0].table_reference_list);
    }
#line 2709 "yacc_sql.cpp"
    break;

  case 97: /* table_ref: table_factor opt_alias  */
#line 862 "yacc_sql.y"
    {
      (yyval.table_reference) = (yyvsp[-1].table_factor_node);
      if ((yyvsp[0].string) != nullptr) {
        (yyval.table_reference)->alias = (yyvsp[0].string);
        free((yyvsp[0].string));
      }
    }
#line 2721 "yacc_sql.cpp"
    break;

  case 98: /* table_ref: table_ref opt_inner JOIN table_factor opt_alias opt_join_condition  */
#line 870 "yacc_sql.y"
    {
      TableJoinSqlNode *tmp = new TableJoinSqlNode;
      tmp->left = (yyvsp[-5].table_reference);
      tmp->right = (yyvsp[-2].table_factor_node);
      if ((yyvsp[-1].string)) {
        tmp->right->alias = (yyvsp[-1].string);
      }
      if ((yyvsp[0].expression)) {
        tmp->condition = (yyvsp[0].expression);
      }

      (yyval.table_reference) = tmp;
    }
#line 2739 "yacc_sql.cpp"
    break;

  case 99: /* opt_join_condition: %empty  */
#line 885 "yacc_sql.y"
    {
      (yyval.expression) = nullptr;
    }
#line 2747 "yacc_sql.cpp"
    break;

  case 100: /* opt_join_condition: ON expression  */
#line 889 "yacc_sql.y"
    {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 2755 "yacc_sql.cpp"
    break;

  case 101: /* table_ref_list: table_ref  */
#line 895 "yacc_sql.y"
    {
      (yyval.table_reference_list) = new std::vector<TableReferenceSqlNode *>;
      (yyval.table_reference_list)->push_back((yyvsp[0].table_reference));
    }
#line 2764 "yacc_sql.cpp"
    break;

  case 102: /* table_ref_list: table_ref COMMA table_ref_list  */
#line 900 "yacc_sql.y"
    {
      if ((yyvsp[0].table_reference_list) != nullptr) {
        (yyval.table_reference_list) = (yyvsp[0].table_reference_list);
      } else {
        (yyval.table_reference_list) = new std::vector<TableReferenceSqlNode *>;
      }
      (yyval.table_reference_list)->insert((yyval.table_reference_list)->begin(), (yyvsp[-2].table_reference));
    }
#line 2777 "yacc_sql.cpp"
    break;

  case 103: /* expression_with_order: opt_order_type expression  */
#line 910 "yacc_sql.y"
    {
      ExpressionWithOrderSqlNode *tmp = new ExpressionWithOrderSqlNode;
      tmp->expr = (yyvsp[0].expression);
      tmp->order_type = (yyvsp[-1].order_type);

      (yyval.order_by_node) = tmp;
    }
#line 2789 "yacc_sql.cpp"
    break;

  case 104: /* expression_with_order_list: expression_with_order  */
#line 919 "yacc_sql.y"
    {
      (yyval.order_by_list) = new std::vector<ExpressionWithOrderSqlNode *>;
      (yyval.order_by_list)->push_back((yyvsp[0].order_by_node));
    }
#line 2798 "yacc_sql.cpp"
    break;

  case 105: /* expression_with_order_list: expression_with_order COMMA expression_with_order_list  */
#line 924 "yacc_sql.y"
    {
      if ((yyvsp[0].order_by_list) != nullptr) {
        (yyval.order_by_list) = (yyvsp[0].order_by_list);
      } else {
        (yyval.order_by_list) = new std::vector<ExpressionWithOrderSqlNode *>;
      }
      (yyval.order_by_list)->insert((yyval.order_by_list)->begin(), (yyvsp[-2].order_by_node));
    }
#line 2811 "yacc_sql.cpp"
    break;

  case 106: /* query_expression: expression opt_alias  */
#line 934 "yacc_sql.y"
    {
      ExpressionWithAliasSqlNode *tmp = new ExpressionWithAliasSqlNode;
      tmp->expr = (yyvsp[-1].expression);
      if ((yyvsp[0].string) != nullptr) {
        tmp->alias = (yyvsp[0].string);
        free((yyvsp[0].string));
      }

      (yyval.expression_with_alias) = tmp;
    }
#line 2826 "yacc_sql.cpp"
    break;

  case 107: /* query_expression_list: query_expression  */
#line 946 "yacc_sql.y"
    {
      (yyval.expression_with_alias_list) = new std::vector<ExpressionWithAliasSqlNode *>;
      (yyval.expression_with_alias_list)->push_back((yyvsp[0].expression_with_alias));
    }
#line 2835 "yacc_sql.cpp"
    break;

  case 108: /* query_expression_list: query_expression COMMA query_expression_list  */
#line 951 "yacc_sql.y"
    {
      if ((yyvsp[0].expression_with_alias_list) != nullptr) {
        (yyval.expression_with_alias_list) = (yyvsp[0].expression_with_alias_list);
      } else {
        (yyval.expression_with_alias_list) = new std::vector<ExpressionWithAliasSqlNode *>;
      }
      (yyval.expression_with_alias_list)->insert((yyval.expression_with_alias_list)->begin(), (yyvsp[-2].expression_with_alias));
    }
#line 2848 "yacc_sql.cpp"
    break;

  case 109: /* opt_order_by: %empty  */
#line 961 "yacc_sql.y"
    {
      (yyval.order_by_list) = nullptr;
    }
#line 2856 "yacc_sql.cpp"
    break;

  case 110: /* opt_order_by: ORDER BY expression_with_order_list  */
#line 965 "yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[0].order_by_list);
    }
#line 2864 "yacc_sql.cpp"
    break;

  case 111: /* opt_group_by: %empty  */
#line 971 "yacc_sql.y"
    {
      (yyval.expression_list) =nullptr;
    }
#line 2872 "yacc_sql.cpp"
    break;

  case 112: /* opt_group_by: GROUP BY expression_list  */
#line 975 "yacc_sql.y"
    {
      (yyval.expression_list) = (yyvsp[0].expression_list);
    }
#line 2880 "yacc_sql.cpp"
    break;

  case 113: /* opt_order_type: %empty  */
#line 981 "yacc_sql.y"
    {
      (yyval.order_type) = OrderType::ASC;
    }
#line 2888 "yacc_sql.cpp"
    break;

  case 114: /* opt_order_type: ASC  */
#line 985 "yacc_sql.y"
    {
      (yyval.order_type) = OrderType::ASC;
    }
#line 2896 "yacc_sql.cpp"
    break;

  case 115: /* opt_order_type: DESC  */
#line 989 "yacc_sql.y"
    {
      (yyval.order_type) = OrderType::DESC;
    }
#line 2904 "yacc_sql.cpp"
    break;

  case 116: /* opt_having: %empty  */
#line 995 "yacc_sql.y"
    {
      (yyval.expression) = nullptr;
    }
#line 2912 "yacc_sql.cpp"
    break;

  case 117: /* opt_having: HAVING expression  */
#line 999 "yacc_sql.y"
    {
      (yyval.expression) = (yyvsp[0].expression);
    }
#line 2920 "yacc_sql.cpp"
    break;

  case 118: /* load_data_stmt: LOAD DATA INFILE SSS INTO TABLE ID  */
#line 1005 "yacc_sql.y"
    {
      char *tmp_file_name = common::substr((yyvsp[-3].string), 1, strlen((yyvsp[-3].string)) - 2);
      
      (yyval.sql_node) = new ParsedSqlNode(SCF_LOAD_DATA);
      (yyval.sql_node)->load_data.relation_name = (yyvsp[0].string);
      (yyval.sql_node)->load_data.file_name = tmp_file_name;
      free((yyvsp[0].string));
      free(tmp_file_name);
    }
#line 2934 "yacc_sql.cpp"
    break;

  case 119: /* explain_stmt: EXPLAIN command_wrapper  */
#line 1018 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXPLAIN);
      (yyval.sql_node)->explain.sql_node = std::unique_ptr<ParsedSqlNode>((yyvsp[0].sql_node));
    }
#line 2943 "yacc_sql.cpp"
    break;

  case 120: /* set_variable_stmt: SET ID EQ value  */
#line 1026 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SET_VARIABLE);
      (yyval.sql_node)->set_variable.name  = (yyvsp[-2].string);
      (yyval.sql_node)->set_variable.value = *(yyvsp[0].value);
      free((yyvsp[-2].string));
      delete (yyvsp[0].value);
    }
#line 2955 "yacc_sql.cpp"
    break;

  case 125: /* opt_not: %empty  */
#line 1044 "yacc_sql.y"
    {(yyval.booleans) = false;}
#line 2961 "yacc_sql.cpp"
    break;

  case 126: /* opt_not: NOT  */
#line 1045 "yacc_sql.y"
          {(yyval.booleans) = true;}
#line 2967 "yacc_sql.cpp"
    break;

  case 127: /* opt_alias: %empty  */
#line 1049 "yacc_sql.y"
    {(yyval.string) = nullptr;}
#line 2973 "yacc_sql.cpp"
    break;

  case 128: /* opt_alias: opt_as ID  */
#line 1051 "yacc_sql.y"
    {
      (yyval.string) = (yyvsp[0].string);
    }
#line 2981 "yacc_sql.cpp"
    break;

  case 131: /* opt_nullable: %empty  */
#line 1059 "yacc_sql.y"
               {(yyval.booleans) = true;}
#line 2987 "yacc_sql.cpp"
    break;

  case 132: /* opt_nullable: NOT THE_NULL  */
#line 1060 "yacc_sql.y"
                   {(yyval.booleans) = false;}
#line 2993 "yacc_sql.cpp"
    break;

  case 133: /* opt_nullable: THE_NULL  */
#line 1061 "yacc_sql.y"
               {(yyval.booleans) = true;}
#line 2999 "yacc_sql.cpp"
    break;


#line 3003 "yacc_sql.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (&yylloc, sql_string, sql_result, scanner, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, sql_string, sql_result, scanner);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, sql_string, sql_result, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, sql_string, sql_result, scanner, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, sql_string, sql_result, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, sql_string, sql_result, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 1064 "yacc_sql.y"

//_____________________________________________________________________
extern void scan_string(const char *str, yyscan_t scanner);

int sql_parse(const char *s, ParsedSqlResult *sql_result) {
  yyscan_t scanner;
  yylex_init(&scanner);
  scan_string(s, scanner);
  int result = yyparse(s, sql_result, scanner);
  yylex_destroy(scanner);
  return result;
}
