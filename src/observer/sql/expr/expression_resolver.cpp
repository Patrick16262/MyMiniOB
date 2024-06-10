#include "expression_resolver.h"
#include "common/log/log.h"
#include "common/rc.h"
#include "sql/expr/expr_type.h"
#include "sql/expr/expression.h"
#include "sql/expr/expression_refactor.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/defs/expression_sql_defs.h"
#include "sql/parser/defs/sql_node_fwd.h"
#include "sql/parser/defs/sql_query_nodes.h"
#include "sql/parser/value.h"
#include "sql/stmt/table_ref_desc.h"
#include "storage/db/db.h"
#include <cassert>
#include <fcntl.h>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>
#include "sql/expr/function.h"

using namespace std;

/**
 * @brief 表达式生成器
 * @details 用于根据SQL的表达式节点，生成具体的表达式对象
 */
RC ExpressionGenerator::generate_expression(const ExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  switch (sql_node->expr_type) {
    case ExprType::FIELD: {
      return generate_expression(static_cast<const FieldExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::VALUE: {
      return generate_expression(static_cast<const ValueExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::CAST: {
      return generate_expression(static_cast<const CastExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::COMPARISON: {
      return generate_expression(static_cast<const ComparisonExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::CONJUNCTION: {
      return generate_expression(static_cast<const ConjunctionExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::ARITHMETIC: {
      return generate_expression(static_cast<const ArithmeticExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::FUNCTION: {
      return generate_expression(static_cast<const FunctionExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::NOT: {
      return generate_expression(static_cast<const NotExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::LIKE: {
      return generate_expression(static_cast<const LikeExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::CELL_REF: {
      return generate_expression(static_cast<const TupleCellExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::IN: {
      return generate_expression(static_cast<const InExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::EXISTS: {
      return generate_expression(static_cast<const ExistsExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::IS_NULL: {
      return generate_expression(static_cast<const IsNullExpressionSqlNode *>(sql_node), expr);
    } break;
    default: {
      LOG_WARN("unsupported expression type: %d", static_cast<int>(sql_node->expr_type));
      return RC::INVALID_ARGUMENT;
    } break;
  }
  return RC::INTERNAL;
}

/**
 * 生成一个ValueExpr
 */

RC ExpressionGenerator::generate_expression(const ValueExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  expr.reset(new ValueExpr(sql_node->value));
  expr->set_name(sql_node->name);
  return RC::SUCCESS;
}

/**
 * 生成一个FieldExpr
 * 传进来的attr_name不应为‘*’
 */

RC ExpressionGenerator::generate_expression(const FieldExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  string attr_name = sql_node->field.attribute_name;
  string rel_name  = sql_node->field.relation_name;  // maybe alias

  if (attr_name == "*") {
    LOG_WARN("attr_name should not be '*'");
    return RC::INVALID_ARGUMENT;
  }

  vector<TableFactorDesc> matched_tables;
  bool                    match_alias = outter_alias_set_.find(rel_name) != outter_alias_set_.end();

  auto p = field_table_map_.equal_range(attr_name);
  for (auto it = p.first; it != p.second; it++) {
    matched_tables.push_back(it->second);
  }

  if (match_alias) {
    expr.reset(new TupleCellExpr(TupleCellSpec(attr_name.c_str())));
    return RC::SUCCESS;
  }

  if (!db_) {
    LOG_WARN("table get is not allowed");
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  if (matched_tables.empty()) {
    LOG_WARN("field not found: %s", attr_name.c_str());
    return RC::SCHEMA_FIELD_MISSING;
  }
  if (rel_name.empty() && matched_tables.size() > 1) {
    LOG_WARN("ambiguous field name: %s", attr_name.c_str());
    return RC::SCHEMA_FIELD_AMBIGUOUS;
  }
  if (rel_name.empty() && matched_tables.size() == 1) {
    string table_name = matched_tables[0].table_name();
    Table *table      = db_->find_table(table_name.c_str());
    expr.reset(new FieldExpr(table, table->table_meta().field(attr_name.c_str())));
    return RC::SUCCESS;
  }
  for (auto &table_desc : matched_tables) {
    if (table_desc.table_name() == rel_name) {
      Table *table = db_->find_table(table_desc.table_name().c_str());
      expr.reset(new FieldExpr(table, table->table_meta().field(attr_name.c_str())));
      return RC::SUCCESS;
    }
  }

  LOG_WARN("field not found: %s.%s", rel_name.c_str(), attr_name.c_str());
  return RC::SCHEMA_TABLE_NOT_EXIST;
}

/**
 * 生成一个ArithmeticExpr
 */

RC ExpressionGenerator::generate_expression(
    const ArithmeticExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  std::unique_ptr<Expression> left_expr;
  std::unique_ptr<Expression> right_expr;

  RC rc = generate_expression(sql_node->left, left_expr);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate left expression failed");
    return rc;
  }

  rc = generate_expression(sql_node->right, right_expr);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate right expression failed");
    return rc;
  }

  expr.reset(new ArithmeticExpr(sql_node->arithmetic_type, std::move(left_expr), std::move(right_expr)));
  expr->set_name(sql_node->name);
  return RC::SUCCESS;
}

/**
 *  生成一个ComparisonExpr
 */

RC ExpressionGenerator::generate_expression(
    const ComparisonExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  std::unique_ptr<Expression> left_expr;
  std::unique_ptr<Expression> right_expr;
  std::unique_ptr<Expression> cast_expr;

  RC rc = generate_expression(sql_node->left, left_expr);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate left expression failed");
    return rc;
  }

  rc = generate_expression(sql_node->right, right_expr);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate right expression failed");
    return rc;
  }

  // 对日期类型字符串进行隐式转换
  // 这里实现的和mysql有点不一样
  // 在mysql中，如果日期格式不正确，会将其转换为float类型，这里直接报错
  // 但在MySQL中只有date的值不合法时才会报错
  if (left_expr->value_type() == AttrType::DATES &&
      (right_expr->value_type() == AttrType::CHARS || right_expr->value_type() == AttrType::TEXTS)) {
    cast_expr.reset(new CastExpr(std::move(right_expr), AttrType::DATES));
    Value test_value;
    rc = cast_expr->try_get_value(test_value);
    if (rc == RC::SUCCESS && test_value.attr_type() == AttrType::NULLS) {
      LOG_WARN("Incorrect DATE value");
      return RC::INCORRECT_DATE_VALUE;
    }

    right_expr = std::move(cast_expr);
  }

  if (right_expr->value_type() == AttrType::DATES &&
      (left_expr->value_type() == AttrType::CHARS || left_expr->value_type() == AttrType::TEXTS)) {
    cast_expr.reset(new CastExpr(std::move(left_expr), AttrType::DATES));
    Value test_value;
    rc = cast_expr->try_get_value(test_value);
    if (rc == RC::SUCCESS && test_value.attr_type() == AttrType::NULLS) {
      LOG_WARN("Incorrect DATE value");
      return RC::INCORRECT_DATE_VALUE;
    }

    left_expr = std::move(cast_expr);
  }

  expr.reset(new ComparisonExpr(sql_node->comp_op, std::move(left_expr), std::move(right_expr)));
  expr->set_name(sql_node->name);
  return RC::SUCCESS;
}

/**
 * 生成一个ConjunctionExpr
 */

RC ExpressionGenerator::generate_expression(
    const ConjunctionExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  std::unique_ptr<Expression> left_expr;
  std::unique_ptr<Expression> right_expr;

  RC rc = generate_expression(sql_node->left, left_expr);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate left expression failed");
    return rc;
  }

  rc = generate_expression(sql_node->right, right_expr);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate right expression failed");
    return rc;
  }

  vector<unique_ptr<Expression>> children;
  children.push_back(std::move(left_expr));
  children.push_back(std::move(right_expr));
  expr.reset(new ConjunctionExpr(sql_node->conjunction_type, children));
  expr->set_name(sql_node->name);
  return RC::SUCCESS;
}

/**
 * @brief 生成一个CastExpr
 */

RC ExpressionGenerator::generate_expression(const CastExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  std::unique_ptr<Expression> child;

  RC rc = generate_expression(sql_node->child, child);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate child expression failed");
    return rc;
  }

  expr.reset(new CastExpr(std::move(child), sql_node->target_type));
  expr->set_name(sql_node->name);
  return RC::SUCCESS;
}

/**
 * @brief 生成like表达式
 *
 */
RC ExpressionGenerator::generate_expression(const LikeExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  std::unique_ptr<Expression> child;

  RC rc = generate_expression(sql_node->child, child);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate child expression failed");
    return rc;
  }

  expr.reset(new LikeExpr(sql_node->pattern, std::move(child)));
  expr->set_name(sql_node->name);
  return RC::SUCCESS;
}

/**
 * @brief 生成NotExpr
 *
 */
RC ExpressionGenerator::generate_expression(const NotExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  unique_ptr<Expression> child;

  RC rc = generate_expression(sql_node->child, child);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate child expression failed");
    return rc;
  }

  expr.reset(new NotExpr(std::move(child)));
  expr->set_name(sql_node->name);
  return RC::SUCCESS;
}

RC ExpressionGenerator::generate_expression(
    const FunctionExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  RC                             rc;
  vector<unique_ptr<Expression>> param_exprs;
  unique_ptr<FunctionExpression> function_expression;

  FunctionType type = common::get_function_type(sql_node->function_name.c_str());

  if (type == FunctionType::INVALID) {
    LOG_WARN("unknown function: %s", sql_node->function_name.c_str());
    return RC::INVALID_ARGUMENT;
  }

  // 生成参数表达式
  for (auto *param : sql_node->param_exprs) {
    unique_ptr<Expression> param_expr;
    rc = generate_expression(param, param_expr);
    if (rc != RC::SUCCESS) {
      LOG_WARN("generate param expression failed");
      return rc;
    }
    param_exprs.push_back(std::move(param_expr));
  }

  // 根据函数类型生成具体的函数表达式
  switch (type) {
    case FunctionType::LENGTH: {
      function_expression.reset(new LengthFunction(param_exprs));
    } break;
    case FunctionType::ROUND: {
      function_expression.reset(new RoundFunction(param_exprs));
    } break;
    case FunctionType::DATE_FORMAT: {
      function_expression.reset(new DateFormatFunction(param_exprs));
    } break;
    default: {
      LOG_WARN("unknown function type: %d", static_cast<int>(type));
      return RC::UNIMPLENMENT;
    } break;
  }

  // 检查参数是否合法
  if (function_expression->check_params() != RC::SUCCESS) {
    LOG_WARN("check params failed");
    return RC::INVALID_ARGUMENT;
  }

  // 一切正常，返回函数表达式
  expr.reset(function_expression.release());
  expr->set_name(sql_node->name);
  return RC::SUCCESS;
}

RC ExpressionGenerator::generate_expression(
    const TupleCellExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  expr.reset(new TupleCellExpr(
      TupleCellSpec(sql_node->table_name.c_str(), sql_node->field_name.c_str(), sql_node->alias.c_str())));
  expr->set_name(sql_node->name);
  return RC::SUCCESS;
}

RC ExpressionGenerator::generate_expression(const IsNullExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  std::unique_ptr<Expression> child;

  RC rc = generate_expression(sql_node->child, child);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate child expression failed");
    return rc;
  }

  expr.reset(new IsNullExpr(std::move(child)));
  expr->set_name(sql_node->name);
  return RC::SUCCESS;
}

RC ExpressionGenerator::generate_expression(const InExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  std::unique_ptr<Expression>         child    = nullptr;
  std::unique_ptr<Expression>         subquery = nullptr;  // 如果是子查询，这个字段不为空，反正为空
  std::vector<unique_ptr<Expression>> value_list;

  RC rc = generate_expression(sql_node->child, child);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate left expression failed");
    return rc;
  }

  if (sql_node->subquery) {
    assert(sql_node->value_list.size() == 0);
    rc = generate_expression(sql_node->subquery, subquery);
    if (rc != RC::SUCCESS) {
      LOG_WARN("generate subquery expression failed");
      return rc;
    }

    expr.reset(new InExpr(std::move(child), std::move(subquery)));
    expr->set_name(sql_node->name);
    return RC::SUCCESS;
  } else {
    for (auto *value : sql_node->value_list) {
      unique_ptr<Expression> value_expr;
      rc = generate_expression(value, value_expr);
      if (rc != RC::SUCCESS) {
        LOG_WARN("generate value expression failed");
        return rc;
      }
      value_list.push_back(std::move(value_expr));
    }

    expr.reset(new InExpr(std::move(child), value_list));
    expr->set_name(sql_node->name);
    return RC::SUCCESS;
  }
}

RC ExpressionGenerator::generate_expression(const ExistsExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  assert(false);
  return RC::UNIMPLENMENT;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
RC ConstExpressionResovler::resolve(const ExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  return generator_.generate_expression(sql_node, expr);
}

RC JoinConditionExpressionResolver::resolve(const ExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  return generator_.generate_expression(sql_node, expr);
}

RC WhereConditionExpressionResolver::resolve(ExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  RC rc;

  rc = refactor_.refactor(sql_node);
  if (rc != RC::SUCCESS) {
    LOG_WARN("refactor expression failed");
    return rc;
  }

  if (refactor_.aggregate_childs().size() != 0) {
    LOG_WARN("aggregate function not allowed in where condition");
    return RC::INVALID_AGGREGATE;
  }

  rc = generator_.generate_expression(sql_node, expr);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate expression failed");
    return rc;
  }

  return RC::SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 查询列表生成器
 */
RC ProjectExpressionResovler::generate_query_list(
    const vector<ExpressionWithAliasSqlNode *> &sql_nodes, vector<unique_ptr<Expression>> &query_exprs)
{
  vector<unique_ptr<Expression>> tmp_exprs;
  FieldExpressionSqlNode        *tmp_wildcard_expression = nullptr;
  RC                             rc;

  query_exprs.clear();
  for (auto *sql_node : sql_nodes) {
    if (sql_node->expr->expr_type == ExprType::FIELD &&
        (tmp_wildcard_expression = static_cast<FieldExpressionSqlNode *>(sql_node->expr))->field.attribute_name ==
            "*") {

      // 如果是通配符的话
      rc = wildcard_fields(tmp_wildcard_expression, tmp_exprs);
      if (rc != RC::SUCCESS) {
        LOG_WARN("wildcard fields failed");
        return rc;
      }

      query_exprs.insert(
          query_exprs.end(), std::make_move_iterator(tmp_exprs.begin()), std::make_move_iterator(tmp_exprs.end()));
      tmp_exprs.clear();
    } else {

      // 普通的表达式
      unique_ptr<Expression>   expr;
      ExpressionStructRefactor refactor;
      refactor.refactor(sql_node->expr);
      rc = generator_.generate_expression(sql_node->expr, expr);
      if (rc != RC::SUCCESS) {
        LOG_WARN("generate expression failed");
        return rc;
      }

      query_exprs.push_back(std::move(expr));
      subquery_cell_desc_.insert(
          subquery_cell_desc_.end(), refactor.subquery_cells().begin(), refactor.subquery_cells().end());
      subquery_types_.insert(subquery_types_.end(), refactor.subquery_types().begin(), refactor.subquery_types().end());
      subquerys_.insert(subquerys_.end(),
          std::make_move_iterator(refactor.subqueries().begin()),
          std::make_move_iterator(refactor.subqueries().end()));

      // 生成tuple_schema
      FieldExpressionSqlNode *field = dynamic_cast<FieldExpressionSqlNode *>(sql_node->expr);
      if (field) {
        attr_tuple_.emplace_back(
            field->field.relation_name.c_str(), field->field.attribute_name.c_str(), sql_node->alias.c_str());
      } else {
        if (sql_node->alias.c_str()) {
          attr_tuple_.emplace_back(sql_node->alias.c_str());
        } else {
          attr_tuple_.emplace_back(sql_node->expr->name.c_str());
        }
      }
    }
  }

  return RC::SUCCESS;
}

/**
 * @brief 通配符字段处理
 */
RC ProjectExpressionResovler::wildcard_fields(
    FieldExpressionSqlNode *wildcard_expression, vector<unique_ptr<Expression>> &query_exprs)
{
  query_exprs.clear();
  RC rc;

  if (wildcard_expression->field.relation_name.empty()) {
    // 如果是所有表的通配符
    for (auto &table_desc : table_desc_) {
      for (auto &field : table_desc.fields()) {
#ifndef SHOW_HIDDEN_FIELDS
        if (!field.visible()) {
          continue;
        }
#endif
        FieldExpressionSqlNode *temp_node = new FieldExpressionSqlNode;
        unique_ptr<Expression>  expr;
        temp_node->field.attribute_name = field.field_name();
        temp_node->field.relation_name  = table_desc.table_name();
        temp_node->name                 = field.field_name();
        rc = generator_.generate_expression(static_cast<ExpressionSqlNode *>(temp_node), expr);
        if (rc != RC::SUCCESS) {
          LOG_WARN("generate wildcard expression failed, rc=%d:%s", rc, strrc(rc));
          return rc;
        }

        query_exprs.push_back(std::move(expr));
        delete temp_node;

        attr_tuple_.emplace_back(table_desc.table_name().c_str(), field.field_name().c_str());
      }
    }
  } else {
    // 如果是指定表的通配符
    for (auto &table_desc : table_desc_) {
      if (table_desc.table_name() == wildcard_expression->field.relation_name) {
        for (auto &field : table_desc.fields()) {
#ifndef SHOW_HIDDEN_FIELDS
          if (!field.visible()) {
            continue;
          }
#endif
          FieldExpressionSqlNode *temp_node = new FieldExpressionSqlNode;
          unique_ptr<Expression>  expr;
          temp_node->field.attribute_name = field.field_name();
          temp_node->field.relation_name  = table_desc.table_name();
          temp_node->name                 = field.field_name();
          rc = generator_.generate_expression(static_cast<ExpressionSqlNode *>(temp_node), expr);
          if (rc != RC::SUCCESS) {
            LOG_WARN("generate wildcard expression failed, rc=%d:%s", rc, strrc(rc));
          }

          query_exprs.push_back(std::move(expr));
        }
        return RC::SUCCESS;
      }
    }
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  return RC::SUCCESS;
}