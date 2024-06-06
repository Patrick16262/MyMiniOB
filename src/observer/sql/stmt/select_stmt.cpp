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
// Created by Wangyunlai on 2022/6/6.
//

#include "sql/stmt/select_stmt.h"
#include "common/log/log.h"
#include "sql/expr/expr_type.h"
#include "sql/expr/expression.h"
#include "sql/expr/expression_resolver.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/defs/expression_sql_defs.h"
#include "sql/parser/defs/sql_query_nodes.h"
#include "sql/stmt/table_ref_desc.h"
#include "sql/stmt/table_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include <cassert>
#include <cstdio>
#include <memory>
#include <utility>
#include <vector>

using namespace std;

RC SelectStmt::create(Db *db, const SelectSqlNode &select_sql, const std::vector<TupleCellSpec> outter_tuple,
    const std::vector<TableFactorDesc> outter_table, Stmt *&stmt)
{
  RC                                rc;
  vector<TableFactorDesc>           table_descs;           // 表描述符，这个数组会在后续动态变化
  vector<unique_ptr<SubqueryStmt>>  subquery_stmts;        // 子查询Stmt
  TableStmt                        *table_stmt = nullptr;  // 表Stmt
  vector<unique_ptr<AggregateDesc>> aggregate_descs;
  vector<unique_ptr<Expression>>    group_by_exprs;
  vector<TupleCellSpec>             tuple_schema;

  /**
   * 生成table_stmt
   */
  TableStmtGenerator table_stmt_generator;

  rc = table_stmt_generator.create(db, select_sql.relations, table_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to create table stmt", strrc(rc));
    return rc;
  }
  table_descs.swap(table_stmt_generator.table_descs());
  table_descs.insert(table_descs.end(), outter_table.begin(), outter_table.end());

  /**
   * 生成tuple_schema
   */
  for (auto *desc : select_sql.attributes) {
    if (!desc->alias.empty()) {
      tuple_schema.emplace_back(desc->alias.c_str());
    }
    if (desc->expr->expr_type == ExprType::FIELD) {
      auto *field_expr = static_cast<FieldExpressionSqlNode *>(desc->expr);
      tuple_schema.emplace_back(field_expr->field.relation_name.c_str(), field_expr->field.attribute_name.c_str());
    } else {
      tuple_schema.emplace_back(desc->expr->name.c_str());
    }
  }

  /**
   * 生成attribute_exprs
   */
  vector<unique_ptr<Expression>>                attribute_exprs;
  vector<ExpressionSqlNode *>                   attributes_nodes;
  vector<unique_ptr<SubqueryExpressionSqlNode>> subquery_sqls;
  vector<SubqueryType>                          subquery_types;
  vector<TupleCellSpec>                         subquery_cells;

  QueryListGenerator query_list_generator(db, table_descs, outter_tuple, select_sql.group_by);

  rc = query_list_generator.generate_query_list(attributes_nodes, attribute_exprs);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to generate query list, rc=%s", strrc(rc));
    return rc;
  }

  subquery_sqls.swap(query_list_generator.subquerys());
  subquery_types = query_list_generator.subquery_types();
  subquery_cells = query_list_generator.subquery_cell_desc();

  for (auto &desc : query_list_generator.aggregate_desc()) {
    aggregate_descs.push_back(std::move(desc));
  }

  /**
   * 生成attribute的Subquery
   */
  assert(subquery_cells.size() == subquery_sqls.size());
  assert(subquery_types.size() == subquery_sqls.size());

  SubqueryStmtGenerator            subquery_stmt_generator(db, table_descs, tuple_schema);
  vector<unique_ptr<SubqueryStmt>> current_subquery_stmts;

  vector<SelectSqlNode *> subquery_ptr;
  for (auto &unique : subquery_sqls) {
    subquery_ptr.push_back(&unique->subquery->selection);
  }

  rc = subquery_stmt_generator.create(subquery_ptr, subquery_types, subquery_cells, current_subquery_stmts);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to create subquery stmt, rc=%s", strrc(rc));
    return rc;
  }

  for (auto &stmt : current_subquery_stmts) {
    subquery_stmts.push_back(std::move(stmt));
  }
  current_subquery_stmts.clear();

  /**
   * 生成groupby 的expr
   */
  for (auto *node : select_sql.group_by) {
    GroupByExpressionResolver group_by_resolver(db, table_descs, tuple_schema);
    unique_ptr<Expression>    group_expr;
    rc = group_by_resolver.resolve(node, group_expr);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to resolve group by expression, rc=%s", strrc(rc));
      return rc;
    }
    group_by_exprs.push_back(std::move(group_expr));

    group_by_resolver.subquery_cell_desc();
  }

  /**
   * 生成filter
   */
  unique_ptr<Expression> filter_expr;

  WhereConditionExpressionResolver where_resolver(db, table_descs, tuple_schema);

  rc = where_resolver.resolve(select_sql.condition, filter_expr);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to resolve where condition, rc=%s", strrc(rc));
    return rc;
  }

  subquery_sqls.swap(where_resolver.subquery_sqls());
  subquery_types = where_resolver.subquery_types();
  subquery_cells = where_resolver.subquery_cell_desc();

  /**
   * 生成filter的Subquery
   */
  subquery_stmt_generator.~SubqueryStmtGenerator();
  new (&subquery_stmt_generator) SubqueryStmtGenerator(db, table_descs, tuple_schema);

  subquery_ptr.clear();
  for (auto &unique : subquery_sqls) {
    subquery_ptr.push_back(&unique.get()->subquery->selection);
  }

  rc = subquery_stmt_generator.create(subquery_ptr, subquery_types, subquery_cells, current_subquery_stmts);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to create subquery stmt, rc=%s", strrc(rc));
    return rc;
  }

  for (auto &stmt : current_subquery_stmts) {
    subquery_stmts.push_back(std::move(stmt));
  }
  current_subquery_stmts.clear();

  /**
   * Everything Alright
   * 生成Select Stmt
   */
  SelectStmt *select_stmt = new SelectStmt;

  select_stmt->project_expr_list_.swap(attribute_exprs);
  select_stmt->tuple_schema_.swap(tuple_schema);
  select_stmt->table_stmt_.reset(table_stmt);
  select_stmt->subquery_list_.swap(subquery_stmts);
  select_stmt->aggregate_list_.swap(aggregate_descs);
  select_stmt->group_by_list_.swap(group_by_exprs);
  // select_stmt->order_by_list_
  select_stmt->filter_ = std::move(filter_expr);

  stmt = select_stmt;

  return RC::SUCCESS;
}

RC SelectStmt::create(Db *db, const SelectSqlNode &select_sql, Stmt *&stmt)
{
  return create(db, select_sql, {}, {}, stmt);
}

RC SubqueryStmtGenerator::create(
    const SelectSqlNode *subquery_sql, SubqueryType type, TupleCellSpec cell_spec, std::unique_ptr<SubqueryStmt> &stmt)
{
  RC            rc;
  Stmt         *subquery;
  SubqueryStmt *subquery_stmt;

  rc = SelectStmt::create(db_, *subquery_sql, outter_tuple_schema_, outter_table_descs_, subquery);

  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to create subquery stmt, rc=%s", strrc(rc));
    return rc;
  }

  unique_ptr<SelectStmt> select_stmt(static_cast<SelectStmt *>(subquery));
  subquery_stmt = new SubqueryStmt(select_stmt, type, cell_spec);

  stmt.reset(subquery_stmt);

  return RC::SUCCESS;
}

RC SubqueryStmtGenerator::create(const std::vector<SelectSqlNode *> subquery_sqls,
    const std::vector<SubqueryType> &types, const std::vector<TupleCellSpec> &cell_specs,
    std::vector<std::unique_ptr<SubqueryStmt>> &stmts)
{
  assert(subquery_sqls.size() == types.size());
  assert(subquery_sqls.size() == cell_specs.size());

  stmts.clear();

  for (size_t i = 0; i < subquery_sqls.size(); i++) {
    std::unique_ptr<SubqueryStmt> stmt;
    RC                            rc = create(subquery_sqls[i], types[i], cell_specs[i], stmt);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to create subquery stmt, rc=%s", strrc(rc));
      return rc;
    }
    stmts.push_back(std::move(stmt));
  }

  return RC::SUCCESS;
}
