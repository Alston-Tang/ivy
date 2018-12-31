//
// Created by Tang on 12/21/2018.
//

#include <glog/logging.h>
#include "packet_handler.h"
#include "../crypto.h"
#include "../protocol.h"
#include "compact_logger.h"
#include "logging_formats.h"
#include "logging_util.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
namespace ivy {

using proto::Encrypted;
using proto::Decrypted;

bool handle_keep_alive(Decrypted *buf, int buf_len) {
    if (buf_len != sizeof(proto::KeepAlive)) {
        LOG(ERROR) << "Wrong or incomplete keep alive message received.";
        return false;
    }
    auto &manager = CompactLoggerManager::get_instance();
    auto keep_alive_logger = manager.get_logger("keep_alive");

    auto *msg = (proto::KeepAlive*) buf;
    logging::KeepAliveLog log = {};
    bool res = logging::keep_alive_proto_to_log(msg, &log);
    if (!res) return false;

    return keep_alive_logger->log((char*)&log, sizeof(logging::KeepAliveLog));
}

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
            return handle_keep_alive(decrypted_message, buf_len);
        default:
            LOG(WARNING) << "Invalid proto type: " << decrypted_message->type;
            return false;
    }
}

}
#pragma clang diagnostic pop