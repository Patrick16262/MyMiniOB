
%{

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

%}

%define api.pure full
%define parse.error verbose
/** 启用位置标识 **/
%locations
%lex-param { yyscan_t scanner }
/** 这些定义了在yyparse函数中的参数 **/
%parse-param { const char * sql_string }
%parse-param { ParsedSqlResult * sql_result }
%parse-param { void * scanner }

//标识tokens
%token  SEMICOLON
        CREATE
        DROP
        TABLE
        TABLES
        INDEX
        CALC
        SELECT
        DESC
        SHOW
        SYNC
        INSERT
        DELETE
        UPDATE
        LBRACE
        RBRACE
        COMMA
        TRX_BEGIN
        TRX_COMMIT
        TRX_ROLLBACK
        INT_T
        STRING_T
        FLOAT_T
        DATE_T
        TEXT_T
        THE_NULL
        HELP
        EXIT
        DOT //QUOTE
        INTO
        VALUES
        FROM
        WHERE
        SET
        ON
        LOAD
        DATA
        INFILE
        EXPLAIN


/** union 中定义各种数据类型，真实生成的代码也是union类型，所以不能有非POD类型的数据 **/
%union {
  ParsedSqlNode *                   sql_node;
  Value *                           value;
  RelAttrSqlNode *                  rel_attr;
  std::vector<AttrInfoSqlNode> *    attr_infos;
  AttrInfoSqlNode *                 attr_info;
  ExpressionSqlNode *               expression;
  std::vector<ExpressionSqlNode *> *expression_list;
  std::vector<Value> *              value_list;
  std::vector<std::string> *        relation_list;
  char *                            string;
  int                               number;
  float                             floats;
  bool                              booleans;
  ArithmeticType                    math_type;
}

%token <number> NUMBER
%token <floats> FLOAT
%token <string> ID
%token <string> SSS
//非终结符

/** type 定义了各种解析后的结果输出的是什么类型。类型对应了 union 中的定义的成员变量名称 **/
%type <number>              type
%type <value>               value
%type <number>              number
%type <rel_attr>            rel_attr
%type <attr_infos>          attr_def_list
%type <attr_info>           attr_def
%type <value_list>          value_list
%type <expression>          where
%type <relation_list>       rel_list
%type <expression>          expression
%type <expression_list>     expression_list
%type <sql_node>            calc_stmt
%type <sql_node>            select_stmt
%type <sql_node>            insert_stmt
%type <sql_node>            update_stmt
%type <sql_node>            delete_stmt
%type <sql_node>            create_table_stmt
%type <sql_node>            drop_table_stmt
%type <sql_node>            show_tables_stmt
%type <sql_node>            desc_table_stmt
%type <sql_node>            create_index_stmt
%type <sql_node>            drop_index_stmt
%type <sql_node>            sync_stmt
%type <sql_node>            begin_stmt
%type <sql_node>            commit_stmt
%type <sql_node>            rollback_stmt
%type <sql_node>            load_data_stmt
%type <sql_node>            explain_stmt
%type <sql_node>            set_variable_stmt
%type <sql_node>            help_stmt
%type <sql_node>            exit_stmt
%type <sql_node>            command_wrapper
// commands should be a list but I use a single command instead
%type <sql_node>            commands
%type <booleans>            opt_not

%nonassoc LIKE
%left OR
%left AND
%nonassoc NOT
%left LT GT LE GE EQ NE
%left ADD SUB
%left STAR DIV
%nonassoc UMINUS
%%

commands: command_wrapper opt_semicolon  //commands or sqls. parser starts here.
  {
    std::unique_ptr<ParsedSqlNode> sql_node = std::unique_ptr<ParsedSqlNode>($1);
    sql_result->add_sql_node(std::move(sql_node));
  }
  ;

command_wrapper:
    calc_stmt
  | select_stmt
  | insert_stmt
  | update_stmt
  | delete_stmt
  | create_table_stmt
  | drop_table_stmt
  | show_tables_stmt
  | desc_table_stmt
  | create_index_stmt
  | drop_index_stmt
  | sync_stmt
  | begin_stmt
  | commit_stmt
  | rollback_stmt
  | load_data_stmt
  | explain_stmt
  | set_variable_stmt
  | help_stmt
  | exit_stmt
    ;

exit_stmt:      
    EXIT {
      (void)yynerrs;  // 这么写为了消除yynerrs未使用的告警。如果你有更好的方法欢迎提PR
      $$ = new ParsedSqlNode(SCF_EXIT);
    };

help_stmt:
    HELP {
      $$ = new ParsedSqlNode(SCF_HELP);
    };

sync_stmt:
    SYNC {
      $$ = new ParsedSqlNode(SCF_SYNC);
    }
    ;

begin_stmt:
    TRX_BEGIN  {
      $$ = new ParsedSqlNode(SCF_BEGIN);
    }
    ;

commit_stmt:
    TRX_COMMIT {
      $$ = new ParsedSqlNode(SCF_COMMIT);
    }
    ;

rollback_stmt:
    TRX_ROLLBACK  {
      $$ = new ParsedSqlNode(SCF_ROLLBACK);
    }
    ;

drop_table_stmt:    /*drop table 语句的语法解析树*/
    DROP TABLE ID {
      $$ = new ParsedSqlNode(SCF_DROP_TABLE);
      $$->drop_table.relation_name = $3;
      free($3);
    };

show_tables_stmt:
    SHOW TABLES {
      $$ = new ParsedSqlNode(SCF_SHOW_TABLES);
    }
    ;

desc_table_stmt:
    DESC ID  {
      $$ = new ParsedSqlNode(SCF_DESC_TABLE);
      $$->desc_table.relation_name = $2;
      free($2);
    }
    ;

create_index_stmt:    /*create index 语句的语法解析树*/
    CREATE INDEX ID ON ID LBRACE ID RBRACE
    {
      $$ = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = $$->create_index;
      create_index.index_name = $3;
      create_index.relation_name = $5;
      create_index.attribute_name = $7;
      free($3);
      free($5);
      free($7);
    }
    ;

drop_index_stmt:      /*drop index 语句的语法解析树*/
    DROP INDEX ID ON ID
    {
      $$ = new ParsedSqlNode(SCF_DROP_INDEX);
      $$->drop_index.index_name = $3;
      $$->drop_index.relation_name = $5;
      free($3);
      free($5);
    }
    ;

create_table_stmt:    /*create table 语句的语法解析树*/
    CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE
    {
      $$ = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = $$->create_table;
      create_table.relation_name = $3;
      free($3);

      std::vector<AttrInfoSqlNode> *src_attrs = $6;

      if (src_attrs != nullptr) {
        create_table.attr_infos.swap(*src_attrs);
        delete src_attrs;
      }
      create_table.attr_infos.emplace_back(*$5);
      std::reverse(create_table.attr_infos.begin(), create_table.attr_infos.end());
      delete $5;
    }
    ;

attr_def_list:
    /* empty */
    {
      $$ = nullptr;
    }
    | COMMA attr_def attr_def_list
    {
      if ($3 != nullptr) {
        $$ = $3;
      } else {
        $$ = new std::vector<AttrInfoSqlNode>;
      }
      $$->emplace_back(*$2);
      delete $2;
    }
    ;
    
attr_def:
    ID type LBRACE number RBRACE 
    {
      $$ = new AttrInfoSqlNode;
      $$->type = (AttrType)$2;
      $$->name = $1;
      $$->length = $4;
      free($1);
    }
    | ID type
    {
      $$ = new AttrInfoSqlNode;
      $$->type = (AttrType)$2;
      $$->name = $1;
      $$->length = 4;   /*length 默认为4, 这一点和Mysql不一样*/
      free($1);
    }
    ;

number:
    NUMBER {$$ = $1;}
    ;

type:
    INT_T      { $$=INTS; }
    | STRING_T { $$=CHARS; }
    | FLOAT_T  { $$=FLOATS; }
    | TEXT_T   { $$=TEXTS; }
    | DATE_T   { $$=DATES; }
    ;

insert_stmt:        /*insert   语句的语法解析树*/
    INSERT INTO ID VALUES LBRACE value value_list RBRACE 
    {
      $$ = new ParsedSqlNode(SCF_INSERT);
      $$->insertion.relation_name = $3;
      if ($7 != nullptr) {
        $$->insertion.values.swap(*$7);
        delete $7;
      }
      $$->insertion.values.emplace_back(*$6);
      std::reverse($$->insertion.values.begin(), $$->insertion.values.end());
      delete $6;
      free($3);
    }
    ;

value_list:
    /* empty */
    {
      $$ = nullptr;
    }
    | COMMA value value_list  { 
      if ($3 != nullptr) {
        $$ = $3;
      } else {
        $$ = new std::vector<Value>;
      }
      $$->emplace_back(*$2);
      delete $2;
    }
    ;
    
value:
    NUMBER {
      $$ = new Value((int)$1);
      @$ = @1;
    }
    |FLOAT {
      $$ = new Value((float)$1);
      @$ = @1;
    }
    |SSS {
      char *tmp = common::substr($1,1,strlen($1)-2);
      $$ = new Value(tmp);
      free(tmp);
      free($1);
    }
    |THE_NULL {
      $$ = new Value();
      $$->set_null();
    }
    ;
    
delete_stmt:    /*  delete 语句的语法解析树*/
    DELETE FROM ID where 
    {
      $$ = new ParsedSqlNode(SCF_DELETE);
      $$->deletion.relation_name = $3;
      if ($4 != nullptr) {
        $$->deletion.condition = $4;
      }
      free($3);
    }
    ;
    
update_stmt:      /*  update 语句的语法解析树*/
    UPDATE ID SET ID EQ value where 
    {
      $$ = new ParsedSqlNode(SCF_UPDATE);
      $$->update.relation_name = $2;
      $$->update.attribute_name = $4;
      $$->update.value = *$6;
      if ($7 != nullptr) {
        $$->update.condition = $7;
        delete $7;
      }
      free($2);
      free($4);
    }
    ;

select_stmt:        /*  select 语句的语法解析树*/
    SELECT expression_list FROM ID rel_list where
    {
      $$ = new ParsedSqlNode(SCF_SELECT);
      if ($2 != nullptr) {
        $$->selection.attributes.swap(*$2);
        delete $2;
      }
      if ($5 != nullptr) {
        $$->selection.relations.swap(*$5);
        delete $5;
      }
      $$->selection.relations.push_back($4);
      std::reverse($$->selection.relations.begin(), $$->selection.relations.end());

      if ($6 != nullptr) {
        $$->selection.condition = $6;
      }
      free($4);
    }
    ;

calc_stmt:
    CALC expression_list
    {
      $$ = new ParsedSqlNode(SCF_CALC);
      $$->calc.expressions.swap(*$2);
      delete $2;
    }
    ;

expression_list:
    expression
    {
      $$ = new std::vector<ExpressionSqlNode *>;
      $$->emplace_back($1);
    }
    | expression COMMA expression_list
    {
      if ($3 != nullptr) {
        $$ = $3;
      } else {
        $$ = new std::vector<ExpressionSqlNode *>;
      }
      $$->insert($$->begin(), $1);
    }
    ;

expression:
    expression ADD expression {
      ArithmeticExpressionSqlNode* tmp = new ArithmeticExpressionSqlNode;
      tmp->left = $1;
      tmp->right = $3;
      tmp->arithmetic_type = ArithmeticType::ADD;

      $$ = tmp;
      $$->name = token_name(sql_string, &@$);
    }
    | expression SUB expression {
      ArithmeticExpressionSqlNode* tmp = new ArithmeticExpressionSqlNode;
      tmp->left = $1;
      tmp->right = $3;
      tmp->arithmetic_type = ArithmeticType::SUB;

      $$ = tmp;
      $$->name = token_name(sql_string, &@$);
    }
    | expression STAR expression {
      ArithmeticExpressionSqlNode* tmp = new ArithmeticExpressionSqlNode;
      tmp->left = $1;
      tmp->right = $3;
      tmp->arithmetic_type = ArithmeticType::MUL;

      $$ = tmp;
      $$->name = token_name(sql_string, &@$);
    }
    | expression DIV expression {
      ArithmeticExpressionSqlNode* tmp = new ArithmeticExpressionSqlNode;
      tmp->left = $1;
      tmp->right = $3;
      tmp->arithmetic_type = ArithmeticType::DIV;

      $$ = tmp;
      $$->name = token_name(sql_string, &@$);
    }
    | LBRACE expression RBRACE {
      $$ = $2;

      $$->name = token_name(sql_string, &@$);
    }
    | SUB expression %prec UMINUS {
      ArithmeticExpressionSqlNode* tmp = new ArithmeticExpressionSqlNode;
      tmp->right = $2;
      tmp->left = new ValueExpressionSqlNode;
      tmp->arithmetic_type = ArithmeticType::SUB;

      ((ValueExpressionSqlNode *)(tmp->left))->value = Value(0);

      $$ = tmp;
      $$->name = token_name(sql_string, &@$);
    }
    | expression EQ expression {
      ComparisonExpressionSqlNode *tmp = new ComparisonExpressionSqlNode;
      tmp->comp_op = EQUAL_TO;
      tmp->left = $1;
      tmp->right = $3;

      $$ = tmp;
      $$->name = (token_name(sql_string, &@$));
    }
    | expression LT expression {
      ComparisonExpressionSqlNode *tmp = new ComparisonExpressionSqlNode;
      tmp->comp_op = LESS_THAN;
      tmp->left = $1;
      tmp->right = $3;

      $$ = tmp;
      $$->name = (token_name(sql_string, &@$));
    }
    | expression GT expression {
      ComparisonExpressionSqlNode *tmp = new ComparisonExpressionSqlNode;
      tmp->comp_op = GREAT_THAN;
      tmp->left = $1;
      tmp->right = $3;

      $$ = tmp;
      $$->name = (token_name(sql_string, &@$));
    }
    | expression LE expression {
      ComparisonExpressionSqlNode *tmp = new ComparisonExpressionSqlNode;
      tmp->comp_op = LESS_EQUAL;
      tmp->left = $1;
      tmp->right = $3;

      $$ = tmp;
      $$->name = (token_name(sql_string, &@$));
    }
    | expression GE expression {
      ComparisonExpressionSqlNode *tmp = new ComparisonExpressionSqlNode;
      tmp->comp_op = GREAT_EQUAL;
      tmp->left = $1;
      tmp->right = $3;

      $$ = tmp;
      $$->name = (token_name(sql_string, &@$));
    }
    | expression NE expression {
      ComparisonExpressionSqlNode *tmp = new ComparisonExpressionSqlNode;
      tmp->comp_op = NOT_EQUAL;
      tmp->left = $1;
      tmp->right = $3;

      $$ = tmp;
      $$->name = (token_name(sql_string, &@$));
    }
    | expression AND expression {
      ConjunctionExpressionSqlNode *tmp = new ConjunctionExpressionSqlNode;
      tmp->left = $1;
      tmp->right = $3;
      tmp->conjunction_type = ConjunctionType::AND;
      
      $$ = tmp;
      $$->name = (token_name(sql_string, &@$));
    }
    | expression OR expression {
      ConjunctionExpressionSqlNode *tmp = new ConjunctionExpressionSqlNode;
      tmp->left = $1;
      tmp->right = $3;
      tmp->conjunction_type = ConjunctionType::OR;
      
      $$ = tmp;
      $$->name = (token_name(sql_string, &@$));
    }
    | expression opt_not LIKE SSS {
      LikeExpressionSqlNode *tmp = new LikeExpressionSqlNode;
      tmp->child = $1;
      char *dupped_str = common::substr($4,1,strlen($4)-2);
      tmp->pattern = dupped_str;
      free(dupped_str);
      free($4);

      $$ = tmp;

      if ($2) {
        NotExpressionSqlNode* not_p = new NotExpressionSqlNode;
        not_p->child = $$;
        $$ = not_p;
      }

      $$->name = (token_name(sql_string, &@$));
    }
    | NOT expression {
      NotExpressionSqlNode *tmp = new NotExpressionSqlNode;
      tmp->child = $2;

      $$ = tmp;
      $$->name = (token_name(sql_string, &@$));
    }
    | ID LBRACE expression_list RBRACE {
      FunctionExpressionSqlNode *tmp = new FunctionExpressionSqlNode;
      tmp->param_exprs.swap(*$3);
      tmp->function_name = $1;

      $$ = tmp;
      $$->name = (token_name(sql_string, &@$));
    }
    | value {
      ValueExpressionSqlNode *tmp = new ValueExpressionSqlNode;
      tmp->value = *$1;
      delete $1;

      $$ = tmp;
      $$->name = (token_name(sql_string, &@$));
    }
    | rel_attr {
      FieldExpressionSqlNode *tmp = new FieldExpressionSqlNode;
      tmp->field = *$1;
      delete $1;

      $$ = tmp;
      $$->name = (token_name(sql_string, &@$));
    }
    ;

rel_attr:
    ID {
      $$ = new RelAttrSqlNode;
      $$->relation_name = "";
      $$->attribute_name = $1;
      free($1);
    }
    | ID DOT ID {
      $$ = new RelAttrSqlNode;
      $$->relation_name  = $1;
      $$->attribute_name = $3;
      free($1);
      free($3);
    }
    | ID DOT STAR {
      $$ = new RelAttrSqlNode;
      $$->relation_name  = $1;
      $$->attribute_name = "*";
    }
    | STAR {
      $$ = new RelAttrSqlNode;
      $$->relation_name  = "";
      $$->attribute_name = "*";
    }
    ;

rel_list:
    /* empty */
    {
      $$ = nullptr;
    }
    | COMMA ID rel_list {
      if ($3 != nullptr) {
        $$ = $3;
      } else {
        $$ = new std::vector<std::string>;
      }

      $$->push_back($2);
      free($2);
    }
    ;
  
where:
    /* empty */
    {
      $$ = nullptr;
    }
    | WHERE expression {
      $$ = $2;  
    }
    ;



load_data_stmt:
    LOAD DATA INFILE SSS INTO TABLE ID 
    {
      char *tmp_file_name = common::substr($4, 1, strlen($4) - 2);
      
      $$ = new ParsedSqlNode(SCF_LOAD_DATA);
      $$->load_data.relation_name = $7;
      $$->load_data.file_name = tmp_file_name;
      free($7);
      free(tmp_file_name);
    }
    ;

explain_stmt:
    EXPLAIN command_wrapper
    {
      $$ = new ParsedSqlNode(SCF_EXPLAIN);
      $$->explain.sql_node = std::unique_ptr<ParsedSqlNode>($2);
    }
    ;

set_variable_stmt:
    SET ID EQ value
    {
      $$ = new ParsedSqlNode(SCF_SET_VARIABLE);
      $$->set_variable.name  = $2;
      $$->set_variable.value = *$4;
      free($2);
      delete $4;
    }
    ;

opt_semicolon: /*empty*/
    | SEMICOLON
    ;

opt_not: /*empty*/
    {$$ = false;}
    | NOT {$$ = true;}
    ;

%%
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
