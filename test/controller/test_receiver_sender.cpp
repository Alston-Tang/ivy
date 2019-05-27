//
// Created by tang on 1/26/19.
//

#include <cxxtest/TestSuite.h>
#include <memory>
#include <iostream>

#include "../../controller/tcp_receiver.h"
#include "../controller/sender.h"
#include "../../controller/util/ip_id.h"


using namespace ivy;
using namespace std;

class TestBasic : public CxxTest::TestSuite
{
public:
    TcpReceiver *tcp_receiver;
    Sender *sender;
    shared_ptr<RawMessageQueue> recv_queue;
    shared_ptr<RawMessageQueue> send_queue;

    const static short TEST_PORT = 1568;
    const static short TEST_REUSEADDR_PORT = 3152;

    explicit TestBasic() {
        recv_queue = make_shared<RawMessageQueue>(100);
        send_queue = make_shared<RawMessageQueue>(100);
        tcp_receiver = nullptr;
        sender = nullptr;
    }

    void test_instantiate() {
        TS_ASSERT_THROWS_NOTHING(tcp_receiver = new TcpReceiver(recv_queue, TEST_PORT););
        TS_ASSERT_THROWS_NOTHING(sender = new Sender(send_queue););
    }


    void test_tcp_receiver_start_stop() {
        TS_ASSERT_EQUALS(tcp_receiver->run(), true);
        sleep(1);
        TS_ASSERT_EQUALS(tcp_receiver->is_running(), true);
        TS_ASSERT_EQUALS(tcp_receiver->stop(), true);
        sleep(1);
        TS_ASSERT_EQUALS(tcp_receiver->is_running(), false);
    }

    void test_sender_start_stop() {
        TS_ASSERT_EQUALS(sender->run(), true);
        sleep(1);
        TS_ASSERT_EQUALS(sender->is_running(), true);
        TS_ASSERT_EQUALS(sender->stop(), true);
        sleep(1);
        TS_ASSERT_EQUALS(sender->is_running(), false);
    }

    void test_sender_connect_to_tcp_receiver() {
        tcp_receiver->run();
        sender->run();
        sleep(1);

        char *hello_data = new char[6];
        memcpy(hello_data, "HELLO", 6);
        message::Raw hello_msg((uint8_t*)hello_data, ip_port_to_id("127.0.0.1", TEST_PORT, AF_INET, SOCK_STREAM));
        hello_msg.length = 6;
        send_queue->blockingWrite(hello_msg);
        int recv_count = 0;
        message::Raw recv_msg(nullptr, 0);
        recv_msg.length = 0;
        string recv_data;
        while (recv_count < 6) {
            recv_queue->blockingRead(recv_msg);
            recv_count += recv_msg.length;
            uint8_t* recv_data_ptr = recv_msg.data.get();
            for (int i = 0; i < recv_msg.length; i++) {
                if (recv_data_ptr[i] != 0) {
                    recv_data += (char) recv_data_ptr[i];
                }
            }
        }

        TS_ASSERT_EQUALS(recv_count, 6);
        TS_ASSERT_EQUALS(recv_data, "HELLO");
    };

    void test_reuseaddr() {
        TcpReceiver *tcp_receiver = new TcpReceiver(recv_queue, TEST_REUSEADDR_PORT);
        tcp_receiver->run();
        sleep(1);
        TS_ASSERT_EQUALS(tcp_receiver->is_running(), true);
        tcp_receiver->stop();
        delete(tcp_receiver);

        tcp_receiver = new TcpReceiver(recv_queue, TEST_REUSEADDR_PORT);
        tcp_receiver->run();
        sleep(1);
        TS_ASSERT_EQUALS(tcp_receiver->is_running(), true);
        tcp_receiver->stop();
        delete(tcp_receiver);
    }

};