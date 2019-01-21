//
// Created by tang on 1/4/19.
//

#ifndef IVY_UTIL_H
#define IVY_UTIL_H

#include <stdint-gcc.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace ivy {

const static uint64_t TCP_LISTEN_ID = 1;

const static uint64_t PROTO_MASK = 0xFF00000000000000;
const static uint64_t TYPE_MASK  = 0x00FF000000000000;
const static uint64_t PORT_MASK  = 0x0000FFFF00000000;
const static uint64_t ADDR_MASK  = 0x00000000FFFFFFFF;

const static int PROTO_OFFSET = 56;
const static int TYPE_OFFSET = 48;
const static int PORT_OFFSET = 32;
const static int ADDR_OFFSET = 0;

inline static uint8_t get_proto(uint64_t id) {
    return (uint8_t)((id & PROTO_MASK) >> PROTO_OFFSET);
}

inline static uint8_t get_type(uint64_t id) {
    return (uint8_t)((id & TYPE_MASK) >> TYPE_OFFSET);
}

inline static uint16_t get_port(uint64_t id) {
    return (uint16_t)((id & PORT_MASK) >> PORT_OFFSET);
}

inline static uint32_t get_addr(uint64_t id) {
    return (uint32_t)((id & ADDR_MASK) >> ADDR_OFFSET);
}

inline uint64_t tuple_to_id(sockaddr_in &addr, uint8_t proto, uint8_t type) {
    return (
            (uint64_t)proto << 56
            | (uint64_t)type << 48
            | (uint64_t)(ntohs(addr.sin_port)) << 32
            | ntohl(addr.sin_addr.s_addr));
}

inline static std::string id_to_ip_str(uint64_t id) {
    in_addr addr{};
    addr.s_addr = htonl(get_addr(id));

    return std::string(inet_ntoa(addr));
}

inline static std::string id_to_printable(uint64_t id) {
    return id_to_ip_str(id) + ":" + std::to_string(get_port(id));
}

}

#endif //IVY_UTIL_H
