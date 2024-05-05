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
// Created by WangYunlai on 2023/06/28.
//

#include "sql/parser/value.h"
#include "common/lang/comparator.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "common/types.h"
#include <sstream>
#include <ctime>
#include "common/common_utils.h"

// UNDEFINED,
//     CHARS,     ///< 字符串类型
//     INTS,      ///< 整数类型(4字节)
//     FLOATS,    ///< 浮点数类型(4字节)
//     BOOLEANS,  ///< boolean类型，当前不是由parser解析出来的，是程序内部使用的
//     DATES,     ///< 日期类型(4字节)
//     TEXTS,     ///< 长文本类型，等同于字符串
//     NULLS      ///< null

const char *ATTR_TYPE_NAME[] = {"undefined", "chars", "ints", "floats", "booleans", "dates", "texts", "nulls"};

const char *attr_type_to_string(AttrType type)
{
  if (type >= UNDEFINED && type <= NULLS) {
    return ATTR_TYPE_NAME[type];
  }
  return "unknown";
}
AttrType attr_type_from_string(const char *s)
{
  for (unsigned int i = 0; i < sizeof(ATTR_TYPE_NAME) / sizeof(ATTR_TYPE_NAME[0]); i++) {
    if (0 == strcmp(ATTR_TYPE_NAME[i], s)) {
      return (AttrType)i;
    }
  }
  return UNDEFINED;
}

Value::Value(int val) { set_int(val); }

Value::Value(float val) { set_float(val); }

Value::Value(bool val) { set_boolean(val); }

Value::Value(const char *s, int len /*= 0*/) { set_string(s, len); }

void Value::set_data(char *data, int length)
{
  data_   = new char[length];
  length_ = length;
  memcpy(data_, data, length);
}
void Value::set_int(int val)
{
  attr_type_ = INTS;
  set_data((char *)&val, 4);
}

void Value::set_float(float val)
{
  attr_type_ = FLOATS;
  set_data((char *)&val, 4);
}
void Value::set_boolean(bool val)
{
  attr_type_ = BOOLEANS;
  set_data((char *)&val, 1);
}
void Value::set_string(const char *s, int len /*= 0*/)
{
  attr_type_ = CHARS;
  if (len > 0) {
    set_data(s, len + 1);
  } else {
    set_data("\0", 1);
  }
  length_ = len + 1;
}

void Value::set_value(const Value &value)
{
  attr_type_ = value.attr_type_;
  set_data(value.data_, value.length_);
}

void Value::set_date(const sql_date date) {
  attr_type_ = DATES;
  set_data((char *)&date, 4);
}

const char *Value::data() const { return data_; }

std::string Value::to_string() const
{
  std::stringstream os;
  switch (attr_type_) {
    case INTS: {
      os << *((int *)data_);
    } break;
    case FLOATS: {
      os << common::double_to_str(*((float *)data_));
    } break;
    case BOOLEANS: {
      os << *((bool *)data_);
    } break;
    case CHARS: {
      os << data_;
    } break;
    case DATES: {
      os << common::date::format(*(sql_date *)data_);
    } break;
    case TEXTS: {
      os << data_;
    } break;
    case NULLS: {
      os << NULL_STRING;
    } break;
    default: {
      assert(false);
    } break;
  }
  return os.str();
}

int Value::compare(const Value &other) const
{
  int cmp_res = -1;
  RC  rc      = common::compare(*this, other, cmp_res);
  return cmp_res;
}

int Value::get_int() const
{
  int res;
  assert(common::get_int(*this, res) == RC::SUCCESS);
  return res;
}

float Value::get_float() const
{
  float res;
  assert(common::get_float(*this, res) == RC::SUCCESS);
  return res;
}

std::string Value::get_string() const { return this->to_string(); }

bool Value::get_boolean() const
{
  bool res;
  assert(common::get_boolean(*this, res) == RC::SUCCESS);
  return res;
}
Value::~Value() { delete[] data_; }
Value Value::get_null_value() {
  Value value;
  value.data_ = nullptr;
  value.attr_type_ = NULLS;
  value.length_ = 0;
  return value;
}
Value::Value(sql_date date) {
    set_date(date);
}

RC common::compare(const Value &value1, const Value &value2, int &cmp_res)
{
  if (value1.attr_type() == value2.attr_type() || (value1.attr_type() == CHARS && value2.attr_type() == TEXTS) ||
      (value1.attr_type() == TEXTS && value2.attr_type() == CHARS)) {
    switch (value1.attr_type()) {
      case INTS: {
        cmp_res = common::compare_int((void *)value1.data(), (void *)value2.data());
      } break;
      case FLOATS: {
        cmp_res = common::compare_float((void *)value1.data(), (void *)value2.data());
      } break;
      case TEXTS:
      case CHARS: {
        cmp_res = common::compare_string(
            (void *)value1.data(), value1.length() - 1, (void *)value2.data(), value2.length() - 1);
      } break;
      case BOOLEANS: {
        cmp_res = common::compare_bool((void *)value1.data(), (void *)value2.data());
      } break;
      default: {
        return RC::UNCOMPARIBLE;
      }
    }
  } else if ((value1.attr_type() == INTS && value2.attr_type() == FLOATS) ||
             (value1.attr_type() == FLOATS && value2.attr_type() == INTS)) {
    float this_data  = value1.get_float();
    float other_data = value2.get_float();
    cmp_res          = common::compare_float((void *)&this_data, (void *)&other_data);
  } else {
    return RC::UNCOMPARIBLE;
  }
  return RC::SUCCESS;
}

RC common::get_int(const Value &value, int &res)
{
  switch (value.attr_type()) {
    case TEXTS:
    case CHARS: {
      try {
        res = (int)(std::stol(value.data()));
      } catch (std::exception const &ex) {
        return RC::BAD_CAST;
      }
    } break;
    case INTS: {
      res = *(int *)value.data();
    } break;
    case FLOATS: {
      res = (int)(*(float *)value.data());
    } break;
    case BOOLEANS: {
      res = (int)(*(bool *)value.data());
    } break;
    case NULLS: {
      res = 0;
    } break;
    case DATES: {
      res = common::date::get_timestamp(*(sql_date *)value.data());
    } break;
    default: {
      return RC::BAD_CAST;
    }
  }
  return RC::SUCCESS;
}
RC common::get_float(const Value &value, float &res)
{
  switch (value.attr_type()) {
    case TEXTS:
    case CHARS: {
      try {
        res = std::stof(value.data());
      } catch (std::exception const &ex) {
        return RC::BAD_CAST;
      }
    } break;
    case INTS: {
      res = (float)*(int *)value.data();
    } break;
    case FLOATS: {
      res = *(float *)value.data();
    } break;
    case BOOLEANS: {
      res = (float)*(bool *)value.data();
    } break;
    case DATES: {
      res = (float)common::date::get_timestamp(*(sql_date *)value.data());
    } break;
    case NULLS: {
      res = 0;
    } break;
    default: {
      return RC::BAD_CAST;
    }
  }
  return RC::SUCCESS;
}
RC common::get_string(const Value &value, std::string &res)
{
  res = value.to_string();
  return RC::SUCCESS;
}
RC common::get_boolean(const Value &value, bool &res)
{
  switch (value.attr_type()) {
    case TEXTS:
    case CHARS: {
      try {
        float val = std::stof(value.data());
        if (val >= EPSILON || val <= -EPSILON) {
          res = true;
        }

        int int_val = std::stol(value.data());
        if (int_val != 0) {
          res = true;
        }

        res = value.length() != 0;
      } catch (std::exception const &ex) {
        return RC::BAD_CAST;
      }
    } break;
    case INTS: {
      res = *(int *)value.data() != 0;
    } break;
    case FLOATS: {
      float val = *(float *)value.data();
      res       = val >= EPSILON || val <= -EPSILON;
    } break;
    case BOOLEANS: {
      res = *(bool *)value.data();
    } break;
    case DATES: {
      res = true;
    } break;
    case NULLS: {
      res = false;
    } break;
    default: {
      return RC::BAD_CAST;
    }
  }
  return RC::SUCCESS;
}
