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
// Created by Wangyunlai on 2022/5/4.
//

#pragma once

#include <limits>
#include <sstream>
#include <stddef.h>
#include <vector>

#include "common/log/log.h"
#include "common/rc.h"
#include "common/types.h"
#include "storage/field/field_meta.h"
#include "storage/index/index_meta.h"

class Field;


/**
 * @brief 标识一个记录的位置
 * 一个记录是放在某个文件的某个页面的某个槽位。这里不记录文件信息，记录页面和槽位信息
 */
struct RID {
    PageNum page_num;  // record's page number
    SlotNum slot_num;  // record's slot number

    RID() = default;

    RID(const PageNum _page_num, const SlotNum _slot_num) : page_num(_page_num), slot_num(_slot_num) {}

    const std::string to_string() const {
        std::stringstream ss;
        ss << "PageNum:" << page_num << ", SlotNum:" << slot_num;
        return ss.str();
    }

    bool operator==(const RID &other) const { return page_num == other.page_num && slot_num == other.slot_num; }

    bool operator!=(const RID &other) const { return !(*this == other); }

    static int compare(const RID *rid1, const RID *rid2) {
        int page_diff = rid1->page_num - rid2->page_num;
        if (page_diff != 0) {
            return page_diff;
        } else {
            return rid1->slot_num - rid2->slot_num;
        }
    }

    /**
     * 返回一个不可能出现的最小的RID
     * 虽然page num 0和slot num 0都是合法的，但是page num 0通常用于存放meta数据，所以对数据部分来说都是
     * 不合法的. 这里在bplus tree中查找时会用到。
     */
    static RID *min() {
        static RID rid{0, 0};
        return &rid;
    }

    /**
     * @brief 返回一个“最大的”RID
     * 我们假设page num和slot num都不会使用对应数值类型的最大值
     */
    static RID *max() {
        static RID rid{std::numeric_limits<PageNum>::max(), std::numeric_limits<SlotNum>::max()};
        return &rid;
    }
};

struct RIDHash {
    size_t operator()(const RID &rid) const noexcept {
        return std::hash<PageNum>()(rid.page_num) ^ std::hash<SlotNum>()(rid.slot_num);
    }
};

#define RECORD_TYPE_NORMAL 0
#define RECORD_TYPE_EXPAND_DATA 1

struct RecordHeader {
    /**
     * 0 正常记录
     * 1 扩充数据
     */
    uint8_t record_type;
    RID next_rid;
};

/**
 * @brief 表示一个记录
 * @details 当前的记录都是连续存放的空间（内存或磁盘上）。
 * 为了提高访问的效率，record通常直接记录指向页面上的内存，但是需要保证访问这种数据时，拿着锁资源。
 * 为了方便，也提供了复制内存的方法。可以参考set_data_owner
 * @note 可以拆分成两种实现，一个是需要自己管理内存的，一个是不需要自己管理内存的。
 */
class Record {
public:
    Record() = default;

    ~Record() {
        delete next_record_;
        next_record_ = nullptr;
        if (owner_ && data_ != nullptr) {
            free(data_);
            data_ = nullptr;
        }
    }

    Record(const Record &other) {
        rid_ = other.rid_;
        data_ = other.data_;
        len_ = other.len_;
        owner_ = other.owner_;

        if (other.owner_) {
            char *tmp = (char *) malloc(other.len_);
            ASSERT(nullptr != tmp, "failed to allocate memory. size=%d", other.len_);
            memcpy(tmp, other.data_, other.len_);
            data_ = tmp;
        }
    }


    Record(Record &&other) {
        rid_ = other.rid_;

        data_ = other.data_;
        len_ = other.len_;
        next_record_ = other.next_record_;

        other.data_ = nullptr;
        other.len_ = 0;
        other.next_record_ = nullptr;

        this->owner_ = other.owner_;
    }

    Record &operator=(const Record &other) {
        if (this == &other) {
            return *this;
        }
        new(this) Record(other);

        return *this;
    }

    Record &operator=(Record &&other) {
        if (this == &other) {
            return *this;
        }
        new(this) Record(std::move(other));

        return *this;
    }

    void set_data(char *data, int len = 0) {
        this->data_ = data;
        this->len_ = len;
    }

    void set_data_owner(char *data, int len) {
        ASSERT(len != 0, "the len of data should not be 0");
        this->~Record();

        this->data_ = data;
        this->len_ = len;
        this->owner_ = true;
    }

    RC copy_data(const char *data, int len) {
        ASSERT(len != 0, "the len of data should not be 0");
        this->~Record();
        char *tmp = (char *) malloc(len);
        if (nullptr == tmp) {
            LOG_WARN("failed to allocate memory. size=%d", len);
            return RC::NOMEM;
        }

        memcpy(tmp, data, len);
        set_data_owner(tmp, len);
        return RC::SUCCESS;
    }

    char *data() { return this->data_; }

    const char *data() const { return this->data_; }

    int len() const { return this->len_; }

    void set_rid(const RID &rid) { this->rid_ = rid; }

    void set_rid(const PageNum page_num, const SlotNum slot_num) {
        this->rid_.page_num = page_num;
        this->rid_.slot_num = slot_num;
    }

    RID &rid() { return rid_; }

    const RID &rid() const { return rid_; }

    Record *next_record() { return next_record_; }

    void move_as_near(Record &record) {
        delete next_record_;
        next_record_ = new Record;
        *next_record_ = std::move(record);
    }

private:

    RID rid_;

    Record *next_record_ = nullptr;

    char *data_ = nullptr; /// 不包括RecordHeader的数据
    int len_ = 0;      /// 如果不是record自己来管理内存，这个字段可能是无效的
    bool owner_ = false;  /// 表示当前是否由record来管理内存
};
