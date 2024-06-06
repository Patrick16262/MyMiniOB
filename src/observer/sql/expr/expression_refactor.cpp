#include "expression_refactor.h"
#include "common/rc.h"
#include "sql/expr/expr_type.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/defs/expression_sql_defs.h"
#include "sql/parser/defs/sql_query_nodes.h"
#include <cassert>
#include <cstddef>
#include <vector>
#include "common/log/log.h"
#include "sql/parser/parse_defs.h"

using namespace std;

RC ExpressionStructValidator::validate(const ExpressionSqlNode *sql_node)
{
  allow_aggregate     = true;
  allow_non_aggregate = true;
  return validate(sql_node, {false});
}

RC ExpressionStructValidator::validate(const ExpressionSqlNode *sql_node, State state)
{
  switch (sql_node->expr_type) {
    case ExprType::FIELD: {
      if (!allow_non_aggregate && !state.in_aggregate) {
        return RC::INVALID_AGGREGATE;
      }
      allow_aggregate = false;
      return RC::SUCCESS;
    } break;
    case ExprType::VALUE: {
      return RC::SUCCESS;
    } break;
    case ExprType::CAST: {
      const CastExpressionSqlNode *cast_node = static_cast<const CastExpressionSqlNode *>(sql_node);
      return validate(cast_node->child, {state.in_aggregate, state.allow_multiple_col});
    } break;
    case ExprType::COMPARISON: {
      const ComparisonExpressionSqlNode *comparison_node = static_cast<const ComparisonExpressionSqlNode *>(sql_node);
      RC                                 rc              = validate(comparison_node->left, {state.in_aggregate});
      if (rc != RC::SUCCESS) {
        return rc;
      }
      return validate(comparison_node->right, {state.in_aggregate});
    } break;
    case ExprType::CONJUNCTION: {
      const ConjunctionExpressionSqlNode *conjunction_node =
          static_cast<const ConjunctionExpressionSqlNode *>(sql_node);
      RC rc = validate(conjunction_node->left, {state.in_aggregate});
      if (rc != RC::SUCCESS) {
        return rc;
      }
      return validate(conjunction_node->right, {state.in_aggregate});
    } break;
    case ExprType::ARITHMETIC: {
      const ArithmeticExpressionSqlNode *arithmetic_node = static_cast<const ArithmeticExpressionSqlNode *>(sql_node);
      RC                                 rc              = validate(arithmetic_node->left, {state.in_aggregate});
      if (rc != RC::SUCCESS) {
        return rc;
      }
      return validate(arithmetic_node->right, {state.in_aggregate});
    } break;
    case ExprType::FUNCTION: {
      const FunctionExpressionSqlNode *function_node = static_cast<const FunctionExpressionSqlNode *>(sql_node);

      if (common::is_aggregate_function(function_node->function_name.c_str())) {
        if (state.in_aggregate) {
          return RC::INVALID_AGGREGATE;
        }
        if (!allow_aggregate) {
          return RC::INVALID_AGGREGATE;
        }
        allow_non_aggregate = false;
        if (function_node->param_exprs.size() != 1) {
          return RC::INVALID_AGGREGATE;
        }
        RC rc           = validate(function_node->param_exprs[0], {true});
        allow_aggregate = true;
        return rc;
      }

      for (auto &arg : function_node->param_exprs) {
        RC rc = validate(arg, {state.in_aggregate});
        if (rc != RC::SUCCESS) {
          return rc;
        }
      }
      return RC::SUCCESS;
    } break;
    case ExprType::NOT: {
      const NotExpressionSqlNode *not_node = static_cast<const NotExpressionSqlNode *>(sql_node);
      return validate(not_node->child, {state.in_aggregate});
    } break;
    case ExprType::LIKE: {
      const LikeExpressionSqlNode *like_node = static_cast<const LikeExpressionSqlNode *>(sql_node);
      return validate(like_node->child, {state.in_aggregate});
    } break;
    case ExprType::IN: {
      const InExpressionSqlNode *in_node = static_cast<const InExpressionSqlNode *>(sql_node);
      return validate(in_node->child, {state.in_aggregate});
    } break;
    case ExprType::EXISTS: {
      const ExistsExpressionSqlNode *exists_node = static_cast<const ExistsExpressionSqlNode *>(sql_node);
      return validate(exists_node->subquery, {state.in_aggregate, true});
    } break;
    case ExprType::SUBQUERY: {
      const SubqueryExpressionSqlNode *subquery_node = static_cast<const SubqueryExpressionSqlNode *>(sql_node);
      size_t                           attr_size     = subquery_node->subquery->selection.attributes.size();
      if (attr_size > 1 && !state.allow_multiple_col) {
        return RC::SUBQUERY_MORE_THAN_ONE_COL;
      }
      return RC::SUCCESS;
    } break;
    case ExprType::IS_NULL: {
      const IsNullExpressionSqlNode *is_null_node = static_cast<const IsNullExpressionSqlNode *>(sql_node);
      return validate(is_null_node->child, {state.in_aggregate});
    } break;
    default: {
      assert(false);
      return RC::INTERNAL;
    } break;
  }
}

RC ExpressionStructRefactor::refactor(
    std::vector<ExpressionSqlNode *> query_sql_list, const std::vector<ExpressionSqlNode *> &groupby)
{
  init();
  groupby_                 = groupby;
  bool allow_aggregate     = true;
  bool allow_non_aggregate = true;

  for (auto sql_node : query_sql_list) {
    RC rc = validator_.validate(sql_node);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Validate expression struct failed, rc=%d:%s", rc, strrc(rc));
      return rc;
    }

    if (validator_.got_aggregate()) {
      if (!allow_aggregate) {
        LOG_WARN("Aggregate function is not allowed here, due to non-aggregate field is used in the same context");
        return RC::INVALID_AGGREGATE;
      }
      allow_non_aggregate = false;
    }

    if (validator_.got_field()) {
      if (!allow_non_aggregate) {
        LOG_WARN("Non-aggregate field is not allowed here, due to aggregate function is used in the same context");
        return RC::INVALID_AGGREGATE;
      }
      allow_aggregate = false;
    }
  }

  for (auto sql_node : query_sql_list) {
    RC rc = refactor_internal(sql_node);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to refactor expression struct, rc=%d:%s", rc, strrc(rc));
      return rc;
    }
  }

  assert(aggregate_types_.size() == aggregate_childs_.size());
  LOG_INFO("Refactor expression struct success, got %d aggregate functions, %d subquery", aggregate_types_.size(), subqueries_.size());

  return RC::SUCCESS;
}

RC ExpressionStructRefactor::refactor(ExpressionSqlNode *&sql_node, const std::vector<ExpressionSqlNode *> &groupby)
{
  return refactor(vector{sql_node}, groupby);
}

RC ExpressionStructRefactor::refactor_internal(ExpressionSqlNode *&sql_node)
{
  for (auto group_by_expr : groupby_) {
    if (*group_by_expr == *sql_node) {
      AggregateExpressionSqlNode *aggregate_node = new AggregateExpressionSqlNode();
      aggregate_node->name                       = group_by_expr->name;
      aggregate_node->aggregate_type             = AggregateType::GROUP;
      aggregate_node->child                      = sql_node;
      sql_node                                   = aggregate_node;
    }
  }

  switch (sql_node->expr_type) {
    case ExprType::FIELD:
    case ExprType::VALUE:
    case ExprType::CAST:
    case ExprType::COMPARISON:
    case ExprType::CONJUNCTION:
    case ExprType::ARITHMETIC:
    case ExprType::NOT:
    case ExprType::LIKE:
    case ExprType::IS_NULL: {
      SubqueryType old_subquery_type = current_subquery_type_;
      SubqueryType new_subquery_type;
      if (sql_node->expr_type == ExprType::EXISTS) {
        new_subquery_type = SubqueryType::EXISTS;
      }
      if (sql_node->expr_type == ExprType::IN) {
        new_subquery_type = SubqueryType::SINGLE_COL;
      } else {
        new_subquery_type = SubqueryType::SINGLE_CELL;
      }
      current_subquery_type_ = new_subquery_type;

      size_t children_count = sql_node->child_count();
      for (int i = 0; i < children_count; i++) {
        RC rc                  = refactor_internal(sql_node->get_child(i));
        current_subquery_type_ = old_subquery_type;
        if (rc != RC::SUCCESS) {
          LOG_WARN("Failed to refactor expression struct, rc=%d:%s", rc, strrc(rc));
          return rc;
        }
      }
      return RC::SUCCESS;
    } break;

    case ExprType::FUNCTION: {
      SubqueryType old_subquery_type           = current_subquery_type_;
      current_subquery_type_                   = SubqueryType::SINGLE_CELL;
      FunctionExpressionSqlNode *function_node = static_cast<FunctionExpressionSqlNode *>(sql_node);
      RC                         rc;

      // 如果是聚合函数，将其转换为Aggregate表达式
      if (common::is_aggregate_function(function_node->function_name.c_str())) {
        AggregateExpressionSqlNode *aggregate_node = new AggregateExpressionSqlNode();
        aggregate_node->name                       = function_node->function_name;
        aggregate_node->aggregate_type             = common::get_aggregate_type(function_node->function_name.c_str());
        assert(aggregate_node->aggregate_type != AggregateType::INVALID);
        assert(function_node->param_exprs.size() == 1);

        aggregate_node->child = function_node->param_exprs[0];
        function_node->param_exprs.clear();

        sql_node = aggregate_node;
        delete function_node;

        rc                     = refactor_internal(sql_node);
        current_subquery_type_ = old_subquery_type;
        return rc;
      }

      current_subquery_type_ = old_subquery_type;
      return RC::SUCCESS;
    } break;

    case ExprType::AGGREGATE: {
      AggregateExpressionSqlNode *aggregate_node = static_cast<AggregateExpressionSqlNode *>(sql_node);
      ExpressionSqlNode          *child          = aggregate_node->child;
      TupleCellExpressionSqlNode *outter_expr    = new TupleCellExpressionSqlNode;
      TupleCellSpec               tuple_cell(aggregate_node->name.c_str());

      aggregate_node->child = nullptr;
      outter_expr->alias    = tuple_cell.alias();
      delete aggregate_node;

      if (aggregate_node->aggregate_type == AggregateType::COUNT) {
        if (child->expr_type == ExprType::FIELD && static_cast<FieldExpressionSqlNode *>(child)->name == "*") {
          ValueExpressionSqlNode *value_node = new ValueExpressionSqlNode;
          value_node->value.set_int(1);
          delete child;
          child = value_node;
        }
      }

      SubqueryType old_subquery_type = current_subquery_type_;
      current_subquery_type_         = SubqueryType::SINGLE_CELL;
      RC rc                          = refactor_internal(child);
      if (rc != RC::SUCCESS) {
        LOG_WARN("Failed to refactor expression struct, rc=%d:%s", rc, strrc(rc));
        return rc;
      }
      current_subquery_type_ = old_subquery_type;

      aggregate_types_.push_back(aggregate_node->aggregate_type);
      aggregate_childs_.emplace_back(std::move(child));
      sql_node = outter_expr;

      return RC::SUCCESS;
    } break;

    case ExprType::SUBQUERY: {
      SubqueryExpressionSqlNode  *subquery_node = static_cast<SubqueryExpressionSqlNode *>(sql_node);
      TupleCellExpressionSqlNode *ref_expr      = new TupleCellExpressionSqlNode;
      TupleCellSpec               tuple_cell(subquery_node->name.c_str());
      ref_expr->alias = tuple_cell.alias();

      // 由于在validator中已经验证过subquery的合法性，所以这里不再重复验证
      subqueries_.emplace_back(subquery_node);
      subquery_types_.push_back(current_subquery_type_);
      subquery_cells_.push_back(tuple_cell);
      sql_node                = ref_expr;

      return RC::SUCCESS;
    } break;
    case ExprType::IN: {
      assert(false);
    }
    case ExprType::EXISTS: {
      assert(false);
    } break;
    default: {
      assert(false);
    } break;
  }
}

RC ExpressionStructRefactor::init()
{
  aggregate_types_.clear();
  if (!aggregate_childs_.empty()) {
    LOG_WARN("aggregate childs have not been fetched, size = %d", aggregate_childs_.size());
  }
  aggregate_childs_.clear();
  subqueries_.clear();
  if (!subquery_cells_.empty()) {
    LOG_WARN("subquery cells have not been fetched, size = %d", subquery_cells_.size());
  }
  subquery_cells_.clear();
  subquery_types_.clear();
  groupby_.clear();
  current_subquery_type_ = SubqueryType::SINGLE_CELL;
  return RC::SUCCESS;
}
