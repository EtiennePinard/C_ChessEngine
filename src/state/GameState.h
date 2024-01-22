#ifndef BDF83061_7504_461E_BCDD_602085692048
#define BDF83061_7504_461E_BCDD_602085692048

#include <stddef.h>
#include "Board.h"
#include "Move.h"

typedef struct {
    // Rank is already included in the side to move key
    u64 enPassantFile[9];
    // 16 total possibilities for castling
    u64 castlingPerms[16];
    // 14 is the biggest piece index ((Black pawn piece index - 9) + 1)
    // To access the random number for a piece for a square: pieces[piece - 9][square]
    u64 pieces[14][BOARD_SIZE];
    
    u64 sideToMove;
} ZobristRandomNumber;

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

ZobristRandomNumber* zobristRandomNumber;

ChessGame* newChessGame(ChessGame* result, char* fenString);
void freeChessGame(ChessGame* chessGame);

void zobristKeyInitialize();
void zobristKeyTerminate();

/**
 * This function should be called only once when creating the your initial ChessGame struct
 * Subsequent Zobrist key will be calculated by the makeMove function incrementally
*/
void calculateZobristKey(ChessPosition* state);

#endif /* BDF83061_7504_461E_BCDD_602085692048 */
