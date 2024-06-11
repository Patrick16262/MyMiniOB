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

#include "sql/expr/expression.h"
#include "common/log/log.h"
#include "common/rc.h"
#include "sql/expr/tuple.h"
#include "sql/parser/defs/comp_op.h"
#include "sql/parser/value.h"
#include <cassert>
#include <regex>
#include <sstream>
#include <set>
#include <utility>
#include <vector>

using namespace std;

RC FieldExpr::get_value(const Tuple &tuple, Value &value) const
{
  return tuple.find_cell(TupleCellSpec(table_name(), field_name()), value);
}

RC ValueExpr::get_value(const Tuple &tuple, Value &value) const
{
  value = value_;
  return RC::SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////
CastExpr::CastExpr(unique_ptr<Expression> child, AttrType cast_type) : child_(std::move(child)), cast_type_(cast_type)
{}

CastExpr::~CastExpr() {}

RC CastExpr::cast(const Value &value, Value &cast_value) const
{
  RC rc = RC::SUCCESS;
  if (this->value_type() == value.attr_type()) {
    cast_value = value;
    return rc;
  }

  try {
    switch (cast_type_) {
      case BOOLEANS: {
        bool val = value.get_boolean();
        cast_value.set_boolean(val);
      } break;
      case DATES: {
        string val = value.get_string();
        cast_value.set_date(val.c_str());
      } break;
      case FLOATS: {
        cast_value.set_float(value.get_float());
      } break;
      case INTS: {
        cast_value.set_int(value.get_int());
      } break;
      case NULLS: {
        cast_value.set_null();
      } break;
      case CHARS: {
        string val = value.get_string();
        cast_value.set_string(val.c_str());
      }
      case TEXTS: {
        string val = value.get_string();
        cast_value.set_text(val.c_str());
      } break;
      default: {
        throw null_cast_exception();
      }
    }
  } catch (null_cast_exception) {
    rc = RC::SUCCESS;
    cast_value.set_null();
  }

  return rc;
}

RC CastExpr::get_value(const Tuple &tuple, Value &cell) const
{
  RC rc = child_->get_value(tuple, cell);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  return cast(cell, cell);
}

RC CastExpr::try_get_value(Value &value) const
{
  RC rc = child_->try_get_value(value);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  return cast(value, value);
}

////////////////////////////////////////////////////////////////////////////////

ComparisonExpr::ComparisonExpr(CompOp comp, unique_ptr<Expression> left, unique_ptr<Expression> right)
    : comp_(comp), left_(std::move(left)), right_(std::move(right))
{}

ComparisonExpr::~ComparisonExpr() {}

RC ComparisonExpr::compare_value(const Value &left, const Value &right, bool &result) const
{
  RC  rc = RC::SUCCESS;
  int cmp_result;
  try {
    cmp_result = left.compare(right);
  } catch (null_cast_exception) {
    return RC::NULL_VALUE;
  }

  result = false;
  switch (comp_) {
    case EQUAL_TO: {
      result = (0 == cmp_result);
    } break;
    case LESS_EQUAL: {
      result = (cmp_result <= 0);
    } break;
    case NOT_EQUAL: {
      result = (cmp_result != 0);
    } break;
    case LESS_THAN: {
      result = (cmp_result < 0);
    } break;
    case GREAT_EQUAL: {
      result = (cmp_result >= 0);
    } break;
    case GREAT_THAN: {
      result = (cmp_result > 0);
    } break;
    default: {
      LOG_WARN("unsupported comparison. %d", comp_);
      rc = RC::INTERNAL;
    } break;
  }

  return rc;
}

RC ComparisonExpr::try_get_value(Value &cell) const
{
  ValueExpr *left_value_expr  = static_cast<ValueExpr *>(left_.get());
  ValueExpr *right_value_expr = static_cast<ValueExpr *>(right_.get());
  Value      left_cell;
  Value      right_cell;
  RC         rc;

  rc = left_value_expr->try_get_value(left_cell);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
    return rc;
  }

  rc = right_value_expr->try_get_value(right_cell);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
    return rc;
  }

  bool value = false;
  rc         = compare_value(left_cell, right_cell, value);
  if (rc == RC::NULL_VALUE) {
    cell.set_null();
    rc = RC::SUCCESS;
  } else if (rc != RC::SUCCESS) {
    LOG_WARN("failed to compare tuple cells. rc=%s", strrc(rc));
  } else {
    cell.set_boolean(value);
  }
  return rc;
}

RC ComparisonExpr::get_value(const Tuple &tuple, Value &value) const
{
  Value left_value;
  Value right_value;

  RC rc = left_->get_value(tuple, left_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
    return rc;
  }
  rc = right_->get_value(tuple, right_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
    return rc;
  }

  bool bool_value = false;

  rc = compare_value(left_value, right_value, bool_value);
  if (rc == RC::SUCCESS) {
    value.set_boolean(bool_value);
  } else if (rc == RC::NULL_VALUE) {
    rc = RC::SUCCESS;
    value.set_null();
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
ConjunctionExpr::ConjunctionExpr(ConjunctionType type, vector<unique_ptr<Expression>> &children)
    : conjunction_type_(type), children_(std::move(children))
{}

RC ConjunctionExpr::get_value(const Tuple &tuple, Value &value) const
{
  RC rc = RC::SUCCESS;
  if (children_.empty()) {
    value.set_boolean(true);
    return rc;
  }

  Value tmp_value;
  bool  contain_null = false;

  for (const unique_ptr<Expression> &expr : children_) {
    rc = expr->get_value(tuple, tmp_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value by child expression. rc=%s", strrc(rc));
      return rc;
    }
    bool bool_value;
    if (tmp_value.attr_type() == NULLS) {
      contain_null = true;
      continue;
    } else {
      bool_value = tmp_value.get_boolean();
    }

    if ((conjunction_type_ == ConjunctionType::AND && !bool_value) ||
        (conjunction_type_ == ConjunctionType::OR && bool_value)) {
      value.set_boolean(bool_value);
      return rc;
    }
  }

  if (contain_null) {
    value.set_null();
  } else {
    value.set_boolean(conjunction_type_ == ConjunctionType::AND);
  }
  return rc;
}

RC ConjunctionExpr::try_get_value(Value &value) const
{
  RC rc = RC::SUCCESS;
  if (children_.empty()) {
    value.set_boolean(true);
    return rc;
  }

  Value tmp_value;
  bool  contain_null = false;

  for (const unique_ptr<Expression> &expr : children_) {
    rc = expr->try_get_value(tmp_value);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    bool bool_value;
    if (tmp_value.attr_type() == NULLS) {
      contain_null = true;
      bool_value   = false;
    } else {
      bool_value = tmp_value.get_boolean();
    }

    if ((conjunction_type_ == ConjunctionType::AND && !bool_value) ||
        (conjunction_type_ == ConjunctionType::OR && bool_value)) {
      value.set_boolean(bool_value);
      return rc;
    }
  }

  bool default_value = (conjunction_type_ == ConjunctionType::AND);

  if (contain_null && !default_value) {
    value.set_null();
  } else {
    value.set_boolean(default_value);
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////////////

ArithmeticExpr::ArithmeticExpr(ArithmeticType type, Expression *left, Expression *right)
    : arithmetic_type_(type), left_(left), right_(right)
{}
ArithmeticExpr::ArithmeticExpr(ArithmeticType type, unique_ptr<Expression> left, unique_ptr<Expression> right)
    : arithmetic_type_(type), left_(std::move(left)), right_(std::move(right))
{}

AttrType ArithmeticExpr::value_type() const
{
  if (!right_) {
    return left_->value_type();
  }

  if (left_->value_type() == AttrType::INTS && right_->value_type() == AttrType::INTS &&
      arithmetic_type_ != ArithmeticType::DIV) {
    return AttrType::INTS;
  }

  return AttrType::FLOATS;
}

RC ArithmeticExpr::calc_value(const Value &left_value, const Value &right_value, Value &value) const
{
  RC rc = RC::SUCCESS;

  const AttrType target_type = value_type();

  switch (arithmetic_type_) {
    case ArithmeticType::ADD: {
      if (target_type == AttrType::INTS) {
        value.set_int(left_value.get_int() + right_value.get_int());
      } else {
        value.set_float(left_value.get_float() + right_value.get_float());
      }
    } break;

    case ArithmeticType::SUB: {
      if (target_type == AttrType::INTS) {
        value.set_int(left_value.get_int() - right_value.get_int());
      } else {
        value.set_float(left_value.get_float() - right_value.get_float());
      }
    } break;

    case ArithmeticType::MUL: {
      if (target_type == AttrType::INTS) {
        value.set_int(left_value.get_int() * right_value.get_int());
      } else {
        value.set_float(left_value.get_float() * right_value.get_float());
      }
    } break;

    case ArithmeticType::DIV: {
      if (target_type == AttrType::INTS) {
        if (right_value.get_int() == 0) {
          value.set_null();
        } else {
          value.set_int(left_value.get_int() / right_value.get_int());
        }
      } else {
        if (right_value.get_float() > -EPSILON && right_value.get_float() < EPSILON) {
          value.set_null();
        } else {
          value.set_float(left_value.get_float() / right_value.get_float());
        }
      }
    } break;

    case ArithmeticType::NEGATIVE: {
      if (target_type == AttrType::INTS) {
        value.set_int(-left_value.get_int());
      } else {
        value.set_float(-left_value.get_float());
      }
    } break;

    default: {
      rc = RC::INTERNAL;
      LOG_WARN("unsupported arithmetic type. %d", arithmetic_type_);
    } break;
  }
  return rc;
}

RC ArithmeticExpr::get_value(const Tuple &tuple, Value &value) const
{
  RC rc = RC::SUCCESS;

  Value left_value;
  Value right_value;

  rc = left_->get_value(tuple, left_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
    return rc;
  }
  rc = right_->get_value(tuple, right_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
    return rc;
  }
  try {
    rc = calc_value(left_value, right_value, value);
  } catch (null_cast_exception) {
    value.set_null();
    rc = RC::SUCCESS;
  }
  return rc;
}

RC ArithmeticExpr::try_get_value(Value &value) const
{
  RC rc = RC::SUCCESS;

  Value left_value;
  Value right_value;

  rc = left_->try_get_value(left_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
    return rc;
  }

  if (right_) {
    rc = right_->try_get_value(right_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
      return rc;
    }
  }

  try {
    rc = calc_value(left_value, right_value, value);
  } catch (null_cast_exception) {
    value.set_null();
    rc = RC::SUCCESS;
  }

  return rc;
}

LikeExpr::LikeExpr(std::string partten, std::unique_ptr<Expression> child) : child_(std::move(child))
{
  stringstream ss;
  for (char c : partten) {
    if (special_chars_.find(c) != special_chars_.end()) {
      ss << "\\" << c;
    } else if (c == '%') {
      ss << ".*";
    } else if (c == '_') {
      ss << ".";
    } else {
      ss << c;
    }
  }
  // Mysql的Like是大小写不敏感的
  partten_ = regex(ss.str(), regex_constants::icase);
}

RC LikeExpr::get_value(const Tuple &tuple, Value &value) const
{
  Value  child_value;
  string child_str;
  bool   bool_value;
  RC     rc;

  rc = child_->get_value(tuple, child_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of child expression. rc=%s", strrc(rc));
    return rc;
  }

  try {
    child_str = child_value.get_string();
  } catch (null_cast_exception) {
    value.set_null();
    return RC::SUCCESS;
  }

  rc = match(child_str, bool_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to match string. rc=%s", strrc(rc));
    return rc;
  }

  value.set_boolean(bool_value);
  return rc;
}

RC LikeExpr::try_get_value(Value &value) const
{
  Value  child_value;
  string child_str;
  bool   bool_value;
  RC     rc;

  rc = child_->try_get_value(child_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of child expression. rc=%s", strrc(rc));
    return rc;
  }

  try {
    child_str = child_value.get_string();
  } catch (null_cast_exception) {
    value.set_null();
    return RC::SUCCESS;
  }

  rc = match(child_value.get_string(), bool_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to match string. rc=%s", strrc(rc));
    return rc;
  }

  value.set_boolean(bool_value);
  return rc;
}

RC LikeExpr::match(const std::string &str, bool &value) const
{
  value = regex_match(str, partten_);
  return RC::SUCCESS;
}

RC NotExpr::get_value(const Tuple &tuple, Value &value) const
{
  RC    rc;
  Value child_value;

  rc = child_->get_value(tuple, child_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of child expression. rc=%s", strrc(rc));
    return rc;
  }

  try {
    bool bool_value = child_value.get_boolean();
    value.set_boolean(!bool_value);
  } catch (null_cast_exception) {
    value.set_null();
    return RC::SUCCESS;
  }

  return RC::SUCCESS;
}

RC NotExpr::try_get_value(Value &value) const
{
  RC    rc;
  Value child_value;

  rc = child_->try_get_value(child_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of child expression. rc=%s", strrc(rc));
    return rc;
  }

  try {
    bool bool_value = child_value.get_boolean();
    value.set_boolean(!bool_value);
  } catch (null_cast_exception) {
    value.set_null();
    return RC::SUCCESS;
  }

  return RC::SUCCESS;
}

RC TupleCellExpr::get_value(const Tuple &tuple, Value &value) const { return tuple.find_cell(cell_spec_, value); }

RC InExpr::get_value(const Tuple &tuple, Value &value) const
{
  Value      left;
  set<Value> values;  // 不包括NULL的值
  RC         rc;
  bool       contain_null = false;

  rc = left_->get_value(tuple, left);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
    return rc;
  }

  if (subquery_ref_) {
    Value  subquery_string_value;
    string subquery_string;
    subquery_ref_->get_value(tuple, subquery_string_value);
    subquery_string = subquery_string_value.get_string();

    auto cur_values = common::string_to_arr(subquery_string.c_str());
    for (Value &value : cur_values) {
      if (value.attr_type() == NULLS) {
        contain_null = true;
      } else {
        values.insert(value);
      }
    }
  } else {
    for (const auto &expr : value_list_) {
      Value v;
      RC    rc = expr->get_value(tuple, v);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
        return rc;
      }
      if (v.attr_type() == NULLS) {
        contain_null = true;
      } else {
        values.insert(v);
      }
    }
  }

  return is_in(left, values, contain_null, value);
}

RC InExpr::try_get_value(Value &value) const
{
  Value      left;
  set<Value> values;  // 不包括NULL的值
  RC         rc;
  bool       contain_null = false;

  rc = left_->try_get_value(left);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  if (subquery_ref_) {
    Value  subquery_string_value;
    string subquery_string;
    subquery_ref_->try_get_value(subquery_string_value);
    subquery_string = subquery_string_value.get_string();

    auto cur_values = common::string_to_arr(subquery_string.c_str());
    for (Value &value : cur_values) {
      if (value.attr_type() == NULLS) {
        contain_null = true;
      } else {
        values.insert(value);
      }
    }
  } else {
    for (const auto &expr : value_list_) {
      Value v;
      RC    rc = expr->try_get_value(v);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
        return rc;
      }
      if (v.attr_type() == NULLS) {
        contain_null = true;
      } else {
        values.insert(v);
      }
    }
  }

  return is_in(left, values, contain_null, value);
}

RC InExpr::is_in(const Value &left_value, const std::set<Value> &right_values, bool contain_null, Value &res) const
{

  if (left_value.attr_type() == NULLS) {
    res.set_null();
    return RC::SUCCESS;
  }

  auto it = right_values.find(left_value);
  if (it != right_values.end()) {
    res.set_boolean(true);
  } else if (contain_null) {
    res.set_null();
  } else {
    res.set_boolean(false);
  }

  return RC::SUCCESS;
}

RC ExistsExpr::get_value(const Tuple &tuple, Value &value) const
{
  assert(subquery_ref_);
  return subquery_ref_->get_value(tuple, value);
}

RC ExistsExpr::try_get_value(Value &value) const { return RC::UNIMPLENMENT; }

RC IsNullExpr::get_value(const Tuple &tuple, Value &value) const
{
  RC rc = child_->get_value(tuple, value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of child expression. rc=%s", strrc(rc));
    return rc;
  }

  value.set_boolean(value.attr_type() == NULLS);
  return RC::SUCCESS;
}

RC IsNullExpr::try_get_value(Value &value) const
{
  RC rc = child_->try_get_value(value);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  value.set_boolean(value.attr_type() == NULLS);
  return RC::SUCCESS;
}
