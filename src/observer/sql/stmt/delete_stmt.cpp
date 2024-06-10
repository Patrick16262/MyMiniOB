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
// Created by Wangyunlai on 2022/5/22.
//

#include "sql/stmt/delete_stmt.h"
#include "sql/expr/expression_resolver.h"
#include "sql/stmt/table_ref_desc.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include <cassert>
#include <memory>
#include <utility>

using namespace std;

RC DeleteStmt::create(Db *db, const DeleteSqlNode &delete_sql, Stmt *&stmt)
{
  RC                     rc          = RC::SUCCESS;
  DeleteStmt            *delete_stmt = nullptr;
  unique_ptr<Expression> filter_expr;

  // check whether the table exists
  Table *table = db->find_table(delete_sql.relation_name.c_str());
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), delete_sql.relation_name.c_str());
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  TableFactorDesc table_desc = common::create_table_desc(table);

  if (delete_sql.condition) {
    WhereConditionExpressionResolver resolver(db, {table_desc}, {});
    rc = resolver.resolve(delete_sql.condition, filter_expr);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
      return rc;
    }
  }

  delete_stmt = new DeleteStmt(table, std::move(filter_expr));
  stmt        = delete_stmt;
  return rc;
}
