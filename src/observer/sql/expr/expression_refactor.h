#pragma once

#include "common/log/log.h"
#include "sql/expr/expr_type.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/defs/expression_sql_defs.h"
#include <functional>

/**
 * @brief 对表达式结构进行重构
 *
 */
class ExpressionStructRefactor
{
public:
  RC refactor_expression(ExpressionSqlNode *&sql_node);

private:
  RC refactor_expression(ValueExpressionSqlNode *&sql_node);
  RC refactor_expression(FieldExpressionSqlNode *&sql_node);
  RC refactor_expression(ArithmeticExpressionSqlNode *&sql_node);
  RC refactor_expression(ComparisonExpressionSqlNode *&sql_node);
  RC refactor_expression(ConjunctionExpressionSqlNode *&sql_node);
  RC refactor_expression(CastExpressionSqlNode *&sql_node);
  RC refactor_expression(LikeExpressionSqlNode *&sql_node);
  RC refactor_expression(NotExpressionSqlNode *&sql_node);
  RC refactor_expression(FunctionExpressionSqlNode *&sql_node);
  RC refactor_expression(AggregateExpressionSqlNode *&sql_node);

private:
  std::function<RC(AggregateType, ExpressionSqlNode *child_sql_node, TupleCellSpec aggregate_cell)> on_aggregate_found_callback_ =
      [](AggregateType, ExpressionSqlNode *, TupleCellSpec) {
        LOG_WARN("Aggregate function is not allowed here");
        return RC::INVALID_AGGREGATE;
      }; 
  std::function<RC()>

};