#ifndef BDF83061_7504_461E_BCDD_602085692048
#define BDF83061_7504_461E_BCDD_602085692048

#include "Board.h"
typedef u64 ZobristKey;

/**
 * @brief Represents a chess position. This structs holds the same information that is in a fen string
 * with the Zobrist key of the position.
 */
typedef struct ChessPosition {
    Board board;
    PieceCharacteristics colorToGo;
    u8 castlingPerm; // 4 bits are used. The first bit is for white king side, second bit is for white queen side and pattern continues but for black
    u8 turnsForFiftyRule; // the max this can go in a legal game is 75
    u16 nbMoves; // the max this can go in a legal game is 8849
    Square enPassantTargetSquare; // enPassantTargetSquare is 0 when no pawn has double pushed
    ZobristKey key;
} ChessPosition;

#endif /* BDF83061_7504_461E_BCDD_602085692048 */
