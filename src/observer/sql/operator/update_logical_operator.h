#pragma once

#include "sql/operator/logical_operator.h"
#include "sql/parser/value.h"
#include "storage/field/field.h"
#include <cassert>
#include <vector>

/**
 * @brief Update的逻辑算子
 * @ingroup LogicalOperator
 *
 * @param children_ 表算子, 应只有一个
 * @param Field  更新的字段
 */

class UpdateLogicalOperator : public LogicalOperator
{
public:
  UpdateLogicalOperator(Table *table, std::vector<Field> fields) : fields_(std::move(fields)), table_(table) {}
  ~UpdateLogicalOperator() = default;

  LogicalOperatorType type() const override { return LogicalOperatorType::UPDATE; }

  std::vector<Field>  fields() const { return fields_; }
  std::vector<Field> &fields() { return fields_; }
  Table              *table() const { return table_; }

  std::vector<Value> new_values() const { return values_; }
  void set_values(const std::vector<Value> &values) { values_ = values; }

private:
  std::vector<Field> fields_;
  std::vector<Value> values_;  // 应该是一个表达式而不是value;
  Table             *table_;
};