//*******************************************************************************
// aGE
// Copyright (c) 2026
//
// File: main.cpp
//*******************************************************************************

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdio>

struct App {
    SDL_Window* window = nullptr;
};

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
    auto* app = new App{};
    *appstate = app;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "aGE SDL TEST");
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 1280);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 960);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);

    app->window = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    if (!app->window) {
        SDL_Log("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    int pw = 0, ph = 0;
    SDL_GetWindowSizeInPixels(app->window, &pw, &ph);
    std::printf("Window created with size: %dx%d\n", pw, ph);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;
    if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE)
        return SDL_APP_SUCCESS;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    SDL_Delay(16);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    auto* app = static_cast<App*>(appstate);
    if (app) {
        SDL_DestroyWindow(app->window);
        delete app;
    }
    SDL_Quit();
}