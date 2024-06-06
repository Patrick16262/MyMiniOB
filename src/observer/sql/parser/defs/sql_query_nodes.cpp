#include "sql_query_nodes.h"
#include "../parse_defs.h"

bool InExpressionSqlNode::operator==(const ExpressionSqlNode &other) const
{
  if (auto other_node = dynamic_cast<const InExpressionSqlNode *>(&other)) {
    if (*child == *(other_node->child)) {
      if (subquery != nullptr && other_node->subquery != nullptr) {
        // 暂不实现
        return false;
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

int InExpressionSqlNode::child_count() const// 1 for child, 2 for subquery, 1 + value_list.size() for value_list
{
  if (child != nullptr) {
    return 2;
  }
  return 1 + value_list.size();
}
ExpressionSqlNode *&InExpressionSqlNode::get_child(int index)
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
InExpressionSqlNode::~InExpressionSqlNode()
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

bool ExistsExpressionSqlNode::operator==(const ExpressionSqlNode &other) const
{
  if (auto other_node = dynamic_cast<const ExistsExpressionSqlNode *>(&other)) {
    if (subquery != nullptr && other_node->subquery != nullptr) {
      return *static_cast<ExpressionSqlNode *>(subquery) == *static_cast<ExpressionSqlNode *>(other_node->subquery);
    }
  }
  return false;
}

ExpressionSqlNode *&ExistsExpressionSqlNode::get_child(int index)
{
  if (index == 0) {
    return *reinterpret_cast<ExpressionSqlNode **>(&subquery);
  }
  throw std::out_of_range("ExistsExpressionSqlNode has no child");
}

ExistsExpressionSqlNode::~ExistsExpressionSqlNode()
{
  if (subquery) {
    delete subquery;
    subquery = nullptr;
  }
}

TableJoinSqlNode::~TableJoinSqlNode()
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

SubqueryExpressionSqlNode::~SubqueryExpressionSqlNode()
{
  if (subquery) {
    delete subquery;
    subquery = nullptr;
  }
}

SelectSqlNode::~SelectSqlNode()
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

ExpressionWithAliasSqlNode::~ExpressionWithAliasSqlNode()
{
  if (expr) {
    delete expr;
    expr = nullptr;
  }
}

ExpressionWithOrderSqlNode::~ExpressionWithOrderSqlNode()
{
  if (expr) {
    delete expr;
    expr = nullptr;
  }
}
