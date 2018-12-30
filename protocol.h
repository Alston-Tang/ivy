//
// Created by Tang on 12/20/2018.
//

#ifndef SIMPLE_EXAMPLE_PROTOCOL_H
#define SIMPLE_EXAMPLE_PROTOCOL_H

#include "crypto.h"

namespace ivy {
namespace proto {


// Type Begin 0 - 255
enum Type {
    KEEP_ALIVE = 1
};
// Type End

struct KeepAlive {
    iv_t iv;
    uint8_t type;
    char device_id[15];
    uint64_t client_time;
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
