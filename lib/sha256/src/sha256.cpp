#include "sha256/sha256.h"
#include <assert.h>
#include <string.h>

#ifdef _MSC_VER

#include <stdlib.h>
#define __bswap_32(x) _byteswap_ulong(x)
#define __bswap_64(x) _byteswap_uint64(x)

#elif defined(__APPLE__)

// Mac OS X / Darwin features
#include <libkern/OSByteOrder.h>
#define __bswap_32(x) OSSwapInt32(x)
#define __bswap_64(x) OSSwapInt64(x)

#elif defined(__sun) || defined(sun)

#include <sys/byteorder.h>
#define __bswap_32(x) BSWAP_32(x)
#define __bswap_64(x) BSWAP_64(x)

#elif defined(__FreeBSD__)

#include <sys/endian.h>
#define __bswap_32(x) bswap32(x)
#define __bswap_64(x) bswap64(x)

#elif defined(__OpenBSD__)

#include <sys/types.h>
#define __bswap_32(x) swap32(x)
#define __bswap_64(x) swap64(x)

#elif defined(__NetBSD__)

#include <sys/types.h>
#include <machine/bswap.h>
#if defined(__BSWAP_RENAME) && !defined(__bswap_32)
#define __bswap_32(x) bswap32(x)
#define __bswap_64(x) bswap64(x)
#endif

#else

#include <byteswap.h>

#endif

static const size_t BITS_IN_BYTE = 8;

static inline uint32_t rotr(const uint32_t value, const unsigned short count) {
    assert(count > 0 && count < sizeof(int32_t) * BITS_IN_BYTE);

    return value >> count | value << (sizeof(int32_t) * BITS_IN_BYTE - count);
}

static inline uint32_t maj(const uint32_t a, const uint32_t b, const uint32_t c) {

    // Straightforward solution: (a & b) | (b & c) | (c & a);
    // But this one is faster, because it requires one less operation:
    return (a & (b | c)) | (b & c);
}

static inline uint32_t choice(const uint32_t driver_bits,
                              const uint32_t a,
                              const uint32_t b) {
    // Truth table for choice function:

    // d a b |
    // ------+---
    // 0 0 0 | 0
    // 0 0 1 | 1
    // 0 1 0 | 0
    // 0 1 1 | 1
    // 1 0 0 | 0
    // 1 0 1 | 0
    // 1 1 0 | 1
    // 1 1 1 | 1

    return (a & driver_bits) | (b & ~ driver_bits);
}

static inline uint32_t sigma0(const uint32_t value) {
    return rotr(value, 7) ^ rotr(value, 18) ^ (value >> 3);
}

static inline uint32_t sigma1(const uint32_t value) {
    return rotr(value, 17) ^ rotr(value, 19) ^ (value >> 10);
}

static inline uint32_t upsigma0(const uint32_t value) {
    return rotr(value, 2) ^ rotr(value, 13) ^ rotr(value, 22);
}

static inline uint32_t upsigma1(const uint32_t value) {
    return rotr(value, 6) ^ rotr(value, 11) ^ rotr(value, 25);
}

// Constants mandated by the SHA-256 specification
static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// Message data processing stage
enum separating_stage {
    WRITE_COMPLETED_WORDS,
    WRITE_LAST_WORD,
    WRITE_PADDING_ZEROS,
    WRITE_SIZE, FINISHED
};

struct message_data {
    const unsigned char* const data;
    const size_t size;

    size_t bytes_read;
    separating_stage stage;
};

static message_data construct_message_data(const void* data, size_t size) {
    return { (const unsigned char*) data, size, 0, WRITE_COMPLETED_WORDS };
}

// This number is mandated by the SHA-256 specification
static const size_t WORDS_IN_MESSAGE = 16;

static inline bool test_for_little_endian(void) {
    // Use volatile to prevent compiler optimizations
    volatile uint32_t value = 0x01234567;
    return *((volatile uint8_t*) (&value)) == 0x67;
    // Same number with bytes reversed        ^~~~
}

// Calculate endian only once
static const bool is_little_endian = test_for_little_endian();

#define MIN(a, b)               \
   ({ __typeof__ (a) __a = (a); \
      __typeof__ (b) __b = (b); \
     __a < __b ? __a : __b; })

inline static bool message_write_completed_words(message_data* const data,
                                                 uint32_t* const message,
                                                 size_t* const written_words) {

    const size_t total_of_completed_words =
        (data->size - data->bytes_read) / sizeof(uint32_t);

    const size_t completed_words_in_this_message =
        MIN(WORDS_IN_MESSAGE, total_of_completed_words);

    // Write full 32 bit words to the output array, if there's too
    // many of them, we're taking first 512 bits (16 of 32 bit words)
    for (size_t i = 0LU; i < completed_words_in_this_message; ++ i)
        message[(*written_words) ++] =
            ((const uint32_t*) (data->data + data->bytes_read))[i];

    data->bytes_read += completed_words_in_this_message * sizeof(uint32_t);

    if (total_of_completed_words >= WORDS_IN_MESSAGE) {
        if (total_of_completed_words == WORDS_IN_MESSAGE)
            data->stage = WRITE_LAST_WORD;

        return true; // Otherwise stage remains same: writing completed words
    }

    // This stage is completed, but there's still space for next one
    return false; 
}

inline static void message_write_last_word(message_data* const data,
                                           uint32_t* const message,
                                           size_t* const written_words) {

    // Last stage ensures that there's enough space
    // for completion of this one, so it always succeeds.

    // Write '1' that separates data and padding
    uint32_t last_data_word = 0x80; // 10000000 in binary

    if (data->size % 4LU != 0) {
        const size_t bytes_left = data->size - data->bytes_read;

        for (size_t i = bytes_left; i > 0; -- i) {
            last_data_word <<= BITS_IN_BYTE; // Shift left one byte

            // Insert byte in the free space
            last_data_word |= *(data->data + data->bytes_read + i - 1);
        }
    }

    // TODO: Fix this function for big endian

    message[(*written_words) ++] = last_data_word;
    data->stage = WRITE_PADDING_ZEROS;
}

// Size according to SHA-256 specification spans 64 bits
const size_t SIZE_SPANS_WORDS = sizeof(uint64_t) / sizeof(uint32_t);

inline static bool message_write_padding_zeros(uint32_t* const message,
                                               size_t* const written_words) {

    size_t space_for_size = 0;

    // If we can place size in this message, then reserve space for it:
    if (WORDS_IN_MESSAGE - *written_words >= SIZE_SPANS_WORDS)
        space_for_size = SIZE_SPANS_WORDS;

    while (*written_words + space_for_size < WORDS_IN_MESSAGE)
        message[(*written_words) ++] = 0U;

    if (WORDS_IN_MESSAGE - *written_words < SIZE_SPANS_WORDS)
        return true; // Stage remains same, next block will be zeros & size

    // This stage ensures that there's enough space left for
    // completing the next one
    return false;
}

inline static void message_write_size(message_data* const data,
                                      uint32_t* const message,
                                      size_t* const written_words) {

    assert(*written_words == WORDS_IN_MESSAGE - SIZE_SPANS_WORDS);

    uint64_t message_size = data->size * BITS_IN_BYTE;
    if (is_little_endian)
        message_size = __bswap_64(message_size);

    *(uint64_t*) &message[*written_words] = message_size;
    data->stage = FINISHED;
}

// Returns true if new message were constructed id est you
// can use it in while like so:

// while(get_next_message_block(...))
//     ... // Processs block

inline static void swap_words_endian(uint32_t* words, size_t size) {
    for (size_t i = 0; i < size; ++ i)
        words[i] = __bswap_32(words[i]);
}

inline static bool get_next_message_block(message_data* const data,
                                          uint32_t message[WORDS_IN_MESSAGE]) {

    size_t written_words = 0;

    switch (data->stage) {
    default:
        assert(false && "Illegal state!");

        [[fallthrough]];

    case FINISHED:
        return false; // All the data has already been processed

    case WRITE_COMPLETED_WORDS:
        if (message_write_completed_words(data, message, &written_words))
            break;

        [[fallthrough]];

    case WRITE_LAST_WORD:
        message_write_last_word(data, message, &written_words);

        [[fallthrough]];

    case WRITE_PADDING_ZEROS:
        if (message_write_padding_zeros(message, &written_words))
            break;

        [[fallthrough]];

    case WRITE_SIZE:
        message_write_size(data, message, &written_words);
        // After this stage data gets marked as finished, so
        // this function will return false on the next call
    }

    if (is_little_endian)
        swap_words_endian(message, WORDS_IN_MESSAGE);

    return true; // This function has written some data
}

inline static void generate_message_schedule(uint32_t words[64]) {
    for (int i = 16; i < 64; ++ i)
        words[i] = sigma1(words[i -  2]) + words[i -  7]
                 + sigma0(words[i - 15]) + words[i - 16];
}

static const __uint32_t H[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

inline static int modulo(int a, int b) {
    int mod = a % b;

    if (mod < 0)
        mod = (b < 0) ? mod - b : mod + b;

    return mod;
}

inline static uint32_t* modulo_get(uint32_t* array, int index, int size) {
    return &array[modulo(index, size)];
};

// Registers for compression stage of computing SHA-256.
// They named according to the standard. Order and according number matters.
enum registers {
    REG_A, REG_B, REG_C,
    REG_D, REG_E, REG_F,
    REG_G, REG_H, NUM_REGISTERS
};

inline static void compress(uint32_t schedule[64],
                            uint32_t registers[NUM_REGISTERS]) {

    uint32_t starting_values[8];
    memcpy(starting_values, registers, sizeof(starting_values));

    for (int i = 0; i < 64; ++ i) {
        #define GET_REG(reg) \
            modulo_get(registers, REG_##reg - i, NUM_REGISTERS)

        uint32_t a = *GET_REG(A), b = *GET_REG(B), c = *GET_REG(C),
                 e = *GET_REG(E), f = *GET_REG(F), g = *GET_REG(G),
                 h = *GET_REG(H);

        uint32_t t1 = upsigma1(e) + choice(e, f, g) + h + K[i] + schedule[i],
                 t2 = upsigma0(a) +    maj(a, b, c);

        // H will become A after the cycle:
        *GET_REG(H) = t1 + t2;

        // D will become E after the cycle:
        *GET_REG(D) += t1;

        #undef GET_REG
    }

    for (int i = 0; i < 8; ++ i)
        registers[i] += starting_values[i];
}

auto sha256::hash(block hashed_block) -> sha256_t {

    message_data data = construct_message_data(hashed_block.buffer, hashed_block.size);

    sha256_t output_hash = {};

    // Fill registers with H0 values
    memcpy(output_hash.begin(), H, 8 * sizeof(uint32_t));

    uint32_t message[64];
    while(get_next_message_block(&data, message)) {
        // Generate another 64 - 16 message entries:
        generate_message_schedule(message);

        // Compression stage, use previous values in registers for it:
        compress(message, output_hash.begin());
    }

    return output_hash;
}
