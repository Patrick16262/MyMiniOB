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
// Created by Wangyunlai on 2022/5/27.
//

#pragma once

#include "sql/expr/expression.h"
#include "sql/stmt/stmt.h"
#include <memory>

class Table;
class FilterStmt;

/**
 * @brief Delete 语句
 * @ingroup Statement
 */
class DeleteStmt : public Stmt
{
public:
  DeleteStmt(Table *table, std::unique_ptr<Expression> filter_expr) : table_(table), filter_(std::move(filter_expr)) {}
  ~DeleteStmt() = default;

  Table *table() const { return table_; }

  StmtType type() const override { return StmtType::DELETE; }

public:
  static RC create(Db *db, const DeleteSqlNode &delete_sql, Stmt *&stmt);

  std::unique_ptr<Expression> &filter()  { return filter_; }

private:
  Table                      *table_ = nullptr;
  std::unique_ptr<Expression> filter_;
};
