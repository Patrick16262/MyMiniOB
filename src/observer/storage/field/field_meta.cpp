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
// Created by Meiyi & Wangyunlai on 2021/5/12.
//

#include "storage/field/field_meta.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "sql/parser/parse_defs.h"

#include "json/json.h"

const static Json::StaticString FIELD_NAME("name");
const static Json::StaticString FIELD_TYPE("type");
const static Json::StaticString FIELD_OFFSET("offset");
const static Json::StaticString FIELD_LEN("len");
const static Json::StaticString FIELD_VISIBLE("visible");
const static Json::StaticString FIELD_LEN_VARIABLE("len_variable");
const static Json::StaticString FIELD_NULLABLE("null");
const static Json::StaticString FIELD_UNIQUE("unique");

FieldMeta::FieldMeta() {}


void
FieldMeta::init(const char *name, AttrType attr_type, int attr_offset, int attr_len, bool visible, bool len_variable,
                bool nullable, bool unique) {
    assert(!common::is_blank(name));
    assert(AttrType::UNDEFINED != attr_type);
    assert(attr_offset >= 0);
    assert(attr_len > 0);

    name_ = name;
    attr_type_ = attr_type;
    attr_len_ = attr_len;
    attr_offset_ = attr_offset;
    visible_ = visible;
    actual_len_variable_ = len_variable;
    nullable_ = nullable;
    unique_ = unique;
}

const char *FieldMeta::name() const { return name_.c_str(); }

AttrType FieldMeta::type() const { return attr_type_; }

int FieldMeta::offset() const { return attr_offset_; }

int FieldMeta::len() const { return attr_len_; }

bool FieldMeta::visible() const { return visible_; }

bool FieldMeta::is_len_variable() const {
    return actual_len_variable_;
}

bool FieldMeta::unique() const {
    return unique_;
}

bool FieldMeta::nullable() const {
    return nullable_;
}

void FieldMeta::desc(std::ostream &os) const {
    os << "field name=" << name_
       << ", type=" << attr_type_to_string(attr_type_)
       << ", len=" << attr_len_
       << ", nullable=" << (nullable_ ? "yes" : "no")
       << ", unique=" << (unique_ ? "yes" : "no")
       << ", visible=" << (visible_ ? "yes" : "no");
}

void FieldMeta::to_json(Json::Value &json_value) const {
    json_value[FIELD_NAME] = name_;
    json_value[FIELD_TYPE] = attr_type_to_string(attr_type_);
    json_value[FIELD_OFFSET] = attr_offset_;
    json_value[FIELD_LEN] = attr_len_;
    json_value[FIELD_VISIBLE] = visible_;
    json_value[FIELD_LEN_VARIABLE] = actual_len_variable_;
    json_value[FIELD_NULLABLE] = nullable_;
    json_value[FIELD_UNIQUE] = unique_;
}

void FieldMeta::from_json(const Json::Value &json_value, FieldMeta &field) {
    assert(json_value.isObject());

    const Json::Value &name_value = json_value[FIELD_NAME];
    const Json::Value &type_value = json_value[FIELD_TYPE];
    const Json::Value &offset_value = json_value[FIELD_OFFSET];
    const Json::Value &len_value = json_value[FIELD_LEN];
    const Json::Value &visible_value = json_value[FIELD_VISIBLE];
    const Json::Value &len_variable_value = json_value[FIELD_LEN_VARIABLE];
    const Json::Value &unique_value = json_value[FIELD_UNIQUE];
    const Json::Value &nullable_value = json_value[FIELD_NULLABLE];

    assert(name_value.isString());
    assert(type_value.isString());
    assert(offset_value.isInt());
    assert(len_value.isInt());
    assert(visible_value.isBool());
    assert(len_variable_value.isBool());
    assert(unique_value.isBool());
    assert(nullable_value.isBool());

    AttrType type = attr_type_from_string(type_value.asCString());
    assert(type != UNDEFINED);

    const char *name = name_value.asCString();
    int offset = offset_value.asInt();
    int len = len_value.asInt();
    bool visible = visible_value.asBool();
    bool len_variable = len_variable_value.asBool();
    bool unique = unique_value.asBool();
    bool nullable = nullable_value.asBool();

    field.init(name, type, offset, len, visible, len_variable, nullable, unique);
}


