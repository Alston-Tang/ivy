//
// Created by tang on 1/20/19.
//

#ifndef IVY_SENDER_H
#define IVY_SENDER_H

#include "../concurrentqueue/blockingconcurrentqueue.h"
#include "messages/raw.h"
#include "controller.h"

namespace ivy {

class Sender {

public:
    explicit Sender(
            std::shared_ptr<RawMessageQueue> up_queue,
            std::shared_ptr<PeerSyncQueue> peer_recv_queue,
            std::shared_ptr<PeerSyncQueue> peer_send_queue = nullptr);

    bool run();

    bool stop();

private:
    const static int UP_QUEUE_TIMEOUT = 1000; //ms

    uint16_t port;
    std::shared_ptr<RawMessageQueue> up_queue;
    std::shared_ptr<PeerSyncQueue> peer_send_queue;
    std::shared_ptr<PeerSyncQueue> peer_recv_queue;
    std::thread *thread;
    std::atomic<bool> should_stop;
    ConnectionsRevType connections;

    void main_loop();

    void handle_close(ivy::message::Raw &message);

    void handle_send(ivy::message::Raw &message);
};


}
class sender {

};


#endif //IVY_SENDER_H
