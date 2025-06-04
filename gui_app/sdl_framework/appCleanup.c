#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_ttf.h>

#include "AppCleanup.h"

void cleanupTextures(Textures textures) {
    if (textures.data == NULL) return;

    for (size_t index = 0; index < textures.capacity; index++) {
        if (textures.data[index].texture != NULL) {
            SDL_DestroyTexture(textures.data[index].texture);
            textures.data[index].texture = NULL;
        }
    }
}

void cleanupClickableAreas(AppEvents* events) {
    if (events->clickableAreas.data != NULL) free(events->clickableAreas.data);
}

void cleanupSDL_State(SDL_State sdlState) {
    if (sdlState.font != NULL) TTF_CloseFont(sdlState.font);
    if (sdlState.renderer != NULL) SDL_DestroyRenderer(sdlState.renderer);
    if (sdlState.window != NULL) SDL_DestroyWindow(sdlState.window);
}

void quitSDL() {
    TTF_Quit();
    SDL_Quit();
}

