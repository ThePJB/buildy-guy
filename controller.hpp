#pragma once
#include <stdint.h>
#include "motion.hpp"
#include "collisionEvent.hpp"

#define COTOYE_TIME 0.1

struct PlatformerController {
    uint32_t id;

    double coyote_time_start = 0;
    bool held = false;
    bool jumping = false;
    bool falling = false;

    bool within_coyote_time(double time) {
        return (time - coyote_time_start) < COTOYE_TIME;
    }

    void force_unground() {
        coyote_time_start = 0;
    }

    void begin_fall(double when) { // this might be a bit stupid
        if (falling) return;
        coyote_time_start = when;
    }

    void space_pressed(double when, MotionComponent *motion) {
        if (jumping) return;
        if (!within_coyote_time(when)) return;

        motion->vy = -1.5;
        force_unground();
        held = true;
        jumping = true;
        falling = false;
    }

    void space_released(double when, MotionComponent *motion) {
        held = false;
        if (motion->vy < 0) {
            motion->vy /= 2;
        }
    }

    void check_collisions(vla<CollisionEvent> *collisions, MotionComponent *motion, double when) {
        for (auto col = collisions->begin(); col < collisions->end(); col++) {
            if (col->e1 == id) {
                if (col->normal.y == -1) {
                    // from above
                    motion->vy = 0;
                    coyote_time_start = when;
                    falling = true;
                    jumping = false;
                } else if (col->normal.y == 1) {
                    // from below
                    motion->vy = 0;
                }
            }
        }
    }
};