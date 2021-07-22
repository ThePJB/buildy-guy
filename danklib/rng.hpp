#include <stdint.h>

// Hash stuff
int hash_intn(uint32_t seed, int min, int max);
float hash_floatn(uint32_t seed, float min, float max);
uint32_t hash(uint32_t seed);


// Floating point stuff
uint32_t just_float_bytes(float f);
float bytes_to_float(uint32_t float_bytes);
uint32_t round_float(uint32_t float_bytes, int num_bits);


// Time stuff
uint64_t current_time();