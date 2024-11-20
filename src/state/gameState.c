#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "GameState.h"
#include "../utils/FenString.h"

// TODO: Make the previous state not a heap allocated pointer
bool setupChesGame(ChessGame *result, ChessPosition *currentPosition, const char *fenString) {
    assert(result != NULL && "Result pointer is NULL");
    assert(fenString != NULL && "Fen string is NULL");
    
    if (!setChessPositionFromFenString(fenString, currentPosition)) {
        printf("Invalid fen string %s\n", fenString);
        return false;
    }

    result->currentState = *currentPosition;
    result->previousStatesCount = 0;
    return true;
}