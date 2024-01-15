#include "Board.h"

Piece pieceAtIndex(Board board, int index) {

    Piece K = ((board.bitboards[0] >> index) & 1UL) * makePiece(WHITE, KING);
    Piece N = ((board.bitboards[1] >> index) & 1UL) * makePiece(WHITE, KNIGHT);
    Piece B = ((board.bitboards[2] >> index) & 1UL) * makePiece(WHITE, BISHOP);
    Piece Q = ((board.bitboards[3] >> index) & 1UL) * makePiece(WHITE, QUEEN);
    Piece R = ((board.bitboards[4] >> index) & 1UL) * makePiece(WHITE, ROOK);
    Piece P = ((board.bitboards[5] >> index) & 1UL) * makePiece(WHITE, PAWN);

    Piece k = ((board.bitboards[8 ] >> index) & 1UL) * makePiece(BLACK, KING);
    Piece n = ((board.bitboards[9 ] >> index) & 1UL) * makePiece(BLACK, KNIGHT);
    Piece b = ((board.bitboards[10] >> index) & 1UL) * makePiece(BLACK, BISHOP);
    Piece q = ((board.bitboards[11] >> index) & 1UL) * makePiece(BLACK, QUEEN);
    Piece r = ((board.bitboards[12] >> index) & 1UL) * makePiece(BLACK, ROOK);
    Piece p = ((board.bitboards[13] >> index) & 1UL) * makePiece(BLACK, PAWN);

    return (Piece) (K + Q + N + B + R + P +
                    k + q + n + b + r + p);
}

u64 bitBoardForPiece(Board board, Piece piece) {
    int arrayIndex = piece - 9; // The best hash function there is!
    return board.bitboards[arrayIndex];
}

u64 whitePiecesBitBoard(Board board) {
    return 
    board.bitboards[0] | 
    board.bitboards[1] |
    board.bitboards[2] | 
    board.bitboards[3] |
    board.bitboards[4] | 
    board.bitboards[5];
}

u64 blackPiecesBitBoard(Board board) {
    return 
    board.bitboards[8 ] | 
    board.bitboards[9 ] |
    board.bitboards[10] | 
    board.bitboards[11] |
    board.bitboards[12] | 
    board.bitboards[13];
}

u64 allPiecesBitBoard(Board board) {
    return whitePiecesBitBoard(board) | blackPiecesBitBoard(board);
}

void togglePieceAtIndex(Board* board, int index, Piece piece) {
    u64 toggle = (u64) 1; // I need to do this else the shift overflows the 32 bits of int
    toggle <<= index;

    int arrayIndex = piece - 9; // The best hash function there is!
    board->bitboards[arrayIndex] ^= toggle;
}

void handleMove(Board* board, int from, int to) {
    int pieceToMove = pieceAtIndex(*board, from);

    togglePieceAtIndex(board, from, pieceToMove);

    Piece capturePiece = pieceAtIndex(*board, to);
    if (capturePiece != NOPIECE) {
        togglePieceAtIndex(board, to, capturePiece);
    }

    togglePieceAtIndex(board, to, pieceToMove);
}

void fromArray(Board* result, Piece array[BOARD_SIZE]) {
    for (int index = 0; index < BOARD_SIZE; index++) {
        Piece piece = array[index];
        if (piece != NOPIECE) {
            togglePieceAtIndex(result, index, piece);
        }
    }
}