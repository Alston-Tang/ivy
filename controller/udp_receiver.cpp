#include <utility>

//
// Created by tang on 5/26/19.
//

#include "udp_receiver.h"

#include <glog/logging.h>
#include <sys/time.h>

#include "util/scope_guard.h"

namespace ivy {

const static int RECV_BUFFER_LEN = 1500;
const static int UDP_SOCK_TIMEOUT = 1000; //ms
const static int UP_QUEUE_TIMEOUT = 100; //ms

UdpReceiver::UdpReceiver(std::shared_ptr<ivy::RawMessageQueue> up_queue,
                         uint16_t port) {
    this->up_queue = std::move(up_queue);
    this->port = port;
    this->running = false;
    this->should_stop = true;
    this->thread = nullptr;
}

UdpReceiver::~UdpReceiver() = default;

bool UdpReceiver::run() {
    LOG(INFO) << "Try to run UdpReceiver thread";
    if (!this->should_stop) {
        LOG(WARNING) << "Thread is running as should_stop = false";
        return false;
    }
    this->should_stop = false;
    if (this->thread) {
        LOG(WARNING) << "Previous thread exists as thread != nullptr";
        return false;
    }
    this->thread = new std::thread([this] { this->main_loop(); });
    LOG(INFO) << "UdpReceiver thread is running";

    return true;
}

bool UdpReceiver::stop() {
    LOG(INFO) << "Try to stop UdpReceiver thread";
    if (this->should_stop) {
        LOG(WARNING) << "Thread is stopping as should_stop == true";
        return false;
    }
    this->should_stop = true;
    if (!this->thread) {
        LOG(WARNING) << "Thread doesn't exist";
        return false;
    }
    LOG(INFO) << "Wait for the thread exiting";
    this->thread->join();
    delete this->thread;
    this->thread = nullptr;
    LOG(INFO) << "Thread has stopped";

    return true;
}

bool UdpReceiver::is_running() {
    return running;
}

void UdpReceiver::main_loop() {
    int rv;
    bool res;

    LOG(INFO) << "UdpReceiver thread is initializing";

    int udp_fd = -1;

    ScopeGuard guard([&](){
        this->running = false;
        if (udp_fd > 0) {
            if (close(udp_fd) != 0) {
                LOG(WARNING) << "Cannot close udp fd " << udp_fd;
                perror("Error: close");
            }
        }
    });

    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_fd == 1) {
        LOG(ERROR) << "Cannot open udp socket";
        perror("Error: socket");
        return;
    }

    sockaddr_in udp_bind_addr{};
    udp_bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    udp_bind_addr.sin_port = htons(this->port);
    udp_bind_addr.sin_family = AF_INET;

    struct timeval udp_timeout_tv = {};
    udp_timeout_tv.tv_sec = UDP_SOCK_TIMEOUT / 1000;
    udp_timeout_tv.tv_usec = (UDP_SOCK_TIMEOUT % 1000) * 1000;

    rv = setsockopt(udp_fd, SOL_SOCKET, SO_RCVTIMEO, &udp_timeout_tv, sizeof(udp_timeout_tv));
    if (rv != 0) {
        LOG(ERROR) << "Cannot set udp socket timeout";
        perror("Error: setsockopt");
        return;
    }

    rv = bind(udp_fd, (sockaddr *) &udp_bind_addr, sizeof(sockaddr_in));
    if (rv != 0) {
        LOG(ERROR)
                << "Cannot bind socket to " << inet_ntoa(udp_bind_addr.sin_addr) << " : " << this->port;
        perror("Error: bind");
        return;
    }

    while (!should_stop) {
        this->running = true;

        uint8_t buf[RECV_BUFFER_LEN];
        struct sockaddr_in remote_addr = {};
        socklen_t remote_addr_struct_len = sizeof(struct sockaddr_in);
        ssize_t received = recvfrom(udp_fd, buf, RECV_BUFFER_LEN, 0, (sockaddr*)&remote_addr, &remote_addr_struct_len);
        if (received > 0) {
            uint64_t incoming_id = tuple_to_id(remote_addr, AF_INET,SOCK_DGRAM);
            ivy::message::Raw message(new uint8_t[received], 0);
            message.length = static_cast<unsigned  int>(received);
            message.id = incoming_id;
            memcpy(message.data.get(), buf, static_cast<size_t>(received));
            res = up_queue->tryWriteUntil(std::chrono::system_clock::now() + std::chrono::milliseconds(UP_QUEUE_TIMEOUT), message);
            if (!res) {
                LOG(FATAL) << "Cannot enqueue message into shared queue";
            }
        } else if (received < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                LOG(ERROR) << "Cannot receive from udp socket";
                perror("Error: recvfrom");
            }
        }
    }
    LOG(INFO) << "UdpReceiver thread is doing cleaning";
}

};