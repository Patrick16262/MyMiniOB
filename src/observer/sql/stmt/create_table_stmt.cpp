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
// Created by Wangyunlai on 2023/6/13.
//

#include "sql/stmt/create_table_stmt.h"
#include "common/rc.h"
#include "event/sql_debug.h"
#include "sql/parser/parse_defs.h"
#include "sql/parser/value.h"
#include <vector>

RC validate_attr_info(const std::vector<AttrInfoSqlNode> &attr_infos);

RC CreateTableStmt::create(Db *db, const CreateTableSqlNode &create_table, Stmt *&stmt)
{
  RC rc = validate_attr_info(create_table.attr_infos);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  stmt = new CreateTableStmt(create_table.relation_name, create_table.attr_infos);

  sql_debug("create table statement: table name %s", create_table.relation_name.c_str());
  return RC::SUCCESS;
}

RC validate_attr_info(const std::vector<AttrInfoSqlNode> &attr_infos) {
  for (const auto &attr_info : attr_infos)  {
    if (attr_info.type == DATES && attr_info.length != 4) {
      return RC::INVALID_ARGUMENT;
    }
  }
  return RC::SUCCESS;
}
