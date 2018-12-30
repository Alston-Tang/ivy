//
// Created by Tang on 12/20/2018.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <csignal>
#include <cstdio>
#include <cerrno>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <iostream>

#include <glog/logging.h>

#include "packet_handler.h"

const static unsigned short SERVER_PORT = IVY_SERVER_PORT;
const static int RECV_BUF_SIZE = 1500;

bool stop = false;
int sock_fd = -1;
uint8_t buf[RECV_BUF_SIZE];

void signal_handler(int signal)
{
    LOG(INFO) << "SIGINT received. Server will stop";
    close(sock_fd);
    stop = true;
}

int main(int argc, char **argv) {

    FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);

    LOG(INFO) << "Server initializing";
    LOG(INFO) << "Port: " << SERVER_PORT << std::endl;

    signal(SIGINT, signal_handler);

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        perror("Error: socket()");
        return errno;
    }

    sockaddr_in server_addr = {
        sin_family: AF_INET,
        sin_port: htons(SERVER_PORT),
        sin_addr: {s_addr :INADDR_ANY}
    };
    int rv = bind(sock_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    if (rv == -1) {
        perror("Error: bind()");
        return errno;
    }

    while (!stop) {
        LOG(INFO) << "Looping...";
        sockaddr_in client_addr = {};
        socklen_t client_addr_len = sizeof(client_addr);
        ssize_t recv_length = recvfrom(sock_fd, buf, RECV_BUF_SIZE, 0, (sockaddr*)&client_addr, &client_addr_len);

        if (recv_length < 0) {
            if (!stop) {
                perror("Error: recvfrom");
                return errno;
            }
            continue;
        }
        if (recv_length == 0) continue;

        auto res = ivy::handle_packet(buf, (int)recv_length);
        if (!res) {
            LOG(WARNING) << "Can not handle a packet";
        }
    }

    return 0;
};