//
// Created by tang on 12/30/18.
//

#ifndef IVY_LOGGING_FORMATS_H
#define IVY_LOGGING_FORMATS_H

#include <stdint-gcc.h>
#include "../protocol.h"

namespace ivy {
namespace logging{

struct KeepAliveLog {
    uint8_t unused;
    char device_id[proto::MAX_DEVICE_LEN];
    uint32_t client_time;
    uint32_t server_time;
} __attribute__ ((packed));

}
}

#endif //IVY_LOGGING_FORMATS_H
