//
// Created by tang on 12/30/18.
//

#ifndef IVY_LOGGING_UTIL_H
#define IVY_LOGGING_UTIL_H

#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "../protocol.h"
#include "logging_formats.h"

namespace ivy {
namespace logging {

bool keep_alive_proto_to_log(proto::KeepAlive *proto, KeepAliveLog *log) {
    memcpy(log->device_id, proto->device_id, proto::MAX_DEVICE_LEN);
    log->client_time = ntohl(proto->client_time);
    long cur_time = time(nullptr);
    if (cur_time < 0) {
        perror("Error: time");
        return false;
    }
    log->server_time = (uint32_t)cur_time;

    return true;
}

}
}

#endif //IVY_LOGGING_UTIL_H
