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
// Created by Wangyunlai on 2022/07/05.
//

#pragma once

#include <memory>
#include <regex>
#include <string>
#include <unordered_set>

#include "sql/expr/tuple_cell.h"
#include "sql/parser/defs/comp_op.h"
#include "sql/parser/value.h"
#include "storage/field/field.h"
#include "expr_type.h"
#include "storage/table/table.h"

class Tuple;
class Table;

/**
 * @defgroup Expression
 * @brief 表达式
 */

/**
 * @brief 表达式的抽象描述
 * @ingroup Expression
 * @details 在SQL的元素中，任何需要得出值的元素都可以使用表达式来描述
 * 比如获取某个字段的值、比较运算、类型转换
 * 当然还有一些当前没有实现的表达式，比如算术运算。
 *
 * 通常表达式的值，是在真实的算子运算过程中，拿到具体的tuple后
 * 才能计算出来真实的值。但是有些表达式可能就表示某一个固定的
 * 值，比如ValueExpr。
 */
class Expression
{
public:
  Expression()          = default;
  virtual ~Expression() = default;

  /**
   * @brief 根据具体的tuple，来计算当前表达式的值。tuple有可能是一个具体某个表的行数据
   */
  virtual RC get_value(const Tuple &tuple, Value &value) const = 0;

  /**
   * @brief 在没有实际运行的情况下，也就是无法获取tuple的情况下，尝试获取表达式的值
   * @details 有些表达式的值是固定的，比如ValueExpr，这种情况下可以直接获取值
   */
  virtual RC try_get_value(Value &value) const { return RC::UNIMPLENMENT; }

  /**
   * @brief 表达式的类型
   * 可以根据表达式类型来转换为具体的子类
   */
  virtual ExprType type() const = 0;

  /**
   * @brief 表达式值的类型
   * @details 一个表达式运算出结果后，只有一个值
   */
  virtual AttrType value_type() const = 0;

  /**
   * @brief 表达式的名字，比如是字段名称，或者用户在执行SQL语句时输入的内容
   */
  virtual std::string name() const { return name_; }
  virtual void        set_name(std::string name) { name_ = name; }

private:
  std::string name_;
};

/**
 * @brief 用于排序的表达式
 * 
 */
struct ExprWithOrder
{
  std::unique_ptr<Expression> expr;
  bool                        is_asc;
};

/**
 * @brief 字段表达式
 * @ingroup Expression
 */
class FieldExpr : public Expression
{
public:
  FieldExpr() = default;
  FieldExpr(const Table *table, const FieldMeta *field) : field_(table, field) {}
  FieldExpr(const Field &field) : field_(field) {}

  virtual ~FieldExpr() = default;

  ExprType type() const override { return ExprType::FIELD; }
  AttrType value_type() const override { return field_.attr_type(); }

  Field &field() { return field_; }

  const Field &field() const { return field_; }

  const char *table_name() const { return field_.table_name(); }
  const char *field_name() const { return field_.field_name(); }

  RC get_value(const Tuple &tuple, Value &value) const override;

private:
  Field field_;
};

/**
 * @brief 常量值表达式
 * @ingroup Expression
 */
class ValueExpr : public Expression
{
public:
  ValueExpr() = default;
  explicit ValueExpr(const Value &value) : value_(value) {}

  virtual ~ValueExpr() = default;

  RC get_value(const Tuple &tuple, Value &value) const override;
  RC try_get_value(Value &value) const override
  {
    value = value_;
    return RC::SUCCESS;
  }

  ExprType type() const override { return ExprType::VALUE; }
  AttrType value_type() const override { return value_.attr_type(); }

  void         get_value(Value &value) const { value = value_; }
  const Value &get_value() const { return value_; }

private:
  Value value_;
};

/**
 * @brief 类型转换表达式
 * @ingroup Expression
 */
class CastExpr : public Expression
{
public:
  CastExpr(std::unique_ptr<Expression> child, AttrType cast_type);
  virtual ~CastExpr();

  ExprType type() const override { return ExprType::CAST; }
  RC       get_value(const Tuple &tuple, Value &value) const override;

  RC try_get_value(Value &value) const override;

  AttrType value_type() const override { return cast_type_; }

  std::unique_ptr<Expression> &child() { return child_; }

private:
  RC cast(const Value &value, Value &cast_value) const;

private:
  std::unique_ptr<Expression> child_;      ///< 从这个表达式转换
  AttrType                    cast_type_;  ///< 想要转换成这个类型
};

/**
 * @brief 比较表达式
 * @ingroup Expression
 */
class ComparisonExpr : public Expression
{
public:
  ComparisonExpr(CompOp comp, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
  virtual ~ComparisonExpr();

  ExprType type() const override { return ExprType::COMPARISON; }
  RC       get_value(const Tuple &tuple, Value &value) const override;
  AttrType value_type() const override { return BOOLEANS; }
  CompOp   comp() const { return comp_; }

  std::unique_ptr<Expression> &left() { return left_; }
  std::unique_ptr<Expression> &right() { return right_; }

  /**
   * 尝试在没有tuple的情况下获取当前表达式的值
   * 在优化的时候，可能会使用到
   */
  RC try_get_value(Value &value) const override;

  /**
   * compare the two tuple cells
   * @param value the result of comparison
   */
  RC compare_value(const Value &left, const Value &right, bool &value) const;

private:
  CompOp                      comp_;
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;
};

/**
 * @brief 联结表达式
 * @ingroup Expression
 * 多个表达式使用同一种关系(AND或OR)来联结
 * 当前miniob仅有AND操作
 */
class ConjunctionExpr : public Expression
{
public:
  ConjunctionExpr(ConjunctionType type, std::vector<std::unique_ptr<Expression>> &children);
  virtual ~ConjunctionExpr() = default;

  ExprType type() const override { return ExprType::CONJUNCTION; }
  AttrType value_type() const override { return BOOLEANS; }
  RC       get_value(const Tuple &tuple, Value &value) const override;
  RC       try_get_value(Value &value) const override;

  ConjunctionType conjunction_type() const { return conjunction_type_; }

  std::vector<std::unique_ptr<Expression>> &children() { return children_; }

private:
  ConjunctionType                          conjunction_type_;
  std::vector<std::unique_ptr<Expression>> children_;
};

/**
 * @brief 算术表达式
 * @ingroup Expression
 */
class ArithmeticExpr : public Expression
{
public:
  ArithmeticExpr(ArithmeticType type, Expression *left, Expression *right);
  ArithmeticExpr(ArithmeticType type, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
  virtual ~ArithmeticExpr() = default;

  ExprType type() const override { return ExprType::ARITHMETIC; }
  AttrType value_type() const override;

  RC get_value(const Tuple &tuple, Value &value) const override;
  RC try_get_value(Value &value) const override;

  ArithmeticType arithmetic_type() const { return arithmetic_type_; }

  std::unique_ptr<Expression> &left() { return left_; }
  std::unique_ptr<Expression> &right() { return right_; }

private:
  /**
   * @throw bad_cast_exception 如果存在null导致无法运算
   */
  RC calc_value(const Value &left_value, const Value &right_value, Value &value) const;

private:
  ArithmeticType              arithmetic_type_;
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;
};

class LikeExpr : public Expression
{
public:
  explicit LikeExpr(std::string partten, std::unique_ptr<Expression> child);
  virtual ~LikeExpr() = default;

  RC       get_value(const Tuple &tuple, Value &value) const override;
  RC       try_get_value(Value &value) const override;
  RC       match(const std::string &str, bool &value) const;
  ExprType type() const override { return ExprType::LIKE; };
  AttrType value_type() const override { return BOOLEANS; };

private:
  std::regex                     partten_;
  std::unique_ptr<Expression>    child_;
  const std::unordered_set<char> special_chars_ = {
      '^', '$', '.', '*', '+', '?', '(', ')', '[', ']', '{', '}', '|', '\\'};
};

class NotExpr : public Expression
{
public:
  explicit NotExpr(std::unique_ptr<Expression> child) : child_(std::move(child)) {}
  virtual ~NotExpr() = default;

  RC       get_value(const Tuple &tuple, Value &value) const override;
  RC       try_get_value(Value &value) const override;
  ExprType type() const override { return ExprType::NOT; }
  AttrType value_type() const override { return BOOLEANS; }

private:
  std::unique_ptr<Expression> child_;
};

class TupleCellExpr : public Expression
{
public:
  TupleCellExpr(const TupleCellSpec &cell_spec, AttrType cell_value_type = AttrType::UNDEFINED)
      : cell_spec_(cell_spec), cell_value_type_(cell_value_type)
  {}
  RC       get_value(const Tuple &tuple, Value &value) const override;
  ExprType type() const override { return ExprType::CELL_REF; }
  AttrType value_type() const override { return cell_value_type_; }

private:
  TupleCellSpec cell_spec_;
  AttrType      cell_value_type_;
};

class InExpr : Expression
{
public:
  InExpr(std::unique_ptr<Expression> &left, std::vector<std::unique_ptr<Expression>> &right)
      : left_(std::move(left)), value_list_(right)
  {}
  InExpr(std::unique_ptr<Expression> &left, std::unique_ptr<Expression> &subquery_ref)
      : left_(std::move(left)), subquery_ref_(std::move(subquery_ref))
  {}

  virtual ~InExpr() = default;

  RC       get_value(const Tuple &tuple, Value &value) const override;
  RC       try_get_value(Value &value) const override;
  ExprType type() const override { return ExprType::IN; }
  AttrType value_type() const override { return BOOLEANS; }

private:
  std::unique_ptr<Expression> left_;

  std::unique_ptr<Expression>              subquery_ref_; /*nullbale*/
  std::vector<std::unique_ptr<Expression>> value_list_;
};

class ExistsExpr : public Expression
{
public:
  ExistsExpr(std::unique_ptr<Expression> &subquery_ref) : subquery_ref_(std::move(subquery_ref)) {}

  virtual ~ExistsExpr() = default;
  RC       get_value(const Tuple &tuple, Value &value) const override;
  RC       try_get_value(Value &value) const override;
  ExprType type() const override { return ExprType::EXISTS; }
  AttrType value_type() const override { return BOOLEANS; }

private:
  std::unique_ptr<Expression> subquery_ref_;
};

class IsNullExpression : public Expression
{
public:
  IsNullExpression(std::unique_ptr<Expression> &child) : child_(std::move(child)) {}
  virtual ~IsNullExpression() = default;

  RC       get_value(const Tuple &tuple, Value &value) const override;
  RC       try_get_value(Value &value) const override;
  ExprType type() const override { return ExprType::IS_NULL; }
  AttrType value_type() const override { return BOOLEANS; }

private:
  std::unique_ptr<Expression> child_;
};