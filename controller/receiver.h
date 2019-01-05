//
// Created by tang on 1/4/19.
//

#ifndef IVY_RECEIVER_H
#define IVY_RECEIVER_H

#include <memory>
#include <atomic>

#include "../concurrentqueue/blockingconcurrentqueue.h"
#include "messages/raw.h"

typedef moodycamel::BlockingConcurrentQueue<ivy::message::Raw> RawMessageQueue;

namespace ivy {

class Receiver {

public:
    explicit Receiver(std::shared_ptr<RawMessageQueue> up_queue, uint16_t port);
    bool run();
    bool stop();

    const static int RECV_BUFFER_LEN = 1500;

private:
    const static int MAX_PENDING_LISTEN = 10;
    const static int MAX_EPOLL_EVENTS = 20;
    const static int EPOLL_TIMEOUT = 1000; //ms

    uint16_t port;
    std::shared_ptr<RawMessageQueue> up_queue;
    std::thread *thread;
    void main_loop();

    std::atomic<bool> should_stop;
};

}


#endif //IVY_RECEIVER_H
