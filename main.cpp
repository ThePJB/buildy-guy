#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "danklib/vla.hpp"
#include "danklib/dict.hpp"
#include "danklib/colour.hpp"
#include "danklib/fatal.hpp"

#include "rendercontext.hpp"
#include "world.hpp"

#define len(X) (sizeof(X)/sizeof(X[0]))

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) fatal("couldn't initialize SDL");

    const auto tick_freq = SDL_GetPerformanceFrequency();

    auto dt = 0.0;

    const auto xres = 1600;
    const auto yres = 900;


    const auto window = SDL_CreateWindow(
        "buildy guy", 
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        xres,
        yres,
        SDL_WINDOW_SHOWN
    );
    if (window == NULL) fatal("null window");

    const auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) fatal("null renderer");

    auto rc = render_context(renderer, 0, 0, xres, yres);
    world w = world(current_time(), rc.a);

    auto keep_going = true;
    while (keep_going) {
        const auto start_tick = SDL_GetPerformanceCounter();
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (w.handle_event(e)) continue;
            if (e.type == SDL_QUIT) {
                keep_going = false;
                fflush(stdout);
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            } else if (e.type == SDL_KEYDOWN) {
                const auto sym = e.key.keysym.sym;
                if (sym == SDLK_r) {
                    w.destroy();
                    w = world(current_time(), rc.a);
                }
            }
        }

        w.update(dt, rc.a);

        // draw
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        w.draw(&rc);

        auto keystate = SDL_GetKeyboardState(NULL);

        SDL_RenderPresent(renderer);
        const auto end_tick = SDL_GetPerformanceCounter();
        dt = ((double)(end_tick - start_tick))/tick_freq;
        const auto fps = 1.0 / dt;
    }
}
