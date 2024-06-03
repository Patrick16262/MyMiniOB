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
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include "filter_stmt.h"

RC SelectStmt::create(Db *db, const SelectSqlNode &select_sql, Stmt *&stmt)
{
  RC                                  rc;
  Table                              *default_table = nullptr;
  vector<unique_ptr<Expression>>      query_list;
  vector<std::string>                 tuple_schema;
  FilterStmt                         *filter_stmt = nullptr;

  vector<unique_ptr<Expression>> cell_ref_child_list;
  vector<TupleCellSpec>          cell_ref_list;

  // check if the table exists and is unique
  // for (auto &rel_name : select_sql.relations) {
  //   Table *table = db->find_table(rel_name.c_str());
  //   if (table == nullptr) {
  //     LOG_WARN("table not exist: %s", rel_name.c_str());
  //     return RC::SCHEMA_TABLE_NOT_EXIST;
  //   }
  //   if (query_tables.find(rel_name) != query_tables.end()) {
  //     LOG_WARN(" Not unique table: %s", rel_name.c_str());
  //     return RC::SCHEMA_TABLE_EXIST;
  //   }
  //   query_tables[rel_name] = table;
  // }

  if (select_sql.relations.size() == 1) {
    default_table = query_tables[select_sql.relations[0]];
  }

  // 设置聚合回调
  QueryListGenerator query_list_generator = QueryListGenerator(default_table, query_tables);
  query_list_generator.set_on_aggregate_found(
      [&cell_ref_list, &cell_ref_child_list](
          AggregateType, std::unique_ptr<Expression> child, TupleCellSpec spec) -> RC {
        cell_ref_child_list.push_back(std::move(child));
        cell_ref_list.push_back(spec);
        return RC::SUCCESS;
      });

  rc = query_list_generator.generate_query_list(select_sql.attributes, query_list);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate query list failed");
    return rc;
  }

  for (auto &expr : query_list) {
    tuple_schema.push_back(expr->name());
  }

  LOG_INFO("got %d tables in from stmt and %d fields (constants) in query stmt", query_tables.size(), query_list.size());

  // create filter statement in `where` statement

  if (select_sql.condition != nullptr) {
    rc = FilterStmt::create(db, default_table, query_tables, select_sql.condition, filter_stmt);
    if (rc != RC::SUCCESS) {
      LOG_WARN("cannot construct filter stmt, due to %d:%s", rc, strrc(rc));
      return rc;
    }
  }

  // everything alright
  SelectStmt *select_stmt = new SelectStmt();
  select_stmt->query_expr_list_.swap(query_list);
  for (auto &pair : query_tables) {
    select_stmt->query_tables_.push_back(pair.second);
  }
  select_stmt->filter_stmt_ = filter_stmt;
  select_stmt->tuple_schema_.swap(tuple_schema);
  stmt = select_stmt;
  return RC::SUCCESS;
}
