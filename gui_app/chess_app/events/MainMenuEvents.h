#ifndef C61E4442_9787_481D_A215_AC1494D87CFD
#define C61E4442_9787_481D_A215_AC1494D87CFD

#include "../AppState.h"

SDL_AppResult clickedWhiteEngineConfig(SDL_Event* event, SDL_FRect rect, App *app);
SDL_AppResult clickedBlackEngineConfig(SDL_Event* event, SDL_FRect rect, App *app);

SDL_AppResult clickedWhiteTimeControl(SDL_Event* event, SDL_FRect rect, App* app);
SDL_AppResult clickedBlackTimeControl(SDL_Event* event, SDL_FRect rect, App* app);

SDL_AppResult clickedStartingPosition(SDL_Event* event, SDL_FRect rect, App* app);
SDL_AppResult enteredStartingPosition(SDL_Event* event, SDL_FRect rect, App* app);
SDL_AppResult exitedStartingPosition(SDL_Event* event, SDL_FRect rect, App* app);

SDL_AppResult clickedStartGame(SDL_Event* event, SDL_FRect rect, App* app);

#endif /* C61E4442_9787_481D_A215_AC1494D87CFD */
