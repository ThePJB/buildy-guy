#pragma once
#include "rendercontext.hpp"
#include "danklib/dict.hpp"
#include "vla.hpp"
#include "controller.hpp"
#include "motion.hpp"
#include "collisionEvent.hpp"



// storing ids probably superfluous
struct AABBComponent {
    uint32_t id;

    float x;
    float y;
    float w;
    float h;
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
    int z;
};

struct world {
    double time = 0;

    uint32_t rng = 0;
    float cam_x = 0;
    float last_wall = 0;
    int num_walls = 0;

    uint32_t player_id;

    dict<AABBComponent> comp_aabb;
    dict<MotionComponent> comp_motion;
    dict<EntityBaseComponent> comp_base; // this is probably dumb

    world(uint32_t seed, float a);

    void draw(render_context *rc);
    bool update(double dt, double time, float a);
    bool handle_event(SDL_Event e, double time);

    PlatformerController controller;

    // Collision System
    vla<CollisionEvent> collisions = vla<CollisionEvent>(); // length set to 0 every frame, perfect
    void slide_entity(uint32_t id, float dx, float dy);


    void make_platform(float x, float y, float w, float h, rgb colour);
    void make_wall(float x, float y, float w, float h);

    void player_build_platform(uint32_t player_id, int which_platform);

    void destroy() {
        comp_aabb.destroy();
        comp_motion.destroy();
        comp_base.destroy();
        collisions.destroy();
    }
};