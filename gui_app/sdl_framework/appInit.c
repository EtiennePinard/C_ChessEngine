#include <SDL3_image/SDL_image.h>

#include "AppInit.h"

bool initializeSDlLibraries(u32 sdlFlags) {
    if (!SDL_Init(sdlFlags)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

    if (!TTF_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_Init Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    return true;
}

bool initializeSDLState(SDL_State* sdlState,
    const char* windowTitle, int windowX, int windowY, int windowWidth, int windowHeight, u32 windowFlags,
    const char* rendererName,
    const char* fontPath, float fontSize) {
    sdlState->window = SDL_CreateWindow(windowTitle, windowWidth, windowHeight, windowFlags);
    SDL_SetWindowPosition(sdlState->window, windowX, windowY);
    if (sdlState->window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    sdlState->renderer = SDL_CreateRenderer(sdlState->window, rendererName);
    if (sdlState->renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdlState->window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    sdlState->font = TTF_OpenFont(fontPath, fontSize);
    if (sdlState->font == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_OpenFont Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(sdlState->renderer);
        SDL_DestroyWindow(sdlState->window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    return true;
}

bool initializeTextures(Textures* textures, size_t initialCapacity) {
    if (textures->data != NULL) return true; // textures is already initialized

    textures->capacity = initialCapacity;
    textures->data = SDL_calloc(textures->capacity, sizeof(TextureState));
    return textures->data != NULL;
}

bool loadImageFromFilePath(SDL_State* sdlState, Textures* textures, const char** filePaths, size_t nbImages) {
    for (size_t index = 0; index < nbImages; index++) {
        SDL_IOStream* ioStream = SDL_IOFromFile(filePaths[index], "rb");
        SDL_Surface* surface = IMG_Load_IO(ioStream, 1);
        if (surface == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image %s: %s\n", filePaths[index], SDL_GetError());
            return false;
        }

        TextureState textureState = { 0 };

        textureState.texture = SDL_CreateTextureFromSurface(sdlState->renderer, surface);
        if (textureState.texture == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create texture for %s: %s\n", filePaths[index], SDL_GetError());
            SDL_DestroySurface(surface);
            return false;
        }

        textureState.width = surface->w;
        textureState.height = surface->h;
        SDL_DestroySurface(surface);

        // We assume that textures is already initialized
        da_append(textures, textureState);
    }
    return true;
}
