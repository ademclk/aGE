//*******************************************************************************
// aGE
// Copyright (c) 2026
//
// File: main.cpp
//*******************************************************************************

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <string>

import age.core.log;
import age.gfx.device;
import age.gfx.pipeline;
import age.gfx.renderer;

namespace {
// passed in as a compile definition so main.cpp never hardcodes a path.
constexpr auto kShaderBinDir = AGE_SHADER_BIN_DIR;

std::string shader_path(const char* name) { return std::string{kShaderBinDir} + "/" + name; }

struct App {
    SDL_Window* window = nullptr;
    age::Device device;
    age::Pipeline pipeline;
    age::Renderer renderer;
    bool ready = false;
};

}  // namespace

SDL_AppResult SDL_AppInit(void** appstate, int, char**) {
    auto* app = new App{};
    *appstate = app;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        age::log_error("SDL_Init failed: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    app->window = SDL_CreateWindow("aGE - v0.1.0", 1280, 720, SDL_WINDOW_RESIZABLE);

    if (app->window == nullptr) {
        age::log_error("SDL_CreateWindow failed: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    int pixel_w = 0;
    int pixel_h = 0;
    SDL_GetWindowSizeInPixels(app->window, &pixel_w, &pixel_h);
    age::log_info("window created: {}x{} pixels", pixel_w, pixel_h);

    if (!app->device.init(app->window)) {
        return SDL_APP_FAILURE;
    }
    if (!app->pipeline.init(app->device, shader_path("triangle.vsMain"), shader_path("triangle.fsMain"))) {
        return SDL_APP_FAILURE;
    }
    if (!app->renderer.init(app->device, app->pipeline)) {
        return SDL_APP_FAILURE;
    }

    app->ready = true;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    (void)appstate;
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    auto* app = static_cast<App*>(appstate);
    if (!app->ready) {
        return SDL_APP_CONTINUE;
    }

    switch (app->renderer.draw_frame()) {
        case age::FrameResult::ok:
        case age::FrameResult::skipped:
            break;
        case age::FrameResult::lost:
            age::log_error("device lost; exiting");
            return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult) {
    auto* app = static_cast<App*>(appstate);

    if (app != nullptr) {
        // Destroy dependent resources before their parents,
        // the reverse order of initialization.
        app->renderer.shutdown();
        app->pipeline.shutdown();
        app->device.shutdown();
        if (app->window != nullptr) {
            SDL_DestroyWindow(app->window);
        }
        delete app;
    }

    SDL_Quit();
}
