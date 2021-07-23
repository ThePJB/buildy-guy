#include "world.hpp"

#define min(A,B) ((A) < (B) ? (A) : (B))
#define max(A,B) ((A) > (B) ? (A) : (B))

const auto cam_speed = 0.5;
const auto cam_start_offset = -0.7;
const auto gravity = 3.5;
const auto jump_velocity = -1.5;
const auto movement_speed = 0.8;

const auto platform_line1 = 0.2;
const auto platform_line2 = 0.5;
const auto platform_line3 = 0.8;

const auto platform_height = 0.05;
const auto platform_width = 0.1;

const auto indicator_height = 0.025;
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
const auto wall_w = 0.2;

const auto max_z = 4;

void print_aabb(AABBComponent item) {
    printf("%f %f %f %f", item.x, item.y, item.w, item.h);
}

bool world::handle_event(SDL_Event e) {
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_SPACE) {
            auto it = comp_controller.iter();
            while (auto controller = it.next()) {
                auto motion = comp_motion.get(controller->id);
                if (motion->grounded) {
                    motion->vy = jump_velocity;
                    motion->grounded = false;
                }
            }
            return true;
        } else if (e.key.keysym.sym == SDLK_j) {
            player_build_platform(player_id, 0);
            return true;
        } else if (e.key.keysym.sym == SDLK_k) {
            player_build_platform(player_id, 1);
            return true;
        } else if (e.key.keysym.sym == SDLK_l) {
            player_build_platform(player_id, 2);
            return true;
        }
    }
    return false;
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

bool aabb_overlap(AABBComponent a, AABBComponent b) {
    // might need epsilon
    // overlap IFF x overlap and Y overlap
    return ((a.x < b.x && a.x + a.w > b.x) || (b.x < a.x && b.x + b.w > a.x)) &&// x overlap
        ((a.y < b.y && a.y + a.h > b.y) || (b.y < a.y && b.y + b.h > a.y));      // y overlap
}

// return the collision direction
// basically if x overlap is more, in what direction

// must be returning wrong collision normal in the we are below case
// not detecting a below conllision
vec2 aabb_direction(AABBComponent old_pos, AABBComponent candidate_pos, AABBComponent other) {
    const auto old_left = old_pos.x;
    const auto old_right = old_pos.x + old_pos.w;
    const auto old_top = old_pos.y;
    const auto old_bot = old_pos.y + old_pos.h;

    const auto candidate_left = candidate_pos.x;
    const auto candidate_right = candidate_pos.x + candidate_pos.w;
    const auto candidate_top = candidate_pos.y;
    const auto candidate_bot = candidate_pos.y + candidate_pos.h;

    const auto other_left = other.x;
    const auto other_right = other.x + other.w;
    const auto other_top = other.y;
    const auto other_bot = other.y + other.h;

    // was not colliding, is now colliding
    if (old_right < other_left && candidate_right >= other_left) {
        return vec2(-1, 0); // we are to left
    }
    if (old_bot < other_top && candidate_bot >= other_top) {    // this could be eating it, or one of the other ifs could be
        return vec2(0, -1); // we are above
    }
    if (old_left > other_right && candidate_left <= other_right) {
        return vec2(1, 0); // we are to right
    }
    if (old_top > other_bot && candidate_top <= other_bot) {    // this branch might not be happening
        return vec2(0, 1); // we are below // this looks correct to me fuk
    }


    printf("bad aabb thing\n");
    //exit(1);

    return vec2(0,-1); // umm we are now in the realm of floating point weirdness
    // stops you falling through the floor at least
}

void world::aabb_slide(uint32_t id, float dx, float dy) {
    // how does this work
    // check if they would overlap
    // if yes figure out compromise position AND appropriately kill velocity, or maybe bool touching
        // determine which side collision occurred
            // one which has least overlap
                // eg up has least overlap, therefore align bottom to top and clamp vy
            // bonus points: include velocity (prob not necessary though)
    auto this_aabb = comp_aabb.get(id);
    
    auto current_aabb = *this_aabb;
    auto wanted_aabb = *this_aabb;
    wanted_aabb.x += dx;
    wanted_aabb.y += dy;

    float max_x = INFINITY;
    float min_x = -INFINITY;
    float max_y = INFINITY;
    float min_y = -INFINITY;

    const auto epsilon = 0.0005;

    auto aabb_it = comp_aabb.iter();
    while (auto other_aabb = aabb_it.next()) {
        if (other_aabb == this_aabb) continue;

        if (aabb_overlap(wanted_aabb, *other_aabb)) {
            //printf("overlap %f %f %f %f and %f %f %f %f\n", wanted_aabb.x, wanted_aabb.y, wanted_aabb.w, wanted_aabb.h, other_aabb->item.x, other_aabb->item.y, other_aabb->item.w, other_aabb->item.h);
            const auto collision_normal = aabb_direction(current_aabb, wanted_aabb, *other_aabb);
            //printf("collision normal %f %f\n", collision_normal.x, collision_normal.y);
            if (collision_normal.x == -1) max_x = other_aabb->x - this_aabb->w;
            if (collision_normal.x == 1) min_x = other_aabb->x + other_aabb->w;
            if (collision_normal.y == -1) {
                max_y = other_aabb->y - this_aabb->h;

                if (auto motion = comp_motion.get(id)) {
                    motion->grounded = true;
                    motion->vy = 0;
                }
            }
            if (collision_normal.y == 1) {
                min_y = other_aabb->y + other_aabb->h;

                if (auto motion = comp_motion.get(id)) {
                    motion->vy = 0;
                }
            }

        }
    }

    //printf("max y %f\n", max_y);

    if (dx > 0) {
        this_aabb->x = min(wanted_aabb.x, max_x - epsilon);
    }
    if (dx < 0) {
        this_aabb->x = max(wanted_aabb.x, min_x + epsilon);
    }

    if (dy > 0) {   // we are going down
        const auto old_y = this_aabb->y;
        this_aabb->y = min(wanted_aabb.y, max_y - epsilon);
        auto motion = comp_motion.get(id);
        if (motion && this_aabb->y - old_y > epsilon) {
            motion->grounded = false;
        }
        
    }
    if (dy < 0) {
        this_aabb->y = max(wanted_aabb.y, min_y + epsilon); // and then this case
    }
    //printf("aabb set to %f %f %f %f\n", this_aabb->x, this_aabb->y, this_aabb->w, this_aabb->h);

}

void world::update(float dt, float a) {
    auto keystates = SDL_GetKeyboardState(NULL);
    
    {
        auto it = comp_controller.iter();
        while (auto controller = it.next()) {
            auto motion = comp_motion.get(controller->id);
            motion->vx = keystates[SDL_SCANCODE_A] ? -movement_speed:
                        keystates[SDL_SCANCODE_D] ? movement_speed:
                        0.0;
        }
    }

    {
        auto it = comp_motion.iter();
        while (auto motion = it.next()) {
            // apply gravity
            motion->vy += gravity * dt;
            // slide its aabb
            aabb_slide(motion->id, motion->vx * dt, motion->vy * dt);
        }
    }

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

    const auto next_height = hash_floatn(hash(rng + num_walls), 0, 1 - wall_gap_size - 0.05); // so you can see it
    make_wall(a/2 + cam_x, 0, wall_w, next_height);
    make_wall(a/2 + cam_x, -100, wall_w, 100);
    make_wall(a/2 + cam_x, wall_gap_size + next_height, wall_w, 1);

    const auto next_height2 = hash_floatn(hash(rng + num_walls), 0, 1 - wall_gap_size - 0.05); // so you can see it
    make_wall(a + cam_x, 0, wall_w, next_height2);
    make_wall(a + cam_x, -100, wall_w, 100);
    make_wall(a + cam_x, wall_gap_size + next_height2, wall_w, 1);

    // make player
    rng = hash(rng);
    player_id = rng;
    comp_aabb.set(rng, (AABBComponent){.id = rng, .x = a/2 + cam_x + wall_w/2, .y = next_height + wall_gap_size - 0.05, .w = 0.05, .h = 0.05});
    comp_motion.set(rng, (MotionComponent){.id = rng, .vx = 0, .vy = 0});
    comp_controller.set(rng, (ControllerComponent){.id = rng});
    comp_base.set(rng, (EntityBaseComponent){.id = rng, .type = ET_PLAYER, .colour = player_colour, .z = 2});

}