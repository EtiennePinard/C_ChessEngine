#ifndef FEN_STRING_H
#define FEN_STRING_H

#include "../state/GameState.h"

GameState* setGameStateFromFenString(char* fenString, GameState* result);

#endif