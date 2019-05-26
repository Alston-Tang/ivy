//
// Created by tang on 1/5/19.
//

#include <cxxtest/TestSuite.h>
#include <arpa/inet.h>

#include "../../../controller/util/ip_id.h"

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
        addr.sin_family = AF_INET;
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

    void test_to_sockaddr_in() {
        auto test_addr = id_to_sockaddr_in(id);
        TS_ASSERT_EQUALS(test_addr.sin_addr.s_addr, addr.sin_addr.s_addr);
        TS_ASSERT_EQUALS(test_addr.sin_port, addr.sin_port);
        TS_ASSERT_EQUALS(test_addr.sin_family, addr.sin_family);
    };
    
    void test_ip_port_to_id() {
        TS_ASSERT_EQUALS(id, ip_port_to_id("192.168.1.2", 1234, AF_INET, SOCK_STREAM));
    }
};