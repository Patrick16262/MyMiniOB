#include "gtest/gtest.h"
#include "storage/record/record_manager.h"
#include "storage/record/record.h"
#include "storage/buffer/disk_buffer_pool.h"
#include "storage/buffer/double_write_buffer.h"
#include "storage/clog/log_handler.h"
#include "storage/clog/vacuous_log_handler.h"
#include "src/debug/memdetect/mem_detection.h"
#include "storage/common/record_utils.h"

/**
 * 测试RecordHandler及其附属类
 * 测试一：单条记录的插入和读取
 * 1. 创建一个RecordHandler
 * 2. 创建一个扩展的Record
 * 3. 插入Record
 * 4. 读取Record，比较数据
 * 测试二：多条记录的插入和读取
 * 1. 创建一个RecordHandler
 * 2. 创建100条Record，每条Record有10条子Record
 * 3. 插入Record
 * 4. 读取Record，比较数据
 */

class RecordManagerTest : public ::testing::Test {
protected:
    void SetUp() override {

        ::remove(file_name);

        rc = buffer_pool_manager.init(std::make_unique<VacuousDoubleWriteBuffer>(write_buffer));
        STD_INFO("%s", strrc(rc));
        ASSERT_EQ(rc, RC::SUCCESS);

        rc = frame_manager.init(2);
        STD_INFO("%s", strrc(rc));
        ASSERT_EQ(rc, RC::SUCCESS);

        rc = buffer_pool_manager.create_file(file_name);
        STD_INFO("%s", strrc(rc));
        ASSERT_EQ(rc, RC::SUCCESS);

        rc = buffer_pool_manager.open_file(log_handler, file_name, buffer_pool);
        STD_INFO("%s", strrc(rc));
        ASSERT_EQ(rc, RC::SUCCESS);

        rc = handler.init(*buffer_pool, log_handler);
        STD_INFO("%s", strrc(rc));
        ASSERT_EQ(rc, RC::SUCCESS);
    }

    const char *file_name = "test_file.data";
    BPFrameManager frame_manager = BPFrameManager("test");
    DiskBufferPool *buffer_pool = nullptr;
    RecordFileHandler handler;
    VacuousDoubleWriteBuffer write_buffer;
    VacuousLogHandler log_handler;
    RC rc;
    BufferPoolManager buffer_pool_manager;
};


TEST_F(RecordManagerTest, single_record) {
//part1 test single record;

    Record simple_record;
    Record temp_record1;
    Record temp_record2;
    Record temp_record3;
    Record temp_record4;

    char record_data1[8] = "hello1!";
    char record_data2[8] = "hello2!";
    char record_data3[8] = "hello3!";
    char record_data4[8] = "hello4!";
    char record_data5[8] = "hello5!";

    simple_record.copy_data(record_data1, 8);
    temp_record1.copy_data(record_data2, 8);
    temp_record2.copy_data(record_data3, 8);
    temp_record3.copy_data(record_data4, 8);
    temp_record4.copy_data(record_data5, 8);
    simple_record.move_as_near(temp_record1);
    simple_record.next_record()->move_as_near(temp_record2);
    simple_record.next_record()->next_record()->move_as_near(temp_record3);
    simple_record.next_record()->next_record()->next_record()->move_as_near(temp_record4);
    ASSERT_EQ(temp_record1.data(), nullptr);

    rc = handler.insert_record(simple_record);
    STD_INFO("%s", strrc(rc));
    ASSERT_EQ(rc, RC::SUCCESS);

    Record getted_record;
    rc = handler.get_record(simple_record.rid(), getted_record);
    STD_INFO("%s", strrc(rc));
    ASSERT_EQ(rc, RC::SUCCESS);

    ASSERT_EQ(strcmp(getted_record.data(), record_data1), 0);
    ASSERT_EQ(strcmp(getted_record.next_record()->data(), record_data2), 0);
    ASSERT_EQ(strcmp(getted_record.next_record()->next_record()->data(), record_data3), 0);
    ASSERT_EQ(strcmp(getted_record.next_record()->next_record()->next_record()->data(), record_data4), 0);
    ASSERT_EQ(
            strcmp(getted_record.next_record()->next_record()->next_record()->next_record()->data(), record_data5),
            0);

    simple_record.~Record();

    ASSERT_EQ(simple_record.data(), nullptr);
    ASSERT_EQ(simple_record.next_record(), nullptr);

    getted_record.~Record();
}

TEST_F(RecordManagerTest, muti_record) {
    vector<RID> rids;
    for (long i = 0; i < 100; i++) {
        Record temp_record;
        temp_record.copy_data((char *) &i, sizeof(long));
        for (long j = 0; j < 10; j++) {
            Record next_record;
            next_record.copy_data((char *) &j, sizeof(long));
            move_to_last(temp_record, &next_record);
        }
        handler.insert_record(temp_record);
        rids.push_back(temp_record.rid());
    }

    //重启数据库
    handler.close();
    buffer_pool->close_file();
    buffer_pool_manager.close_file(file_name);
    frame_manager.cleanup();

    rc = buffer_pool_manager.init(std::make_unique<VacuousDoubleWriteBuffer>(write_buffer));
    STD_INFO("%s", strrc(rc));
    ASSERT_EQ(rc, RC::SUCCESS);

    rc = frame_manager.init(2);
    STD_INFO("%s", strrc(rc));
    ASSERT_EQ(rc, RC::SUCCESS);

    rc = buffer_pool_manager.open_file(log_handler, file_name, buffer_pool);
    STD_INFO("%s", strrc(rc));
    ASSERT_EQ(rc, RC::SUCCESS);

    rc = handler.init(*buffer_pool, log_handler);
    STD_INFO("%s", strrc(rc));
    ASSERT_EQ(rc, RC::SUCCESS);


    for (long i = 0; i < 100; i++) {
        STD_INFO("i=%ld", i);
        Record getted_record;
        rc = handler.get_record(rids[i], getted_record);
        ASSERT_EQ(rc, RC::SUCCESS);
        ASSERT_EQ(*(long *) getted_record.data(), i);

        Record *cur = getted_record.next_record();
        for (long j = 0; j < 10; j++) {
            ASSERT_EQ(*(long *) cur->data(), j);
            cur = cur->next_record();
        }
    }
}

