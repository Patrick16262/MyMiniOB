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

#pragma once

#include "common/rc.h"
#include "sql/expr/expression.h"
#include "sql/stmt/stmt.h"
#include "storage/field/field.h"
#include <memory>
#include <vector>

/**
 * @brief 更新语句
 * @ingroup Statement
 */
class UpdateStmt : public Stmt
{
public:
  UpdateStmt() = default;

  StmtType type() const override { return StmtType::UPDATE; };

public:
  static RC create(Db *db, const UpdateSqlNode &update_sql, Stmt *&stmt);

public:
  Table                    *table() const { return table_; }
  const std::vector<Field> &fields() const { return fields_; }
  const std::vector<Value> &values() const { return values_; }

  std::unique_ptr<Expression> &filter()  { return filter_; }

private:
  Table                      *table_;
  std::vector<Field>          fields_;
  std::vector<Value>          values_;
  std::unique_ptr<Expression> filter_ = nullptr;
};
