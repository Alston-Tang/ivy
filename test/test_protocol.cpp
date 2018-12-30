//
// Created by Tang on 12/21/2018.
//

#include <cxxtest/TestSuite.h>
#include <string>
#include <iostream>

#include "../protocol.h"

using namespace ivy::proto;

class TestProtocol : public CxxTest::TestSuite
{
public:
    void test_protocol_offset() {
        Encrypted message = {};
        Encrypted *message_p = &message;
        TS_ASSERT((uint8_t *)message_p->payload == (uint8_t *)message_p + (int)AES::DEFAULT_KEYLENGTH);
    }
};