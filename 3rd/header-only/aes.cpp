//
// Created by Alexi on 17.11.2023.
//

/**
 *    This code is derived from Tiny AES in C project
 */

#include "aes.h"
#include <algorithm>

// AES expanded key bytes
using round_key_t = std::array<uint8_t, AES_keyExpSize>;

// state - array holding the intermediate results during decryption.
typedef std::array<std::array<uint8_t, 4>, 4> state_t;

/*****************************************************************************/
/* Private variables:                                                        */
/*****************************************************************************/

// The lookup-tables are marked const, so they can be placed in read-only storage instead of RAM
// The numbers below can be computed dynamically trading ROM for RAM -
// This can be useful in (embedded) bootloader applications, where ROM is often limited.
static const std::array<uint8_t, 256> sbox = {
        //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

// The round constant word array, Rcon[i], contains the values given by
// x to the power (i-1) being powers of x (x is denoted as {02}) in the field GF(2^8)
static const std::array<uint8_t, 11> Rcon = {
        0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

/*
 * Jordan Goulder points out in PR #12 (https://github.com/kokke/tiny-AES-C/pull/12),
 * that you can remove most of the elements in the Rcon array, because they are unused.
 *
 * From Wikipedia's article on the Rijndael key schedule @ https://en.wikipedia.org/wiki/Rijndael_key_schedule#Rcon
 *
 * "Only the first some of these constants are actually used – up to rcon[10] for AES-128 (as 11 round keys are needed),
 *  up to rcon[8] for AES-192, up to rcon[7] for AES-256. rcon[0] is not used in AES algorithm."
 */


/*****************************************************************************/
/* Private functions:                                                        */
/*****************************************************************************/

inline static uint8_t getSBoxValue(uint8_t num) {
  return sbox[num];
}

// Function RotWord()
// This function shifts the 4 bytes in a word to the left once.
// [a0,a1,a2,a3] becomes [a1,a2,a3,a0]

inline static void RotWord(std::array<uint8_t, 4>& word) {
    const uint8_t u8tmp = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = word[3];
    word[3] = u8tmp;
}

// Function Subword()
// SubWord() is a function that takes a four-byte input word and
// applies the S-box to each of the four bytes to produce an output word.

inline static void SubWord(std::array<uint8_t, 4>& word) {
    word[0] = getSBoxValue(word[0]);
    word[1] = getSBoxValue(word[1]);
    word[2] = getSBoxValue(word[2]);
    word[3] = getSBoxValue(word[3]);
}


// This function produces Nb(Nr+1) round keys. The round keys are used in each round to decrypt the states.
static void KeyExpansion(round_key_t& RoundKey, const aes_key_t& Key) {
    unsigned i, j, k;
    std::array<uint8_t, 4> tempa {}; // Used for the column/row operations

    // The first round key is the key itself.
    for (i = 0; i < Nk; ++i) {
        RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
        RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
        RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
        RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
    }

    // Other round keys are found from the previous round keys.
    for (i = Nk; i < Nb * (Nr + 1); ++i) {
        {
            k = (i - 1) * 4;
            tempa[0] = RoundKey[k + 0];
            tempa[1] = RoundKey[k + 1];
            tempa[2] = RoundKey[k + 2];
            tempa[3] = RoundKey[k + 3];

        }

        if (i % Nk == 0) {

            RotWord(tempa);
            SubWord(tempa);
            tempa[0] = tempa[0] ^ Rcon[i / Nk];
        }

        if (i % Nk == 4) {
            SubWord(tempa);
        }

        j = i * 4;
        k = (i - Nk) * 4;
        RoundKey[j + 0] = RoundKey[k + 0] ^ tempa[0];
        RoundKey[j + 1] = RoundKey[k + 1] ^ tempa[1];
        RoundKey[j + 2] = RoundKey[k + 2] ^ tempa[2];
        RoundKey[j + 3] = RoundKey[k + 3] ^ tempa[3];
    }
}

// This function adds the round key to state.
// The round key is added to the state by an XOR function.
inline static void AddRoundKey(uint8_t round, state_t& state, const round_key_t &RoundKey) {
    for (uint8_t i = 0; i < 4; ++i) {
        for (uint8_t j = 0; j < 4; ++j) {
            state[i][j] ^= RoundKey[(round * Nb * 4) + (i * Nb) + j];
        }
    }
}

// The SubBytes Function Substitutes the values in the
// state matrix with values in an S-box.
inline static void SubBytes(state_t& state) {
    for (uint8_t i = 0; i < 4; ++i) {
        for (uint8_t j = 0; j < 4; ++j) {
            state[j][i] = getSBoxValue(state[j][i]);
        }
    }
}

// The ShiftRows() function shifts the rows in the state to the left.
// Each row is shifted with different offset.
// Offset = Row number. So the first row is not shifted.
static void ShiftRows(state_t& state) {
    uint8_t temp;

    // Rotate first row 1 column to left
    temp = state[0][1];
    state[0][1] = state[1][1];
    state[1][1] = state[2][1];
    state[2][1] = state[3][1];
    state[3][1] = temp;

    // Rotate second row 2 columns to left
    temp = state[0][2];
    state[0][2] = state[2][2];
    state[2][2] = temp;

    temp = state[1][2];
    state[1][2] = state[3][2];
    state[3][2] = temp;

    // Rotate third row 3 columns to left
    temp = state[0][3];
    state[0][3] = state[3][3];
    state[3][3] = state[2][3];
    state[2][3] = state[1][3];
    state[1][3] = temp;
}

inline static uint8_t xtime(uint8_t x) {
    return ((x << 1) ^ (((x >> 7) & 1) * 0x1b));
}

// MixColumns function mixes the columns of the state matrix
inline static void MixColumns(state_t& state) {
    for (uint8_t i = 0; i < 4; ++i) {
        uint8_t t = state[i][0];
        uint8_t Tmp = state[i][0] ^ state[i][1] ^ state[i][2] ^ state[i][3];
        uint8_t Tm = state[i][0] ^ state[i][1];
        Tm = xtime(Tm);
        state[i][0] ^= Tm ^ Tmp;
        Tm = state[i][1] ^ state[i][2];
        Tm = xtime(Tm);
        state[i][1] ^= Tm ^ Tmp;
        Tm = state[i][2] ^ state[i][3];
        Tm = xtime(Tm);
        state[i][2] ^= Tm ^ Tmp;
        Tm = state[i][3] ^ t;
        Tm = xtime(Tm);
        state[i][3] ^= Tm ^ Tmp;
    }
}

// Cipher is the main function that encrypts the PlainText.
static void Cipher(state_t& state, const round_key_t& RoundKey) {

    // Add the First round key to the state before starting the rounds.
    AddRoundKey(0, state, RoundKey);

    // There will be Nr rounds.
    // The first Nr-1 rounds are identical.
    // These Nr rounds are executed in the loop below.
    // Last one without MixColumns()
    for (uint8_t round = 1;; ++round) {
        SubBytes(state);
        ShiftRows(state);
        if (round == Nr) {
            break;
        }
        MixColumns(state);
        AddRoundKey(round, state, RoundKey);

    }
    // Add round key to last round
    AddRoundKey(Nr, state, RoundKey);
}

inline static void XorWithIv(aes_block_t& buf, const aes_block_t& Iv) {
    for (auto i = 0; i < buf.size(); ++i) {// The block in AES is always 128bit no matter the key size
        buf[i] ^= Iv[i];
    }
}

struct AES_ctx {
    round_key_t RoundKey;
    aes_block_t Iv;
};

inline void AES_init_ctx_iv(AES_ctx &ctx, const aes_key_t &key, const aes_block_t &iv) {
    KeyExpansion(ctx.RoundKey, key);
    std::copy_n(iv.begin(), iv.size(), ctx.Iv.begin());
}

/* Symmetrical operation: same function for encrypting as for decrypting. Note any IV/nonce should never be reused with the same key */
inline static void xcrypt_blocks(AES_ctx& ctx, aes_block_t* blocks, size_t length) {
    union {
        aes_block_t block;
        state_t state;
    } buffer {};

    auto limit = blocks + length;
    for (auto current = blocks; current < limit; ++current) {
        std::copy_n(ctx.Iv.begin(), ctx.Iv.size(), buffer.block.begin());
        Cipher(buffer.state, ctx.RoundKey);
        aesctr256_increment_v(ctx.Iv);
        XorWithIv(*current, buffer.block);
    }
}

/*****************************************************************************/
/* Public functions:                                                         */
/*****************************************************************************/

void aesctr256(std::vector<aes_block_t>& blocks, const aes_key_t &key, const aes_block_t &counter) {
    AES_ctx ctx{};
    AES_init_ctx_iv(ctx, key, counter);
    xcrypt_blocks(ctx, blocks.data(), blocks.size());
}

void aesctr256(aes_block_t& block, const aes_key_t &key, const aes_block_t &counter) {
    AES_ctx ctx{};
    AES_init_ctx_iv(ctx, key, counter);
    xcrypt_blocks(ctx, &block, 1);
}
