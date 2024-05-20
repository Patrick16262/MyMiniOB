/* Copyright (c) 2021 Xie Meiyi(xiemeiyi@hust.edu.cn) and OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Meiyi & Longda on 2021/4/13.
//
#include "storage/record/record_manager.h"
#include "common/log/log.h"
#include "storage/common/condition_filter.h"
#include "storage/trx/trx.h"
#include "storage/clog/log_handler.h"
#include "storage/common/record_utils.h"

using namespace std;
using namespace common;

static constexpr int PAGE_HEADER_SIZE = (sizeof(PageHeader));

/**
 * @brief 8字节对齐
 * 注: ceiling(a / b) = floor((a + b - 1) / b)
 *
 * @param size 待对齐的字节数
 */
int align8(int size) { return (size + 7) & ~7; }

/**
 * @brief 计算指定大小的页面，可以容纳多少个记录
 *
 * @param page_size   页面的大小
 * @param record_size 记录的大小
 */
int page_record_capacity(int page_size, int record_size) {
    // (record_capacity * record_size) + record_capacity/8 + 1 <= (page_size - fix_size)
    // ==> record_capacity = ((page_size - fix_size) - 1) / (record_size + 0.125)
    return (int) ((page_size - PAGE_HEADER_SIZE - 1) / (record_size + 0.125));
}

/**
 * @brief bitmap 记录了某个位置是否有有效的记录数据，这里给定记录个数时需要多少字节来存放bitmap数据
 * 注: ceiling(a / b) = floor((a + b - 1) / b)
 *
 * @param record_capacity 想要存放多少记录
 */
int page_bitmap_size(int record_capacity) { return (record_capacity + 7) / 8; }

string PageHeader::to_string() const {
    stringstream ss;
    ss << "record_num:" << record_num
       << ",record_real_size:" << record_real_size
       << ",record_size:" << record_size
       << ",record_capacity:" << record_capacity
       << ",first_record_offset:" << first_record_offset;
    return ss.str();
}

////////////////////////////////////////////////////////////////////////////////

RecordPageHandler::~RecordPageHandler() { cleanup(); }

RC RecordPageHandler::init(DiskBufferPool &buffer_pool, PageNum page_num, ReadWriteMode mode) {
    this->cleanup();
    if (disk_buffer_pool_ != nullptr) {
        if (frame_->page_num() == page_num) {
            STD_ERROR("Disk buffer pool has been opened for page_num %d.", page_num);
            return RC::RECORD_OPENNED;
        } else {
            cleanup();
        }
    }

    RC ret = RC::SUCCESS;
    if ((ret = buffer_pool.get_this_page(page_num, &frame_)) != RC::SUCCESS) {
        STD_WARN("Failed to get page handle from disk buffer pool. ret=%s:%s", strrc(ret), strrc(ret));
        return ret;
    }
    char *data = frame_->data();

    if (mode == ReadWriteMode::READ_ONLY) {
        frame_->read_latch();
    } else {
        frame_->write_latch();
    }
    disk_buffer_pool_ = &buffer_pool;

    rw_mode_ = mode;
    page_header_ = (PageHeader *) (data);
    bitmap_ = data + PAGE_HEADER_SIZE;


    LOG_TRACE("Successfully init page_num %d.", page_num);
    return ret;
}

RC RecordPageHandler::recover_init(DiskBufferPool &buffer_pool, PageNum page_num) {
    if (disk_buffer_pool_ != nullptr) {
        LOG_WARN("Disk buffer pool has been opened for page_num %d.", page_num);
        return RC::RECORD_OPENNED;
    }

    RC ret = RC::SUCCESS;
    if ((ret = buffer_pool.get_this_page(page_num, &frame_)) != RC::SUCCESS) {
        LOG_ERROR("Failed to get page handle from disk buffer pool. ret=%d:%s", ret, strrc(ret));
        return ret;
    }

    char *data = frame_->data();

    frame_->write_latch();
    disk_buffer_pool_ = &buffer_pool;
    rw_mode_ = ReadWriteMode::READ_WRITE;
    page_header_ = (PageHeader *) (data);
    bitmap_ = data + PAGE_HEADER_SIZE;

    buffer_pool.recover_page(page_num);

    LOG_TRACE("Successfully init page_num %d.", page_num);
    return ret;
}

RC RecordPageHandler::init_empty_page(
        DiskBufferPool &buffer_pool, LogHandler &log_handler, PageNum page_num, int record_size) {
    RC rc = init(buffer_pool, page_num, ReadWriteMode::READ_WRITE);
    if (OB_FAIL(rc)) {
        LOG_ERROR("Failed to init empty page page_num:record_size %d:%d. rc=%s", page_num, record_size, strrc(rc));
        return rc;
    }

    if (OB_FAIL(rc)) {
        LOG_ERROR("Failed to init empty page: write log failed. page_num:record_size %d:%d. rc=%s",
                  page_num, record_size, strrc(rc));
        return rc;
    }

    page_header_->record_num = 0;
    page_header_->record_real_size = record_size;
    page_header_->record_size = align8(record_size + sizeof(RecordHeader));
    page_header_->record_capacity = page_record_capacity(BP_PAGE_DATA_SIZE, page_header_->record_size);
    page_header_->first_record_offset = align8(PAGE_HEADER_SIZE + page_bitmap_size(page_header_->record_capacity));
    this->fix_record_capacity();
    ASSERT(page_header_->first_record_offset + page_header_->record_capacity * page_header_->record_size
           <= BP_PAGE_DATA_SIZE,
           "Record overflow the page size");

    bitmap_ = frame_->data() + PAGE_HEADER_SIZE;
    memset(bitmap_, 0, page_bitmap_size(page_header_->record_capacity));

    return RC::SUCCESS;
}

RC RecordPageHandler::cleanup() {
    if (disk_buffer_pool_ != nullptr && frame_ != nullptr) {
        if (rw_mode_ == ReadWriteMode::READ_ONLY) {
            frame_->read_unlatch();
        } else {
            frame_->write_unlatch();
        }
        disk_buffer_pool_->unpin_page(frame_);
        disk_buffer_pool_ = nullptr;
    }

    return RC::SUCCESS;
}

RC RecordPageHandler::insert_record_part(const char *data, const RID &next_rid, uint8_t record_type, RID *rid) {
    ASSERT(rw_mode_ != ReadWriteMode::READ_ONLY,
           "cannot insert record into page while the page is readonly");

    if (page_header_->record_num == page_header_->record_capacity) {
        LOG_WARN("Page is full, page_num %d:%d.", disk_buffer_pool_->file_desc(), frame_->page_num());
        return RC::RECORD_NOMEM;
    }

    // 找到空闲位置
    Bitmap bitmap(bitmap_, page_header_->record_capacity);
    int index = bitmap.next_unsetted_bit(0);
    bitmap.set_bit(index);
    page_header_->record_num++;

    // assert index < page_header_->record_capacity
    char *record_content = get_record_content(index);
    RecordHeader header;
    header.record_type = record_type;
    header.next_rid = next_rid;

    //注意：如果拷贝的数据长度不够，会导致heap-buffer-overflow
    memcpy(record_content, &header, sizeof header);
    memcpy(record_content + sizeof header, data,
           page_header_->record_real_size);
    frame_->mark_dirty();

    if (rid) {
        rid->page_num = get_page_num();
        rid->slot_num = index;
    }

    return RC::SUCCESS;
}

RC RecordPageHandler::recover_insert_record(const char *data, const RID &rid) {
    if (rid.slot_num >= page_header_->record_capacity) {
        LOG_WARN("slot_num illegal, slot_num(%d) > record_capacity(%d).", rid.slot_num, page_header_->record_capacity);
        return RC::RECORD_INVALID_RID;
    }

    // 更新位图
    Bitmap bitmap(bitmap_, page_header_->record_capacity);
    if (!bitmap.get_bit(rid.slot_num)) {
        bitmap.set_bit(rid.slot_num);
        page_header_->record_num++;
    }

    // 恢复数据
    char *record_data = get_record_content(rid.slot_num);
    memcpy(record_data, data, page_header_->record_real_size);

    frame_->mark_dirty();

    return RC::SUCCESS;
}

RC RecordPageHandler::erase_bitmap(const RID *rid) {
    ASSERT(rw_mode_ != ReadWriteMode::READ_ONLY,
           "cannot delete record from page while the page is readonly");

    if (rid->slot_num >= page_header_->record_capacity) {
        LOG_ERROR("Invalid slot_num %d, exceed page's record capacity, frame=%s, page_header=%s",
                  rid->slot_num, frame_->to_string().c_str(), page_header_->to_string().c_str());
        return RC::INVALID_ARGUMENT;
    }

    Bitmap bitmap(bitmap_, page_header_->record_capacity);
    bitmap.clear_bit(rid->slot_num);
    page_header_->record_num--;
    frame_->mark_dirty();

    return RC::SUCCESS;
}

RC RecordPageHandler::update_record_part(const RID &rid, const char *data) {
    ASSERT(rw_mode_ != ReadWriteMode::READ_ONLY, "cannot delete record from page while the page is readonly");

    if (rid.slot_num >= page_header_->record_capacity) {
        LOG_ERROR("Invalid slot_num %d, exceed page's record capacity, frame=%s, page_header=%s",
                  rid.slot_num, frame_->to_string().c_str(), page_header_->to_string().c_str());
        return RC::INVALID_ARGUMENT;
    }

    Bitmap bitmap(bitmap_, page_header_->record_capacity);
    if (bitmap.get_bit(rid.slot_num)) {
        frame_->mark_dirty();

        char *record_data = get_record_content(rid.slot_num) + sizeof(RecordHeader);
        if (record_data == data) {
            // nothing to do
        } else {
            memcpy(record_data, data, page_header_->record_real_size);
        }

        return RC::SUCCESS;
    } else {
        LOG_DEBUG("Invalid slot_num %d, slot is empty, page_num %d.", rid.slot_num, frame_->page_num());
        return RC::RECORD_NOT_EXIST;
    }
}

RC RecordPageHandler::get_record_part_in_page(const RID &rid, Record &record, RID &next_rid, uint8_t &record_type) {
    if (rid.slot_num >= page_header_->record_capacity) {
        LOG_ERROR("Invalid slot_num %d, exceed page's record capacity, frame=%s, page_header=%s",
                  rid.slot_num, frame_->to_string().c_str(), page_header_->to_string().c_str());
        return RC::RECORD_INVALID_RID;
    }

    Bitmap bitmap(bitmap_, page_header_->record_capacity);
    if (!bitmap.get_bit(rid.slot_num)) {
        LOG_ERROR("Invalid slot_num:%d, slot is empty, page_num %d.", rid.slot_num, frame_->page_num());
        return RC::RECORD_NOT_EXIST;
    }

    record.set_rid(rid);
    char *record_content = get_record_content(rid.slot_num);
    record.set_data(record_content + sizeof(RecordHeader), page_header_->record_real_size);
    next_rid = ((RecordHeader *) record_content)->next_rid;
    record_type = ((RecordHeader *) record_content)->record_type;
    return RC::SUCCESS;
}


PageNum RecordPageHandler::get_page_num() const {
    if (nullptr == page_header_) {
        return (PageNum) (-1);
    }
    return frame_->page_num();
}

bool RecordPageHandler::is_full() const { return page_header_->record_num >= page_header_->record_capacity; }

RC RecordPageHandler::get_record_type(const RID &rid, uint8_t &type) {
    if (rid.slot_num >= page_header_->record_capacity) {
        LOG_ERROR("Invalid slot_num %d, exceed page's record capacity, frame=%s, page_header=%s",
                  rid.slot_num, frame_->to_string().c_str(), page_header_->to_string().c_str());
        return RC::RECORD_INVALID_RID;
    }

    Bitmap bitmap(bitmap_, page_header_->record_capacity);
    if (!bitmap.get_bit(rid.slot_num)) {
        LOG_ERROR("Invalid slot_num:%d, slot is empty, page_num %d.", rid.slot_num, frame_->page_num());
        return RC::RECORD_NOT_EXIST;
    }
    type = ((RecordHeader *) get_record_content(rid.slot_num))->record_type;
    return RC::SUCCESS;
}

common::Bitmap RecordPageHandler::get_bitmap() {
    return {bitmap_, page_header_->record_capacity};
}

////////////////////////////////////////////////////////////////////////////////

RecordFileHandler::~RecordFileHandler() { this->close(); }

RC RecordFileHandler::init(DiskBufferPool &buffer_pool, LogHandler &log_handler) {
    if (disk_buffer_pool_ != nullptr) {
        LOG_ERROR("record file handler has been openned.");
        return RC::RECORD_OPENNED;
    }

    disk_buffer_pool_ = &buffer_pool;
    log_handler_ = &log_handler;

    RC rc = init_free_pages();

    LOG_INFO("open record file handle done. rc=%s", strrc(rc));
    return RC::SUCCESS;
}

void RecordFileHandler::close() {
    if (disk_buffer_pool_ != nullptr) {
        free_pages_.clear();
        disk_buffer_pool_ = nullptr;
        log_handler_ = nullptr;
    }
}

RC RecordFileHandler::init_free_pages() {
    // 遍历当前文件上所有页面，找到没有满的页面
    // 这个效率很低，会降低启动速度
    // NOTE: 由于是初始化时的动作，所以不需要加锁控制并发

    RC rc = RC::SUCCESS;

    PageNumIterator bp_iterator;
    bp_iterator.init(*disk_buffer_pool_, 1);
    RecordPageHandler record_page_handler;
    PageNum current_page_num = 0;

    while (bp_iterator.has_next()) {
        current_page_num = bp_iterator.next();

        rc = record_page_handler.init(*disk_buffer_pool_, current_page_num, ReadWriteMode::READ_ONLY);
        if (rc != RC::SUCCESS) {
            LOG_WARN("failed to init record page handler. page num=%d, rc=%d:%s", current_page_num, rc, strrc(rc));
            return rc;
        }

        if (!record_page_handler.is_full()) {
            free_pages_.insert(current_page_num);
        }
        record_page_handler.cleanup();
    }
    LOG_INFO("record file handler init free pages done. free page num=%d, rc=%s", free_pages_.size(), strrc(rc));
    return rc;
}


RC RecordFileHandler::recover_insert_record(const char *data, int record_size, const RID &rid) {
    assert(false);
    RC ret = RC::SUCCESS;

    RecordPageHandler record_page_handler;

    ret = record_page_handler.recover_init(*disk_buffer_pool_, rid.page_num);
    if (OB_FAIL(ret)) {
        LOG_WARN("failed to init record page handler. page num=%d, rc=%s", rid.page_num, strrc(ret));
        return ret;
    }

    return record_page_handler.recover_insert_record(data, rid);
}

RC RecordFileHandler::insert_record(Record &record) {
    lock_guard lock(lock_);
    RC rc;
    vector<Record *> records;

    Record *cur = &record;
    while (cur != nullptr) {
        records.insert(records.begin(), cur);
        cur = cur->next_record();
    }

    RID last_rid = {-1, -1};
    for (Record *current: records) {
        const uint8_t type = (current == &record ? RECORD_TYPE_NORMAL : RECORD_TYPE_EXPAND_DATA);

        if ((rc = insert_record_part(current->data(), current->len(),
                                     last_rid, type, &current->rid())) != RC::SUCCESS) {
            STD_ERROR("Failed to insert record. rc=%s", strrc(rc));
            return rc;
        }
        last_rid = current->rid();
    }
    return RC::SUCCESS;
}


RC RecordFileHandler::delete_record(const RID *rid) {
    lock_guard lock(lock_);
    RC rc = RC::SUCCESS;

    RID placeHolder;
    Record record_to_be_delete;
    if (OB_FAIL(rc = get_record_part(*rid, record_to_be_delete, placeHolder))) {
        return rc;
    }
    Record *cur = &record_to_be_delete;

    while (cur != nullptr) {
        if (OB_FAIL(rc = delete_record_part(&cur->rid()))) {
            return rc;
        };
        cur = cur->next_record();
    }
    return rc;
}

RC RecordFileHandler::get_record(const RID &rid, Record &record) {
    lock_guard lock(lock_);
    RC rc = RC::SUCCESS;
    const RID *cur_rid = &rid;
    RID next_rid = {-1, -1};
    bool first_record = true;
    uint8_t record_type;
    while (cur_rid->page_num != -1 && cur_rid->slot_num != -1) {
        Record getted_record;
        if (OB_FAIL(rc = get_record_part(*cur_rid, getted_record, next_rid, record_type))) {
            STD_ERROR("Failed to get record. rc=%s", strrc(rc));
            return rc;
        }
        if (first_record) {
            record = std::move(getted_record);

            if (record_type != RECORD_TYPE_NORMAL) {
                STD_TRACE("First record type is not 'normal'. record_type=%d,"
                          " this is normal if you are scanning a table", record_type);
                return RC::RECORD_TYPE_UNEXPECTED;
            }
        } else {
            move_to_last(record, &getted_record);

            if (record_type != RECORD_TYPE_EXPAND_DATA) {
                STD_ERROR("Record type is not expand data. record_type=%d", record_type);
                return RC::RECORD_TYPE_ERROR;
            }
        }
        first_record = false;
        cur_rid = &next_rid;
    }
    return rc;
}

RC RecordFileHandler::visit_record(const RID &rid, std::function<bool(Record &)> updater) {
    RecordPageHandler page_handler;

    RC rc = page_handler.init(*disk_buffer_pool_, rid.page_num, ReadWriteMode::READ_WRITE);
    if (OB_FAIL(rc)) {
        LOG_ERROR("Failed to init record page handler.page number=%d", rid.page_num);
        return rc;
    }

    Record inplace_record;
    RID place_holder1;
    uint8_t place_holder2;
    rc = page_handler.get_record_part_in_page(rid, inplace_record, place_holder1, place_holder2);
    if (OB_FAIL(rc)) {
        LOG_WARN("failed to get record from record page handle. rid=%s, rc=%s", rid.to_string().c_str(), strrc(rc));
        return rc;
    }

    // 需要将数据复制出来再修改，否则update_record调用失败但是实际上数据却更新成功了，
    // 会导致数据库状态不正确
    Record record;
    record.copy_data(inplace_record.data(), inplace_record.len());
    record.set_rid(rid);

    bool updated = updater(record);
    if (updated) {
        rc = page_handler.update_record_part(rid, record.data());
    }
    return rc;
}


RC RecordFileHandler::delete_record_part(const RID *rid) {
    RC rc = RC::SUCCESS;
    RecordPageHandler page_handler;
    if (OB_FAIL(rc = page_handler.init(*disk_buffer_pool_,
                                       rid->page_num, ReadWriteMode::READ_WRITE))) {
        LOG_ERROR("Failed to init record page handler.page number=%d. rc=%s", rid->page_num, strrc(rc));
        return rc;
    }

    if (OB_FAIL(rc = page_handler.erase_bitmap(rid))) {
        return rc;
    }

    free_pages_.insert(rid->page_num);
    LOG_TRACE("add free page %d to free page list", rid->page_num);
    return rc;
}

RC RecordFileHandler::get_record_part(const RID &rid, Record &record) {
    RID place_holder;
    return get_record_part(rid, record, place_holder);
}

RC RecordFileHandler::get_record_part(const RID &rid, Record &record, RID &next_rid) {
    uint8_t place_holder;
    return get_record_part(rid, record, next_rid, place_holder);
}

RC RecordFileHandler::get_record_part(const RID &rid, Record &record, RID &next_rid, uint8_t &record_type) {
    RC rc = RC::SUCCESS;
    RecordPageHandler page_handler;

    if (OB_FAIL(rc = page_handler.init(*disk_buffer_pool_,
                                       rid.page_num, ReadWriteMode::READ_ONLY))) {
        STD_ERROR("Failed to init record page handler.page number=%d", rid.page_num);
        return rc;
    }
    Record inplace_record;
    if (OB_FAIL(rc = page_handler.get_record_part_in_page(rid, inplace_record, next_rid, record_type))) {
        STD_ERROR("Failed to get record from record page handle. rid=%s, rc=%s", rid.to_string().c_str(), strrc(rc));
        return rc;
    }
    record.copy_data(inplace_record.data(), inplace_record.len());
    record.set_rid(rid);
    return rc;
}


RC RecordFileHandler::insert_record_part(const char *data, int data_size, const RID &next_rid, uint8_t record_type,
                                         RID *rid) {
    RC ret = RC::SUCCESS;

    RecordPageHandler record_page_handler;
    bool page_found = false;
    PageNum current_page_num = 0;

    // 找到没有填满的页面
    while (!free_pages_.empty()) {
        current_page_num = *free_pages_.begin();

        ret = record_page_handler.init(*disk_buffer_pool_, current_page_num, ReadWriteMode::READ_WRITE);
        if (OB_FAIL(ret)) {
            STD_ERROR("failed to init record page handler. page num=%d, rc=%s", current_page_num, strrc(ret));
            return ret;
        }

        if (!record_page_handler.is_full()) {
            page_found = true;
            break;
        }
        record_page_handler.cleanup();
        free_pages_.erase(free_pages_.begin());
    }

    // 找不到就分配一个新的页面
    if (!page_found) {
        Frame *frame = nullptr;
        if ((ret = disk_buffer_pool_->allocate_page(&frame)) != RC::SUCCESS) {
            STD_ERROR("Failed to allocate page while inserting record. ret:%s", strrc(ret));
            return ret;
        }

        current_page_num = frame->page_num();

        ret = record_page_handler.init_empty_page(*disk_buffer_pool_, *log_handler_, current_page_num, data_size);
        if (OB_FAIL(ret)) {
            STD_ERROR("Failed to init empty page. ret:%s", strrc(ret));
            // this is for allocate_page
            return ret;
        }

        free_pages_.insert(current_page_num);
    }

    // 找到空闲位置
    return record_page_handler.insert_record_part(data, next_rid, record_type, rid);
}

RC RecordFileHandler::get_page_num_iterator(PageNumIterator &iterator) {
    return iterator.init(*disk_buffer_pool_, 1);
}

RC RecordFileHandler::get_record_iterator(RecordNumIterator &iterator, PageNum page_num) {
    return iterator.init(*disk_buffer_pool_, page_num);
}

////////////////////////////////////////////////////////////////////////////////

RecordFileScanner::~RecordFileScanner() { close_scan(); }

void RecordFileScanner::open_scan(RecordFileHandler *handler) {
    close_scan();
    handler_ = handler;
    handler_->get_page_num_iterator(page_iterator_);
    page_num_ = page_iterator_.next();
    handler_->get_record_iterator(record_iterator_, 1);
    slot_num_ = record_iterator_.next();
}

/**
 * @brief 从当前位置开始找到下一条有效的记录
 *
 * 如果当前页面还有记录没有访问，就遍历当前的页面。
 * 当前页面遍历完了，就遍历下一个页面，然后找到有效的记录
 */
RC RecordFileScanner::fetch_next_record() {

    STD_TRACE("if have next record");
    if (record_iterator_.has_next()) {
        slot_num_ = record_iterator_.next();
        return fetch_record();
    }

    STD_TRACE("next page");
    while (page_iterator_.has_next()) {
        page_num_ = page_iterator_.next();
        handler_->get_record_iterator(record_iterator_, page_num_);
        if (record_iterator_.has_next()) {
            STD_TRACE("find a rid in page %d.", page_num_);
            slot_num_ = record_iterator_.next();
            return fetch_record();
        }
    }

    return RC::RECORD_EOF;
}

/**
 * @brief 遍历当前页面，尝试找到一条有效的记录
 */
RC RecordFileScanner::fetch_record() {
    RC rc;


    STD_TRACE("get record");
    rc = handler_->get_record({page_num_, slot_num_}, current_record_);

    //Record type might be RECORD_TYPE_NORMAL or RECORD_TYPE_EXPAND_DATA
    //this will be handled in the upper layer
    if (rc != RC::RECORD_TYPE_UNEXPECTED
        && rc != RC::SUCCESS) {
        STD_ERROR("Failed to get record in RecordFileScanner."
                  "rc=%s, PageNum=%d, SlotNum=%d", strrc(rc), page_num_, slot_num_);
    }
    return rc;
}

void RecordFileScanner::close_scan() {
    handler_ = nullptr;
    page_num_ = 0;
    slot_num_ = 0;
    page_iterator_.~PageNumIterator();
    record_iterator_.~RecordNumIterator();
}

RC RecordFileScanner::next(Record &record) {
    STD_TRACE("RecordFileScanner::next");

    RC rc = RC::SUCCESS;

    STD_TRACE("fetch_next_record");
    rc = fetch_next_record();
    while (rc == RC::RECORD_TYPE_UNEXPECTED) {
        STD_TRACE("got a unexpected record type, continue to fetch next record");
        rc = fetch_next_record();
    }

    if (rc == RC::SUCCESS) {
        STD_TRACE("Assign record");
        record = current_record_;
    }
    STD_TRACE("return rc=%s", strrc(rc));

    return rc;
}



/////////////////////////////////////////////////////////////////////////

RC RecordNumIterator::init(DiskBufferPool &bp, PageNum page_num, SlotNum start_slot) {
    RC rc;
    rc = handler_.init(bp, page_num, ReadWriteMode::READ_ONLY);
    if (rc == RC::BUFFERPOOL_INVALID_PAGE_NUM) {
        next_slot_num = -1;
        return RC::SUCCESS;
    }
    if (rc != RC::SUCCESS) {
        STD_ERROR("%s", strrc(rc));
        return rc;
    }
    page_num_ = page_num;
    next_slot_num = start_slot - 1;
    bitmap_ = handler_.get_bitmap();
    fetch_next_num();

    STD_TRACE("RecordNumIterator::init page_num=%d, start_slot=%d", page_num, start_slot);
    return RC::SUCCESS;
}

RecordNumIterator::RecordNumIterator() {
    //do nothing
}

RecordNumIterator::~RecordNumIterator() {
    //do nothing
}

bool RecordNumIterator::has_next() {
    if (next_slot_num != -1) {
        return true;
    }
    return false;
}

PageNum RecordNumIterator::next() {
    if (next_slot_num == -1) {
        return -1;
    }
    int res = next_slot_num;
    fetch_next_num();
    return res;
}

void RecordNumIterator::fetch_next_num() {
    while (!bitmap_.get_bit(++next_slot_num)) {
        if (bitmap_.size() == next_slot_num - 1) {
            next_slot_num = -1;
            return;
        }
    }
}
