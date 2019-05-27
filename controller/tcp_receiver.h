//
// Created by tang on 1/4/19.
//

#ifndef IVY_TCP_RECEIVER_H
#define IVY_TCP_RECEIVER_H

#include <atomic>
#include <memory>
#include <thread>
#include <unordered_map>

#include "controller.h"
#include "messages/raw.h"



namespace ivy {

class TcpReceiver {

public:
    explicit TcpReceiver(
            std::shared_ptr<RawMessageQueue> up_queue,
            uint16_t port,
            std::shared_ptr<PeerSyncQueue> peer_recv_queue = nullptr,
            std::shared_ptr<PeerSyncQueue> peer_send_queue = nullptr);
    ~TcpReceiver();

    bool run();

    bool stop();

    bool is_running();

    const static int RECV_BUFFER_LEN = 65536;

private:
    uint16_t port;
    std::shared_ptr<RawMessageQueue> up_queue;
    std::shared_ptr<PeerSyncQueue> peer_send_queue;
    std::shared_ptr<PeerSyncQueue> peer_recv_queue;
    std::thread *thread;
    std::atomic<bool> should_stop;
    std::atomic<bool> running;

    void main_loop();

    void handle_accept(ConnectionsType &connections, int tcp_listen_fd, int epoll_fd);

    void handle_receive(ConnectionsType &connections, int incoming_fd, uint64_t incoming_id, int epoll_fd);
};

}


#endif //IVY_TCP_RECEIVER_H
