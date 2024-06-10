/* Copyright (c) 2023 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2023/08/16.
//

#include "sql/optimizer/logical_plan_generator.h"

#include <cassert>
#include <common/log/log.h>
#include <memory>
#include <utility>
#include <vector>

#include "sql/expr/expression.h"
#include "sql/operator/delete_logical_operator.h"
#include "sql/operator/explain_logical_operator.h"
#include "sql/operator/insert_logical_operator.h"
#include "sql/operator/join_logical_operator.h"
#include "sql/operator/logical_operator.h"
#include "sql/operator/predicate_logical_operator.h"
#include "sql/operator/project_logical_operator.h"
#include "sql/operator/table_get_logical_operator.h"

#include "sql/stmt/delete_stmt.h"
#include "sql/stmt/explain_stmt.h"
#include "sql/stmt/insert_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/stmt/stmt.h"
#include "storage/field/field.h"
#include "storage/table/table.h"

using namespace std;

RC LogicalPlanGenerator::create(Stmt *stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  RC rc = RC::SUCCESS;
  switch (stmt->type()) {

    case StmtType::SELECT: {
      SelectStmt *select_stmt = static_cast<SelectStmt *>(stmt);

      rc = create_plan(select_stmt, logical_operator);
    } break;

    case StmtType::INSERT: {
      InsertStmt *insert_stmt = static_cast<InsertStmt *>(stmt);

      rc = create_plan(insert_stmt, logical_operator);
    } break;

    case StmtType::DELETE: {
      DeleteStmt *delete_stmt = static_cast<DeleteStmt *>(stmt);

      rc = create_plan(delete_stmt, logical_operator);
    } break;

    case StmtType::EXPLAIN: {
      ExplainStmt *explain_stmt = static_cast<ExplainStmt *>(stmt);

      rc = create_plan(explain_stmt, logical_operator);
    } break;
    case StmtType::TABLE: {
      TableStmt *table_stmt = static_cast<TableStmt *>(stmt);

      rc = create_plan(table_stmt, logical_operator);
    } break;
    default: {
      LOG_INFO("current stmt does not need to generate logical plan, stmt type: %d", static_cast<int>(stmt->type()));

      rc = RC::UNIMPLENMENT;
    }
  }
  return rc;
}

RC LogicalPlanGenerator::create_plan(TableStmt *table_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  switch (table_stmt->relation_type()) {
    case RelationType::TABLE: {
      Table        *table = table_stmt->table();
      vector<Field> fields;
      assert(table != nullptr);

      for (auto &field : *table->table_meta().field_metas()) {
        fields.emplace_back(table, &field);
      }

      TableGetLogicalOperator *table_get_oper = new TableGetLogicalOperator(table, fields, ReadWriteMode::READ_ONLY);
      logical_operator.reset(table_get_oper);

      return RC::SUCCESS;
    } break;
    case RelationType::SUBQUERY: {
      LOG_WARN("subquery has not been supported yet");
      return RC::UNIMPLENMENT;
    } break;
    case RelationType::JOIN: {
      JoinLogicalOperator        *join_oper = new JoinLogicalOperator;
      unique_ptr<LogicalOperator> left      = nullptr;
      unique_ptr<LogicalOperator> right     = nullptr;

      RC rc = create(table_stmt->left_table().get(), left);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to create left table operator. rc=%s", strrc(rc));
        return rc;
      }

      rc = create(table_stmt->right_table().get(), right);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to create right table operator. rc=%s", strrc(rc));
        return rc;
      }

      join_oper->add_child(std::move(left));
      join_oper->add_child(std::move(right));

      if (table_stmt->join_condition()) {
        join_oper->set_condition(std::move(table_stmt->join_condition()));
      }

      logical_operator.reset(join_oper);

      return RC::SUCCESS;
    } break;
    case RelationType::VIEW: {
      LOG_WARN("view has not been supported yet");
      return RC::UNIMPLENMENT;
    } break;
    default: {
      LOG_WARN("unsupported relation type: %d", table_stmt->relation_type());
      assert(false);
      return RC::UNIMPLENMENT;
    } break;
  }
}

RC LogicalPlanGenerator::create_plan(SelectStmt *select_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  ProjectLogicalOperator *project =
      new ProjectLogicalOperator(select_stmt->project_expr_list(), select_stmt->tuple_schema());

  if (select_stmt->table_stmt()) {
    unique_ptr<LogicalOperator> table_oper;
    RC                          rc = create(select_stmt->table_stmt().get(), table_oper);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create table operator. rc=%s", strrc(rc));
      return rc;
    }

    project->add_child(std::move(table_oper));
  }

  if (select_stmt->filter()) {
    project->set_filter(std::move(select_stmt->filter()));
  }

  logical_operator.reset(project);
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(InsertStmt *insert_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  Table        *table = insert_stmt->table();
  vector<Value> values(insert_stmt->values(), insert_stmt->values() + insert_stmt->value_amount());

  InsertLogicalOperator *insert_operator = new InsertLogicalOperator(table, values);
  logical_operator.reset(insert_operator);
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(DeleteStmt *delete_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  Table                 *table   = delete_stmt->table();
  unique_ptr<Expression> perdict = std::move(delete_stmt->filter());
  std::vector<Field>     fields;

  for (int i = table->table_meta().sys_field_num(); i < table->table_meta().field_num(); i++) {
    const FieldMeta *field_meta = table->table_meta().field(i);
    fields.push_back(Field(table, field_meta));
  }
  unique_ptr<LogicalOperator> table_get_oper(new TableGetLogicalOperator(table, fields, ReadWriteMode::READ_WRITE));

  DeleteLogicalOperator *delete_oper = new DeleteLogicalOperator(table);
  delete_oper->set_filter(std::move(perdict));

  delete_oper->add_child(std::move(table_get_oper));

  logical_operator.reset(delete_oper);

  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(ExplainStmt *explain_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  unique_ptr<LogicalOperator> child_oper;

  Stmt *child_stmt = explain_stmt->child();

  RC rc = create(child_stmt, child_oper);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create explain's child operator. rc=%s", strrc(rc));
    return rc;
  }

  logical_operator = unique_ptr<LogicalOperator>(new ExplainLogicalOperator);
  logical_operator->add_child(std::move(child_oper));
  return rc;
}

RC LogicalPlanUtils::get_tuple_schema(const LogicalOperator *logical_operator, std::vector<TupleCellSpec> &tuple_schema)
{
  switch (logical_operator->type()) {
    case LogicalOperatorType::TABLE_GET: {
      const TableGetLogicalOperator *table_get_oper = static_cast<const TableGetLogicalOperator *>(logical_operator);
      tuple_schema                                  = table_get_oper->tuple_schema();
    } break;
    case LogicalOperatorType::PREDICATE: {
      const PredicateLogicalOperator *predicate_oper = static_cast<const PredicateLogicalOperator *>(logical_operator);
      RC                              rc = get_tuple_schema(predicate_oper->children().front().get(), tuple_schema);
      if (rc != RC::SUCCESS) {
        return rc;
      }
    } break;
    case LogicalOperatorType::PROJECTION: {
      const ProjectLogicalOperator *project_oper = static_cast<const ProjectLogicalOperator *>(logical_operator);
      tuple_schema                               = project_oper->tuple_schema();
    } break;
    case LogicalOperatorType::JOIN: {
      const JoinLogicalOperator *join_oper = static_cast<const JoinLogicalOperator *>(logical_operator);
      vector<TupleCellSpec>      left_tuple_schema;
      vector<TupleCellSpec>      right_tuple_schema;
      RC                         rc;

      rc = get_tuple_schema(join_oper->children().front().get(), left_tuple_schema);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get left tuple schema. rc=%s", strrc(rc));
        return rc;
      }

      rc = get_tuple_schema(join_oper->children().back().get(), right_tuple_schema);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get right tuple schema. rc=%s", strrc(rc));
        return rc;
      }

      tuple_schema.clear();
      tuple_schema.insert(tuple_schema.end(), left_tuple_schema.begin(), left_tuple_schema.end());
      tuple_schema.insert(tuple_schema.end(), right_tuple_schema.begin(), right_tuple_schema.end());
    } break;

    default: {
      LOG_WARN("current logical operator cannot get schema , operator type: %d", static_cast<int>(logical_operator->type()));
      return RC::UNIMPLENMENT;
    }
  }
  return RC::SUCCESS;
}
