//
// Created by tang on 5/26/19.
//

#ifndef IVY_UDP_RECEIVER_H
#define IVY_UDP_RECEIVER_H

#include <atomic>
#include <memory>
#include <thread>
#include <unordered_map>

#include "controller.h"
#include "messages/raw.h"

namespace ivy {

class UdpReceiver {
public:
    explicit UdpReceiver(
            std::shared_ptr<RawMessageQueue> up_queue,
            uint16_t port);
    ~UdpReceiver();

    bool run();

    bool stop();

    bool is_running();

private:
    uint16_t port;
    std::shared_ptr<RawMessageQueue> up_queue;
    std::thread *thread;
    std::atomic<bool> should_stop;
    std::atomic<bool> running;

    void main_loop();
};

}


#endif //IVY_UDP_RECEIVER_H
