//
// Created by tang on 1/4/19.
//

#ifndef IVY_RECEIVER_H
#define IVY_RECEIVER_H

#include <memory>
#include <atomic>
#include <unordered_map>

#include "../concurrentqueue/blockingconcurrentqueue.h"
#include "messages/raw.h"
#include "controller.h"

namespace ivy {

class Receiver {

public:
    explicit Receiver(
            std::shared_ptr<RawMessageQueue> up_queue,
            uint16_t port,
            std::shared_ptr<PeerSyncQueue> peer_recv_queue = nullptr,
            std::shared_ptr<PeerSyncQueue> peer_send_queue = nullptr);

    bool run();

    bool stop();

    const static int RECV_BUFFER_LEN = 1500;

private:
    const static int MAX_PENDING_LISTEN = 10;
    const static int MAX_EPOLL_EVENTS = 20;
    const static int EPOLL_TIMEOUT = 1000; //ms

    uint16_t port;
    std::shared_ptr<RawMessageQueue> up_queue;
    std::shared_ptr<PeerSyncQueue> peer_send_queue;
    std::shared_ptr<PeerSyncQueue> peer_recv_queue;
    std::thread *thread;
    std::atomic<bool> should_stop;
    ConnectionsType connections;

    void main_loop();

    void handle_accept(int tcp_listen_fd, int epoll_fd);

    void handle_receive(int incoming_fd, uint64_t incoming_id, int epoll_fd);
};

}


#endif //IVY_RECEIVER_H
