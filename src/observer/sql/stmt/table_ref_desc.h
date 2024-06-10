#pragma once

#include "sql/expr/expr_type.h"
#include "sql/expr/expression.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/defs/sql_node_fwd.h"
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
  FieldDesc(std::string field_names, bool visible) : field_name_(std::move(field_names)), visible_(visible) {}
  
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
  TableFactorDesc(const std::string &table_name, std::vector<FieldDesc> field_names, RelationType type)
      : type_(type), table_name_(table_name), field_names_(std::move(field_names)) {};

  RelationType             type() const { return type_; }
  std::string              table_name() const { return table_name_; }
  std::vector<FieldDesc> fields() const { return field_names_; }

private:
  RelationType             type_;
  std::string              table_name_;
  std::vector<FieldDesc> field_names_;
};

/**
 * @brief 对聚合函数的描述，会被用在计划生成中
 *
 */

class AggregateDesc
{
public:
  AggregateDesc(AggregateType type, std::unique_ptr<Expression> &child, const TupleCellSpec &child_spec)
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
