#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../../src/utils/FenString.h"
#include "../../../src/state/ChessPosition.h"
#include "../../../src/state/Board.h"
#include "../../LogChessStructs.h"


// Starting position
#define TEST_1 INITIAL_FEN

bool test1(ChessPosition* position) {
    if (position->colorToGo != WHITE) { return false; }
    if (position->castlingPerm != (0b1111)) { return false; }
    if (position->turnsForFiftyRule != 0) { return false; }
    if (position->nbMoves != 1) { return false; }

    if (Board_pieceAtIndex(position->board, 0) != Piece_makePiece(BLACK, ROOK)) { return false; }
    if (Board_pieceAtIndex(position->board, 1) != Piece_makePiece(BLACK, KNIGHT)) { return false; }
    if (Board_pieceAtIndex(position->board, 2) != Piece_makePiece(BLACK, BISHOP)) { return false; }
    if (Board_pieceAtIndex(position->board, 3) != Piece_makePiece(BLACK, QUEEN)) { return false; }
    if (Board_pieceAtIndex(position->board, 4) != Piece_makePiece(BLACK, KING)) { return false; }
    if (Board_pieceAtIndex(position->board, 5) != Piece_makePiece(BLACK, BISHOP)) { return false; }
    if (Board_pieceAtIndex(position->board, 6) != Piece_makePiece(BLACK, KNIGHT)) { return false; }
    if (Board_pieceAtIndex(position->board, 7) != Piece_makePiece(BLACK, ROOK)) { return false; }

    for (int index = 8; index < 16; index++) {
        if (Board_pieceAtIndex(position->board, index) != Piece_makePiece(BLACK, PAWN)) { return false; }
    }

    for (int index = 16; index < 48; index++) {
        if (Board_pieceAtIndex(position->board, index) != NOPIECE) { return false; }
    }

    for (int index = 48; index < 56; index++) {
        if (Board_pieceAtIndex(position->board, index) != Piece_makePiece(WHITE, PAWN)) { return false; }
    }

    if (Board_pieceAtIndex(position->board, 56) != Piece_makePiece(WHITE, ROOK)) { return false; }
    if (Board_pieceAtIndex(position->board, 57) != Piece_makePiece(WHITE, KNIGHT)) { return false; }
    if (Board_pieceAtIndex(position->board, 58) != Piece_makePiece(WHITE, BISHOP)) { return false; }
    if (Board_pieceAtIndex(position->board, 59) != Piece_makePiece(WHITE, QUEEN)) { return false; }
    if (Board_pieceAtIndex(position->board, 60) != Piece_makePiece(WHITE, KING)) { return false; }
    if (Board_pieceAtIndex(position->board, 61) != Piece_makePiece(WHITE, BISHOP)) { return false; }
    if (Board_pieceAtIndex(position->board, 62) != Piece_makePiece(WHITE, KNIGHT)) { return false; }
    if (Board_pieceAtIndex(position->board, 63) != Piece_makePiece(WHITE, ROOK)) { return false; }

    return true;
}

#define TEST_2 ("r2qkb1r/3b1ppp/p1n1pn2/3p4/1p1P4/P1NBPN2/1P3PPP/R1BQ1RK1 w kq - 0 1")

bool test2(ChessPosition* position) {
    if (position->colorToGo != WHITE) { return false; }
    if (position->castlingPerm != (0b11)) { return false; }
    if (position->enPassantTargetSquare != 0) { return false; }
    if (position->turnsForFiftyRule != 0) { return false; }
    if (position->nbMoves != 1) { return false; }

    if (Board_pieceAtIndex(position->board, 0) != Piece_makePiece(BLACK, ROOK)) { return false; }
    if (Board_pieceAtIndex(position->board, 1) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 2) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 3) != Piece_makePiece(BLACK, QUEEN)) { return false; }
    if (Board_pieceAtIndex(position->board, 4) != Piece_makePiece(BLACK, KING)) { return false; }
    if (Board_pieceAtIndex(position->board, 5) != Piece_makePiece(BLACK, BISHOP)) { return false; }
    if (Board_pieceAtIndex(position->board, 6) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 7) != Piece_makePiece(BLACK, ROOK)) { return false; }

    if (Board_pieceAtIndex(position->board, 8) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 9) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 10) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 11) != Piece_makePiece(BLACK, BISHOP)) { return false; }
    if (Board_pieceAtIndex(position->board, 12) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 13) != Piece_makePiece(BLACK, PAWN)) { return false; }
    if (Board_pieceAtIndex(position->board, 14) != Piece_makePiece(BLACK, PAWN)) { return false; }
    if (Board_pieceAtIndex(position->board, 15) != Piece_makePiece(BLACK, PAWN)) { return false; }

    if (Board_pieceAtIndex(position->board, 16) != Piece_makePiece(BLACK, PAWN)) { return false; }
    if (Board_pieceAtIndex(position->board, 17) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 18) != Piece_makePiece(BLACK, KNIGHT)) { return false; }
    if (Board_pieceAtIndex(position->board, 19) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 20) != Piece_makePiece(BLACK, PAWN)) { return false; }
    if (Board_pieceAtIndex(position->board, 21) != Piece_makePiece(BLACK, KNIGHT)) { return false; }
    if (Board_pieceAtIndex(position->board, 22) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 23) != NOPIECE) { return false; }

    if (Board_pieceAtIndex(position->board, 24) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 25) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 26) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 27) != Piece_makePiece(BLACK, PAWN)) { return false; }
    if (Board_pieceAtIndex(position->board, 28) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 29) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 30) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 31) != NOPIECE) { return false; }

    if (Board_pieceAtIndex(position->board, 32) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 33) != Piece_makePiece(BLACK, PAWN)) { return false; }
    if (Board_pieceAtIndex(position->board, 34) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 35) != Piece_makePiece(WHITE, PAWN)) { return false; }
    if (Board_pieceAtIndex(position->board, 36) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 37) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 38) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 39) != NOPIECE) { return false; }

    if (Board_pieceAtIndex(position->board, 40) != Piece_makePiece(WHITE, PAWN)) { return false; }
    if (Board_pieceAtIndex(position->board, 41) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 42) != Piece_makePiece(WHITE, KNIGHT)) { return false; }
    if (Board_pieceAtIndex(position->board, 43) != Piece_makePiece(WHITE, BISHOP)) { return false; }
    if (Board_pieceAtIndex(position->board, 44) != Piece_makePiece(WHITE, PAWN)) { return false; }
    if (Board_pieceAtIndex(position->board, 45) != Piece_makePiece(WHITE, KNIGHT)) { return false; }
    if (Board_pieceAtIndex(position->board, 46) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 47) != NOPIECE) { return false; }

    if (Board_pieceAtIndex(position->board, 48) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 49) != Piece_makePiece(WHITE, PAWN)) { return false; }
    if (Board_pieceAtIndex(position->board, 50) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 51) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 52) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 53) != Piece_makePiece(WHITE, PAWN)) { return false; }
    if (Board_pieceAtIndex(position->board, 54) != Piece_makePiece(WHITE, PAWN)) { return false; }
    if (Board_pieceAtIndex(position->board, 55) != Piece_makePiece(WHITE, PAWN)) { return false; }

    if (Board_pieceAtIndex(position->board, 56) != Piece_makePiece(WHITE, ROOK)) { return false; }
    if (Board_pieceAtIndex(position->board, 57) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 58) != Piece_makePiece(WHITE, BISHOP)) { return false; }
    if (Board_pieceAtIndex(position->board, 59) != Piece_makePiece(WHITE, QUEEN)) { return false; }
    if (Board_pieceAtIndex(position->board, 60) != NOPIECE) { return false; }
    if (Board_pieceAtIndex(position->board, 61) != Piece_makePiece(WHITE, ROOK)) { return false; }
    if (Board_pieceAtIndex(position->board, 62) != Piece_makePiece(WHITE, KING)) { return false; }
    if (Board_pieceAtIndex(position->board, 63) != NOPIECE) { return false; }

    return true;
}

#define TEST3 ("")

bool test3(__attribute__((unused)) ChessPosition* position) {
    return false;
}

#define TEST4 ("8/8/8/8/8/8/8/8 b - - 51 000008849")

bool test4(ChessPosition* position) {
    if (position->colorToGo != BLACK) { return false; }
    if (position->castlingPerm != (0)) { return false; }
    if (position->enPassantTargetSquare != 0) { return false; }
    if (position->turnsForFiftyRule != 51) { return false; }
    if (position->nbMoves != MAX_NB_LEGAL_MOVES_IN_GAME) { return false; }

    for (int square = 0; square < 64; square++) {
        if (Board_pieceAtIndex(position->board, square) != NOPIECE) { return false; }
    }

    return true;
}

#define TEST5 ("8/8/8/8/8/8/8/8 t")

bool test5(__attribute__((unused)) ChessPosition* position) {
    return false;
}

#define TEST6 ("8/8/8/8/8/8/8/8 b - f7 0 1")

bool test6(ChessPosition* position) {
    if (position->colorToGo != BLACK) { return false; }
    if (position->castlingPerm != (0)) { return false; }
    if (position->enPassantTargetSquare != 13) { return false; }
    if (position->turnsForFiftyRule != 0) { return false; }
    if (position->nbMoves != 1) { return false; }

    for (int square = 0; square < 64; square++) {
        if (Board_pieceAtIndex(position->board, square) != NOPIECE) { return false; }
    }

    return true;
}

typedef struct FenStringTestCase {
    char* fen;
    bool (*test_func)(ChessPosition*);
    bool setPositionReturnValue;
} FenStringTestCase;

#define NB_TEST (6)
FenStringTestCase fenStringTests[NB_TEST] = {
    (FenStringTestCase) {
        .fen = TEST_1,
        .test_func = &test1,
        .setPositionReturnValue = true
    },
    (FenStringTestCase) {
        .fen = TEST_2,
        .test_func = &test2,
        .setPositionReturnValue = true
    },
    (FenStringTestCase) {
        .fen = TEST3,
        .test_func = &test3,
        .setPositionReturnValue = false
    },
    (FenStringTestCase) {
        .fen = TEST4,
        .test_func = &test4,
        .setPositionReturnValue = true
    },
    (FenStringTestCase) {
        .fen = TEST5,
        .test_func = &test5,
        .setPositionReturnValue = false
    },
    (FenStringTestCase) {
        .fen = TEST6,
        .test_func = &test6,
        .setPositionReturnValue = true
    },
};

bool test_FenString_chessPositionToFenString() {
    char actual[MAX_FEN_STRING_SIZE];
    ChessPosition position1 = {
            .key = 0,
            .colorToGo = WHITE,
            .castlingPerm = 0b1111,
            .enPassantTargetSquare = 0,
            .turnsForFiftyRule = 0,
            .nbMoves = 1,
            .board = (Board) {
                    .bitboards = {
                            (BitBoard)0xFF000000000000, // White Pawn
                            (BitBoard)0x4200000000000000, // White Knight
                            (BitBoard)0x2400000000000000, // White Bishop
                            (BitBoard)0x8100000000000000, // White Rook
                            (BitBoard)0x800000000000000, // White Queen
                            (BitBoard)0x1000000000000000, // White King
                            (BitBoard)0x0, // UNUSED
                            (BitBoard)0x0, // UNUSED
                            (BitBoard)0xFF00, // Black Pawn
                            (BitBoard)0x42, // Black Knight
                            (BitBoard)0x24, // Black Bishop
                            (BitBoard)0x81, // Black Rook
                            (BitBoard)0x8, // Black Queen
                            (BitBoard)0x10, // Black King
                    }
            }
    };
    char* expected1 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    int expectedReturnValue = 56;
    int actualReturnValue = FenString_chessPositionToFenString(position1, actual);
    if (!string_compareStrings(expected1, actual) || expectedReturnValue != actualReturnValue) {
        printf("ERROR: FenString_chessPositionToFenString(position1, actual)\n");
        printf("\tExpected: %s\n", expected1);
        printf("\tActual  : %s\n", actual);
        printf("Expected return value: %d\n", expectedReturnValue);
        printf("Actual return value: %d\n", actualReturnValue);
        return false;
    }
    memset(actual, 0, MAX_FEN_STRING_SIZE);

    ChessPosition position2 = {
            .key = 0,
            .colorToGo = WHITE,
            .castlingPerm = 0b0011,
            .enPassantTargetSquare = 0,
            .turnsForFiftyRule = 0,
            .nbMoves = 1,
            .board = (Board) {
                    .bitboards = {
                            (BitBoard)0xE2110800000000, // White Pawn
                            (BitBoard)0x240000000000, // White Knight
                            (BitBoard)0x400080000000000, // White Bishop
                            (BitBoard)0x2100000000000000, // White Rook
                            (BitBoard)0x800000000000000, // White Queen
                            (BitBoard)0x4000000000000000, // White King
                            (BitBoard)0x0, // UNUSED
                            (BitBoard)0x0, // UNUSED
                            (BitBoard)0x20811E000, // Black Pawn
                            (BitBoard)0x240000, // Black Knight
                            (BitBoard)0x820, // Black Bishop
                            (BitBoard)0x81, // Black Rook
                            (BitBoard)0x8, // Black Queen
                            (BitBoard)0x10, // Black King
                    }
            }
    };
    char* expected2 = "r2qkb1r/3b1ppp/p1n1pn2/3p4/1p1P4/P1NBPN2/1P3PPP/R1BQ1RK1 w kq - 0 1";
    expectedReturnValue = 67;
    actualReturnValue = FenString_chessPositionToFenString(position2, actual);
    if (!string_compareStrings(expected2, actual) || expectedReturnValue != actualReturnValue) {
        printf("ERROR: FenString_chessPositionToFenString(position2, actual)\n");
        printf("\tExpected: %s\n", expected2);
        printf("\tActual  : %s\n", actual);
        printf("Expected return value: %d\n", expectedReturnValue);
        printf("Actual return value: %d\n", actualReturnValue);
        return false;
    }
    memset(actual, 0, MAX_FEN_STRING_SIZE);

    ChessPosition position4 = {
            .key = 0,
            .colorToGo = BLACK,
            .castlingPerm = 0b0000,
            .enPassantTargetSquare = 0,
            .turnsForFiftyRule = 49,
            .nbMoves = MAX_NB_LEGAL_MOVES_IN_GAME,
            .board = (Board) {
                    .bitboards = {
                            (BitBoard)0x0, // White Pawn
                            (BitBoard)0x0, // White Knight
                            (BitBoard)0x0, // White Bishop
                            (BitBoard)0x0, // White Rook
                            (BitBoard)0x0, // White Queen
                            (BitBoard)0x0, // White King
                            (BitBoard)0x0, // UNUSED
                            (BitBoard)0x0, // UNUSED
                            (BitBoard)0x0, // Black Pawn
                            (BitBoard)0x0, // Black Knight
                            (BitBoard)0x0, // Black Bishop
                            (BitBoard)0x0, // Black Rook
                            (BitBoard)0x0, // Black Queen
                            (BitBoard)0x0, // Black King
                    }
            }
    };
    char* expected4 = "8/8/8/8/8/8/8/8 b - - 49 8849";
    expectedReturnValue = 29;
    actualReturnValue = FenString_chessPositionToFenString(position4, actual);
    if (!string_compareStrings(expected4, actual) || expectedReturnValue != actualReturnValue) {
        printf("ERROR: FenString_chessPositionToFenString(position4, actual)\n");
        printf("\tExpected: %s\n", expected4);
        printf("\tActual  : %s\n", actual);
        printf("Expected return value: %d\n", expectedReturnValue);
        printf("Actual return value: %d\n", actualReturnValue);
        return false;
    }
    memset(actual, 0, MAX_FEN_STRING_SIZE);

    ChessPosition position6 = {
            .key = 0,
            .colorToGo = BLACK,
            .castlingPerm = 0b0000,
            .enPassantTargetSquare = 13,
            .turnsForFiftyRule = 0,
            .nbMoves = 1,
            .board = (Board) {
                    .bitboards = {
                            (BitBoard)0x0, // White Pawn
                            (BitBoard)0x0, // White Knight
                            (BitBoard)0x0, // White Bishop
                            (BitBoard)0x0, // White Rook
                            (BitBoard)0x0, // White Queen
                            (BitBoard)0x0, // White King
                            (BitBoard)0x0, // UNUSED
                            (BitBoard)0x0, // UNUSED
                            (BitBoard)0x0, // Black Pawn
                            (BitBoard)0x0, // Black Knight
                            (BitBoard)0x0, // Black Bishop
                            (BitBoard)0x0, // Black Rook
                            (BitBoard)0x0, // Black Queen
                            (BitBoard)0x0, // Black King
                    }
            }
    };
    char* expected6 = "8/8/8/8/8/8/8/8 b - f7 0 1";
    expectedReturnValue = 26;
    actualReturnValue = FenString_chessPositionToFenString(position6, actual);
    if (!string_compareStrings(expected6, actual) || expectedReturnValue != actualReturnValue) {
        printf("ERROR: FenString_chessPositionToFenString(position6, actual)\n");
        printf("\tExpected: %s\n", expected6);
        printf("\tActual  : %s\n", actual);
        printf("Expected return value: %d\n", expectedReturnValue);
        printf("Actual return value: %d\n", actualReturnValue);
        return false;
    }
    memset(actual, 0, MAX_FEN_STRING_SIZE);

    return true;
}

bool Test_FenString() {
    // Note: We do not care about the Zobrist keys of the positions so we don't call ZobristKey_init here

    ChessPosition position = { 0 };
    for (int testIndex = 0; testIndex < NB_TEST; testIndex++) {
        FenStringTestCase testCase = fenStringTests[testIndex];

        // We need to copy the strings because we are dealing with string literals which are read only
        bool returnValue = FenString_setChessPositionFromCopiedFenString(testCase.fen, &position);

        if (returnValue == false) {
            if (testCase.setPositionReturnValue != returnValue) {
                printf("TEST #%d failed\n", testIndex + 1);
                printf("\tReturn value is false when it should be true\n");
                return false;
            }
            continue;
        }

        if (!testCase.test_func(&position)) {
            printf("TEST #%d failed!\n", testIndex + 1);
            printf("\tColor to go: %s\n", position.colorToGo == WHITE ? "white" : "black");
            printf("\tCastling perm: 0b%d%d%d%d\n", position.castlingPerm >> 3 & 1, position.castlingPerm >> 2 & 1, position.castlingPerm >> 1 & 1, position.castlingPerm & 1);
            printf("\tFifty move rule: %d\n", position.turnsForFiftyRule);
            printf("\tNb moves: %d\n", position.nbMoves);
            printf("Board:\n");
            printBoard(position.board);
            return false;
        }
    }

    return test_FenString_chessPositionToFenString();
}
