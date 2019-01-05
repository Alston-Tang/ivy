//
// Created by tang on 12/30/18.
//

#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <atomic>
#include <iostream>

const static unsigned short SERVER_PORT = IVY_SERVER_PORT;

struct complex {
    int a;
};

int main() {
    std::atomic<complex> test{};

    std::cout << test.is_lock_free() << std::endl;
}


