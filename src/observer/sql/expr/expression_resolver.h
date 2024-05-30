#pragma once

#include "common/rc.h"
#include "sql/expr/expression.h"
#include "sql/parser/defs/expression_sql_defs.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class ExpressionGenerator
{
public:
  ExpressionGenerator() = default;
  ExpressionGenerator(Table *default_table_, unordered_map<std::string, Table *> &query_tables)
      : default_table_(default_table_), query_tables_(query_tables)
  {}
  virtual ~ExpressionGenerator() = default;

  RC generate_expression(const ExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const ValueExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const FieldExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const ArithmeticExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const ComparisonExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const ConjunctionExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const CastExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const LikeExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);

private:
  Table                              *default_table_ = nullptr;
  unordered_map<std::string, Table *> query_tables_;
};

class QueryListGenerator
{
public:
  QueryListGenerator() = default;
  QueryListGenerator(Table *default_table, unordered_map<std::string, Table *> &query_tables)
      :  query_tables_(query_tables), generator_(default_table, query_tables)
  {}
  virtual ~QueryListGenerator() = default;

  RC generate_query_list(const vector<ExpressionSqlNode *> &sql_nodes, vector<unique_ptr<Expression>> &query_exprs);

private:
  RC wildcard_fields(FieldExpressionSqlNode *wildcard_expression, vector<unique_ptr<Expression>> &query_exprs);

private:
  unordered_map<std::string, Table *> query_tables_;
  ExpressionGenerator                 generator_;
};