#ifndef BDF83061_7504_461E_BCDD_602085692048
#define BDF83061_7504_461E_BCDD_602085692048

#include <stddef.h>
#include "Board.h"

typedef u64 ZobristKey;

typedef struct {
    Board board;
    PieceCharacteristics colorToGo;
    char castlingPerm; // 4 bits are used. The first bit is for white king side, second bit is for white queen side and pattern continues but for black
    int enPassantTargetSquare; 
    int turnsForFiftyRule; 
    int nbMoves;
    ZobristKey key;
} ChessPosition;

/**
 * Represents a game of chess.
 * currentState holds all information of a position (same information that is in a fen string)
 * Zobrist keys are used to quickly compute repetitions
*/
typedef struct {
    ChessPosition* currentState;
    
    ZobristKey* previousStates;
    int previousStatesCount;
    int previousStatesCapacity;
} ChessGame;

/**
 * Setups a chess game from the position specified by the fen string
*/
ChessGame* setupChesGame(ChessGame* result, char* fenString);

/**
 * Correctly frees the chessGame structs. The argument will also be freed
*/
void freeChessGame(ChessGame* chessGame);

#endif /* BDF83061_7504_461E_BCDD_602085692048 */
