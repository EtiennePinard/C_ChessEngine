#ifndef C61E4442_9787_481D_A215_AC1494D87CFD
#define C61E4442_9787_481D_A215_AC1494D87CFD

#include "../AppState.h"

SDL_AppResult clickedDownWhitePlayerType(SDL_Event* event, SDL_Rect rect, App *app);
SDL_AppResult clickedDownBlackPlayerType(SDL_Event* event, SDL_Rect rect, App *app);

SDL_AppResult clickedDownWhiteEnginePath(SDL_Event* event, SDL_Rect rect, App* app);
SDL_AppResult clickedDownBlackEnginePath(SDL_Event* event, SDL_Rect rect, App* app);

SDL_AppResult clickedDownTimeControlButton(SDL_Event* event, SDL_Rect rect, App* app);
SDL_AppResult clickedDownTimeControlModal(SDL_Event* event, SDL_Rect rect, App* app);

SDL_AppResult clickedUpStartGame(SDL_Event* event, SDL_Rect rect, App* app);

#endif /* C61E4442_9787_481D_A215_AC1494D87CFD */
