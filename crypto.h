//
// Created by Tang on 12/18/2018.
//

#ifndef IVY_CRYPTO_H
#define IVY_CRYPTO_H

#include <cstdint>

#include <cryptopp/aes.h>

using CryptoPP::AES;

namespace ivy {

#ifndef IVY_MASTER_KEY
#error "Please define a master key"
#endif

typedef std::array<uint8_t, AES::DEFAULT_KEYLENGTH> iv_t;

static const unsigned char master_key[32] = IVY_MASTER_KEY;
static const size_t master_key_length = 32;

iv_t aes256_encrypt(uint8_t *src, size_t len, uint8_t *dst);

void aes256_decrypt(uint8_t *src, size_t len, uint8_t *dst, iv_t &iv);

}

#endif //IVY_CRYPTO_H


