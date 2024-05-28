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
// Created by Wangyunlai 2023/6/27
//

#pragma once

#include "common/rc.h"
#include <string>

/**
 * @brief 属性的类型
 *
 */
#define ATTR_TYPE_DEFS                                 \
  ATTR_TYPE_DEF(UNDEFINED)                             \
  ATTR_TYPE_DEF(CHARS)    /*字符串类型*/          \
  ATTR_TYPE_DEF(INTS)     /*整数类型(4字节)*/    \
  ATTR_TYPE_DEF(FLOATS)   /*浮点数类型(4字节)*/ \
  ATTR_TYPE_DEF(BOOLEANS) /*boolean类型*/            \
  ATTR_TYPE_DEF(DATES)    /*日期类型*/             \
  ATTR_TYPE_DEF(TEXTS)    /*长文本类型*/          \
  ATTR_TYPE_DEF(NULLS)    /*null类型*/

enum AttrType
{
#define ATTR_TYPE_DEF(type) type,
  ATTR_TYPE_DEFS
#undef ATTR_TYPE_DEF
};

const char *attr_type_to_string(AttrType type);
AttrType    attr_type_from_string(const char *s);

class bad_cast_exception
{};

/**
 * @brief 属性的值
 *
 */
class Value
{
public:
  Value() = default;

  Value(AttrType attr_type, char *data, int length = 4) : attr_type_(attr_type) { this->set_data(data, length); }

  explicit Value(int val);
  explicit Value(float val);
  explicit Value(bool val);
  explicit Value(const char *s, int len = 0);

  Value(const Value &other)            = default;
  Value &operator=(const Value &other) = default;

  void set_type(AttrType type) { this->attr_type_ = type; }
  void set_data(char *data, int length);
  void set_data(const char *data, int length) { this->set_data(const_cast<char *>(data), length); }
  void set_int(int val);
  void set_float(float val);
  void set_boolean(bool val);
  void set_string(const char *s, int len = 0);
  void set_value(const Value &value);
  void set_date(const char *str);
  void set_text(const char *str);
  void set_null();

  std::string to_string() const;

  //@throw bad_cast_exception 如果存在null导致无法比较
  int compare(const Value &other) const;

  const char *data() const;
  int         length() const { return length_; }

  AttrType attr_type() const { return attr_type_; }

public:
  /**
   * 获取对应的值
   * 如果当前的类型与期望获取的类型不符，就会执行转换操作
   * @throw bad_cast_exception 如果存在null导致转换失败
   */
  int         get_int() const;
  float       get_float() const;
  std::string get_string() const;
  bool        get_boolean() const;
  double      get_double() const; // get double value，并非是解析出来的

private:
  AttrType attr_type_ = UNDEFINED;
  int      length_    = 0;

  union
  {
    int   int_value_;
    float float_value_;
    bool  bool_value_;
  } num_value_;
  std::string str_value_;
};


namespace common {
  RC try_convert_value(const Value &value, AttrType type, Value &res);
}