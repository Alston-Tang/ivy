//
// Created by tang on 1/20/19.
//

#include "sender.h"

#include <glog/logging.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "util/ip_id.h"
#include "util/scope_guard.h"

namespace ivy {

const static int UP_QUEUE_TIMEOUT = 1000;

Sender::Sender(std::shared_ptr<ivy::RawMessageQueue> up_queue,
               std::shared_ptr<ivy::PeerSyncQueue> peer_recv_queue,
               std::shared_ptr<ivy::PeerSyncQueue> peer_send_queue) {
    if (!up_queue) {
        LOG(FATAL) << "up_queue cannot be null";
        return;
    }
    this->up_queue = std::move(up_queue);
    this->peer_recv_queue = std::move(peer_recv_queue);
    this->peer_send_queue = std::move(peer_send_queue);
    this->should_stop = true;
    this->running = false;
    this->thread = nullptr;

}

Sender::~Sender() {
    stop();
}

bool Sender::run() {
    LOG(INFO) << "Try to run sender thread";
    if (!this->should_stop) {
        LOG(WARNING) << "Thread is running as should_stop == false";
        return false;
    }
    this->should_stop = false;
    if (this->thread) {
        LOG(WARNING) << "Previous thread exists as thread != nullptr";
        return false;
    }
    this->thread = new std::thread([this] { this->main_loop(); });
    LOG(INFO) << "Sender thread is running";

    return true;
}

bool Sender::stop() {
    LOG(INFO) << "Try to stop sender thread";
    if (this->should_stop) {
        LOG(WARNING) << "Thread is stopping as should_stop == true";
        return false;
    }
    this->should_stop = true;
    if (!this->thread) {
        LOG(WARNING) << "Wait for the thread exiting";
        return false;
    }
    LOG(INFO) << "Wait for the thread exiting";
    this->thread->join();
    delete this->thread;
    this->thread = nullptr;
    LOG(INFO) << "Thread has stopped";

    return true;
}

bool Sender::is_running() {
    return running;
}

void Sender::main_loop() {
    bool res;
    int rv;


    LOG(INFO) << "Sender thread is initializing";

    ConnectionTrait trait = {};
    ConnectionsRevType connections_rev;

    ScopeGuard guard([&](){
        this->running = false;
        for (auto &connection_rev : connections_rev) {
            for (auto connection_fd : connection_rev.second) {
                 if (connection_fd >= 0) {
                     if (close(connection_fd) != 0) {
                         LOG(ERROR) << "Cannot close connection fd " << connection_fd;
                         perror("Error: close");
                     }
                 }
            }
        }
    });

    if (peer_recv_queue) {
        while (peer_recv_queue->readIfNotEmpty(trait)) {
            update_connection_rev(trait, connections_rev);
        }
    }
    LOG(INFO) << "Sender thread finish initailization";

    while (!should_stop) {
        running = true;
        if (peer_recv_queue) {
            while (peer_recv_queue->readIfNotEmpty(trait)) {
                update_connection_rev(trait, connections_rev);
            }
        }

        ivy::message::Raw message(nullptr, 0);

        res = up_queue->tryReadUntil(std::chrono::system_clock::now() + std::chrono::milliseconds(UP_QUEUE_TIMEOUT), message);
        if (!res) {
            continue;
        }

        if (message.length > 0) {
            handle_send(connections_rev, message);
        } else if (message.length == 0) {
            handle_close(connections_rev, message);
        }
    }
}

void Sender::handle_close(ConnectionsRevType &connections_rev, ivy::message::Raw &message) {
    int rv;
    bool res;
    std::vector<int> fds_closed;
    ConnectionTrait trait = {};
    for (int outgoing_fd : connections_rev[message.id]) {
        rv = close(outgoing_fd);
        if (rv != 0) {
            LOG(ERROR) << "Cannot close fd " << outgoing_fd << " connected to " << id_to_printable(message.id);
            perror("Error: close");
            continue;
        }
        fds_closed.push_back(outgoing_fd);
        if (peer_send_queue) {
            trait.action = ConnectionAction::CLOSE;
            trait.id = message.id;
            trait.fd = outgoing_fd;

            res = peer_send_queue->write(trait);
            if (!res) {
                LOG(WARNING) << "Cannot enqueue trait to peer_send_queue";
            }
        }
    }
    for (int fd_closed : fds_closed) {
        connections_rev[message.id].erase(fd_closed);
    }
}

void Sender::handle_send(ConnectionsRevType &connections_rev, ivy::message::Raw &message) {
    switch (get_type(message.id)) {
        case SOCK_STREAM:
            return handle_tcp_send(connections_rev, message);
        case SOCK_DGRAM:
            return handle_udp_send(connections_rev, message);
        default:
            LOG(ERROR) << "Unsupported network type";
            return;
    }
}

void Sender::handle_tcp_send(ConnectionsRevType &connections_rev, ivy::message::Raw &message) {
    ssize_t byte_sent;
    int rv;
    if (!connections_rev.count(message.id) || connections_rev[message.id].empty()) {
        int new_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (new_fd == -1) {
            LOG(ERROR) << "Cannot create new tcp socket. Ignore sending to " << id_to_printable(message.id);
            perror("Error: socket");
            return;
        }
        sockaddr_in new_addr = id_to_sockaddr_in(message.id);
        rv = connect(new_fd, (sockaddr *) &new_addr, sizeof(new_addr));
        if (rv != 0) {
            LOG(WARNING) << "Cannot connect to " << id_to_printable(message.id);
            perror("Error: connect");
            close(new_fd);
            return;
        }

        connections_rev[message.id].insert(new_fd);
    }

    int outgoing_fd = *connections_rev[message.id].begin();
    byte_sent = send(outgoing_fd, message.data.get(), message.length, 0);
    if (byte_sent < 0) {
        LOG(ERROR) << "Cannot send message to " << id_to_printable(message.id) << " with fd " << outgoing_fd;
        perror("Error: send");
        return;
    }
    if (byte_sent < message.length) {
        // TODO? Probably should handle partial sent case
        LOG(ERROR) << "Only partial data has been sent";
        return;
    }
};

void Sender::handle_udp_send(ConnectionsRevType &connections_rev, ivy::message::Raw &message) {
    ssize_t byte_sent;
    int rv;
    if (!connections_rev.count(message.id) || connections_rev[message.id].empty()) {
        int new_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (new_fd == -1) {
            LOG(ERROR) << "Cannot create new udp socket. Ignore sending to " << id_to_printable(message.id);
            perror("Error: socket");
            return;
        }
        connections_rev[message.id].insert(new_fd);
    }

    int outgoing_fd = *connections_rev[message.id].begin();
    auto addr = id_to_sockaddr_in(message.id);
    byte_sent = sendto(outgoing_fd, message.data.get(), message.length, 0, (sockaddr*)&addr, sizeof(addr));
    if (byte_sent < 0) {
        LOG(ERROR) << "Cannot send message to " << id_to_printable(message.id) << " with fd " << outgoing_fd;
        perror("Error: sendto");
        return;
    }
    if (byte_sent < message.length) {
        // TODO? Probably should handle partial sent case
        LOG(ERROR) << "Only partial data has been sent";
        return;
    }
}


}