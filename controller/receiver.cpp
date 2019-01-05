#include <utility>

//
// Created by tang on 1/4/19.
//

#include "receiver.h"

#include <glog/logging.h>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "util.h"

namespace ivy {

namespace {

int set_socket_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        LOG(ERROR) << "Cannot get original flags from fd " << fd;
        perror("Error: fcntl");
        return flags;
    }
    flags = flags | O_NONBLOCK;
    int rv = fcntl(fd, F_SETFL, flags);
    if (rv != 0) {
        LOG(ERROR) << "Cannot set new flag to fd " << fd;
        perror("Error: fcntl");
    }
    return rv;
}

void handle_accept(int tcp_listen_fd, int epoll_fd, std::unordered_map<int, uint64_t> &fd_map) {
    int rv;
    sockaddr_in incoming_addr{};
    socklen_t incoming_addr_len = sizeof(incoming_addr);

    while (true) {
        int incoming_fd = accept(tcp_listen_fd, (sockaddr *) &incoming_addr, &incoming_addr_len);
        if (incoming_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            LOG(ERROR) << "Cannot accept new tcp connection";
            perror("Error: accept");
            continue;
        }

        rv = set_socket_non_blocking(incoming_fd);
        if (rv != 0) {
            rv = close(incoming_fd);
            if (rv != 0) {
                LOG(WARNING) << "Cannot close incoming fd " << incoming_fd;
                perror("Error: close");
            }
            continue;
        }

        uint64_t incoming_id = tuple_to_id(incoming_addr, AF_INET, SOCK_STREAM);
        fd_map[incoming_fd] = incoming_id;

        epoll_event new_event{};
        new_event.events = EPOLLIN;
        new_event.data.fd = incoming_fd;
        rv = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, incoming_fd, &new_event);
        if (rv != 0) {
            LOG(ERROR) << "Cannot add incoming fd " << incoming_fd << "into epoll";
            perror("Error: epoll_ctl");

            rv = close(incoming_fd);
            if (rv != 0) {
                LOG(WARNING) << "Cannot close incoming fd " << incoming_fd;
                perror("Error: close");
            }
            continue;
        }

        LOG(INFO) << "New connection from " << inet_ntoa(incoming_addr.sin_addr) << ":" << ntohs(incoming_addr.sin_port)
                  << ". Assigned ID: " << incoming_id;
    }
}

void handle_receive(int incoming_fd, uint64_t incoming_id, int epoll_fd, std::unordered_map<int, uint64_t> &fd_map,
                    std::shared_ptr<RawMessageQueue> &up_queue) {
    int rv;

    while (true) {
        ivy::message::Raw message(new uint8_t[Receiver::RECV_BUFFER_LEN]);

        ssize_t received = recv(incoming_fd, message.data.get(), Receiver::RECV_BUFFER_LEN, 0);
        if (received < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                break;
            }
            LOG(ERROR) << "Cannot receive from tcp connection socket " << incoming_fd;
            perror("Error: recv");
            return;
        }

        if (received == 0) {
            rv = close(incoming_fd);
            if (rv != 0) {
                LOG(ERROR) << "Cannot close closed socket. "
                           << "fd: " << incoming_fd
                           << " ip: " << id_to_ip_str(incoming_id)
                           << " port: " << get_port(incoming_id);
            }
            unsigned long erased = fd_map.erase(incoming_fd);
            if (erased != 1) {
                LOG(WARNING) << "Cannot erase closed tcp connection from fd_map";
            }
            LOG(INFO) << "Connection to " << id_to_ip_str(incoming_id) << ":" << get_port(incoming_id) << " close";
            return;
        }

        message.length = (unsigned int) received;
        bool res = up_queue->enqueue(message);
        if (!res) {
            LOG(FATAL) << "Cannot enqueue message into shared queue";
        }

        if (received < Receiver::RECV_BUFFER_LEN) break;
    }
}

}

Receiver::Receiver(std::shared_ptr<RawMessageQueue> up_queue, uint16_t port) {
    this->up_queue = std::move(up_queue);
    this->should_stop = true;
    this->thread = nullptr;
    this->port = port;
}

bool Receiver::run() {
    LOG(INFO) << "Try to run receiver thread";
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
    LOG(INFO) << "Receiver thread is running";

    return true;
}

bool Receiver::stop() {
    LOG(INFO) << "Try to stop receiver thread";
    if (this->should_stop) {
        LOG(WARNING) << "Thread is stopping as should_stop == true";
        return false;
    }
    this->should_stop = true;
    if (!this->thread) {
        LOG(WARNING) << "Thread doesn't exists";
        return false;
    }
    LOG(INFO) << "Wait for the thread exiting";
    this->thread->join();
    delete this->thread;
    this->thread = nullptr;
    LOG(INFO) << "Thread has stopped";

    return true;
}

void Receiver::main_loop() {
    int rv;
    std::unordered_map<int, uint64_t> fd_map;

    LOG(INFO) << "Receiver thread is initializing";

    int tcp_listen_fd = -1;
    int epoll_fd = -1;

    tcp_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_listen_fd == -1) {
        LOG(ERROR) << "Cannot open tcp socket";
        perror("Error: open");
        return;
    }

    sockaddr_in tcp_listen_addr{};
    tcp_listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcp_listen_addr.sin_port = htons(this->port);
    tcp_listen_addr.sin_family = AF_INET;

    rv = bind(tcp_listen_fd, (sockaddr *) &tcp_listen_addr, sizeof(sockaddr_in));
    if (rv != 0) {
        LOG(ERROR)
                << "Cannot bind socket to " << inet_ntoa(tcp_listen_addr.sin_addr) << " : " << this->port;
        perror("Error: bind");
        return;
    }

    rv = listen(tcp_listen_fd, MAX_PENDING_LISTEN);
    if (rv != 0) {
        LOG(ERROR) << "Cannot listen on tcp socket with MAX_PENDING_LISTEN = " << MAX_PENDING_LISTEN;
        perror("Error: listen");
        return;
    }

    rv = set_socket_non_blocking(tcp_listen_fd);
    if (rv != 0) {
        // Error message has printed by set_socket_non_blocking
        return;
    }

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        LOG(ERROR) << "Cannot create epoll fd";
        perror("Error: epoll");
        return;
    }
    epoll_event tcp_listen_event{};
    tcp_listen_event.events = EPOLLIN;
    tcp_listen_event.data.fd = tcp_listen_fd;
    rv = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_listen_fd, &tcp_listen_event);
    if (rv != 0) {
        LOG(ERROR) << "Cannot add tcp listen fd into epoll";
        perror("Error: epoll");
        return;
    }

    fd_map[tcp_listen_fd] = TCP_LISTEN_ID;

    LOG(INFO) << "Receiver thread finish initialization";

    epoll_event epoll_events[MAX_EPOLL_EVENTS];
    while (!should_stop) {
        rv = epoll_wait(epoll_fd, epoll_events, MAX_EPOLL_EVENTS, EPOLL_TIMEOUT);
        if (rv < 0) {
            LOG(ERROR) << "Epoll error during loop";
            perror("Error: epoll");
            continue;
        }
        if (rv == 0) continue; // epoll return due to timeout
        int events_len = rv;
        for (int i = 0; i < events_len; i++) {
            int cur_fd = epoll_events[i].data.fd;
            if (!fd_map.count(cur_fd)) {
                LOG(ERROR) << "A fd not present in fd_map. This should never happen";
                continue;
            }

            uint64_t cur_id = fd_map[cur_fd];

            switch (cur_id) {
                case TCP_LISTEN_ID:
                    handle_accept(cur_fd, epoll_fd, fd_map);
                    break;
                default:
                    handle_receive(cur_fd, cur_id, epoll_fd, fd_map, up_queue);
                    break;
            }
        }
    }

    LOG(INFO) << "Receiver thread is doing cleaning";

    rv = close(epoll_fd);
    if (rv != 0) {
        LOG(ERROR) << "Cannot close epoll fd";
        perror("Error: close");
    }
    rv = close(tcp_listen_fd);
    if (rv != 0) {
        LOG(ERROR) << "Cannot close tcp listen fd";
        perror("Error: close");
    }
}

}
