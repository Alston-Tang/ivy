//
// Created by Tang on 12/30/2018.
//

#include <cxxtest/TestSuite.h>

#include <stdlib.h>

#include <iostream>

#include "../server/compact_logger.h"

using namespace ivy;

class TestCompactLogger : public CxxTest::TestSuite {
public:
    void test_clean_log() {
        // Delete previous logger test
        int rv = system("./pre_logger_test.sh");
        TS_ASSERT(rv == 0);

        auto &logger_manager = CompactLoggerManager::get_instance();
        auto test_logger = logger_manager.get_logger("test");

        char msg[] = "Hello";
        bool res = test_logger->log(msg, 5);
        if (!res) {
            std::cout << "Cannot log @ test_clean_log" << std::endl;
        }
        // Write my first log
        TS_ASSERT(test_logger->get_size() == 5);
    }

    void test_reopen_log() {
        auto &logger_manager = CompactLoggerManager::get_instance();
        logger_manager.close_logger("test");
        auto test_logger = logger_manager.get_logger("test");

        char msg[] = "Hello";
        bool res = test_logger->log(msg, 5);
        if (!res) {
            std::cout << "Cannot log @ test_reopen_log" << std::endl;
        }
        // Write my second log
        TS_ASSERT(test_logger->get_size() == 10);
    }
};
