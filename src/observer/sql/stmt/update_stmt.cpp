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

#include "sql/stmt/update_stmt.h"
#include "common/log/log.h"
#include "common/rc.h"
#include "sql/expr/expression.h"
#include "sql/expr/expression_resolver.h"
#include "sql/parser/value.h"
#include "sql/stmt/table_ref_desc.h"
#include "storage/field/field_meta.h"
#include "storage/table/table.h"
#include "storage/db/db.h"
#include <cassert>
#include <memory>
#include <utility>
#include <vector>

using namespace std;

RC UpdateStmt::create(Db *db, const UpdateSqlNode &update, Stmt *&stmt)
{
  unique_ptr<UpdateStmt> update_stmt(new UpdateStmt());
  unique_ptr<Expression> filter;
  Table                 *table = db->find_table(update.relation_name.c_str());
  vector<Value>          values;
  vector<Field>          fields;
  RC                     rc;

  if (table == nullptr) {
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  if (update.condition) {
    WhereConditionExpressionResolver resolver(db, {TableFactorDesc(table)}, {});
    RC                               rc = resolver.resolve(update.condition, filter);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to resolve where condition, rc = %s", strrc(rc));
      return rc;
    }
  }

  for (auto assignment : update.assignments) {
    const FieldMeta *field_meta = table->table_meta().field(assignment->attribute_name.c_str());
    if (field_meta == nullptr) {
      return RC::SCHEMA_FIELD_NOT_EXIST;
    }

    Value                   inplace_value;
    Value                   converted_value;
    unique_ptr<Expression>  value_expr;
    ConstExpressionResovler resolver;
    Field                   asgin_field(table, field_meta);

    fields.push_back(asgin_field);

    rc = resolver.resolve(assignment->value, value_expr);
    if (rc != RC::SUCCESS) {
      LOG_WARN("No const expression in update assignment has not been supported yet, rc = %s", strrc(rc));
      return rc;
    }

    rc = value_expr->try_get_value(inplace_value);
    if (rc != RC::SUCCESS) {
      assert(false);
    }

    rc = common::try_convert_value(inplace_value, asgin_field.attr_type(), converted_value);
    if (rc != RC::SUCCESS) {
      return rc;
    }

    values.push_back(converted_value);
  }

  update_stmt->table_  = table;
  update_stmt->fields_ = fields;
  update_stmt->values_ = values;
  update_stmt->filter_ = std::move(filter);

  stmt = update_stmt.release();
  return RC::SUCCESS;
}
