#include "expression_refactor.h"
#include "common/log/log.h"
#include "sql/parser/defs/expression_sql_defs.h"


RC ExpressionStructRefactor::refactor_expression(ValueExpressionSqlNode *&sql_node) { return RC::SUCCESS; }
RC ExpressionStructRefactor::refactor_expression(FieldExpressionSqlNode *&sql_node) { return RC::SUCCESS; }
RC ExpressionStructRefactor::refactor_expression(ArithmeticExpressionSqlNode *&sql_node)
{
  RC rc;
  rc = refactor_expression(sql_node->left);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to refactor left expression");
    return rc;
  }

  rc = refactor_expression(sql_node->right);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to refactor right expression");
    return rc;
  }

  return RC::SUCCESS;
}
RC ExpressionStructRefactor::refactor_expression(ComparisonExpressionSqlNode *&sql_node)
{
  RC rc;
  rc = refactor_expression(sql_node->left);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to refactor left expression");
    return rc;
  }

  rc = refactor_expression(sql_node->right);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to refactor right expression");
    return rc;
  }

  return RC::SUCCESS;
}
RC ExpressionStructRefactor::refactor_expression(ConjunctionExpressionSqlNode *&sql_node)
{
  RC rc;
  rc = refactor_expression(sql_node->left);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to refactor left expression");
    return rc;
  }

  rc = refactor_expression(sql_node->right);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to refactor right expression");
    return rc;
  }

  return RC::SUCCESS;
}
RC ExpressionStructRefactor::refactor_expression(CastExpressionSqlNode *&sql_node)
{
  RC rc = refactor_expression(sql_node->child);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to refactor expression");
    return rc;
  }

  return RC::SUCCESS;
}
RC ExpressionStructRefactor::refactor_expression(LikeExpressionSqlNode *&sql_node)
{
  RC rc;
  rc = refactor_expression(sql_node->child);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to refactor left e xpression");
    return rc;
  }

  return RC::SUCCESS;
}
RC ExpressionStructRefactor::refactor_expression(NotExpressionSqlNode *&sql_node)
{
  RC rc = refactor_expression(sql_node->child);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to refactor expression");
    return rc;
  }

  return RC::SUCCESS;
}
RC ExpressionStructRefactor::refactor_expression(FunctionExpressionSqlNode *&sql_node)
{

  for (auto &arg : sql_node->param_exprs) {
    RC rc = refactor_expression(arg);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to refactor expression");
      return rc;
    }
  }

  return RC::SUCCESS;
}
RC ExpressionStructRefactor::refactor_expression(AggregateExpressionSqlNode *&sql_node)
{
  RC rc = refactor_expression(sql_node->child);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to refactor expression");
    return rc;
  }

  return RC::SUCCESS;
}
