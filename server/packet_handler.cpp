//
// Created by Tang on 12/21/2018.
//

#include <glog/logging.h>
#include "packet_handler.h"
#include "../crypto.h"
#include "../protocol.h"


namespace ivy {

using proto::Encrypted;
using proto::Decrypted;

bool handle_packet(uint8_t *buf, int buf_len) {
    // Decrypt
    auto *encrypted_message = (Encrypted *) buf;
    int payload_len = buf_len - AES::DEFAULT_KEYLENGTH;
    if (payload_len <= 0) {
        return false;
    }
    aes256_decrypt(
            encrypted_message->payload,
            (size_t) payload_len,
            encrypted_message->payload,
            encrypted_message->iv);

    auto *decrypted_message = (Decrypted *) buf;

    switch (decrypted_message->type) {
        case proto::Type::KEEP_ALIVE:
            break;
        default:
            LOG(WARNING) << "Invalid proto type: " << decrypted_message->type;
            return false;
    }
}

}