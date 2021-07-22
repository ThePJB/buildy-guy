#include "rng.hpp"
#include "dankmath.hpp"
#include <stdio.h>
#include <time.h>

#define U32_MAX 0xFFFFFFFF

// could reinterpret cast to seed with float
uint32_t hash(uint32_t seed) {
    const unsigned int BIT_NOISE1 = 0xB5297A4D;
    const unsigned int BIT_NOISE2 = 0x68E31DA4;
    const unsigned int BIT_NOISE3 = 0x1B56C4E9;

    unsigned int mangled = seed;
    mangled *= BIT_NOISE1;
    mangled ^= (mangled >> 8);
    mangled += BIT_NOISE2;
    mangled ^= (mangled << 8);
    mangled *= BIT_NOISE3;
    mangled ^= (mangled >> 8);
    return mangled;
}

float hash_floatn(uint32_t seed, float min, float max) {
    return ((double)hash(seed)/U32_MAX) * (max - min) + min;
}

int hash_intn(uint32_t seed, int min, int max) {
    return (hash(seed)%(max - min)) + min;
}


uint32_t just_float_bytes(float f) {
    return *reinterpret_cast<uint32_t *>(&f);
}

float bytes_to_float(uint32_t float_bytes) {
    return *reinterpret_cast<float *>(&float_bytes);
}

// for best results num_bits should be < mantissa length lmao
uint32_t round_float(uint32_t float_bytes, int num_bits) {
    // not sure about rounding up in binary, is that a thing?
    auto mask = 0xFF800000;
    for (int i = 0; i < num_bits; i++) {
        mask |= (1 << (23 - i));
    }
    return float_bytes & mask;
}

uint64_t current_time() {
  time_t rawtime;
  time(&rawtime);
  return rawtime;
}
