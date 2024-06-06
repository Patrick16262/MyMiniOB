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
// Created by Wangyunlai on 2021/5/14.
//

#pragma once

#include <cassert>
#include <cfloat>
#include <cstdio>
#include <json/value.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "common/log/log.h"
#include "sql/expr/expr_type.h"
#include "sql/expr/expression.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/value.h"
#include "storage/record/record.h"

class Table;

/**
 * @defgroup Tuple
 * @brief Tuple 元组，表示一行数据，当前返回客户端时使用
 * @details
 * tuple是一种可以嵌套的数据结构。
 * 比如select t1.a+t2.b from t1, t2;
 * 需要使用下面的结构表示：
 * @code {.cpp}
 *  Project(t1.a+t2.b)
 *        |
 *      Joined
 *      /     \
 *   Row(t1) Row(t2)
 * @endcode
 *
 */

/**
 * @brief 元组的结构，包含哪些字段(这里成为Cell)，每个字段的说明
 * @ingroup Tuple
 */
class TupleSchema
{
public:
  void append_cell(const TupleCellSpec &cell) { cells_.push_back(cell); }
  void append_cell(const char *table, const char *field) { append_cell(TupleCellSpec(table, field)); }
  void append_cell(const char *alias) { append_cell(TupleCellSpec(alias)); }
  int  cell_num() const { return static_cast<int>(cells_.size()); }

  const TupleCellSpec &cell_at(int i) const { return cells_[i]; }

private:
  std::vector<TupleCellSpec> cells_;
};

/**
 * @brief 元组的抽象描述
 * @ingroup Tuple
 */
class Tuple
{
public:
  Tuple()          = default;
  virtual ~Tuple() = default;

  /**
   * @brief 获取元组中的Cell的个数
   * @details 个数应该与tuple_schema一致
   */
  virtual int cell_num() const = 0;

  /**
   * @brief 获取指定位置的Cell
   *
   * @param index 位置
   * @param[out] cell  返回的Cell
   */
  virtual RC cell_at(int index, Value &cell) const = 0;

  /**
   * @brief 根据cell的描述，获取cell的值
   *
   * @param spec cell的描述
   * @param[out] cell 返回的cell
   */
  virtual RC find_cell(const TupleCellSpec &spec, Value &cell) const = 0;

  virtual std::string to_string() const
  {
    std::string str;
    const int   cell_num = this->cell_num();
    for (int i = 0; i < cell_num - 1; i++) {
      Value cell;
      cell_at(i, cell);
      str += cell.to_string();
      str += ", ";
    }

    if (cell_num > 0) {
      Value cell;
      cell_at(cell_num - 1, cell);
      str += cell.to_string();
    }
    return str;
  }
};

/**
 * @brief 一行数据的元组
 * @ingroup Tuple
 * @details 直接就是获取表中的一条记录
 */
class RowTuple : public Tuple
{
public:
  RowTuple() = default;
  virtual ~RowTuple()
  {
    for (FieldExpr *spec : fields_) {
      delete spec;
    }
    fields_.clear();
  }

  void set_record(Record *record) { this->record_ = record; }

  void set_schema(const Table *table, const std::vector<FieldMeta> *fields)
  {
    table_ = table;
    this->fields_.clear();
    this->fields_.reserve(fields->size());
    for (const FieldMeta &field : *fields) {
      fields_.push_back(new FieldExpr(table, &field));
    }
  }

  int cell_num() const override { return fields_.size(); }

  RC cell_at(int index, Value &cell) const override
  {
    if (index < 0 || index >= static_cast<int>(fields_.size())) {
      LOG_WARN("invalid argument. index=%d", index);
      return RC::INVALID_ARGUMENT;
    }

    FieldExpr       *field_expr = fields_[index];
    const FieldMeta *field_meta = field_expr->field().meta();
    cell.set_type(field_meta->type());
    cell.set_data(this->record_->data() + field_meta->offset(), field_meta->len());
    return RC::SUCCESS;
  }

  RC find_cell(const TupleCellSpec &spec, Value &cell) const override
  {
    const char *table_name = spec.table_name();
    const char *field_name = spec.field_name();
    if (0 != strcmp(table_name, table_->name())) {
      return RC::NOTFOUND;
    }

    for (size_t i = 0; i < fields_.size(); ++i) {
      const FieldExpr *field_expr = fields_[i];
      const Field     &field      = field_expr->field();
      if (0 == strcmp(field_name, field.field_name())) {
        return cell_at(i, cell);
      }
    }
    return RC::NOTFOUND;
  }

#if 0
  RC cell_spec_at(int index, const TupleCellSpec *&spec) const override
  {
    if (index < 0 || index >= static_cast<int>(speces_.size())) {
      LOG_WARN("invalid argument. index=%d", index);
      return RC::INVALID_ARGUMENT;
    }
    spec = speces_[index];
    return RC::SUCCESS;
  }
#endif

  Record &record() { return *record_; }

  const Record &record() const { return *record_; }

private:
  Record                  *record_ = nullptr;
  const Table             *table_  = nullptr;
  std::vector<FieldExpr *> fields_;
};

/**
 * @brief 从一行数据中，选择部分字段组成的元组，也就是投影操作
 * @ingroup Tuple
 * @details 一般在select语句中使用。
 * 投影也可以是很复杂的操作，比如某些字段需要做类型转换、重命名、表达式运算、函数计算等。
 * 当前的实现是比较简单的，只是选择部分字段，不做任何其他操作。
 */
class ProjectTuple : public Tuple
{
public:
  ProjectTuple()          = default;
  virtual ~ProjectTuple() = default;

  void set_tuple(Tuple *tuple) { this->tuple_ = tuple; }

  void set_project_exprs(std::vector<std::unique_ptr<Expression>> &project_exprs)
  {
    project_exprs_.clear();
    project_exprs_.swap(project_exprs);
  }

  int cell_num() const override { return project_exprs_.size(); }

  RC cell_at(int index, Value &cell) const override
  {
    RC rc;
    if (index < 0 || index >= static_cast<int>(project_exprs_.size())) {
      return RC::INTERNAL;
    }
    if (tuple_ == nullptr) {
      return RC::INTERNAL;
    }

    const auto &expr = project_exprs_[index];
    rc               = expr->get_value(*tuple_, cell);
    if (rc != RC::SUCCESS) {
      LOG_WARN("get value failed. rc=%d", rc);
    }
    return rc;
  }

  RC find_cell(const TupleCellSpec &spec, Value &cell) const override { return tuple_->find_cell(spec, cell); }

private:
  std::vector<std::unique_ptr<Expression>> project_exprs_;
  Tuple                                   *tuple_ = nullptr;
};

/**
 * @brief 用在calc语句中
 *
 */
class ExpressionTuple : public Tuple
{
public:
  ExpressionTuple(std::vector<std::unique_ptr<Expression>> &expressions) : expressions_(expressions) {}

  virtual ~ExpressionTuple() {}

  int cell_num() const override { return expressions_.size(); }

  RC cell_at(int index, Value &cell) const override
  {
    if (index < 0 || index >= static_cast<int>(expressions_.size())) {
      return RC::INTERNAL;
    }

    const Expression *expr = expressions_[index].get();
    return expr->try_get_value(cell);
  }

  RC find_cell(const TupleCellSpec &spec, Value &cell) const override
  {
    for (const std::unique_ptr<Expression> &expr : expressions_) {
      if (0 == strcmp(spec.alias(), expr->name().c_str())) {
        return expr->try_get_value(cell);
      }
    }
    return RC::NOTFOUND;
  }

private:
  const std::vector<std::unique_ptr<Expression>> &expressions_;
};

/**
 * @brief 一些常量值组成的Tuple
 * @ingroup Tuple
 */
class ValueListTuple : public Tuple
{
public:
  ValueListTuple()          = default;
  virtual ~ValueListTuple() = default;

  void set_cells(const std::vector<Value> &cells) { cells_ = cells; }

  virtual int cell_num() const override { return static_cast<int>(cells_.size()); }

  virtual RC cell_at(int index, Value &cell) const override
  {
    if (index < 0 || index >= cell_num()) {
      return RC::NOTFOUND;
    }

    cell = cells_[index];
    return RC::SUCCESS;
  }

  virtual RC find_cell(const TupleCellSpec &spec, Value &cell) const override { return RC::INTERNAL; }

private:
  std::vector<Value> cells_;
};

/**
 * @brief 将两个tuple合并为一个tuple
 * @ingroup Tuple
 * @details 在join算子中使用
 */
class JoinedTuple : public Tuple
{
public:
  JoinedTuple()          = default;
  virtual ~JoinedTuple() = default;

  void set_left(Tuple *left) { left_ = left; }
  void set_right(Tuple *right) { right_ = right; }

  int cell_num() const override { return left_->cell_num() + right_->cell_num(); }

  RC cell_at(int index, Value &value) const override
  {
    const int left_cell_num = left_->cell_num();
    if (index > 0 && index < left_cell_num) {
      return left_->cell_at(index, value);
    }

    if (index >= left_cell_num && index < left_cell_num + right_->cell_num()) {
      return right_->cell_at(index - left_cell_num, value);
    }

    return RC::NOTFOUND;
  }

  RC find_cell(const TupleCellSpec &spec, Value &value) const override
  {
    RC rc = left_->find_cell(spec, value);
    if (rc == RC::SUCCESS || rc != RC::NOTFOUND) {
      return rc;
    }

    return right_->find_cell(spec, value);
  }

private:
  Tuple *left_  = nullptr;
  Tuple *right_ = nullptr;
};

class AggregateTupleManager;

/**
 * @brief 聚合操作所需的Tuple
 */
class AggregateTuple : public Tuple
{
public:
  /**
   * @brief 获取元组中的Cell的个数
   * @details 个数应该与tuple_schema一致
   */
  int cell_num() const override { return aggr_exprs_.size(); }

  /**
   * @brief 获取指定位置的Cell
   *
   * @param index 位置
   * @param[out] cell  返回的Cell
   */
  RC cell_at(int index, Value &cell) const override
  {
    if (index < 0 || index >= static_cast<int>(aggr_values_.size())) {
      return RC::NOTFOUND;
    }

    const Value &value = aggr_values_[index];

    if (aggr_types_[index] == AggregateType::AVG) {
      int   count;
      float sum;
      sscanf(value.get_string().c_str(), "count: %d, sum: %f", &count, &sum);
      cell.set_float(sum / count);
    } else {
      cell = value;
    }

    return RC::SUCCESS;
  }

  /**
   * @brief 根据cell的描述，获取cell的值
   *
   * @param spec cell的描述
   * @param[out] cell 返回的cell
   */
  RC find_cell(const TupleCellSpec &spec, Value &cell) const override
  {
    const char *alias = spec.alias();

    if (std::strcmp(alias, "") == 0) {
      LOG_WARN("Cell spec should have alias, spec table=%s, spec field=%s", spec.table_name(), spec.field_name());
    }

    for (size_t i = 0; i < aggr_specs_.size(); ++i) {
      if (std::strcmp(alias, aggr_specs_[i].alias()) == 0) {
        return cell_at(i, cell);
      }
    }

    LOG_WARN("Cell not found, spec alias = %s", alias);
    return RC::NOTFOUND;
  }

  RC aggregate_tuple(Tuple &tuple)
  {
    for (size_t i = 0; i < aggr_exprs_.size(); ++i) {
      Value         value;
      Value        &aggregated_value = aggr_values_[i];
      AggregateType aggregated_type  = aggr_types_[i];
      Expression   *expr             = aggr_exprs_[i].get();

      RC rc = expr->get_value(tuple, value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("get value failed. rc=%d", rc);
        return rc;
      }
      if (value.attr_type() == AttrType::NULLS) {
        continue;
      }

      switch (aggregated_type) {
        case AggregateType::SUM: {
          if (aggregated_value.attr_type() == AttrType::NULLS) {
            aggregated_value.set_float(value.get_double());
          } else {
            aggregated_value.set_float(aggregated_value.get_float() + value.get_float());
          }
        } break;
        case AggregateType::COUNT: {
          aggregated_value.set_int(aggregated_value.get_int() + 1);
        } break;
        case AggregateType::AVG:
          // 使用字符串简单粗暴地记录count和sum
          {
            const char *fmt = "count: %d, sum: %f";
            if (aggregated_value.attr_type() == AttrType::NULLS) {
              char info[100];
              sprintf(info, fmt, 1, value.get_float());
              aggregated_value.set_string(info);
            } else {
              int         count;
              float       sum;
              std::string info = aggregated_value.get_string();
              char        res[100];
              sscanf(info.c_str(), fmt, &count, &sum);

              sprintf(res, fmt, count + 1, sum + value.get_float());
              aggregated_value.set_string(res);
            }
          }
          break;
        case AggregateType::MAX: {
          if (aggregated_value.attr_type() == AttrType::NULLS) {
            aggregated_value.set_value(value);
          } else {
            int cmp_res = value.compare(aggregated_value);
            if (cmp_res > 0) {
              aggregated_value.set_value(value);
            }
          }
        } break;
        case AggregateType::MIN:
          if (aggregated_value.attr_type() == AttrType::NULLS) {
            aggregated_value.set_value(value);
          } else {
            int cmp_res = value.compare(aggregated_value);
            if (cmp_res < 0) {
              aggregated_value.set_value(value);
            }
          }
          break;
        case AggregateType::GROUP:
          if (aggregated_value != value) {
            return RC::INTERNAL;
          }
          break;
        default: {
          LOG_WARN("Invalid aggregate type. type=%d", static_cast<int>(aggr_types_[i]));
          return RC::INTERNAL;
        };
      }
    }
    return RC::SUCCESS;
  }

private:
  friend class AggregateTupleManager;

  AggregateTuple(const std::vector<std::unique_ptr<Expression>> &aggr_exprs,
      const std::vector<AggregateType> &aggr_types, const std::vector<TupleCellSpec> &aggr_specs)
      : aggr_exprs_(aggr_exprs), aggr_types_(aggr_types), aggr_specs_(aggr_specs)
  {
    assert(aggr_exprs.size() == aggr_types_.size() && aggr_types_.size() == aggr_specs_.size());
  }

private:
  // 资源引用向AggregateTupleFactory的成员
  const std::vector<std::unique_ptr<Expression>> &aggr_exprs_;
  const std::vector<AggregateType>               &aggr_types_;
  const std::vector<TupleCellSpec>               &aggr_specs_;
  std::vector<Value>                              aggr_values_;
};

class AggregateTupleManager
{
public:
  AggregateTupleManager(std::vector<std::unique_ptr<Expression>> &aggr_exprs,
      const std::vector<AggregateType> &aggr_types, const std::vector<TupleCellSpec> &aggr_specs)
      : aggr_exprs_(std::move(aggr_exprs)), aggr_types_(aggr_types), aggr_specs_(aggr_specs)
  {
    assert(aggr_exprs.size() == aggr_types_.size() && aggr_types_.size() == aggr_specs_.size());
  }

  AggregateTuple generateTuple(const std::vector<Value> group_by)
  {
    AggregateTuple tuple(aggr_exprs_, aggr_types_, aggr_specs_);
    auto           group_by_it = group_by.begin();

    for (int i = 0; i < aggr_exprs_.size(); i++) {
      Value initial_value;
      switch (aggr_types_[i]) {
        case AggregateType::COUNT: {
          initial_value.set_int(0);
        } break;
        case AggregateType::SUM: {
          initial_value.set_null();
        } break;
        case AggregateType::AVG: {
          initial_value.set_null();
        } break;
        case AggregateType::MAX: {
          initial_value.set_null();
        } break;
        case AggregateType::MIN: {
          initial_value.set_null();
        } break;
        case AggregateType::GROUP: {
          if (group_by_it == group_by.end()) {
            throw std::runtime_error("Group by value not enough");
          }
          initial_value = *group_by_it;
          group_by_it++;
        } break;
        default: {
          throw std::runtime_error("Invalid aggregate type");
        }
      }
      tuple.aggr_values_.push_back(initial_value);
    }

    if (group_by_it != group_by.end()) {
      throw std::runtime_error("Group by value too much");
    }

    return tuple;
  }

  std::vector<Expression *> group_exprs()
  {
    std::vector<Expression *> exprs;
    for (const auto &expr : aggr_exprs_) {
      exprs.push_back(expr.get());
    }
    return exprs;
  }

private:
  std::vector<std::unique_ptr<Expression>> aggr_exprs_;
  std::vector<AggregateType>               aggr_types_;
  std::vector<TupleCellSpec>               aggr_specs_;
};

/**
 * @brief 用于缓存元组
 *
 */

class CacheTuple : public Tuple
{
public:
  int cell_num() const override { return cell_specs_.size(); };

  RC cell_at(int index, Value &cell) const override
  {
    if (index < 0 || index >= static_cast<int>(cells_.size())) {
      return RC::NOTFOUND;
    }
    cell = cells_[index];
    return RC::SUCCESS;
  };

  RC find_cell(const TupleCellSpec &spec, Value &cell) const override
  {
    const char *alias = spec.alias();
    const char *table = spec.table_name();
    const char *field = spec.field_name();

    for (size_t i = 0; i < cell_specs_.size(); ++i) {
      if (0 == strcmp(alias, cell_specs_[i].alias())) {
        return cell_at(i, cell);
      }
    }

    for (size_t i = 0; i < cell_specs_.size(); ++i) {
      if (0 == strcmp(table, cell_specs_[i].table_name()) && 0 == strcmp(field, cell_specs_[i].field_name())) {
        return cell_at(i, cell);
      }
    }

    return RC::NOTFOUND;
  };

private:
  friend class CacheTupleManager;
  CacheTuple(std::vector<Value> cells, const std::vector<TupleCellSpec> &cell_specs)
      : cells_(std::move(cells)), cell_specs_(cell_specs)
  {}

  const std::vector<Value>          cells_;
  const std::vector<TupleCellSpec> &cell_specs_;
};

class CacheTupleManager
{
public:
  CacheTupleManager(std::vector<TupleCellSpec> &cell_specs) : cell_specs_(std::move(cell_specs)) {}
  RC cloneTuple(const Tuple &tuple, std::unique_ptr<Tuple> &cache) const
  {
    if (tuple.cell_num() != cell_specs_.size()) {
      LOG_WARN("cell num not match. tuple cell num=%d, cell spec num=%d", tuple.cell_num(), cell_specs_.size());
      return RC::INVALID_ARGUMENT;
    }

    std::vector<Value> cells;
    for (int i = 0; i < tuple.cell_num(); i++) {
      Value cell;
      RC    rc = tuple.cell_at(i, cell);
      if (rc != RC::SUCCESS) {
        LOG_WARN("get cell failed. rc=%d", rc);
        return rc;
      }
      cells.push_back(cell);
    }

    cache.reset(new CacheTuple(cells, cell_specs_));
    return RC::SUCCESS;
  }

private:
  const std::vector<TupleCellSpec> cell_specs_;
};
