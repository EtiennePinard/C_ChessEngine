#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../../src/state/Board.h"

bool test_Board_bitBoardForPiece() {
    Board board = { 0 };
    board.bitboards[0] = 0xFF; // White Pawns at rank 2

    BitBoard expected = 0xFF;
    BitBoard actual = Board_bitBoardForPiece(board, Piece_makePiece(WHITE, PAWN));

    if (expected != actual) {
        printf("test_Board_bitBoardForPiece failed with test case: Board_bitBoardForPiece(board, Piece_Piece_makePiece(WHITE, PAWN))\n");
        printf("\tExpected: 0x%lx\n", expected);
        printf("\tActual: 0x%lx\n", actual);
        return false;
    }
    return true;
}

bool test_Board_specificColorBitBoard() {
    Board board = { 0 };
    board.bitboards[0] = 0xFF; // White Pawns
    board.bitboards[1] = 0x42; // White Knights

    BitBoard expected = 0xFF | 0x42;
    BitBoard actual = Board_specificColorBitBoard(board, WHITE);

    if (expected != actual) {
        printf("test_Board_specificColorBitBoard failed with test case: Board_specificColorBitBoard(board, WHITE)\n");
        printf("\tExpected: 0x%lx\n", expected);
        printf("\tActual: 0x%lx\n", actual);
        return false;
    }
    return true;
}

bool test_Board_allPiecesBitBoard() {
    Board board = {0};
    board.bitboards[0] = 0xFF; // White Pawns
    board.bitboards[8] = 0x00FF000000000000; // Black Pawns

    BitBoard expected = 0xFF | 0x00FF000000000000;
    BitBoard actual = Board_allPiecesBitBoard(board);

    if (expected != actual) {
        printf("test_Board_allPiecesBitBoard failed with test case: Board_allPiecesBitBoard(board)\n");
        printf("\tExpected: 0x%lx\n", expected);
        printf("\tActual: 0x%lx\n", actual);
        return false;
    }
    return true;
}

bool test_Board_pieceAtIndex() {
    Board board = {0};
    board.bitboards[0] = 1ULL << 8; // White Pawn at a7

    Piece expected = Piece_makePiece(WHITE, PAWN);
    Piece actual = Board_pieceAtIndex(board, 8);

    if (expected != actual) {
        printf("test_Board_pieceAtIndex failed with test case: Board_pieceAtIndex(board, 8)\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }
    return true;
}

bool test_Board_togglePieceAtIndex() {
    Board board = { 0 };
    Piece piece = Piece_makePiece(WHITE, KNIGHT);
    int index = 57;

    Board_togglePieceAtIndex(&board, index, piece);

    if (!(board.bitboards[1] & (1ULL << index))) {
        printf("test_Board_togglePieceAtIndex failed with test case: Board_togglePieceAtIndex(&board, 57, Piece_makePiece(WHITE, KNIGHT))\n");
        printf("\tExpected: Bit at 57 to be toggled\n");
        printf("\tActual: Bit at 57 not toggled\n");
        return false;
    }

    // Toggle it again, should be zero
    Board_togglePieceAtIndex(&board, index, piece);

    if (board.bitboards[1] & (1ULL << index)) {
        printf("test_Board_togglePieceAtIndex failed with test case: Board_togglePieceAtIndex(&board, 57, Piece_makePiece(WHITE, KNIGHT))\n");
        printf("\tExpected: Bit at 57 to be toggled off\n");
        printf("\tActual: Bit at 57 still toggled on\n");
        return false;
    }
    return true;
}

bool test_Board_fromArray() {
    Board board = { 0 };
    Piece boardArray[64] = { 0 };

    boardArray[0] = Piece_makePiece(WHITE, ROOK);
    boardArray[7] = Piece_makePiece(WHITE, ROOK);
    boardArray[56] = Piece_makePiece(BLACK, ROOK);
    boardArray[63] = Piece_makePiece(BLACK, ROOK);

    Board_fromArray(&board, boardArray);

    if (!(board.bitboards[3] & (1ULL << 0)) || !(board.bitboards[3] & (1ULL << 7))) {
        printf("test_Board_fromArray failed with test case: Board_fromArray(&board, boardArray)\n");
        printf("\tExpected: White rooks at 0 and 7\n");
        printf("\tActual: White rooks not set correctly\n");
        return false;
    }

    if (!(board.bitboards[11] & (1ULL << 56)) || !(board.bitboards[11] & (1ULL << 63))) {
        printf("test_Board_fromArray failed with test case: Board_fromArray(&board, boardArray)\n");
        printf("\tExpected: Black rooks at 56 and 63\n");
        printf("\tActual: Black rooks not set correctly\n");
        return false;
    }
    return true;
}

bool Test_Board() {
    
    if (!test_Board_bitBoardForPiece()) return false;
    if (!test_Board_specificColorBitBoard()) return false;
    if (!test_Board_allPiecesBitBoard()) return false;
    if (!test_Board_pieceAtIndex()) return false;
    if (!test_Board_togglePieceAtIndex()) return false;
    if (!test_Board_fromArray()) return false;
    
    return true;
}
