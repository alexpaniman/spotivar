#pragma once

#include <cstdint>
#include <cstddef>

#include <array>

namespace sha256 {


    // Hashed block, can have arbitrary size 
    struct block {
        const void* buffer;
        std::size_t size;
    };


    inline constexpr std::size_t HASH_SIZE = 8;
    using sha256_t = std::array<uint32_t, HASH_SIZE>;


    // Hash block with sha256
    sha256_t hash(block hashed_block);


};

