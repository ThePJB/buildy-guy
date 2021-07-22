#include "noise.hpp"
#include "dankmath.hpp"
#include "rng.hpp"

float hash_noise2(vec2 v, int seed) {
    int x1 = dm_floor(v.x);
    int x2 = x1 + 1;
    int y1 = dm_floor(v.y);
    int y2 = y1 + 1;

    return dm_bilinear3(
        hash_floatn(x1 + y1 * 987423234 + seed * 342447, 0, 1), 
        hash_floatn(x2 + y1 * 987423234 + seed * 342447, 0, 1), 
        hash_floatn(x1 + y2 * 987423234 + seed * 342447, 0, 1), 
        hash_floatn(x2 + y2 * 987423234 + seed * 342447, 0, 1), 
        dm_frac(v.x), dm_frac(v.y)
    );
}

// _4 is how many hash_noise2 calls
float hash_fbm2_4(vec2 v, int seed) {
    return (
        1.000 * hash_noise2(1.0f * v, seed + 234243) +
        0.500 * hash_noise2(2.0f * v, seed + 980732) +
        0.250 * hash_noise2(4.0f * v, seed + 895642) +
        0.125 * hash_noise2(8.0f * v, seed + 987443)
    ) / 1.875;
}