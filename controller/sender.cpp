//
// Created by tang on 1/20/19.
//

#include "sender.h"

#include <glog/logging.h>
#include "util.h"

namespace ivy {


Sender::Sender(std::shared_ptr<ivy::RawMessageQueue> up_queue,
               std::shared_ptr<ivy::PeerSyncQueue> peer_recv_queue,
               std::shared_ptr<ivy::PeerSyncQueue> peer_send_queue) {
    if (!up_queue || !peer_recv_queue) {
        LOG(FATAL) << "up_queue or peer_recv_queue cannot be null";
        return;
    }
    this->up_queue = std::move(up_queue);
    this->peer_recv_queue = std::move(peer_recv_queue);
    this->peer_send_queue = std::move(peer_send_queue);
    this->should_stop = true;
    this->thread = nullptr;
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
    LOG(INFO) << "Thread has stopped";

    return true;
}

void Sender::main_loop() {
    bool res;
    int rv;

    LOG(INFO) << "Sender thread is initializing";
    ConnectionTrait trait = {};
    while (peer_recv_queue->try_dequeue(trait)) {
        update_connection_rev(trait, connections);
    }
    LOG(INFO) << "Sender thread finish initailization";

    while (!should_stop) {
        while (peer_recv_queue->try_dequeue(trait)) {
            update_connection_rev(trait, connections);
        }

        ivy::message::Raw message(nullptr, 0);

        res = up_queue->wait_dequeue_timed(message, UP_QUEUE_TIMEOUT);
        if (!res) {
            continue;
        }

        // Find an arbitrary fd if multiple fd connected to target id exists
        if (!connections.count(message.id) || connections[message.id].empty()) {
            LOG(ERROR) << "No connection to " << id_to_printable(message.id) << " has established. Drop message";
            continue;
        }

        if (message.length > 0) {
            handle_send(message);
        } else if (message.length == 0) {
            handle_close(message);
        }
    }
}

void Sender::handle_close(ivy::message::Raw &message) {
    int rv;
    bool res;
    std::vector<int> fds_closed;
    ConnectionTrait trait = {};
    for (int outgoing_fd : connections[message.id]) {
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

            res = peer_send_queue->try_enqueue(trait);
            if (!res) {
                LOG(WARNING) << "Cannot enqueue trait to peer_send_queue";
            }
        }
    }
    for (int fd_closed : fds_closed) {
        connections[message.id].erase(fd_closed);
    }
}

void Sender::handle_send(ivy::message::Raw &message) {
    int outgoing_fd = *connections[message.id].begin();
    ssize_t byte_sent = send(outgoing_fd, message.data.get(), message.length, 0);
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
}


}