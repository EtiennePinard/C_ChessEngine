#ifndef CHESSAI_H
#define CHESSAI_H

#include "moveGenerator.h"

Moves* bestMovesAccordingToComputer(int depth, const GameState *state, GameStates *previousStates);

#define _pawnValue 100
#define _knightValue 300
#define _bishopValue 300
#define _rookValue 500
#define _queenValue 900

#define _mobilityWeight 1 // Mobility is not that important

#define _positiveInfinity 9999999
#define _negativeInfinity -9999999

#define _capturedPieceValueMultiplier 10

#endif