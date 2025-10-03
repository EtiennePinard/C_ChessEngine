#ifndef E6E0F8BA_CE2C_4405_9A57_37FF9156930D
#define E6E0F8BA_CE2C_4405_9A57_37FF9156930D

#include "../GameModals.h"

SDL_AppResult closedSettingsModal(SDL_Event* event, App* app);
SDL_AppResult cancelSettingsModal(SDL_Event* event, App* app);

SDL_AppResult clickedSettingsModal(SDL_Event* event, SDL_FRect rect, App* app);
SDL_AppResult hoveredSettingsModal(SDL_Event* event, SDL_FRect rect, App* app);

#endif /* E6E0F8BA_CE2C_4405_9A57_37FF9156930D */
