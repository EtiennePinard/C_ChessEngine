#ifndef CHESS_GAME_EMLATOR_H
#define CHESS_GAME_EMLATOR_H

#include "state/GameState.h"
#include "state/Move.h"
/**
 * Makes the move and updates the ChessGame
 * The move is assumed to be legal
*/
void playMove(Move move, ChessGame* game);

#endif