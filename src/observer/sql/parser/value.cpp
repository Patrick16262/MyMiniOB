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
#include "sql/parser/parse_defs.h"
#include <cassert>
#include <ranges>
#include <sstream>
#include <string>

// const char *ATTR_TYPE_NAME[] = {"undefined", "chars", "ints", "floats", "booleans"};

const char *attr_type_to_string(AttrType type)
{
#define ATTR_TYPE_DEF(type) \
  case type: return #type;
  switch (type) {
    ATTR_TYPE_DEFS
#undef ATTR_TYPE_DEF

    default: {
      LOG_WARN("unknown type: %d", type);
      return "unknown";
    }
  }
}
AttrType attr_type_from_string(const char *s)
{
#define ATTR_TYPE_DEF(type)  \
  if (strcmp(s, #type) == 0) \
    return type;
  ATTR_TYPE_DEFS
#undef ATTR_TYPE_DEF
  return UNDEFINED;
}

Value::Value(int val) { set_int(val); }

Value::Value(float val) { set_float(val); }

Value::Value(bool val) { set_boolean(val); }

Value::Value(const char *s, int len /*= 0*/) { set_string(s, len); }

void Value::set_data(char *data, int length)
{
  switch (attr_type_) {
    case CHARS: {
      set_string(data, length);
    } break;
    case INTS: {
      num_value_.int_value_ = *(int *)data;
      length_               = length;
    } break;
    case FLOATS: {
      num_value_.float_value_ = *(float *)data;
      length_                 = length;
    } break;
    case BOOLEANS: {
      num_value_.bool_value_ = *(int *)data != 0;
      length_                = length;
    } break;
    case DATES: {
      num_value_.int_value_ = *(int *)data;
      length_               = length;
    } break;
    case TEXTS: {
      ASSERT(false, "TODO not implemented");
    }
    case NULLS: {
      assert(false);
    }
    default: {
      assert(false);
    } break;
  }
}
void Value::set_int(int val)
{
  attr_type_            = INTS;
  num_value_.int_value_ = val;
  length_               = sizeof(val);
}

void Value::set_float(float val)
{
  attr_type_              = FLOATS;
  num_value_.float_value_ = val;
  length_                 = sizeof(val);
}
void Value::set_boolean(bool val)
{
  attr_type_             = BOOLEANS;
  num_value_.bool_value_ = val;
  length_                = sizeof(val);
}
void Value::set_string(const char *s, int len /*= 0*/)
{
  attr_type_ = CHARS;
  if (len > 0) {
    len = strnlen(s, len);
    str_value_.assign(s, len);
  } else {
    str_value_.assign(s);
  }
  length_ = str_value_.length();
}

namespace common {
// 日期格式：YYYY-MM-DD
// 遇到非法日期格式，返回0
int parseDate(const char *str)
{
  int year  = 0;
  int month = 0;
  int day   = 0;
  int ret   = sscanf(str, "%d-%d-%d", &year, &month, &day);
  if (ret != 3) {
    return 0;
  }

  if (year < 0 || month < 1 || month > 12 || day < 1 || day > 31) {
    return 0;
  }

  if (month == 2) {
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
      if (day > 29) {
        return 0;
      }
    } else {
      if (day > 28) {
        return 0;
      }
    }
  } else if (month == 4 || month == 6 || month == 9 || month == 11) {
    if (day > 30) {
      return 0;
    }
  }

  return year * 10000 + month * 100 + day;
}

/**
 * 按照mysql的日期格式，将日期转换为字符串
 */
std::string date_to_string(int date)
{
  if (date == 0) {
    return "0000-00-00";
  }

  int year  = date / 10000;
  int month = (date % 10000) / 100;
  int day   = date % 100;

  char buf[16];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
  return buf;
}

}  // namespace common

void Value::set_date(const char *str)
{
  num_value_.int_value_ = common::parseDate(str);
  if (num_value_.int_value_ == 0) {
    set_null();
  } else {
    attr_type_ = DATES;
    length_    = sizeof(num_value_);
  }
}

void Value::set_text(const char *str)
{
  attr_type_ = TEXTS;
  str_value_ = str;
  length_    = str_value_.length();
}

void Value::set_null()
{
  attr_type_ = NULLS;
  length_    = 0;
}

void Value::set_value(const Value &value)
{
  switch (value.attr_type_) {
    case INTS: {
      set_int(value.get_int());
    } break;
    case FLOATS: {
      set_float(value.get_float());
    } break;
    case CHARS: {
      set_string(value.get_string().c_str());
    } break;
    case BOOLEANS: {
      set_boolean(value.get_boolean());
    } break;
    case DATES: {
      std::string str = value.to_string();
      set_date(str.c_str());
    } break;
    case TEXTS: {
      set_text(value.get_string().c_str());
    } break;
    case NULLS: {
      set_null();
    } break;
    case UNDEFINED: {
      ASSERT(false, "got an invalid value type");
    } break;
  }
}

const char *Value::data() const
{
  switch (attr_type_) {
    case CHARS:
    case TEXTS: {
      return str_value_.c_str();
    } break;
    case NULLS: {
      assert(false);
    }
    case UNDEFINED: {
      assert(false);
    }
    default: {
      return (const char *)&num_value_;
    } break;
  }
}

std::string Value::to_string() const
{
  std::stringstream os;
  switch (attr_type_) {
    case INTS: {
      os << num_value_.int_value_;
    } break;
    case FLOATS: {
      os << common::double_to_str(num_value_.float_value_);
    } break;
    case BOOLEANS: {
      os << num_value_.bool_value_;
    } break;
    case CHARS: {
      os << str_value_;
    } break;
    case DATES: {
      os << common::date_to_string(num_value_.int_value_);
    } break;
    case TEXTS: {
      os << str_value_;
    } break;
    case NULLS: {
      os << "NULL";
    } break;
    case UNDEFINED: {
      os << "UNDEFINED";
    } break;
  }
  return os.str();
}

// todo: compare data and null
int Value::compare(const Value &other) const
{
  Value  tmp;
  
  double first;
  double second;

  if (attr_type_ == DATES || other.attr_type_ == CHARS) {
    std::string str = other.get_string();
    tmp.set_date(str.c_str());
    first  = static_cast<double>(get_int());
    second = static_cast<double>(tmp.get_int());
  } else if (attr_type_ == CHARS || other.attr_type_ == DATES) {
    std::string str = get_string();
    tmp.set_date(str.c_str());
    first  = static_cast<double>(tmp.get_int());
    second = static_cast<double>(other.get_int());
  } else {
    first  = get_double();
    second = other.get_double();
  }

  double diff = first - second;
  if (diff > EPSILON) {
    return 1;
  } else if (diff < -EPSILON) {
    return -1;
  } else {
    return 0;
  }
}

int Value::get_int() const
{
  switch (attr_type_) {
    case TEXTS:
    case CHARS: {
      try {
        return (int)(std::stol(str_value_));
      } catch (std::exception const &ex) {
        LOG_TRACE("failed to convert string to number. s=%s, ex=%s", str_value_.c_str(), ex.what());
        return 0;
      }
    }
    case INTS: {
      return num_value_.int_value_;
    }
    case FLOATS: {
      return (int)(num_value_.float_value_);
    }
    case BOOLEANS: {
      return (int)(num_value_.bool_value_);
    }
    case DATES: {
      return num_value_.int_value_;
    }
    case NULLS: {
      throw bad_cast_exception();
    }
    case UNDEFINED: {
      assert(false);
    }
  }
  return 0;
}

float Value::get_float() const
{
  switch (attr_type_) {
    case TEXTS:
    case CHARS: {
      try {
        return std::stof(str_value_);
      } catch (std::exception const &ex) {
        LOG_TRACE("failed to convert string to float. s=%s, ex=%s", str_value_.c_str(), ex.what());
        return 0.0;
      }
    } break;
    case INTS: {
      return float(num_value_.int_value_);
    } break;
    case FLOATS: {
      return num_value_.float_value_;
    } break;
    case BOOLEANS: {
      return float(num_value_.bool_value_);
    } break;
    case DATES: {
      // todo 检查精度
      return float(num_value_.int_value_);
    }
    case NULLS: {
      throw bad_cast_exception();
    }
    case UNDEFINED: {
      assert(false);
    }
  }
  return 0;
}

std::string Value::get_string() const { return this->to_string(); }

// 在mysql中 boolean实际为tinyint(1)
bool Value::get_boolean() const
{
  switch (attr_type_) {
    case TEXTS:
    case CHARS: {
      try {
        float val = std::stof(str_value_);
        if (val >= EPSILON || val <= -EPSILON) {
          return true;
        }

        int int_val = std::stol(str_value_);
        if (int_val != 0) {
          return true;
        }

        return !str_value_.empty();
      } catch (std::exception const &ex) {
        LOG_TRACE("failed to convert string to float or integer. s=%s, ex=%s", str_value_.c_str(), ex.what());
        return !str_value_.empty();
      }
    } break;
    case INTS: {
      return num_value_.int_value_ != 0;
    } break;
    case FLOATS: {
      float val = num_value_.float_value_;
      return val >= EPSILON || val <= -EPSILON;
    } break;
    case BOOLEANS: {
      return num_value_.bool_value_;
    } break;
    case DATES: {
      return num_value_.int_value_ != 0;
    }
    case NULLS: {
      throw bad_cast_exception();
    }
    case UNDEFINED: {
      assert(false);
    }
  }
  return false;
}

double Value::get_double() const
{
  switch (attr_type_) {
    case TEXTS:
    case CHARS: {
      try {
        return std::stod(str_value_);
      } catch (std::exception const &ex) {
        LOG_TRACE("failed to convert string to double. s=%s, ex=%s", str_value_.c_str(), ex.what());
        return 0.0;
      }
    } break;
    case INTS: {
      return double(num_value_.int_value_);
    } break;

    case FLOATS: {
      return double(num_value_.float_value_);
    } break;
    case BOOLEANS: {
      return double(num_value_.bool_value_);
    } break;
    case DATES: {
      return double(num_value_.int_value_);
    }
    case NULLS: {
      throw bad_cast_exception();
    }
    case UNDEFINED: {
      assert(false);
    }
  }
  return 0;
}

namespace common {
/**
 * 将value转换为指定的类型
 * @param value 需要转换的值
 * @param type 期望转换的类型, 不应为Undefined
 * @param res 转换后的值
 * @return RC::SUCCESS 转换成功
 *         RC::NULL_VALUE 如果value为NULL
 */
RC try_convert_value(const Value &value, AttrType type, Value &res)
{
  try {
    switch (type) {
      case UNDEFINED: {
        assert(false);
      } break;
      case CHARS: {
        std::string str = value.get_string();
        res.set_string(str.c_str());
      } break;
      case INTS: {
        res.set_int(value.get_int());
      } break;
      case FLOATS: {
        res.set_float(value.get_float());
      } break;
      case BOOLEANS: {
        res.set_boolean(value.get_boolean());
      } break;
      case DATES: {
        std::string str = value.get_string();
        res.set_date(str.c_str());
        if (res.attr_type() == NULLS) {
          throw bad_cast_exception();
        }
      } break;
      case TEXTS: {
        std::string str = value.get_string();
        res.set_text(str.c_str());
      } break;
      case NULLS: {
        res.set_null();
        return RC::NULL_VALUE;
      } break;
    }
  } catch (bad_cast_exception) {
    res.set_null();
    return RC::NULL_VALUE;
  }
  return RC::SUCCESS;
}

}  // namespace common
