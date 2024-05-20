//
// Created by root on 5/17/24.
//
#include <random>
#include "gtest/gtest.h"
#include "storage/buffer/double_write_buffer.h"
#include "storage/clog/vacuous_log_handler.h"
#include "storage/buffer/disk_buffer_pool.h"
#include "storage/record/record_manager.h"
#include "storage/common/record_utils.h"

/**
 * 测试RecordFileScanner及其附属类
 *
 * 1. 测试无记录时scan
 * 2. 按顺序插入1000条记录记录，然后scan，看看是否能够按顺序读取，且读取的记录正确
 */


using namespace std;

class SerialIdGenerator {
public:
    long next() {
        return serial_id_++;
    }

private:
    long serial_id_ = 0;
};

class RandomStringGenerator {
public:
    string next(int length) {            // length: 产生字符串的长度
        char tmp;                            // tmp: 暂存一个随机数
        string buffer;                        // buffer: 保存返回值

        // 下面这两行比较重要:
        random_device rd;                    // 产生一个 std::random_device 对象 rd
        default_random_engine random(rd());    // 用 rd 初始化一个随机数发生器 random

        for (int i = 0; i < length; i++) {
            tmp = random() % 36;    // 随机一个小于 36 的整数，0-9、A-Z 共 36 种字符
            if (tmp < 10) {            // 如果随机数小于 10，变换成一个阿拉伯数字的 ASCII
                tmp += '0';
            } else {                // 否则，变换成一个大写字母的 ASCII
                tmp -= 10;
                tmp += 'A';
            }
            buffer += tmp;
        }
        return buffer;
    }
};

/**
 * 制作测试用记录，前8个字节为id，后12字节为随机字符串
 * @param id
 * @param record
 */
struct record_content {
    long id;
    char random_string[12];
};

class RandomRecordGenerator {
public:
    Record next() {
        static int recursive_num = 0;
        recursive_num++;

        Record record;
        record_content content{};

        //制作记录
        content.id = id_generator_.next();
        string random_string = string_generator_.next(12);
        memcpy(content.random_string, random_string.c_str(), 12);
        record.copy_data((char *) &content, sizeof(content));

        //插入记录
        records_[content.id] = record;

        //制作扩展记录
        if (recursive_num < 10) {
            Record extend_record = next();
            move_to_last(record, &extend_record);
        }

        recursive_num--;
        return record;
    }

    bool validate_record(Record &record) {
        //查找记录
        auto it = records_.find(((record_content *) record.data())->id);
        if (it == records_.end()) {
            return false;
        }

        //比较记录
        Record &map_record = it->second;
        return (memcmp(map_record.data(), record.data(), sizeof(record_content)) == 0
               && record.next_record() == nullptr) || validate_record(*record.next_record());
    }

    void delete_record(long id) {
        records_.erase(id);
    }

private:
    unordered_map<long, Record> records_;
    SerialIdGenerator id_generator_;
    RandomStringGenerator string_generator_;


};

/**
 * Set up
 * 设置一个文件，插入一些记录
 */
class RecordIteratorTest : public ::testing::Test {

protected:
    void SetUp() override {
        init();
    }

    void init() {

        const char *file_name = "test_file.data";
        ::remove(file_name);

        rc = buffer_pool_manager_.init(std::make_unique<VacuousDoubleWriteBuffer>(write_buffer_));
        STD_INFO("buffer_pool_manager_.init %s", strrc(rc));
        ASSERT_EQ(rc, RC::SUCCESS);

        rc = frame_manager_.init(1);
        STD_INFO("frame_manager_.init %s", strrc(rc));
        ASSERT_EQ(rc, RC::SUCCESS);

        rc = buffer_pool_manager_.create_file(file_name);
        STD_INFO("buffer_pool_manager_.create_file %s", strrc(rc));
        ASSERT_EQ(rc, RC::SUCCESS);


        rc = buffer_pool_manager_.open_file(log_handler_, file_name, buffer_pool_);
        STD_INFO("buffer_pool_manager_.open_file %s", strrc(rc));
        ASSERT_EQ(rc, RC::SUCCESS);

        rc = handler_.init(*buffer_pool_, log_handler_);
        STD_INFO("handler_.init %s", strrc(rc));
        ASSERT_EQ(rc, RC::SUCCESS);
    }

    /**
     * 测试记录生成器
     */

    void test_record_generator() {
        for (int i = 0; i < 100; i++) {
            Record record = record_generator_.next();
            ASSERT_TRUE(record_generator_.validate_record(record));
            record_generator_.delete_record(((record_content *) record.data())->id);
            ASSERT_FALSE(record_generator_.validate_record(record));

            record = record_generator_.next();
            record.data()[i / 10]  = 0;
            ASSERT_FALSE(record_generator_.validate_record(record));
            record_generator_.delete_record(((record_content *) record.data())->id);
        }
    }

protected:
    RC rc = RC::SUCCESS;
    VacuousDoubleWriteBuffer write_buffer_;
    VacuousLogHandler log_handler_;
    BufferPoolManager buffer_pool_manager_;
    BPFrameManager frame_manager_ = BPFrameManager("test");
    DiskBufferPool *buffer_pool_ = nullptr;
    RecordFileHandler handler_;
    RandomRecordGenerator record_generator_;
    unordered_map<long, Record> gen_records;
};

TEST_F(RecordIteratorTest, some_test) {
    RecordFileScanner scanner;
    //测试无记录时scan
    scanner.open_scan(&handler_);
    Record place_holder;
    rc = scanner.next(place_holder);
    ASSERT_EQ(rc, RC::RECORD_EOF);

    //插入记录
    vector<long> ids;
    for (int i = 0; i < 1000; i++) {
        STD_INFO("insert %d", i);
        Record record = record_generator_.next();
        ids.push_back(((record_content *) record.data())->id);
        handler_.insert_record(record);
    }

    //测试RecordFileScanner
    scanner.open_scan(&handler_);
    for (int i = 0; i < 1000; i++) {
        STD_INFO("next %d", i);
        Record record;
        rc = scanner.next(record);

        ASSERT_EQ(rc,  RC::SUCCESS);
        ASSERT_EQ(ids[i], ((record_content *) record.data())->id);
        ASSERT_TRUE(record_generator_.validate_record(record));
    }

}