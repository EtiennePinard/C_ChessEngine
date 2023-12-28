#include "Board.h"

// TODO: Iterate through 1 to 6 to get all types of pieces and so simplify your life so much!
Piece pieceAtIndex(Board board, int index) {
    uint64_t mask = (uint64_t) 1; // I need to do this else the shift overflows the 32 bits of int
    mask <<= index;

    Piece K = ((board.bitboards[0] & mask) >> index) * makePiece(WHITE, KING);
    Piece Q = ((board.bitboards[1] & mask) >> index) * makePiece(WHITE, QUEEN);
    Piece N = ((board.bitboards[2] & mask) >> index) * makePiece(WHITE, KNIGHT);
    Piece B = ((board.bitboards[3] & mask) >> index) * makePiece(WHITE, BISHOP);
    Piece R = ((board.bitboards[4] & mask) >> index) * makePiece(WHITE, ROOK);
    Piece P = ((board.bitboards[5] & mask) >> index) * makePiece(WHITE, PAWN);

    Piece k = ((board.bitboards[8] & mask) >> index) * makePiece(BLACK, KING);
    Piece q = ((board.bitboards[9] & mask) >> index) * makePiece(BLACK, QUEEN);
    Piece n = ((board.bitboards[10] & mask) >> index) * makePiece(BLACK, KNIGHT);
    Piece b = ((board.bitboards[11] & mask) >> index) * makePiece(BLACK, BISHOP);
    Piece r = ((board.bitboards[12] & mask) >> index) * makePiece(BLACK, ROOK);
    Piece p = ((board.bitboards[13] & mask) >> index) * makePiece(BLACK, PAWN);

    return (Piece) (K + Q + N + B + R + P +
                    k + q + n + b + r + p);
}

void togglePieceAtIndex(Board* board, int index, Piece piece) {
    uint64_t toggle = (uint64_t) 1;
    toggle <<= index;

    int arrayIndex = piece - 9;
    board->bitboards[arrayIndex] ^= toggle;
    // switch (piece) {

    // case WHITE | KING:
    //     board->Kbb ^= toggle;
    //     break;
    // case WHITE | QUEEN:
    //     board->Qbb ^= toggle;
    //     break;
    // case WHITE | ROOK:
    //     board->Rbb ^= toggle;
    //     break;
    // case WHITE | KNIGHT:
    //     board->Nbb ^= toggle;
    //     break;
    // case WHITE | BISHOP:
    //     board->Bbb ^= toggle;
    //     break;
    // case WHITE | PAWN:
    //     board->Pbb ^= toggle;
    //     break;

    // case BLACK | KING:
    //     board->kbb ^= toggle;
    //     break;
    // case BLACK | QUEEN:
    //     board->qbb ^= toggle;
    //     break;
    // case BLACK | ROOK:
    //     board->rbb ^= toggle;
    //     break;
    // case BLACK | KNIGHT:
    //     board->nbb ^= toggle;
    //     break;
    // case BLACK | BISHOP:
    //     board->bbb ^= toggle;
    //     break;
    // case BLACK | PAWN:
    //     board->pbb ^= toggle;
    //     break;

    // default:
    //     break;
    // }
}

void handleMove(Board* board, int from, int to) {
    int pieceToMove = pieceAtIndex(*board, from);

    togglePieceAtIndex(board, from, pieceToMove);

    Piece capturePiece = pieceAtIndex(*board, to);
    if (capturePiece != NONE) {
        togglePieceAtIndex(board, to, capturePiece);
    }

    togglePieceAtIndex(board, to, pieceToMove);
}

void fromArray(Board* result, Piece array[BOARD_SIZE]) {
    for (int index = 0; index < BOARD_SIZE; index++) {
        Piece piece = array[index];
        if (piece != NONE) {
            togglePieceAtIndex(result, index, piece);
        }
    }
}