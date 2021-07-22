#pragma once
#include "rendercontext.hpp"
#include "danklib/dict.hpp"

// storing ids probably superfluous
struct AABBComponent {
    uint32_t id;

    float x;
    float y;
    float w;
    float h;
};

struct MotionComponent {
    uint32_t id;

    float vx;
    float vy;

    bool grounded = false;

};

struct ControllerComponent {
    uint32_t id;
};

enum EntityType {
    ET_PLAYER,
    ET_PLATFORM,
    ET_WALL,
};

struct EntityBaseComponent {
    uint32_t id;
    EntityType type;
    rgb colour;
};

struct world {
    void draw(render_context *rc);
    void update(float dt, float a);
    bool handle_event(SDL_Event e);

    uint32_t player_id;

    dict<AABBComponent> comp_aabb;
    dict<MotionComponent> comp_motion;
    dict<ControllerComponent> comp_controller;
    dict<EntityBaseComponent> comp_base; // this is probably dumb

    world(uint32_t seed, float a);


    void aabb_slide(uint32_t id, float dx, float dy);

    uint32_t rng = 0;
    float cam_x = 0;
    float last_wall = 0;
    int num_walls = 0;

    void make_platform(float x, float y, float w, float h, rgb colour);
    void make_wall(float x, float y, float w, float h);

    void player_build_platform(uint32_t player_id, int which_platform);

    void destroy() {
        comp_aabb.destroy();
        comp_motion.destroy();
        comp_controller.destroy();
        comp_base.destroy();
    }
};