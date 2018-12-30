//
// Created by Tang on 12/30/2018.
//

#include <cxxtest/TestSuite.h>

#include "../server/compact_logger.h"

using namespace ivy;

class TestCompactLogger : public CxxTest::TestSuite
{
public:
    void test_clean_log() {
        auto& logger_manager = CompactLoggerManager::get_instance();
        auto test_logger = logger_manager.get_logger("test");
        
        char msg[] = "Hello";
        test_logger->log(msg, 5);
        // Write my first log

        TS_ASSERT(test_logger->get_size() == 5);


    }
};