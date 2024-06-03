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
// Created by Wangyunlai on 2022/6/5.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/rc.h"
#include "sql/expr/expression.h"

#include "sql/stmt/filter_stmt.h"
#include "sql/stmt/stmt.h"

class FieldMeta;
class Db;
class Table;

/**
 * @brief 表示select语句
 * @ingroup Statement
 */
class SelectStmt : public Stmt
{
public:
  SelectStmt() = default;
  virtual ~SelectStmt() {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  };

  StmtType type() const override { return StmtType::SELECT; }

public:
  static RC create(Db *db, const SelectSqlNode &select_sql, Stmt *&stmt);

public:
  const std::vector<Table *>                     &tables() const { return query_tables_; }
  const std::vector<std::unique_ptr<Expression>> &queryExprList() const { return query_expr_list_; }

  std::vector<std::unique_ptr<Expression>> &query_expr_list() { return query_expr_list_; }
  FilterStmt                               *filter_stmt() const { return filter_stmt_; }
  const std::vector<std::string>           &tuple_schema() const { return tuple_schema_; }

private:
  std::vector<std::unique_ptr<Expression>> query_expr_list_; // 用于生成ProjectOperator
  std::vector<Table *>                     query_tables_;
  std::vector<std::string> tuple_schema_;  // query_expr_list_中的资源会被转移，因此需要额外设置tuple_schema字段
  FilterStmt *filter_stmt_ = nullptr;
};
