#ifndef FEN_STRING_H
#define FEN_STRING_H

#include <stdbool.h>
#include "../state/GameState.h"

bool setGameStateFromFenString(char* fenString, GameState* result);

#endif