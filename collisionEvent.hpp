#pragma once

#include "vec2.hpp"
#include <stdint.h>

struct CollisionEvent {
    uint32_t e1;
    uint32_t e2;
    vec2 normal;
};