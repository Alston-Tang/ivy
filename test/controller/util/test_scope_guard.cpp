//
// Created by tang on 5/26/19.
//

#include <cxxtest/TestSuite.h>

#include "../../../controller/util/scope_guard.h"


class TestUtil : public CxxTest::TestSuite {
public:
    bool class_has_reset = false;
    void test_reset_outer_scope() {
        bool has_reset = false;
        {
            TS_ASSERT_EQUALS(has_reset, false);
            ScopeGuard guard([&](){
                has_reset = true;
            });
            TS_ASSERT_EQUALS(has_reset, false);
        }
        TS_ASSERT_EQUALS(has_reset, true);
    }
    void test_reset_class() {
        TS_ASSERT_EQUALS(class_has_reset, false);
        {
            TS_ASSERT_EQUALS(class_has_reset, false);
            ScopeGuard guard([&](){
                class_has_reset = true;
            });
            TS_ASSERT_EQUALS(class_has_reset, false);
        }
        TS_ASSERT_EQUALS(class_has_reset, true);
    }
};
