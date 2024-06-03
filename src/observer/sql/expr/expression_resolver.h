#pragma once

#include "common/log/log.h"
#include "common/rc.h"
#include "sql/expr/expr_type.h"
#include "sql/expr/expression.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/defs/expression_sql_defs.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include <cassert>
#include <functional>
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
  RC generate_expression(const NotExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const FunctionExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const AggregateExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);

  // 将模式设置为带有group by从句
  void set_resovle_mode_group(const vector<ExpressionSqlNode *> &group_by_exprs)
  {
    group_by_exprs_.insert(group_by_exprs_.end(), group_by_exprs.begin(), group_by_exprs.end());
  }

  void set_on_aggregate_found(
      const std::function<RC(AggregateType type, std::unique_ptr<Expression> child_expr, TupleCellSpec aggregate_cell)>
          &on_aggregate_found_callback)
  {
    on_aggregate_found_callback_ = on_aggregate_found_callback;
  }

private:
  std::function<RC(AggregateType type, std::unique_ptr<Expression> child_expr, TupleCellSpec aggregate_cell)>
      on_aggregate_found_callback_ = [](AggregateType, std::unique_ptr<Expression>, TupleCellSpec) {
        LOG_WARN("Aggregate function is not allowed here");
        return RC::INVALID_AGGREGATE;
      };

private:
  Table                              *default_table_ = nullptr;
  unordered_map<std::string, Table *> query_tables_;

  vector<ExpressionSqlNode *> group_by_exprs_;
};

// 当前暂不支持聚合函数表达式和常量表达式混合使用
class QueryListGenerator
{
public:
  QueryListGenerator() = default;
  QueryListGenerator(Table *default_table, unordered_map<std::string, Table *> &query_tables)
      : query_tables_(query_tables), generator_(default_table, query_tables)
  {}
  virtual ~QueryListGenerator() = default;

  RC generate_query_list(const vector<ExpressionSqlNode *> &sql_nodes, vector<unique_ptr<Expression>> &query_exprs);

  void set_group_by(const vector<ExpressionSqlNode *> &group_by_exprs)
  {
    generator_.set_resovle_mode_group(group_by_exprs);
  }

  void set_on_aggregate_found(
      const std::function<RC(AggregateType type, std::unique_ptr<Expression> child_expr, TupleCellSpec aggregate_cell)>
          &on_aggregate_found_callback)
  {
    generator_.set_on_aggregate_found(on_aggregate_found_callback);
  }

private:
  RC wildcard_fields(FieldExpressionSqlNode *wildcard_expression, vector<unique_ptr<Expression>> &query_exprs);

private:
  unordered_map<std::string, Table *> query_tables_;
  ExpressionGenerator                 generator_;
};