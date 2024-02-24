#ifndef FC7C9618_1157_4392_921D_5EA7262D829E
#define FC7C9618_1157_4392_921D_5EA7262D829E

#include "../utils/Utils.h"
#include "../state/Piece.h"

#define MIDGAME 0
#define ENGGAME 1
#define NB_PHASE 2

// This packs the midgame and endgame into one integer
// This packing allows us to operate on both of them simultaneously
#define S(midGame, endGame) (int) (( ((uint16_t) (endGame)) << 16) | ((uint16_t) (midGame)) )

#define mg_value(score) ((int16_t) (score & 0xFFFF))

#define eg_value(score) ((int16_t) (((uint32_t) (score + 0x8000)) >> 16))
// The piece square is gotten from https://github.com/official-stockfish/Stockfish/blob/sf_14/src/psqt.cpp
extern int pieceSquareTable[MAX_PIECE_INDEX - 9][BOARD_SIZE];

void pieceSquareTableInitialize();

#endif /* FC7C9618_1157_4392_921D_5EA7262D829E */
