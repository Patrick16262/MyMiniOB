/* Copyright (c) 2021OceanBase and/or its affiliates. All rights reserved.
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

#include "sql/stmt/insert_stmt.h"
#include "common/log/log.h"
#include "sql/expr/expression.h"
#include "sql/parser/value.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include <cstdlib>

InsertStmt::InsertStmt(Table *table, const Value *values, int value_amount)
    : table_(table), values_(values), value_amount_(value_amount)
{}

RC make_values(const vector<Value> &values, const TableMeta& meta, Value *&res, int &res_size);

RC InsertStmt::create(Db *db, const InsertSqlNode &inserts, Stmt *&stmt)
{
  const char *table_name = inserts.relation_name.c_str();

  if (nullptr == db || nullptr == table_name || inserts.values.empty()) {
    LOG_WARN("invalid argument. db=%p, table_name=%p, value_num=%d",
        db, table_name, static_cast<int>(inserts.values.size()));
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  //make values
  Value *values = nullptr;
  int value_num = 0;
  RC rc = make_values(inserts.values, table->table_meta(), values, value_num);
  if (rc != RC::SUCCESS) {
    LOG_WARN("make values failed. rc=%d", rc);
    return rc;
  }

  // everything alright
  stmt = new InsertStmt(table, values, value_num);
  return RC::SUCCESS;
}

/**
 * @brief 将插入的数据转换为合法的Value数组
 */
RC make_values(const vector<Value> &values, const TableMeta& meta, Value *&res, int &res_size)
{
  // check the fields number
  const int        value_num  = static_cast<int>(values.size());
  const int        field_num  = meta.field_num() - meta.sys_field_num();
  if (field_num != value_num) {
    LOG_WARN("schema mismatch. value num=%d, field num in schema=%d", value_num, field_num);
    return RC::SCHEMA_FIELD_MISSING;
  }

  res = new Value[value_num];
  res_size = value_num;

  // check fields type
  const int sys_field_num = meta.sys_field_num();
  for (int i = 0; i < value_num; i++) {
    const FieldMeta *field_meta = meta.field(i + sys_field_num);
    const AttrType   field_type = field_meta->type();
    const AttrType   value_type = values[i].attr_type();
    if (field_type == value_type) {
      res[i] = values[i];
    } else {
      RC rc = common::try_convert_value(values[i], field_type, res[i]);
      if (rc != RC::SUCCESS) {
        LOG_WARN("type mismatch. field type=%d, value type=%d", field_type, value_type);
        return RC::INVALID_ARGUMENT;
      }
    }
  }
  return RC::SUCCESS;
}