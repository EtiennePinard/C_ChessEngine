#ifndef CHESSAI_H
#define CHESSAI_H

#include "moveGenerator.h"

Moves* bestMovesAccordingToComputer(int depth, const GameState *state, GameStates *previousStates);

#endif