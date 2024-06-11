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
// Created by WangYunlai on 2022/12/08.
//

#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "sql/expr/expression.h"
#include "sql/expr/tuple_cell.h"
#include "sql/operator/logical_operator.h"

/**
 * @brief project 表示投影运算
 * @ingroup LogicalOperator
 * @details 从表中获取数据后，可能需要过滤，投影，连接等等。
 *
 * @field expressions_ 表示select中的投影表达式
 * @field tuple_schema_ 表示select中的列
 * @field filter_ 表示select中的过滤条件
 */
class ProjectLogicalOperator : public LogicalOperator
{
public:
  ProjectLogicalOperator(
      std::vector<std::unique_ptr<Expression>> &expressions, std::vector<TupleCellSpec> tuple_schema_)
      : tuple_schema_(std::move(tuple_schema_))
  {
    expressions_.swap(expressions);
  }
  virtual ~ProjectLogicalOperator() = default;

  LogicalOperatorType type() const override { return LogicalOperatorType::PROJECTION; }

  // select中的列
  std::vector<std::unique_ptr<Expression>>       &expressions() { return expressions_; }
  const std::vector<std::unique_ptr<Expression>> &expressions() const { return expressions_; }
  const std::vector<TupleCellSpec>               &tuple_schema() const { return tuple_schema_; }

  std::unique_ptr<Expression> &filter() { return filter_; }
  void                         set_filter(std::unique_ptr<Expression> filter) { filter_ = std::move(filter); }

private:
  std::vector<TupleCellSpec> tuple_schema_;

  std::unique_ptr<Expression> filter_ = nullptr; /*nullable*/ 
};
