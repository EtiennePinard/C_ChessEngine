#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "GameState.h"
#include "../utils/FenString.h"

bool Game_setupChesGame(ChessGame *result,
                   ChessPosition *currentPosition,
                   const char *fenString,
                   u32 whiteTimeInMs, u32 blackTimeInMs) {
    assert(result != NULL && "Result pointer is NULL");
    assert(fenString != NULL && "Fen string is NULL");

    if (!FenString_setChessPositionFromFenString(fenString, currentPosition)) {
        printf("Invalid fen string %s\n", fenString);
        return false;
    }
    
    result->whiteTimeMs = whiteTimeInMs;
    result->blackTimeMs = blackTimeInMs;
    
    result->currentPosition = *currentPosition;
    result->previousPositionsCount = 0;
    return true;
}

bool Game_isThereThreeFoldRepetition(ChessGame* game) {
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