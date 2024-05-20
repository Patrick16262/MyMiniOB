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

#include <string>
#include "common/lang/mutex.h"
#include "common/rc.h"
#include "common/types.h"


#define NULL_STRING "NULL"

/**
 * @brief 属性的类型
 *
 */
enum AttrType {
    UNDEFINED,
    CHARS,     ///< 字符串类型
    INTS,      ///< 整数类型(4字节)
    FLOATS,    ///< 浮点数类型(4字节)
    BOOLEANS,  ///< boolean类型，当前不是由parser解析出来的，是程序内部使用的
    DATES,     ///< 日期类型(4字节)
    TEXTS,     ///< 长文本类型，等同于字符串
    NULLS      ///< null
};

const char *attr_type_to_string(AttrType type);

AttrType attr_type_from_string(const char *s);

/**
 * 线程不安全的Value类，是数据库中所有数据的载体
 *  8字节类型安全，所有数据都会对齐到8字节
 */
class Value {
public:
    Value() = default;

    Value(AttrType attr_type, char *data, int length = 4);

    virtual ~Value();

    explicit Value(int val);

    explicit Value(float val);

    explicit Value(bool val);

    explicit Value(const char *s, int len = 0);

    explicit Value(sql_date date);

    static Value get_null_value();

    Value &operator=(const Value &other);

    Value &operator=( Value &&other);

    Value(const Value &other);

    Value(Value &&other);
public:

    void set_type(AttrType type) { this->attr_type_ = type; }

    /**
     * 拷贝data内容
     * @param data
     * @param length
     */
    void copy_data(const char *data, int length);

    void set_int(int val);

    void set_float(float val);

    void set_boolean(bool val);

    void set_string(const char *s, int len = 0);

    void set_value(const Value &value);

    void set_date(sql_date date);

    std::string to_string() const;

    int compare(const Value &other) const;

    const char *data() const;

    int length() const { return length_; }

    AttrType attr_type() const { return attr_type_; }

public:
    /**
     * 获取对应的值
     * 如果当前的类型与期望获取的类型不符，就会执行转换操作
     */
    int get_int() const;

    float get_float() const;

    std::string get_string() const;

    bool get_boolean() const;

private:
    AttrType attr_type_ = UNDEFINED;
    int length_ = 0;
    const char *data_ = nullptr;
    //  union
    //  {
    //    int   int_value_;
    //    float float_value_;
    //    bool  bool_value_;
    //  } num_value_;
    //  std::string str_value_;
};

namespace common {
    RC compare(const Value &value1, const Value &value2, int &cmp_res);

    RC get_as_int(const Value &value, int &res);

    RC get_as_float(const Value &value, float &res);

    RC get_as_string(const Value &value, std::string &res);

    RC get_as_boolean(const Value &value, bool &res);
}  // namespace common