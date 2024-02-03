#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "GameState.h"
#include "../utils/FenString.h"

ChessGame* newChessGame(ChessGame* result, char* fenString) {
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

void zobristKeyInitialize() {
    srand(time(NULL));
    zobristRandomNumber = malloc(sizeof(ZobristRandomNumber));
    assert(zobristRandomNumber != NULL && "Buy more RAM lol");

    for (int color = 0; color < 2; color++) {
        PieceCharacteristics pieceColor = WHITE * (color + 1);
        // Piece order influenced
        for (PieceCharacteristics piece = PAWN; piece <= KING; piece++) {
            Piece index = makePiece(pieceColor, piece) - 9;
            for (int square = 0; square < BOARD_SIZE; square++) {
                zobristRandomNumber->pieces[(int) index][square] = random_u64();
            }
        }
    }
    for (int i = 0; i < 16; i++) {
        zobristRandomNumber->castlingPerms[i] = random_u64();
    }
    for (int i = 0; i < 9; i++) {
        zobristRandomNumber->enPassantFile[i] = i == 0 ? (u64) 0 : random_u64();
    }
    zobristRandomNumber->sideToMove = random_u64();
}

void zobristKeyTerminate() {
    free(zobristRandomNumber);
}

// Only use this function to set the initial Zobrist key
// The subsequent keys are computed incrementally by the makeMove function
void calculateZobristKey(ChessPosition* state) {
    u64 zobristKey = (u64) 0;

    for (int square = 0; square < BOARD_SIZE; square++) {
        Piece piece = pieceAtIndex(state->board, square);
        if (piece != NOPIECE) {
            zobristKey ^= zobristRandomNumber->pieces[(int) piece - 9][square];
        }
    }

    zobristKey ^= zobristRandomNumber->enPassantFile[state->enPassantTargetSquare];

    if (state->colorToGo == WHITE) {
        zobristKey ^= zobristRandomNumber->sideToMove;
    }

    zobristKey ^= zobristRandomNumber->castlingPerms[(int) state->castlingPerm];

    state->key = zobristKey;
}