//
// Created by tang on 1/5/19.
//

#include <cxxtest/TestSuite.h>
#include <arpa/inet.h>

#include "../../controller/util.h"

using namespace ivy;

class TestUtil : public CxxTest::TestSuite
{
public:
    struct sockaddr_in addr{};
    std::string addr_str;
    uint64_t id;

    explicit TestUtil() {
        addr_str = "192.168.1.2";
        inet_pton(AF_INET, addr_str.c_str(), &(addr.sin_addr));
        addr.sin_port = htons(1234);
        id = tuple_to_id(addr, AF_INET, SOCK_STREAM);
    }
    void test_get() {
        TS_ASSERT(get_port(id) == 1234);
        TS_ASSERT(get_proto(id) == AF_INET);
        TS_ASSERT(get_type(id) == SOCK_STREAM);
        TS_ASSERT(id_to_ip_str(id) == addr_str);
    }
    
    void test_printable() {
        TS_ASSERT(id_to_printable(id) == "192.168.1.2:1234");
    };
};