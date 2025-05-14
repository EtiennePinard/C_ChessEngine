#include <stdio.h>

#include <SDL2/SDL_image.h>

#include "AppInit.h"

bool initializeSDlLibraries(u32 sdlFlags, u32 imageFlags) {
    if (SDL_Init(sdlFlags) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return false;
    }

    if (IMG_Init(imageFlags) == 0) {
        fprintf(stderr, "Failed to initialize SDL_image: %s\n", IMG_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    return true;
}

bool initializeSDLState(SDL_State* sdlState,
    const char* windowTitle, int windowX, int windowY, int windowWidth, int windowHeight, u32 windowFlags,
    u32 rendererFlags,
    const char* fontPath, int fontSize) {
    sdlState->window = SDL_CreateWindow(windowTitle, windowX, windowY, windowWidth, windowHeight, windowFlags);
    if (sdlState->window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    sdlState->renderer = SDL_CreateRenderer(sdlState->window, -1, rendererFlags);
    if (sdlState->renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdlState->window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    sdlState->font = TTF_OpenFont(fontPath, fontSize);
    if (sdlState->font == NULL) {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(sdlState->renderer);
        SDL_DestroyWindow(sdlState->window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    return true;
}

bool initTextures(Textures* textures) {
    if (textures->data != NULL) return true; // textures is already initialized

    textures->capacity = DEFAULT_TEXTURE_CAPACITY;
    textures->data = calloc(textures->capacity, sizeof(TextureState));
    return textures->data != NULL;
}

bool loadImageFromFilePath(SDL_State* sdlState, Textures* textures, const char** filePaths, size_t nbImages) {

    for (size_t index = 0; index < nbImages; index++) {
        SDL_RWops* rw = SDL_RWFromFile(filePaths[index], "rb");
        SDL_Surface* surface = IMG_Load_RW(rw, 1);
        if (surface == NULL) {
            fprintf(stderr, "Failed to load image %s: %s\n", filePaths[index], SDL_GetError());
            return false;
        }

        TextureState textureState = { 0 };

        textureState.texture = SDL_CreateTextureFromSurface(sdlState->renderer, surface);
        if (textureState.texture == NULL) {
            fprintf(stderr, "Failed to create texture for %s: %s\n", filePaths[index], SDL_GetError());
            SDL_FreeSurface(surface);
            return false;
        }

        textureState.width = surface->w;
        textureState.height = surface->h;
        SDL_FreeSurface(surface);

        // We assume that textures is already initialized
        da_append(textures, textureState);
    }
    return true;
}

bool initializeClickableArea(AppEvents* appEvents, size_t numClickableAreas) {
    if (appEvents->clickableAreas.data != NULL) return true; // clickableAreas is already initialized

    appEvents->clickableAreas.capacity = numClickableAreas;
    appEvents->clickableAreas.data = malloc(appEvents->clickableAreas.capacity * sizeof(ClickableAreas));
    return appEvents->clickableAreas.data != NULL;
}
