//
// Created by Tang on 12/20/2018.
//

#ifndef SIMPLE_EXAMPLE_PROTOCOL_H
#define SIMPLE_EXAMPLE_PROTOCOL_H

#include <netinet/in.h>
#include "crypto.h"

namespace ivy {
namespace proto {

inline static uint64_t htonll(uint64_t x) {
    return ((1 == htonl(1)) ? (x) : ((uint64_t) htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32));
}

inline static uint64_t ntohll(uint64_t x) {
    return ((1 == ntohl(1)) ? (x) : ((uint64_t) ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32));
}

// Type Begin 0 - 255
enum Type {
    KEEP_ALIVE = 1
};
// Type End

// Choose 15 because we need 1 byte type field at the beginning
// 15 + 1 = 16 to improve memory alignment
const static int MAX_DEVICE_LEN = 15;

struct KeepAlive {
    iv_t iv;
    uint8_t type;
    char device_id[MAX_DEVICE_LEN];
    uint32_t client_time;
} __attribute__ ((__packed__));

struct Decrypted {
    iv_t iv;
    uint8_t type;
} __attribute__ ((__packed__));

struct Encrypted {
    iv_t iv;
    uint8_t payload[0];
} __attribute__ ((__packed__));

}
}

#endif //SIMPLE_EXAMPLE_PROTOCOL_H
