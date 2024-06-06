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
#include "storage/db/db.h"
#include "storage/table/table.h"
#include <cassert>

DeleteStmt::DeleteStmt(Table *table, FilterStmt *filter_stmt) : table_(table), filter_stmt_(filter_stmt) {}

DeleteStmt::~DeleteStmt()
{
  // if (nullptr != filter_stmt_) {
  //   delete filter_stmt_;
  //   filter_stmt_ = nullptr;
  // }
}

RC DeleteStmt::create(Db *db, const DeleteSqlNode &delete_sql, Stmt *&stmt)
{
  return RC::UNIMPLENMENT;
  // RC          rc = RC::SUCCESS;
  // DeleteStmt *delete_stmt = nullptr;
  // FilterStmt *filter_stmt = nullptr;

  // // check whether the table exists
  // Table *table = db->find_table(delete_sql.relation_name.c_str());
  // if (nullptr == table) {
  //   LOG_WARN("no such table. db=%s, table_name=%s", db->name(), delete_sql.relation_name.c_str());
  //   return RC::SCHEMA_TABLE_NOT_EXIST;
  // }

  // std::unordered_map<std::string, Table *> table_map;
  // table_map.insert(std::pair<std::string, Table *>(delete_sql.relation_name, table));

  // if (delete_sql.condition) {
  //   rc = FilterStmt::create(db, table, table_map, delete_sql.condition, filter_stmt);
  //   if (rc != RC::SUCCESS) {
  //     LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
  //     return rc;
  //   }
  // }

  // delete_stmt = new DeleteStmt(table, filter_stmt);
  // stmt = delete_stmt;
  // return rc;
}
