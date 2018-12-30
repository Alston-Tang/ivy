//
// Created by Tang on 12/18/2018.
//

#include <cxxtest/TestSuite.h>
#include <string>
#include <iostream>

#include "../crypto.h"

using namespace ivy;

class TestCrypto : public CxxTest::TestSuite
{
public:
    void test_encrypt() {
        iv = aes256_encrypt((unsigned char*)message, sizeof(message), (unsigned char*)message);
        std::cout << std::endl;
        for (char i : message) {
            std::cout << i << ' ';
        }
        std::cout << std::endl;
    }
    void test_decrypt() {
        std::cout << std::endl;
        for (char i : message) {
            std::cout << i << ' ';
        }
        std::cout << std::endl;
        aes256_decrypt((unsigned char*)message, sizeof(message), (unsigned char*)message, iv);
        TS_ASSERT(std::string(message) == "Hello World");
    }
    char message[12] = "Hello World";
    iv_t iv{};
};