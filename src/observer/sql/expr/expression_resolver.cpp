#include "expression_resolver.h"
#include "common/log/log.h"
#include "common/rc.h"
#include "sql/expr/expr_type.h"
#include "sql/expr/expression.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/defs/expression_sql_defs.h"
#include "sql/parser/defs/sql_node_fwd.h"
#include "sql/parser/value.h"
#include "storage/field/field_meta.h"
#include <fcntl.h>
#include <map>
#include <memory>
#include <sstream>
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
  // 找到group by的表达式，将其转换为聚合函数
  for (auto *group_by_expr : group_by_exprs_) {
    if (*group_by_expr == *sql_node) {
      AggregateExpressionSqlNode *tmp = new AggregateExpressionSqlNode;
      tmp->aggregate_type             = AggregateType::GROUP;
      tmp->child                      = const_cast<ExpressionSqlNode *>(sql_node);
      tmp->name                       = sql_node->name;
      sql_node                        = tmp;
      break;
    }
  }

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
    case ExprType::AGGREGATE: {
      return generate_expression(static_cast<const AggregateExpressionSqlNode *>(sql_node), expr);
    } break;
    case ExprType::SUBQUERY: {
      return RC::UNIMPLENMENT;
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
    default: {
      LOG_WARN("unknown expression type: %d", static_cast<int>(sql_node->expr_type));
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
  if (query_tables_.empty()) {
    LOG_WARN("query tables is empty");
    return RC::SCHEMA_FIELD_MISSING;
  }

  const RelAttrSqlNode &attr  = sql_node->field;
  const Table          *table = nullptr;
  const FieldMeta      *field = nullptr;

  if (attr.attribute_name == "*") {
    LOG_WARN("field name is *, which might have not been resovled");
    return RC::INVALID_ARGUMENT;
  }

  if (attr.relation_name.empty()) {
    if (default_table_ == nullptr) {
      // 如果没有指定表，则使用默认表, 如果默认表也为空，则报错
      LOG_WARN("default table is nullptr");
      return RC::SCHEMA_FIELD_MISSING;
    }
    table = default_table_;
  } else {
    auto it = query_tables_.find(attr.relation_name.c_str());
    if (it == query_tables_.end()) {
      LOG_WARN("table not exist: %s", attr.relation_name.c_str());
      return RC::SCHEMA_FIELD_MISSING;
    }
    table = it->second;
  }

  field = table->table_meta().field(attr.attribute_name.c_str());
  if (field == nullptr) {
    LOG_WARN("field not exist: %s", attr.attribute_name.c_str());
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  expr.reset(new FieldExpr(table, field));
  expr->set_name(sql_node->name);
  return RC::SUCCESS;
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
  // 只有date的值不合法时mysql才会报错
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
  RC                               rc;
  vector<unique_ptr<Expression>>   param_exprs;
  unique_ptr<FunctionExpression>   function_expression;
  static map<string, FunctionType> function_name_map{
      {"length", FunctionType::LENGTH}, {"round", FunctionType::ROUND}, {"date_format", FunctionType::DATE_FORMAT}};
  static map<string, AggregateType> aggregate_name_map{{"count", AggregateType::COUNT},
      {"sum", AggregateType::SUM},
      {"avg", AggregateType::AVG},
      {"max", AggregateType::MAX},
      {"min", AggregateType::MIN}};

  // 转换函数名为小写
  stringstream lower_case_name;
  for (auto c : sql_node->function_name) {
    lower_case_name << static_cast<char>(tolower(c));
  }

  // 判断是什么函数
  auto it = function_name_map.find(lower_case_name.str());
  if (it == function_name_map.end()) {
    auto it = aggregate_name_map.find(lower_case_name.str());
    if (it == aggregate_name_map.end()) {
      LOG_WARN("unknown function name: %s", sql_node->function_name.c_str());
      return RC::INVALID_FUNCTION_NAME;
    }

    // 发现聚合函数
    LOG_INFO("found aggregate function: %s", sql_node->function_name.c_str());

    AggregateExpressionSqlNode *aggregate_sql_node = new AggregateExpressionSqlNode();
    aggregate_sql_node->aggregate_type             = it->second;
    if (sql_node->param_exprs.size() != 1) {
      LOG_WARN("invalid number of parameters for aggregate function: %s, expected 1, got %ld",
       sql_node->function_name.c_str(), sql_node->param_exprs.size());
      return RC::INVALID_ARGUMENT;
    }

    aggregate_sql_node->child = sql_node->param_exprs[0];
    rc                        = generate_expression(aggregate_sql_node, expr);
    if (rc != RC::SUCCESS) {
      LOG_WARN("generate aggregate expression failed");
      return rc;
    }

    expr->set_name(sql_node->name);
    return RC::SUCCESS;
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
  switch (it->second) {
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
      LOG_WARN("unknown function type: %d", static_cast<int>(it->second));
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
    const AggregateExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr)
{
  RC rc;
  unique_ptr<Expression>  child_expr     = nullptr;
  unique_ptr<CellRefExpr> cell_expr      = nullptr;
  FieldExpressionSqlNode *tmp_field_node = nullptr;
  TupleCellSpec           aggregate_cell(sql_node->name.c_str());  // TODO 应该用一个更好的名字

  // 对count(*)进行特殊处理
  if (sql_node->aggregate_type == AggregateType::COUNT && sql_node->child->expr_type == ExprType::FIELD &&
      (tmp_field_node = static_cast<FieldExpressionSqlNode *>(sql_node->child))->field.attribute_name == "*") {
    // MySQL中count(*)不能带表名
    if (!tmp_field_node->field.relation_name.empty()) {
      LOG_WARN("count(*) should not have table name");
      return RC::INVALID_AGGREGATE;
    }

    //生成一个ValueExpressionSqlNode给count(*)用
    //在mysql中，count(1)和count(*)是等价的
    auto tmp = new ValueExpressionSqlNode;
    tmp->value = Value(1);
    const_cast<AggregateExpressionSqlNode *>(sql_node)->child = tmp; // 不是很规范0o0
  }

  // 生成聚合函数的参数表达式
  rc = generate_expression(sql_node->child, child_expr);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate child expression failed, rc %d: %s", static_cast<int>(rc), strrc(rc));
    return rc;
  }

  rc = on_aggregate_found_callback_(sql_node->aggregate_type, std::move(child_expr), aggregate_cell);
  if (rc != RC::SUCCESS) {
    LOG_WARN("on aggregate found callback failed, rc %d: %s", static_cast<int>(rc), strrc(rc));
    return rc;
  }

  switch (sql_node->aggregate_type) {
    case AggregateType::COUNT: {
      expr.reset(new CellRefExpr(aggregate_cell, AttrType::INTS));
    } break;
    case AggregateType::AVG:
    case AggregateType::MAX:
    case AggregateType::MIN:
    case AggregateType::SUM: {
      expr.reset(new CellRefExpr(aggregate_cell, AttrType::FLOATS));
    } break;
    case AggregateType::GROUP: {
      expr.reset(new CellRefExpr(aggregate_cell, child_expr->value_type()));
    } break;
  }

  expr->set_name(sql_node->name);
  return RC::SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 查询列表生成器
 */
RC QueryListGenerator::generate_query_list(
    const vector<ExpressionSqlNode *> &sql_nodes, vector<unique_ptr<Expression>> &query_exprs)
{
  vector<unique_ptr<Expression>> tmp_exprs;
  FieldExpressionSqlNode        *tmp_wildcard_expression = nullptr;
  RC                             rc;

  query_exprs.clear();
  for (auto *sql_node : sql_nodes) {

    if (sql_node->expr_type == ExprType::FIELD &&
        (tmp_wildcard_expression = static_cast<FieldExpressionSqlNode *>(sql_node))->field.attribute_name == "*") {
      // 如果是通配符的话
      tmp_exprs.clear();

      rc = wildcard_fields(tmp_wildcard_expression, tmp_exprs);
      if (rc != RC::SUCCESS) {
        LOG_WARN("wildcard fields failed");
        return rc;
      }

      query_exprs.insert(
          query_exprs.end(), std::make_move_iterator(tmp_exprs.begin()), std::make_move_iterator(tmp_exprs.end()));

    } else {
      // 普通的表达式
      unique_ptr<Expression> expr;
      rc = generator_.generate_expression(sql_node, expr);

      if (rc != RC::SUCCESS) {
        LOG_WARN("generate expression failed");
        return rc;
      }

      query_exprs.push_back(std::move(expr));
    }
  }

  return RC::SUCCESS;
}

/**
 * @brief 通配符字段处理
 */
RC QueryListGenerator::wildcard_fields(
    FieldExpressionSqlNode *wildcard_expression, vector<unique_ptr<Expression>> &query_exprs)
{
  query_exprs.clear();

  if (wildcard_expression->field.relation_name.empty()) {
    // 如果是所有表的通配符
    for (auto &table_pair : query_tables_) {
      for (auto &field : *table_pair.second->table_meta().field_metas()) {
#ifndef SHOW_HIDDEN_FIELDS
        if (!field.visible()) {
          continue;
        }
#endif
        FieldExpr *expr = new FieldExpr(table_pair.second, &field);
        expr->set_name(field.name());
        query_exprs.emplace_back(expr);
      }
    }
  } else {
    // 如果是指定表的通配符
    auto it = query_tables_.find(wildcard_expression->field.relation_name.c_str());
    if (it == query_tables_.end()) {
      LOG_WARN("table not exist: %s", wildcard_expression->field.relation_name.c_str());
      return RC::SCHEMA_FIELD_MISSING;
    }
    Table *table = it->second;

    for (auto &field : *table->table_meta().field_metas()) {
      query_exprs.push_back(std::make_unique<FieldExpr>(table, &field));
      query_exprs.rbegin()->get()->set_name(field.name());
    }
  }

  return RC::SUCCESS;
}
