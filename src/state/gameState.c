#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "GameState.h"
#include "../utils/FenString.h"

bool setupChesGame(ChessGame *result, ChessPosition *currentPosition, const char *fenString) {
    assert(result != NULL && "Result pointer is NULL");
    assert(fenString != NULL && "Fen string is NULL");
    
    if (!setChessPositionFromFenString(fenString, currentPosition)) {
        printf("Invalid fen string %s\n", fenString);
        return false;
    }

    result->currentPosition = *currentPosition;
    result->previousPositionsCount = 0;
    return true;
}

bool isThereThreeFoldRepetition(ChessGame* game) {
    bool hasOneDuplicate = false;
    bool result = false;
    for (int i = 0; i < game->previousPositionsCount; i++) {
        if (game->currentPosition.key == game->previousPositions[i]) {
            if (hasOneDuplicate) {
                result = true;
                break;
            }
            hasOneDuplicate = true;
        }
    }
    return result;
}