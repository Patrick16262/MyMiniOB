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

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "common/rc.h"
#include "sql/expr/expression.h"
#include "sql/expr/expression_resolver.h"
#include "sql/stmt/stmt.h"

class Db;
class Table;

/**
 * @brief 描述算术运算语句
 * @ingroup Statement
 */
class CalcStmt : public Stmt
{
public:
  CalcStmt()                   = default;
  virtual ~CalcStmt() override = default;

  StmtType type() const override { return StmtType::CALC; }

public:
  static RC create(CalcSqlNode &calc_sql, Stmt *&stmt)
  {
    ExpressionGenerator generator;
    RC                  rc;
    CalcStmt           *calc_stmt = new CalcStmt();

    for (auto &sql_node : calc_sql.expressions) {
      unique_ptr<Expression> expr;
      rc = generator.generate_expression(sql_node, expr);
      if (rc != RC::SUCCESS) {
        return rc;
      }
      calc_stmt->tuple_schema_.push_back(expr->name());
      calc_stmt->expressions_.push_back(std::move(expr));
    }

    stmt = calc_stmt;
    return RC::SUCCESS;
  }

public:
  std::vector<std::unique_ptr<Expression>> &expressions() { return expressions_; }

  std::vector<std::string> tuple_schema() const { return tuple_schema_; }

private:
  std::vector<std::unique_ptr<Expression>> expressions_;
  std::vector<std::string>                tuple_schema_;
};
