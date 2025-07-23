#ifndef CC49158A_EC23_4803_87CF_BB468638877C
#define CC49158A_EC23_4803_87CF_BB468638877C

#include "../AppState.h"

typedef struct TimeControlModalData {
    TimeControl hovered;
    PieceCharacteristics playerColor;
} TimeControlModalData;

typedef struct EngineConfigModalData {
    EngineConfig currentConfig;
    PieceCharacteristics playerColor;

    SDL_FRect checkboxRect;
    SDL_FRect enginePathRect;
    SDL_FRect thinkTimeRect;
    SDL_FRect okButtonRect;
    SDL_FRect cancelButtonRect;
} EngineConfigModalData;

typedef struct MainMenuSceneData {
    GameConfig gameInfo;

    Textures textures;
} MainMenuSceneData;

#endif /* CC49158A_EC23_4803_87CF_BB468638877C */
