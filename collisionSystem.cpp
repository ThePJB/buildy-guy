#include "world.hpp"
#include "collisionEvent.hpp"

#define min(A,B) ((A) < (B) ? (A) : (B))
#define max(A,B) ((A) > (B) ? (A) : (B))

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

void world::slide_entity(uint32_t id, float dx, float dy) {
    /*
        Slide entity as far as possible in the desired direction,
        writing any collisions to world->collisions

        it used to do 3 things: move things, detect collisions, resolve collisions
        ditch that last one
    */

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
            const auto collision_normal = aabb_direction(current_aabb, wanted_aabb, *other_aabb);

            // report the event
            collisions.push((CollisionEvent){
                .e1 = id,
                .e2 = other_aabb->id,
                .normal = collision_normal,
            });

            // calculate movement bounds
            if (collision_normal.x == -1) max_x = other_aabb->x - this_aabb->w;
            if (collision_normal.x == 1) min_x = other_aabb->x + other_aabb->w;
            if (collision_normal.y == -1) max_y = other_aabb->y - this_aabb->h;
            if (collision_normal.y == 1) min_y = other_aabb->y + other_aabb->h;
        }

        // apply movement
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
                controller.begin_fall(time);    // this is if anything falls though so do need to check
            }
        }
        if (dy < 0) {
            this_aabb->y = max(wanted_aabb.y, min_y + epsilon);
        }
    }
}