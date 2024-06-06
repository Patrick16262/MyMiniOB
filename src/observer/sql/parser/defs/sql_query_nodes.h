#pragma once

#include "expression_sql_defs.h"
#include "sql/expr/expr_type.h"
#include "sql/parser/defs/comp_op.h"
#include "sql/parser/defs/sql_node_fwd.h"
#include "vector"
#include <stdexcept>
#include <vector>

struct ExpressionWithAliasSqlNode
{
  ExpressionSqlNode *expr;
  std::string        alias;

  ~ExpressionWithAliasSqlNode()
  {
    if (expr) {
      delete expr;
      expr = nullptr;
    }
  }
};

struct ExpressionWithOrderSqlNode
{
  ExpressionSqlNode *expr;
  OrderType          order_type;

  ~ExpressionWithOrderSqlNode()
  {
    if (expr) {
      delete expr;
      expr = nullptr;
    }
  }
};

/**
 * @brief 描述一个select语句
 * @ingroup SQLParser
 * @details 一个正常的select语句描述起来比这个要复杂一些，这里做了简化。
 * 一个select语句由五部分组成，分别是select, from, where, groupby, having, orderby。
 * select部分表示要查询的字段，from部分表示要查询的表，where部分表示查询的条件。
 * 比如 from 中可以是多个表，也可以是另一个查询语句，这里仅仅支持表，也就是 relations。
 * where 条件 conditions，这里表示使用AND串联起来多个条件。正常的SQL语句会有OR，NOT等，
 * 甚至可以包含复杂的表达式。
 */

struct SelectSqlNode
{
  std::vector<ExpressionWithAliasSqlNode *> attributes;           ///< attributes in select clause
  std::vector<TableReferenceSqlNode *>      relations;            ///< 查询的表
  ExpressionSqlNode                        *condition = nullptr;  ///< 查询条件，使用AND串联起来多个条件
  std::vector<ExpressionSqlNode *>          group_by;             ///< group by字段
  ExpressionSqlNode                        *having = nullptr;     ///< having条件
  std::vector<ExpressionWithOrderSqlNode *> order_by;             ///< order by字段

  ~SelectSqlNode()
  {
    for (auto attr : attributes) {
      delete attr;
    }
    attributes.clear();
    for (auto relation : relations) {
      delete relation;
    }
    relations.clear();
    for (auto group : group_by) {
      delete group;
    }
    group_by.clear();
    if (having) {
      delete having;
      having = nullptr;
    }
    if (condition) {
      delete condition;
      condition = nullptr;
    }
  }
};

class SubqueryExpressionSqlNode : public ExpressionSqlNode
{
public:
  SelectSqlNode *subquery;

  SubqueryExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::SUBQUERY; }
  ~SubqueryExpressionSqlNode()
  {
    if (subquery) {
      delete subquery;
      subquery = nullptr;
    }
  }

  bool operator==(const ExpressionSqlNode &other) const override
  {
    // 暂不实现
    return false;
  };
  int                 child_count() const override { return 0; };
  ExpressionSqlNode *&get_child(int index) override
  {
    throw std::out_of_range("SubqueryExpressionSqlNode has no child");
  };
};

class TablePrimarySqlNode : public TableReferenceSqlNode
{
public:
  std::string relation_name;

  TablePrimarySqlNode() { TableReferenceSqlNode::type = RelationType::TABLE; }
};

class TableSubquerySqlNode : public TableReferenceSqlNode
{
public:
  SelectSqlNode subquery;

  TableSubquerySqlNode() { TableReferenceSqlNode::type = RelationType::SELECT; }
};

class TableJoinSqlNode : public TableReferenceSqlNode
{
public:
  TableReferenceSqlNode *left;
  TableReferenceSqlNode *right;
  ExpressionSqlNode     *condition;

  TableJoinSqlNode() { TableReferenceSqlNode::type = RelationType::JOIN; }
  ~TableJoinSqlNode()
  {
    if (left) {
      delete left;
      left = nullptr;
    }
    if (right) {
      delete right;
      right = nullptr;
    }
    if (condition) {
      delete condition;
      condition = nullptr;
    }
  }
};

class InExpressionSqlNode : public ExpressionSqlNode
{
public:
  ExpressionSqlNode               *child    = nullptr;
  SubqueryExpressionSqlNode       *subquery = nullptr;  // 如果是子查询，这个字段不为空，反正为空
  std::vector<ExpressionSqlNode *> value_list;

  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const InExpressionSqlNode *>(&other)) {
      if (*child == *(other_node->child)) {
        if (subquery != nullptr && other_node->subquery != nullptr) {
          return *static_cast<ExpressionSqlNode *>(subquery) == *static_cast<ExpressionSqlNode *>(other_node->subquery);
        } else if (subquery == nullptr && other_node->subquery == nullptr) {
          if (value_list.size() == other_node->value_list.size()) {
            for (size_t i = 0; i < value_list.size(); i++) {
              if (!(*value_list[i] == *(other_node->value_list[i]))) {
                return false;
              }
            }
            return true;
          }
        }
      }
    }
    return false;
  }

  int child_count() const override  // 1 for child, 2 for subquery, 1 + value_list.size() for value_list
  {
    if (child != nullptr) {
      return 2;
    }
    return 1 + value_list.size();
  }

  ExpressionSqlNode *&get_child(int index) override
  {
    if (index == 0) {
      return child;
    } else if (index == 1 && subquery != nullptr) {
      return *reinterpret_cast<ExpressionSqlNode **>(&subquery);
    } else if (index > 0 && index < 1 + value_list.size()) {
      return value_list[index - 1];
    }
    throw std::out_of_range("InExpressionSqlNode has no child");
  }

  InExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::IN; }
  ~InExpressionSqlNode()
  {
    if (child) {
      delete child;
      child = nullptr;
    }
    if (subquery != nullptr) {
      delete subquery;
      subquery = nullptr;
    }
    for (auto &value : value_list) {
      delete value;
      value = nullptr;
    }
    value_list.clear();
  }
};

class ExistsExpressionSqlNode : public ExpressionSqlNode
{
public:
  SubqueryExpressionSqlNode *subquery = nullptr;

  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const ExistsExpressionSqlNode *>(&other)) {
      if (subquery != nullptr && other_node->subquery != nullptr) {
        return *static_cast<ExpressionSqlNode *>(subquery) == *static_cast<ExpressionSqlNode *>(other_node->subquery);
      }
    }
    return false;
  }

  int                 child_count() const override { return subquery != nullptr ? 1 : 0; }
  ExpressionSqlNode *&get_child(int index) override
  {
    if (index == 0) {
      return *reinterpret_cast<ExpressionSqlNode **>(&subquery);
    }
    throw std::out_of_range("ExistsExpressionSqlNode has no child");
  }

  ExistsExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::EXISTS; }
  ~ExistsExpressionSqlNode()
  {
    if (subquery) {
      delete subquery;
      subquery = nullptr;
    }
  }
};
