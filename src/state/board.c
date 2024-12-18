#include "Board.h"

// TODO: This should be a great place for SIMD, right?
Piece Board_pieceAtIndex(Board board, int index) {

    Piece P = ((board.bitboards[0] >> index) & 1UL) * Piece_makePiece(WHITE, PAWN);
    Piece N = ((board.bitboards[1] >> index) & 1UL) * Piece_makePiece(WHITE, KNIGHT);
    Piece B = ((board.bitboards[2] >> index) & 1UL) * Piece_makePiece(WHITE, BISHOP);
    Piece R = ((board.bitboards[3] >> index) & 1UL) * Piece_makePiece(WHITE, ROOK);
    Piece Q = ((board.bitboards[4] >> index) & 1UL) * Piece_makePiece(WHITE, QUEEN);
    Piece K = ((board.bitboards[5] >> index) & 1UL) * Piece_makePiece(WHITE, KING);

    Piece p = ((board.bitboards[8 ] >> index) & 1UL) * Piece_makePiece(BLACK, PAWN);
    Piece n = ((board.bitboards[9 ] >> index) & 1UL) * Piece_makePiece(BLACK, KNIGHT);
    Piece b = ((board.bitboards[10] >> index) & 1UL) * Piece_makePiece(BLACK, BISHOP);
    Piece r = ((board.bitboards[11] >> index) & 1UL) * Piece_makePiece(BLACK, ROOK);
    Piece q = ((board.bitboards[12] >> index) & 1UL) * Piece_makePiece(BLACK, QUEEN);
    Piece k = ((board.bitboards[13] >> index) & 1UL) * Piece_makePiece(BLACK, KING);

    return (Piece) (K + Q + N + B + R + P +
                    k + q + n + b + r + p);
}

u64 Board_bitBoardForPiece(Board board, Piece piece) {
    int arrayIndex = piece - 9; // The best hash function there is!
    return board.bitboards[arrayIndex];
}

u64 Board_specificColorBitBoard(Board board, PieceCharacteristics color) {
    return 
    board.bitboards[Piece_makePiece(color, PAWN  ) - 9] |
    board.bitboards[Piece_makePiece(color, KNIGHT) - 9] |
    board.bitboards[Piece_makePiece(color, BISHOP) - 9] | 
    board.bitboards[Piece_makePiece(color, ROOK  ) - 9] |
    board.bitboards[Piece_makePiece(color, QUEEN ) - 9] | 
    board.bitboards[Piece_makePiece(color, KING  ) - 9]; 
}

u64 Board_allPiecesBitBoard(Board board) {
    return Board_specificColorBitBoard(board, BLACK) | Board_specificColorBitBoard(board, WHITE);
}

void Board_togglePieceAtIndex(Board* board, int index, Piece piece) {
    u64 toggle = (u64) 1; // I need to do this else the shift overflows the 32 bits of int
    toggle <<= index;

    int arrayIndex = piece - 9; // The best hash function there is!
    board->bitboards[arrayIndex] ^= toggle;
}

void Board_fromArray(Board* result, Piece array[BOARD_SIZE]) {
    for (int index = 0; index < BOARD_SIZE; index++) {
        Piece piece = array[index];
        if (piece != NOPIECE) {
            Board_togglePieceAtIndex(result, index, piece);
        }
    }
}