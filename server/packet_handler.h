//
// Created by Tang on 12/21/2018.
//

#ifndef IVY_PACKET_HANDLER_H
#define IVY_PACKET_HANDLER_H

#include <cstdint>

namespace ivy {

bool handle_packet(uint8_t *buf, int buf_len);

}

#endif //IVY_PACKET_HANDLER_H
