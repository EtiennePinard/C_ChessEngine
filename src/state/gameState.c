#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "GameState.h"
#include "../utils/FenString.h"

ChessGame* setupChesGame(ChessGame* result, char* fenString) {
    if (result == NULL) {
        result = (ChessGame*) malloc(sizeof(ChessGame));
        assert(result != NULL && "Buy more RAM lol");
    }

    ChessPosition* currentPosition = (ChessPosition*) malloc(sizeof(ChessPosition));
    assert(currentPosition != NULL && "Buy more RAM lol");
    if (!setChessPositionFromFenString(fenString, currentPosition)) {
        printf("Invalid fen string %s\n", fenString);
        return NULL;
    }
    result->currentState = currentPosition;

    result->previousStatesCapacity = 256;
    result->previousStatesCount = 0;
    result->previousStates = malloc(sizeof(ZobristKey) * result->previousStatesCapacity);
    assert(result->previousStates != NULL && "Buy more RAM lol");

    return result;
}

void freeChessGame(ChessGame* chessGame) {
    free(chessGame->currentState);
    free(chessGame->previousStates);
    free(chessGame);
}