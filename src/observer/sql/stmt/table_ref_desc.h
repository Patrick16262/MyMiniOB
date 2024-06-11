#pragma once

#include "sql/expr/expr_type.h"
#include "sql/expr/expression.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/defs/comp_op.h"
#include "sql/parser/defs/sql_node_fwd.h"
#include "storage/table/table.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

/**
 * @brief 对字段的描述
 *
 */

class FieldDesc
{

public:
  FieldDesc(std::string field_name, bool visible) : field_name_(std::move(field_name)), visible_(visible) {}

  std::string field_name() const { return field_name_; }
  bool        visible() const { return visible_; }

private:
  std::string field_name_;
  bool        visible_;
};

/**
 * @brief 对表的描述，会被用于expression对象生成中
 *
 */
class TableFactorDesc
{
public:
  TableFactorDesc(const Table *table)
  {
    for (auto field_meta : *table->table_meta().field_metas()) {
      field_names_.push_back(FieldDesc(field_meta.name(), field_meta.visible()));
    }
    type_       = RelationType::TABLE;
    table_name_ = table->name();
    table_      = const_cast<Table *>(table);
  }
  TableFactorDesc(
      const std::string &table_name, std::vector<FieldDesc> field_names, RelationType type, Table *table = nullptr)
      : type_(type), table_name_(table_name), field_names_(std::move(field_names)), table_(table){};

  RelationType           type() const { return type_; }
  std::string            table_name() const { return table_name_; }
  std::vector<FieldDesc> fields() const { return field_names_; }

  Table *table() const { return table_; }

private:
  RelationType           type_;
  std::string            table_name_;
  std::vector<FieldDesc> field_names_;

  Table *table_ = nullptr;
};

/**
 * @brief 对聚合函数的描述，会被用在计划生成中
 *
 */

class AggregateDesc
{
public:
  AggregateDesc(AggregateType type, std::unique_ptr<Expression> child, const TupleCellSpec &child_spec)
      : type_(type), child_(std::move(child)), child_spec_(child_spec)
  {}

  AggregateType                type() const { return type_; }
  std::unique_ptr<Expression> &child() { return child_; }
  const TupleCellSpec         &child_spec() const { return child_spec_; }

private:
  AggregateType               type_;        // 聚合函数类型
  std::unique_ptr<Expression> child_;       // 聚合函数的参数表达式
  TupleCellSpec               child_spec_;  // 聚合函数参数的cell的名字
};

namespace common {
inline TableFactorDesc create_table_desc(Table *table)
{
  std::vector<FieldDesc> fields;
  assert(table != nullptr);

  for (auto &field : *table->table_meta().field_metas()) {
    fields.push_back(FieldDesc(field.name(), field.visible()));
  }

  return TableFactorDesc(table->table_meta().name(), fields, RelationType::TABLE, table);
}
}  // namespace common