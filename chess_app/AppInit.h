#ifndef BC27FC8A_BD51_4933_9D9F_CD61A2488254
#define BC27FC8A_BD51_4933_9D9F_CD61A2488254

#include <stdbool.h>

#include "AppState.h"

bool initializeApp(SDL_State *sdlState, 
    const char* title, int width, int height, 
    const char* fontPath, int fontSize, 
    const char* svgBasePath, ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE]);

void cleanupApp(SDL_State *sldState, ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE], GameState *gameState, ClickableAreas *clickableAreas);

#endif /* BC27FC8A_BD51_4933_9D9F_CD61A2488254 */
