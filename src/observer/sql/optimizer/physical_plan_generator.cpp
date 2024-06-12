/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/12/14.
//

#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <memory>
#include <utility>
#include <vector>

#include "common/log/log.h"
#include "sql/expr/expression.h"
#include "sql/expr/tuple_cell.h"
#include "sql/operator/delete_logical_operator.h"
#include "sql/operator/delete_physical_operator.h"
#include "sql/operator/explain_logical_operator.h"
#include "sql/operator/explain_physical_operator.h"
#include "sql/operator/group_physical_operator.h"
#include "sql/operator/hash_join_physical_operator.h"
#include "sql/operator/index_scan_physical_operator.h"
#include "sql/operator/insert_logical_operator.h"
#include "sql/operator/insert_physical_operator.h"
#include "sql/operator/join_logical_operator.h"
#include "sql/operator/join_physical_operator.h"
#include "sql/operator/logical_operator.h"
#include "sql/operator/physical_operator.h"
#include "sql/operator/predicate_logical_operator.h"
#include "sql/operator/predicate_physical_operator.h"
#include "sql/operator/project_logical_operator.h"
#include "sql/operator/project_physical_operator.h"
#include "sql/operator/table_get_logical_operator.h"
#include "sql/operator/table_scan_physical_operator.h"
#include "sql/operator/update_physical_operator.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/parser/value.h"
#include "sql/optimizer/physical_plan_generator.h"

using namespace std;

RC PhysicalPlanGenerator::create(LogicalOperator &logical_operator, unique_ptr<PhysicalOperator> &oper)
{
  RC rc = RC::SUCCESS;

  switch (logical_operator.type()) {

    case LogicalOperatorType::TABLE_GET: {
      return create_plan(static_cast<TableGetLogicalOperator &>(logical_operator), oper);
    } break;

    case LogicalOperatorType::PREDICATE: {
      return create_plan(static_cast<PredicateLogicalOperator &>(logical_operator), oper);
    } break;

    case LogicalOperatorType::PROJECTION: {
      return create_plan(static_cast<ProjectLogicalOperator &>(logical_operator), oper);
    } break;

    case LogicalOperatorType::INSERT: {
      return create_plan(static_cast<InsertLogicalOperator &>(logical_operator), oper);
    } break;

    case LogicalOperatorType::DELETE: {
      return create_plan(static_cast<DeleteLogicalOperator &>(logical_operator), oper);
    } break;

    case LogicalOperatorType::UPDATE: {
      return create_plan(static_cast<UpdateLogicalOperator &>(logical_operator), oper);
    } break;

    case LogicalOperatorType::EXPLAIN: {
      return create_plan(static_cast<ExplainLogicalOperator &>(logical_operator), oper);
    } break;

    case LogicalOperatorType::JOIN: {
      return create_plan(static_cast<JoinLogicalOperator &>(logical_operator), oper);
    } break;

    case LogicalOperatorType::GROUP: {
      return create_plan(static_cast<GroupLogicalOperator &>(logical_operator), oper);
    } break;

    default: {
      return RC::INVALID_ARGUMENT;
    }
  }
  return rc;
}

RC PhysicalPlanGenerator::create_plan(TableGetLogicalOperator &table_get_oper, unique_ptr<PhysicalOperator> &oper)
{
  vector<unique_ptr<Expression>> &predicates = table_get_oper.predicates();
  // 看看是否有可以用于索引查找的表达式
  Table *table = table_get_oper.table();

  Index     *index      = nullptr;
  ValueExpr *value_expr = nullptr;
  for (auto &expr : predicates) {
    if (expr->type() == ExprType::COMPARISON) {
      auto comparison_expr = static_cast<ComparisonExpr *>(expr.get());
      // 简单处理，就找等值查询
      if (comparison_expr->comp() != EQUAL_TO) {
        continue;
      }

      unique_ptr<Expression> &left_expr  = comparison_expr->left();
      unique_ptr<Expression> &right_expr = comparison_expr->right();
      // 左右比较的一边最少是一个值
      if (!((left_expr->type() == ExprType::VALUE && left_expr->value_type() != NULLS) ||
              (right_expr->type() == ExprType::VALUE && right_expr->value_type() != NULLS))) {
        continue;
      }

      FieldExpr *field_expr = nullptr;
      if (left_expr->type() == ExprType::FIELD) {
        ASSERT(right_expr->type() == ExprType::VALUE, "right expr should be a value expr while left is field expr");
        field_expr = static_cast<FieldExpr *>(left_expr.get());
        value_expr = static_cast<ValueExpr *>(right_expr.get());
      } else if (right_expr->type() == ExprType::FIELD) {
        ASSERT(left_expr->type() == ExprType::VALUE, "left expr should be a value expr while right is a field expr");
        field_expr = static_cast<FieldExpr *>(right_expr.get());
        value_expr = static_cast<ValueExpr *>(left_expr.get());
      }

      if (field_expr == nullptr) {
        continue;
      }

      const Field &field = field_expr->field();
      index              = table->find_index_by_field(field.field_name());
      if (nullptr != index) {
        break;
      }
    }
  }

  if (index != nullptr) {
    ASSERT(value_expr != nullptr, "got an index but value expr is null ?");

    const Value               &value           = value_expr->get_value();
    IndexScanPhysicalOperator *index_scan_oper = new IndexScanPhysicalOperator(table,
        index,
        table_get_oper.read_write_mode(),
        &value,
        true /*left_inclusive*/,
        &value,
        true /*right_inclusive*/);

    index_scan_oper->set_predicates(std::move(predicates));
    oper = unique_ptr<PhysicalOperator>(index_scan_oper);
    LOG_TRACE("use index scan");
  } else {
    auto table_scan_oper = new TableScanPhysicalOperator(table, table_get_oper.read_write_mode());
    table_scan_oper->set_predicates(std::move(predicates));
    oper = unique_ptr<PhysicalOperator>(table_scan_oper);
    LOG_TRACE("use table scan");
  }

  return RC::SUCCESS;
}

RC PhysicalPlanGenerator::create_plan(PredicateLogicalOperator &pred_oper, unique_ptr<PhysicalOperator> &oper)
{
  vector<unique_ptr<LogicalOperator>> &children_opers = pred_oper.children();
  ASSERT(children_opers.size() == 1, "predicate logical operator's sub oper number should be 1");

  LogicalOperator &child_oper = *children_opers.front();

  unique_ptr<PhysicalOperator> child_phy_oper;
  RC                           rc = create(child_oper, child_phy_oper);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create child operator of predicate operator. rc=%s", strrc(rc));
    return rc;
  }

  vector<unique_ptr<Expression>> &expressions = pred_oper.expressions();
  ASSERT(expressions.size() == 1, "predicate logical operator's children should be 1");

  unique_ptr<Expression> expression = std::move(expressions.front());
  oper = unique_ptr<PhysicalOperator>(new PredicatePhysicalOperator(std::move(expression)));
  oper->add_child(std::move(child_phy_oper));
  return rc;
}

RC PhysicalPlanGenerator::create_plan(ProjectLogicalOperator &project_oper, unique_ptr<PhysicalOperator> &oper)
{
  vector<unique_ptr<LogicalOperator>> &child_opers = project_oper.children();

  unique_ptr<PhysicalOperator> child_phy_oper;
  unique_ptr<PhysicalOperator> filter_opr = nullptr; /*nullable*/

  if (project_oper.filter()) {
    filter_opr.reset(new PredicatePhysicalOperator(std::move(project_oper.filter())));
  }

  RC rc = RC::SUCCESS;
  if (!child_opers.empty()) {
    assert(child_opers.size() == 1);
    LogicalOperator *child_oper = child_opers.front().get();
    rc                          = create(*child_oper, child_phy_oper);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create project logical operator's child physical operator. rc=%s", strrc(rc));
      return rc;
    }
  }

  unique_ptr<ProjectPhysicalOperator> project_operator(
      new ProjectPhysicalOperator(project_oper.expressions(), project_oper.tuple_schema()));

  if (child_phy_oper) {
    project_operator->add_child(std::move(child_phy_oper));
  }

  if (filter_opr) {
    filter_opr->add_child(std::move(project_operator));
    oper = std::move(filter_opr);
  } else {
    oper = std::move(project_operator);
  }

  LOG_TRACE("create a project physical operator success.");
  return rc;
}

RC PhysicalPlanGenerator::create_plan(InsertLogicalOperator &insert_oper, unique_ptr<PhysicalOperator> &oper)
{
  Table                  *table           = insert_oper.table();
  vector<Value>          &values          = insert_oper.values();
  InsertPhysicalOperator *insert_phy_oper = new InsertPhysicalOperator(table, std::move(values));
  oper.reset(insert_phy_oper);
  return RC::SUCCESS;
}

RC PhysicalPlanGenerator::create_plan(DeleteLogicalOperator &delete_oper, unique_ptr<PhysicalOperator> &oper)
{
  vector<unique_ptr<LogicalOperator>> &child_opers = delete_oper.children();
  unique_ptr<PhysicalOperator>         child_physical_oper;
  unique_ptr<PhysicalOperator>         perdict = nullptr; /*nulllable*/
  RC                                   rc      = RC::SUCCESS;
  assert(child_opers.size() == 1);

  LogicalOperator *child_oper = child_opers.front().get();

  rc = create(*child_oper, child_physical_oper);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create physical operator. rc=%s", strrc(rc));
    return rc;
  }

  if (delete_oper.filter()) {
    perdict.reset(new PredicatePhysicalOperator(std::move(delete_oper.filter())));
  }

  oper = unique_ptr<PhysicalOperator>(new DeletePhysicalOperator(delete_oper.table()));

  if (perdict) {
    perdict->add_child(std::move(child_physical_oper));
    oper->add_child(std::move(perdict));
  } else {
    oper->add_child(std::move(child_physical_oper));
  }

  return rc;
}

RC PhysicalPlanGenerator::create_plan(ExplainLogicalOperator &explain_oper, unique_ptr<PhysicalOperator> &oper)
{
  vector<unique_ptr<LogicalOperator>> &child_opers = explain_oper.children();

  RC rc = RC::SUCCESS;

  unique_ptr<PhysicalOperator> explain_physical_oper(new ExplainPhysicalOperator);
  for (unique_ptr<LogicalOperator> &child_oper : child_opers) {
    unique_ptr<PhysicalOperator> child_physical_oper;
    rc = create(*child_oper, child_physical_oper);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create child physical operator. rc=%s", strrc(rc));
      return rc;
    }

    explain_physical_oper->add_child(std::move(child_physical_oper));
  }

  oper = std::move(explain_physical_oper);
  return rc;
}

RC PhysicalPlanGenerator::create_plan(JoinLogicalOperator &join_oper, unique_ptr<PhysicalOperator> &oper)
{
  RC                                   rc;
  vector<unique_ptr<LogicalOperator>> &child_opers = join_oper.children();
  vector<unique_ptr<PhysicalOperator>> child_phyis_opers;

  assert(join_oper.expressions().size() <= 1);
  assert(child_opers.size() == 2);

  for (auto &child_oper : child_opers) {
    unique_ptr<PhysicalOperator> child_physical_oper;
    rc = create(*child_oper, child_physical_oper);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create physical child oper. rc=%s", strrc(rc));
      return rc;
    }

    child_phyis_opers.push_back(std::move(child_physical_oper));
  }

  // 简单判断是否使用RbTreeJoin
  if (!join_oper.expressions().empty() && join_oper.expressions().front()->type() == ExprType::COMPARISON) {
    unique_ptr<Expression> &condition  = join_oper.expressions().front();
    ComparisonExpr         *comp_expr  = static_cast<ComparisonExpr *>(condition.get());
    unique_ptr<Expression> &left_expr  = comp_expr->left();
    unique_ptr<Expression> &right_expr = comp_expr->right();

    if (left_expr->type() == ExprType::FIELD && right_expr->type() == ExprType::FIELD &&
        comp_expr->comp() == EQUAL_TO) {
      LOG_TRACE("use RbTreeEqJoin");
      vector<TupleCellSpec> tuple_schema;
      RC                    rc = LogicalPlanUtils::get_tuple_schema(join_oper.children()[0].get(), tuple_schema);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get tuple schema of RbTreeJoin. rc=%s", strrc(rc));
        return rc;
      }

      oper.reset(
          new RbTreeEqJoinPhysicalOperator(std::move(left_expr), std::move(right_expr), std::move(tuple_schema)));

      for (auto &child_oper : child_phyis_opers) {
        oper->add_child(std::move(child_oper));
      }

      condition = nullptr;
      return RC::SUCCESS;
    }
  }

  unique_ptr<PhysicalOperator> perdict = nullptr;

  // 使用NestedLoopJoin
  LOG_TRACE("use NestedLoopJoin join");
  if (join_oper.expressions().empty()) {
  } else {
    perdict.reset(new PredicatePhysicalOperator(std::move(join_oper.expressions().front())));
  }

  oper.reset(new NestedLoopJoinPhysicalOperator());
  oper->add_child(std::move(child_phyis_opers[0]));
  oper->add_child(std::move(child_phyis_opers[1]));

  if (perdict) {
    perdict->add_child(std::move(oper));
    oper = std::move(perdict);
  }
  return rc;
}

RC PhysicalPlanGenerator::create_plan(UpdateLogicalOperator &logical_oper, std::unique_ptr<PhysicalOperator> &oper)
{
  assert(!logical_oper.children().empty());

  unique_ptr<PhysicalOperator>       child_phyis_oper;
  unique_ptr<UpdatePhysicalOperator> update_phyis_opr(
      new UpdatePhysicalOperator(logical_oper.table(), logical_oper.fields()));
  update_phyis_opr->set_new_values(logical_oper.new_values());

  RC rc = create(*logical_oper.children().front(), child_phyis_oper);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create child physical operator of update operator. rc=%s", strrc(rc));
    return rc;
  }

  update_phyis_opr->add_child(std::move(child_phyis_oper));

  oper = std::move(update_phyis_opr);
  return RC::SUCCESS;
}

RC PhysicalPlanGenerator::create_plan(GroupLogicalOperator &logical_oper, std::unique_ptr<PhysicalOperator> &oper)
{
  RC rc;

  unique_ptr<PhysicalOperator> child_physical_oper;

  if (logical_oper.children().empty()) {
    LOG_WARN("Aggregation operator without table has not been implemented yet.");
    return RC::UNIMPLENMENT;
  }

  rc = create(*logical_oper.children().front(), child_physical_oper);

  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create child physical operator of group operator. rc=%s", strrc(rc));
    return rc;
  }

  oper = std::make_unique<GroupPhysicalOperator>(std::move(logical_oper.aggr_descs()));
  oper->add_child(std::move(child_physical_oper));
  return rc;
}
