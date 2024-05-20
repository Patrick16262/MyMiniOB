//
// Created by root on 5/17/24.
//


#include "common/console_logging.h"
#include "gtest/gtest.h"

TEST(console_logging, test1) {
    STD_INFO("This is a test message");
    STD_WARN("This is a test warning message");
    STD_ERROR("This is a test error message");

    STD_TRACE("message1");
    STD_TRACE("message2");
    STD_TRACE("message3");
    STD_TRACE("message4");
    STD_TRACE("message5");

    STD_DEBUG("message1");
    STD_DEBUG("message2");
    STD_DEBUG("message3");
    STD_DEBUG("message4");
    STD_DEBUG("message5");

    STD_ERROR("message1");
    STD_ERROR("message2");
}