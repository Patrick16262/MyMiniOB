/* Copyright (c) OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by WangYunlai on 2022/12/26.
//

#pragma once

#include "sql/expr/expression.h"
#include "sql/operator/logical_operator.h"
#include <memory>
#include <utility>

/**
 * delete operator下必须为可以产生rowTuple的operator
 * @brief 逻辑算子，用于执行delete语句
 * @ingroup LogicalOperator
 */
class DeleteLogicalOperator : public LogicalOperator
{
public:
  DeleteLogicalOperator(Table *table);
  virtual ~DeleteLogicalOperator() = default;

  LogicalOperatorType type() const override { return LogicalOperatorType::DELETE; }
  Table              *table() const { return table_; }

  std::unique_ptr<Expression> &filter() { return filter_; }
  void                         set_filter(std::unique_ptr<Expression> filter) { filter_ = std::move(filter); }

private:
  Table                      *table_ = nullptr;
  std::unique_ptr<Expression> filter_;
};
