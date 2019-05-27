//
// Created by tang on 1/20/19.
//

#ifndef IVY_SENDER_H
#define IVY_SENDER_H

#include "messages/raw.h"
#include "controller.h"
#include <thread>

namespace ivy {

class Sender {

public:
    explicit Sender(
            std::shared_ptr<RawMessageQueue> up_queue,
            std::shared_ptr<PeerSyncQueue> peer_recv_queue = nullptr,
            std::shared_ptr<PeerSyncQueue> peer_send_queue = nullptr);
    ~Sender();

    bool run();

    bool stop();

    bool is_running();

private:
    uint16_t port;
    std::shared_ptr<RawMessageQueue> up_queue;
    std::shared_ptr<PeerSyncQueue> peer_send_queue;
    std::shared_ptr<PeerSyncQueue> peer_recv_queue;
    std::thread *thread;
    std::atomic<bool> should_stop;
    std::atomic<bool> running;

    // Use a single udp socket to handle all udp sending
    // This socket will be only alive inside main_loop()
    int udp_fd;

    void main_loop();

    void handle_close(ConnectionsRevType &connections_rev, ivy::message::Raw &message);

    void handle_send(ConnectionsRevType &connections_rev, ivy::message::Raw &message);

    void handle_tcp_send(ConnectionsRevType &connections_rev, ivy::message::Raw &message);

    void handle_udp_send(ConnectionsRevType &connections_rev, ivy::message::Raw &message);
};


}


#endif //IVY_SENDER_H
