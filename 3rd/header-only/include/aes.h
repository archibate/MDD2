//
// Created by Alexi on 17.11.2023.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>

// Internal constants
// Do not edit

const unsigned int Nb = 4;
const unsigned int Nk = 8;        // The number of 32 bit words in a key.
const unsigned int Nr = 14;       // The number of rounds in AES Cipher.
const unsigned int AES_BLOCK_LEN = 16; // Block length in bytes - AES is 128b block only
const unsigned int AES_keyExpSize = 240; // 16*15 or 16*(Nr+1)

/**
 * AES key bytes array
 */
using aes_key_t = std::array<uint8_t, 4 * Nk>;

/**
 * AES block array
 */
using aes_block_t = std::array<uint8_t, AES_BLOCK_LEN>;

/**
 * Encrypt or decrypt blocks with AES256-CTR
 *
 * @param out Vector of blocks
 * @param sk Secret key
 * @param counter AES counter
 * @param blocks Number of blocks to encrypt
 */
void aesctr256(std::vector<aes_block_t>& out, const aes_key_t &sk, const aes_block_t &counter);

/**
 * Encrypt or decrypt one block with AES256-CTR
 *
 * @param block Data block
 * @param sk Secret key
 * @param counter AES counter
 * @param blocks Number of blocks to encrypt
 */
void aesctr256(aes_block_t& block, const aes_key_t &sk, const aes_block_t &counter);

/**
 * Increment V
 *
 * @param counter AES counter
 */
inline static void aesctr256_increment_v(aes_block_t& counter) {
    // Treat AES counter as a big-endian integer
    for (auto rit = counter.rbegin(); counter.rend() != rit; ++rit) { // NOTE: Apple Clang doesn't support range view
        auto& curr = *rit;
        if (0xff == curr) {
            curr = 0x00;
        } else {
            ++curr;
            break;
        }
    }
}
