//
// Created by tang on 1/26/19.
//

#include <cxxtest/TestSuite.h>
#include <memory>
#include <iostream>

#include "../controller/receiver.h"
#include "../controller/sender.h"
#include "../controller/util.h"


using namespace ivy;
using namespace std;

class TestBasic : public CxxTest::TestSuite
{
public:
    Receiver *receiver;
    Sender *sender;
    shared_ptr<RawMessageQueue> recv_queue;
    shared_ptr<RawMessageQueue> send_queue;

    explicit TestBasic() {
        recv_queue = make_shared<RawMessageQueue>(100);
        send_queue = make_shared<RawMessageQueue>(100);
        receiver = nullptr;
        sender = nullptr;
    }

    void test_instantiate() {
        receiver = new Receiver(recv_queue, 1314);
        sender = new Sender(send_queue);
    }


    void disable_test_receiver_start_stop() {
        receiver->run();
        sleep(5);
        receiver->stop();
    }

    void disable_test_sender_start_stop() {
        sender->run();
        sleep(5);
        sender->stop();
    }

    void test_sender_connect_to_receiver() {
        receiver->run();
        sender->run();
        sleep(5);

        char *hello_data = new char[6];
        memcpy(hello_data, "HELLO", 6);
        message::Raw hello_msg((uint8_t*)hello_data, ip_port_to_id("127.0.0.1", 1314, AF_INET, SOCK_STREAM));
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
                recv_data += (char)recv_data_ptr[i];
            }
        }
    };

};