#pragma once

#include "expression_sql_defs.h"
#include "sql/parser/defs/comp_op.h"
#include "sql/parser/defs/sql_node_fwd.h"
#include "vector"
#include <vector>

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
  std::vector<ExpressionSqlNode *>          attributes;           ///< attributes in select clause
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
  SelectSqlNode subquery;

  SubqueryExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::SUBQUERY; }
};

class TablePrimarySqlNode : public TableFactorSqlNode
{
public:
  std::string relation_name;

  TablePrimarySqlNode() { TableReferenceSqlNode::type = RelationType::TABLE; }
};

class TableSubquerySqlNode : public TableFactorSqlNode
{
public:
  SelectSqlNode subquery;

  TableSubquerySqlNode() { TableReferenceSqlNode::type = RelationType::SELECT; }
};

class TableJoinSqlNode : public TableReferenceSqlNode
{
public:
  TableReferenceSqlNode *left;
  TableFactorSqlNode    *right;
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