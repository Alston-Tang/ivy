//
// Created by Tang on 12/18/2018.
//

#include "crypto.h"


#include <cryptopp/modes.h>
using CryptoPP::CFB_Mode;
#include <cryptopp/osrng.h>
using CryptoPP::AutoSeededRandomPool;

namespace ivy {

iv_t aes256_encrypt(uint8_t *src, size_t len, uint8_t *dst) {
    // Generate random iv
    iv_t iv{};
    AutoSeededRandomPool rnd;
    rnd.GenerateBlock(iv.data(), AES::DEFAULT_KEYLENGTH);
    // Encrypt
    CFB_Mode<AES>::Encryption cfbEncryption(master_key, master_key_length, iv.data());
    cfbEncryption.ProcessData(dst, src, len);

    return iv;
}

void aes256_decrypt(uint8_t *src, size_t len, uint8_t *dst, iv_t &iv) {
    CFB_Mode<AES>::Decryption cfbDecryption(master_key, master_key_length, iv.data());
    cfbDecryption.ProcessData(dst, src, len);
}

}

