#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <stddef.h>
#include "state/GameState.h"
#include "state/Move.h"

/**
 * Returns the valid moves in a given position
 * The results array is assumed to be 0 initialized
*/
// TODO: Transfer the previousState from a dynamic to a regular array using the move concept
void getValidMoves(Move results[MAX_LEGAL_MOVES + 1], const GameState currentState, const GameState* previousStates);

#endif