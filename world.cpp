#include "world.hpp"

const auto cam_speed = 0.4;
const auto cam_start_offset = -0.7;
const auto gravity = 3.5;
const auto jump_velocity = -1.5;
const auto movement_speed = 0.8;

const auto platform_line1 = 0.2;
const auto platform_line2 = 0.5;
const auto platform_line3 = 0.8;

const auto platform_height = 0.05;
const auto platform_width = 0.1;

const auto indicator_height = 0.0125;
const auto indicator_width = 0.025;

//const auto platform_offset = vec2(0.4, 0.1);
const float platform_offset[3] = {0.4, 0.5, 0.6};

const auto sky_colour = rgb(0.8, 0.8, 1.0);
const auto platform_colour1 = rgb(1,0,0);
const auto platform_colour2 = rgb(0,1,0);
const auto platform_colour3 = rgb(0,0,1);
const auto wall_colour = rgb(0.7, 0.7, 0.7);
const auto player_colour = rgb(1,1,1);

const auto wall_gap_size = 0.3;
const auto wall_w = 0.1;

const auto wall_bounds = vec2(0.05, 0.6);

const auto max_z = 4;

void print_aabb(AABBComponent item) {
    printf("%f %f %f %f", item.x, item.y, item.w, item.h);
}

bool world::handle_event(SDL_Event e, double time) {
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
        auto motion = comp_motion.get(player_id);
        controller.space_pressed(time, motion);

    } else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_SPACE) {
        auto motion = comp_motion.get(player_id);
        controller.space_released(time, motion);

    } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_j) {
        player_build_platform(player_id, 0);
    } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_k) {
        player_build_platform(player_id, 1);
    } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_l) {
        player_build_platform(player_id, 2);

    } else {
        return false;
    }
    return true;
}

void world::draw(render_context *rc) {
    rc->draw_rect(sky_colour, 0, 0, rc->a, 1);

    rc->draw_line(platform_colour1, vec2(0, platform_line1), vec2(1, platform_line1), 2);
    rc->draw_line(platform_colour2, vec2(0, platform_line2), vec2(1, platform_line2), 2);
    rc->draw_line(platform_colour3, vec2(0, platform_line3), vec2(1, platform_line3), 2);

    for (int i = 0; i < max_z; i++) {
        auto it = comp_aabb.iter();
        while (auto aabb = it.next()) {
            const auto base = comp_base.get(aabb->id);
            if (i != base->z) continue;
            rc->draw_rect(base->colour, aabb->x - cam_x, aabb->y, aabb->w, aabb->h);
        }
    }

    const auto player_x = comp_aabb.get(player_id)->x;

    rc->draw_rect(platform_colour1, 
        player_x + platform_offset[2] + platform_width/2 - indicator_width/2 - cam_x,
        platform_line1 - indicator_height/2,
        indicator_width, indicator_height
    );

    rc->draw_rect(platform_colour2, 
        player_x + platform_offset[1] + platform_width/2 - indicator_width/2 - cam_x,
        platform_line2 - indicator_height/2,
        indicator_width, indicator_height
    );

    rc->draw_rect(platform_colour3, 
        player_x + platform_offset[0] + platform_width/2 - indicator_width/2 - cam_x,
        platform_line3 - indicator_height/2,
        indicator_width, indicator_height
    );
}

// returns whether to keep going
bool world::update(double dt, double time, float a) {
    this->time = time;
    collisions.length = 0;

    const auto player_motion = comp_motion.get(player_id);

    auto keystates = SDL_GetKeyboardState(NULL);

    auto it = comp_motion.iter();
    while (auto motion = it.next()) {
        // apply gravity
        motion->vy += gravity * dt;
        // slide its aabb
        slide_entity(motion->id, motion->vx * dt, motion->vy * dt);
    }

    controller.check_collisions(&collisions, player_motion, time);

    player_motion->vx = keystates[SDL_SCANCODE_A] ? -movement_speed:
                keystates[SDL_SCANCODE_D] ? movement_speed:
                0.0;

    cam_x += dt * cam_speed;

    const auto next_interval = hash_floatn(rng + num_walls, 0.6, 2.0);
    const auto next_height = hash_floatn(hash(rng + num_walls), 0, 1 - wall_gap_size - 0.05); // so you can see it
    if (cam_x - last_wall > next_interval) {
        num_walls++;
        last_wall = cam_x;

        make_wall(a + cam_x, 0, wall_w, next_height);
        make_wall(a + cam_x, -100, wall_w, 100);
        make_wall(a + cam_x, wall_gap_size + next_height, wall_w, 1);
    }

    // check collisions
    for (auto col = collisions.begin(); col < collisions.end(); col++) {
        if (col->e1 != player_id) continue;
        if (comp_base.get(col->e2)->type == ET_WALL) {
            printf("you died\n");
            return false;
        }
    }
    return true;
}

void world::player_build_platform(uint32_t player_id, int which_platform) {
    auto player_aabb = comp_aabb.get(player_id);

    auto x = 0.0f;
    auto y = 0.0f;
    x = player_aabb->x + platform_offset[which_platform];
    if (which_platform == 0) {
        y = platform_line3 -platform_height/2;
        make_platform(x, y, platform_width, platform_height, platform_colour3);
    } else if (which_platform == 1) {
        y = platform_line2 -platform_height/2;
        make_platform(x, y, platform_width, platform_height, platform_colour2);
    } else if (which_platform == 2) {
        y = platform_line1 -platform_height/2;
        make_platform(x, y, platform_width, platform_height, platform_colour1);
    }
}


void world::make_platform(float x, float y, float w, float h, rgb colour) {
    rng = hash(rng);
    comp_aabb.set(rng, (AABBComponent){.id = rng, .x = x, .y = y, .w = w, .h = h});
    comp_base.set(rng, (EntityBaseComponent){.id = rng, .type = ET_PLATFORM, .colour = colour, .z = 1});
}

void world::make_wall(float x, float y, float w, float h) {
    rng = hash(rng);
    comp_aabb.set(rng, (AABBComponent){.id = rng, .x = x, .y = y, .w = w, .h = h});
    comp_base.set(rng, (EntityBaseComponent){.id = rng, .type = ET_WALL, .colour = wall_colour, .z = 0});
    num_walls++;
}

world::world(uint32_t seed, float a) {
    rng = seed;

    cam_x = cam_start_offset;


    const auto next_height2 = hash_floatn(hash(rng + num_walls), 0, 1 - wall_gap_size - 0.05); // so you can see it
    make_wall(a + cam_x, 0, wall_w, next_height2);
    make_wall(a + cam_x, -100, wall_w, 100);
    make_wall(a + cam_x, wall_gap_size + next_height2, wall_w, 1);

    // make player
    rng = hash(rng);
    player_id = rng;
    comp_aabb.set(rng, (AABBComponent){.id = rng, .x = a/2 + cam_x + platform_width/2, .y = platform_line2 - 0.1, .w = 0.05, .h = 0.05});
    comp_motion.set(rng, (MotionComponent){.id = rng, .vx = 0, .vy = 0});
    controller = (PlatformerController) {
        .id = rng,
    };
    comp_base.set(rng, (EntityBaseComponent){.id = rng, .type = ET_PLAYER, .colour = player_colour, .z = 2});

    make_platform(a/2 + cam_x, platform_line2 - platform_height/2, platform_width, platform_height, platform_colour2);

}