#pragma once

#include "sql/expr/expr_type.h"
#include "sql/parser/value.h"
#include "sql_node_fwd.h"
#include "comp_op.h"
#include "vector"
#include <cstddef>
#include <stdexcept>
#include <vector>

class ExpressionSqlNode
{
public:
  ExprType    expr_type = ExprType::NONE;
  std::string name;

  virtual bool                operator==(const ExpressionSqlNode &other) const = 0;
  virtual int                 child_count() const                              = 0;
  virtual ExpressionSqlNode *&get_child(int index)                             = 0;
  virtual ~ExpressionSqlNode()                                                 = default;
};

class ValueExpressionSqlNode : public ExpressionSqlNode
{
public:
  Value value;

  ValueExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::VALUE; }
  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const ValueExpressionSqlNode *>(&other)) {
      return value == other_node->value;
    }
    return false;
  }

  int                 child_count() const override { return 0; }
  ExpressionSqlNode *&get_child(int index) override { throw std::out_of_range("ValueExpressionSqlNode has no child"); }
};

class FieldExpressionSqlNode : public ExpressionSqlNode
{
public:
  RelAttrSqlNode field;

  FieldExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::FIELD; }
  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const FieldExpressionSqlNode *>(&other)) {
      return (field.attribute_name == other_node->field.attribute_name) &&
             (field.relation_name == other_node->field.relation_name);
    }
    return false;
  }

  int                 child_count() const override { return 0; }
  ExpressionSqlNode *&get_child(int index) override { throw std::out_of_range("FieldExpressionSqlNode has no child"); }
};

class ArithmeticExpressionSqlNode : public ExpressionSqlNode
{
public:
  ExpressionSqlNode *left  = nullptr;
  ExpressionSqlNode *right = nullptr;
  ArithmeticType     arithmetic_type;

  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const ArithmeticExpressionSqlNode *>(&other)) {
      return (arithmetic_type == other_node->arithmetic_type) && (*left == *(other_node->left)) &&
             (*right == *(other_node->right));
    }
    return false;
  }
  ArithmeticExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::ARITHMETIC; }
  ~ArithmeticExpressionSqlNode()
  {
    delete left;
    left = nullptr;
    delete right;
    right = nullptr;
  }

  int                 child_count() const override { return 2; }
  ExpressionSqlNode *&get_child(int index) override
  {
    if (index == 0) {
      return left;
    } else if (index == 1) {
      return right;
    }
    throw std::out_of_range("ArithmeticExpressionSqlNode has only 2 children");
  }
};

class ComparisonExpressionSqlNode : public ExpressionSqlNode
{
public:
  ExpressionSqlNode *left  = nullptr;
  ExpressionSqlNode *right = nullptr;
  CompOp             comp_op;

  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const ComparisonExpressionSqlNode *>(&other)) {
      return (comp_op == other_node->comp_op) && (*left == *(other_node->left)) && (*right == *(other_node->right));
    }
    return false;
  }
  ComparisonExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::COMPARISON; }
  ~ComparisonExpressionSqlNode()
  {
    delete left;
    left = nullptr;
    delete right;
    right = nullptr;
  }

  int                 child_count() const override { return 2; }
  ExpressionSqlNode *&get_child(int index) override
  {
    if (index == 0) {
      return left;
    } else if (index == 1) {
      return right;
    }
    throw std::out_of_range("ComparisonExpressionSqlNode has only 2 children");
  }
};

class ConjunctionExpressionSqlNode : public ExpressionSqlNode
{
public:
  ExpressionSqlNode *left  = nullptr;
  ExpressionSqlNode *right = nullptr;
  ConjunctionType    conjunction_type;

  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const ConjunctionExpressionSqlNode *>(&other)) {
      return (conjunction_type == other_node->conjunction_type) && (*left == *(other_node->left)) &&
             (*right == *(other_node->right));
    }
    return false;
  }
  ConjunctionExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::CONJUNCTION; }
  ~ConjunctionExpressionSqlNode()
  {
    delete left;
    left = nullptr;
    delete right;
    right = nullptr;
  }

  int                 child_count() const override { return 2; }
  ExpressionSqlNode *&get_child(int index) override
  {
    if (index == 0) {
      return left;
    } else if (index == 1) {
      return right;
    }
    throw std::out_of_range("ConjunctionExpressionSqlNode has only 2 children");
  }
};

class CastExpressionSqlNode : public ExpressionSqlNode
{
public:
  AttrType           target_type;
  ExpressionSqlNode *child = nullptr;

  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const CastExpressionSqlNode *>(&other)) {
      return (target_type == other_node->target_type) && (*child == *(other_node->child));
    }
    return false;
  }
  CastExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::CAST; }
  ~CastExpressionSqlNode()
  {
    delete child;
    child = nullptr;
  }

  int                 child_count() const override { return 1; }
  ExpressionSqlNode *&get_child(int index) override
  {
    if (index == 0) {
      return child;
    }
    throw std::out_of_range("CastExpressionSqlNode has only 1 child");
  }
};

class LikeExpressionSqlNode : public ExpressionSqlNode
{
public:
  ExpressionSqlNode *child = nullptr;
  std::string        pattern;

  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const LikeExpressionSqlNode *>(&other)) {
      return (pattern == other_node->pattern) && (*child == *(other_node->child));
    }
    return false;
  }
  LikeExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::LIKE; }
  ~LikeExpressionSqlNode()
  {
    delete child;
    child = nullptr;
  }

  int                 child_count() const override { return 1; }
  ExpressionSqlNode *&get_child(int index) override
  {
    if (index == 0) {
      return child;
    }
    throw std::out_of_range("LikeExpressionSqlNode has only 1 child");
  }
};

class NotExpressionSqlNode : public ExpressionSqlNode
{
public:
  ExpressionSqlNode *child = nullptr;

  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const NotExpressionSqlNode *>(&other)) {
      return *child == *(other_node->child);
    }
    return false;
  }
  NotExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::NOT; }
  ~NotExpressionSqlNode()
  {
    delete child;
    child = nullptr;
  }

  int                 child_count() const override { return 1; }
  ExpressionSqlNode *&get_child(int index) override
  {
    if (index == 0) {
      return child;
    }
    throw std::out_of_range("NotExpressionSqlNode has only 1 child");
  }
};

class FunctionExpressionSqlNode : public ExpressionSqlNode
{
public:
  std::string                      function_name;
  std::vector<ExpressionSqlNode *> param_exprs;

  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const FunctionExpressionSqlNode *>(&other)) {
      if (function_name != other_node->function_name) {
        return false;
      }
      if (param_exprs.size() != other_node->param_exprs.size()) {
        return false;
      }
      for (size_t i = 0; i < param_exprs.size(); i++) {
        if (!(*param_exprs[i] == *(other_node->param_exprs[i]))) {
          return false;
        }
      }
      return true;
    }
    return false;
  }
  FunctionExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::FUNCTION; }
  ~FunctionExpressionSqlNode()
  {
    for (auto &param : param_exprs) {
      delete param;
      param = nullptr;
    }
    param_exprs.clear();
  }

  int                 child_count() const override { return param_exprs.size(); }
  ExpressionSqlNode *&get_child(int index) override
  {
    if (index < param_exprs.size()) {
      return param_exprs[index];
    }
    throw std::out_of_range("FunctionExpressionSqlNode has only " + std::to_string(param_exprs.size()) + " children");
  }
};

class IsNullExpressionSqlNode : public ExpressionSqlNode
{
public:
  ExpressionSqlNode *child = nullptr;

  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const IsNullExpressionSqlNode *>(&other)) {
      return *child == *(other_node->child);
    }
    return false;
  }

  int                 child_count() const override { return 1; }
  ExpressionSqlNode *&get_child(int index) override
  {
    if (index == 0) {
      return child;
    }
    throw std::out_of_range("IsNullExpressionSqlNode has only 1 child");
  }

  IsNullExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::IS_NULL; }
  ~IsNullExpressionSqlNode()
  {
    if (child) {
      delete child;
      child = nullptr;
    }
  }
};

// 这个不是解析出来的，是在内部生成的
// 因为聚合函数的名字不是关键字，所以最开始解析的时候会被当做普通函数
class AggregateExpressionSqlNode : public ExpressionSqlNode
{
public:
  AggregateType      aggregate_type;
  ExpressionSqlNode *child = nullptr;

  bool operator==(const ExpressionSqlNode &other) const override
  {
    if (auto other_node = dynamic_cast<const AggregateExpressionSqlNode *>(&other)) {
      return (aggregate_type == other_node->aggregate_type) && (*child == *(other_node->child));
    }
    return false;
  }
  AggregateExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::AGGREGATE; }
  ~AggregateExpressionSqlNode()
  {
    delete child;
    child = nullptr;
  }

  int                 child_count() const override { return 1; }
  ExpressionSqlNode *&get_child(int index) override
  {
    if (index == 0) {
      return child;
    }
    throw std::out_of_range("AggregateExpressionSqlNode has only 1 child");
  }
};

// 这个不是解析出来的，是在内部生成的
// 指向tuple中的一个cell
class TupleCellExpressionSqlNode : public ExpressionSqlNode
{
public:
  std::string table_name;
  std::string field_name;
  std::string alias;

  virtual bool operator==(const ExpressionSqlNode &other) const override {
    return false;
  }

  TupleCellExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::CELL_REF; }

  int child_count() const override { return 0; }

  ExpressionSqlNode *&get_child(int index) override
  {
    throw std::out_of_range("TupleCellExpressionSqlNode has no child");
  }

  ~TupleCellExpressionSqlNode() = default;
};