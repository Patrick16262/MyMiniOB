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
#include "sql/stmt/stmt.h"
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
  RC          rc;
  SelectStmt *select_stmt   = new SelectStmt;
  select_stmt->group_exprs_ = select_sql.group_by;
  select_stmt->db_          = db;

  if (!select_sql.group_by.empty()) {
    LOG_WARN("Group by is not supported yet");
    return RC::UNIMPLENMENT;
  }

  if (!select_sql.relations.empty()) {
    rc = select_stmt->resolve_table(select_sql.relations);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to resolve table, rc=%s", strrc(rc));
      return rc;
    }
  }

  assert(!select_sql.attributes.empty());
  rc = select_stmt->resovle_attributes(select_sql.attributes);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to resolve attributes, rc=%s", strrc(rc));
    return rc;
  }


  if (select_sql.condition) {
    rc = select_stmt->resolve_where(select_sql.condition); 
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to resolve where, rc=%s", strrc(rc));
      return rc;
    }
  }

  stmt = select_stmt;

  return RC::SUCCESS;
}

RC SelectStmt::resovle_attributes(const std::vector<ExpressionWithAliasSqlNode *> &attributes)
{
  ProjectExpressionResovler resolver(db_, table_descs_, outter_tuple_schema_, group_exprs_);

  RC rc = resolver.resolve_projection_list(attributes, project_expr_list_);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to resolve expression, rc=%s", strrc(rc));
    return rc;
  }

  // 生成tuple_schema
  tuple_schema_ = std::move(resolver.attr_tuple());

  aggregate_list_ = std::move(resolver.aggregate_desc());

  if (!resolver.subquery_cell_desc().empty()) {
    LOG_WARN("Subquery is not supported yet");
    return RC::UNIMPLENMENT;
  }

  return RC::SUCCESS;
}

RC SelectStmt::resolve_where( ExpressionSqlNode *where_expr) {
  WhereConditionExpressionResolver resolver(db_, table_descs_, tuple_schema_);

  RC rc= resolver.resolve(where_expr, filter_);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to resolve where expression, rc=%s", strrc(rc));
    return rc;
  }

  return RC::SUCCESS;
}

RC SelectStmt::resolve_table(const std::vector<TableReferenceSqlNode *> &table_refs)
{
  TableSqlResovler table_stmt_generator;
  TableStmt       *table_stmt;

  RC rc = table_stmt_generator.create(db_, table_refs, table_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to create table stmt, rc=%s", strrc(rc));
    return rc;
  }

  table_stmt_.reset(table_stmt);
  table_descs_ = table_stmt_generator.table_descs();
  return RC::SUCCESS;
}

RC SelectStmt::create(Db *db, const SelectSqlNode &select_sql, Stmt *&stmt)
{
  return create(db, select_sql, {}, {}, stmt);
}

RC SubqueryStmtGenerator::create(
    const SelectSqlNode *subquery_sql, SubqueryType type, TupleCellSpec cell_spec, std::unique_ptr<SubqueryStmt> &stmt)
{
  LOG_WARN("Subquery is not supported yet");
  return RC::UNIMPLENMENT;

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
