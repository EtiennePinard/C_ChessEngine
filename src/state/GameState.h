#ifndef BDF83061_7504_461E_BCDD_602085692048
#define BDF83061_7504_461E_BCDD_602085692048

#include "Board.h"
typedef u64 ZobristKey;

/**
 * @brief Reperents a chess position. This structs holds the same information that is in a fen string
 * with the Zobrist key of the position.
 */
typedef struct ChessPosition {
    Board board;
    PieceCharacteristics colorToGo;
    char castlingPerm; // 4 bits are used. The first bit is for white king side, second bit is for white queen side and pattern continues but for black
    int enPassantTargetSquare; // enPassantTargetSquare is 0 when there is no pawn that has double pushed
    int turnsForFiftyRule; 
    int nbMoves;
    ZobristKey key;
} ChessPosition;

#endif /* BDF83061_7504_461E_BCDD_602085692048 */
