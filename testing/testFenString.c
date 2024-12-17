#include "../src/utils/FenString.h"
#include "../src/state/GameState.h"
#include "../src/state/Board.h"
#include "LogChessStructs.h"


#include <assert.h>
#include <stdio.h>

// Starting position
#define TEST_1 ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")

bool test1(ChessPosition *position) {
    if (position->colorToGo != WHITE) { return false; }
    if (position->castlingPerm != (0b1111)) { return false; }
    if (position->turnsForFiftyRule != 0) { return false; }
    if (position->nbMoves != 1) { return false; }

    if (pieceAtIndex(position->board, 0) != makePiece(BLACK, ROOK)) { return false; }
    if (pieceAtIndex(position->board, 1) != makePiece(BLACK, KNIGHT)) { return false; }
    if (pieceAtIndex(position->board, 2) != makePiece(BLACK, BISHOP)) { return false; }
    if (pieceAtIndex(position->board, 3) != makePiece(BLACK, QUEEN)) { return false; }
    if (pieceAtIndex(position->board, 4) != makePiece(BLACK, KING)) { return false; }
    if (pieceAtIndex(position->board, 5) != makePiece(BLACK, BISHOP)) { return false; }
    if (pieceAtIndex(position->board, 6) != makePiece(BLACK, KNIGHT)) { return false; }
    if (pieceAtIndex(position->board, 7) != makePiece(BLACK, ROOK)) { return false; }

    for (int index = 8; index < 16; index++) {
        if (pieceAtIndex(position->board, index) != makePiece(BLACK, PAWN)) { return false; }
    }

    for (int index = 16; index < 48; index++) {
        if (pieceAtIndex(position->board, index) != NOPIECE) { return false; }
    }

    for (int index = 48; index < 56; index++) {
        if (pieceAtIndex(position->board, index) != makePiece(WHITE, PAWN)) { return false; }
    }

    if (pieceAtIndex(position->board, 56) != makePiece(WHITE, ROOK)) { return false; }
    if (pieceAtIndex(position->board, 57) != makePiece(WHITE, KNIGHT)) { return false; }
    if (pieceAtIndex(position->board, 58) != makePiece(WHITE, BISHOP)) { return false; }
    if (pieceAtIndex(position->board, 59) != makePiece(WHITE, QUEEN)) { return false; }
    if (pieceAtIndex(position->board, 60) != makePiece(WHITE, KING)) { return false; }
    if (pieceAtIndex(position->board, 61) != makePiece(WHITE, BISHOP)) { return false; }
    if (pieceAtIndex(position->board, 62) != makePiece(WHITE, KNIGHT)) { return false; }
    if (pieceAtIndex(position->board, 63) != makePiece(WHITE, ROOK)) { return false; }

    return true;
}

#define TEST_2 ("r2qkb1r/3b1ppp/p1n1pn2/3p4/1p1P4/P1NBPN2/1P3PPP/R1BQ1RK1 w kq - 0 1")

bool test2(ChessPosition *position) {
    if (position->colorToGo != WHITE) { return false; }
    if (position->castlingPerm != (0b11)) { return false; }
    if (position->enPassantTargetSquare != 0) { return false; }
    if (position->turnsForFiftyRule != 0) { return false; }
    if (position->nbMoves != 1) { return false; }

    if (pieceAtIndex(position->board, 0) != makePiece(BLACK, ROOK)) { return false; }
    if (pieceAtIndex(position->board, 1) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 2) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 3) != makePiece(BLACK, QUEEN)) { return false; }
    if (pieceAtIndex(position->board, 4) != makePiece(BLACK, KING)) { return false; }
    if (pieceAtIndex(position->board, 5) != makePiece(BLACK, BISHOP)) { return false; }
    if (pieceAtIndex(position->board, 6) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 7) != makePiece(BLACK, ROOK)) { return false; }

    if (pieceAtIndex(position->board, 8) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 9) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 10) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 11) != makePiece(BLACK, BISHOP)) { return false; }
    if (pieceAtIndex(position->board, 12) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 13) != makePiece(BLACK, PAWN)) { return false; }
    if (pieceAtIndex(position->board, 14) != makePiece(BLACK, PAWN)) { return false; }
    if (pieceAtIndex(position->board, 15) != makePiece(BLACK, PAWN)) { return false; }

    if (pieceAtIndex(position->board, 16) != makePiece(BLACK, PAWN)) { return false; }
    if (pieceAtIndex(position->board, 17) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 18) != makePiece(BLACK, KNIGHT)) { return false; }
    if (pieceAtIndex(position->board, 19) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 20) != makePiece(BLACK, PAWN)) { return false; }
    if (pieceAtIndex(position->board, 21) != makePiece(BLACK, KNIGHT)) { return false; }
    if (pieceAtIndex(position->board, 22) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 23) != NOPIECE) { return false; }

    if (pieceAtIndex(position->board, 24) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 25) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 26) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 27) != makePiece(BLACK, PAWN)) { return false; }
    if (pieceAtIndex(position->board, 28) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 29) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 30) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 31) != NOPIECE) { return false; }

    if (pieceAtIndex(position->board, 32) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 33) != makePiece(BLACK, PAWN)) { return false; }
    if (pieceAtIndex(position->board, 34) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 35) != makePiece(WHITE, PAWN)) { return false; }
    if (pieceAtIndex(position->board, 36) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 37) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 38) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 39) != NOPIECE) { return false; }

    if (pieceAtIndex(position->board, 40) != makePiece(WHITE, PAWN)) { return false; }
    if (pieceAtIndex(position->board, 41) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 42) != makePiece(WHITE, KNIGHT)) { return false; }
    if (pieceAtIndex(position->board, 43) != makePiece(WHITE, BISHOP)) { return false; }
    if (pieceAtIndex(position->board, 44) != makePiece(WHITE, PAWN)) { return false; }
    if (pieceAtIndex(position->board, 45) != makePiece(WHITE, KNIGHT)) { return false; }
    if (pieceAtIndex(position->board, 46) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 47) != NOPIECE) { return false; }

    if (pieceAtIndex(position->board, 48) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 49) != makePiece(WHITE, PAWN)) { return false; }
    if (pieceAtIndex(position->board, 50) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 51) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 52) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 53) != makePiece(WHITE, PAWN)) { return false; }
    if (pieceAtIndex(position->board, 54) != makePiece(WHITE, PAWN)) { return false; }
    if (pieceAtIndex(position->board, 55) != makePiece(WHITE, PAWN)) { return false; }

    if (pieceAtIndex(position->board, 56) != makePiece(WHITE, ROOK)) { return false; }
    if (pieceAtIndex(position->board, 57) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 58) != makePiece(WHITE, BISHOP)) { return false; }
    if (pieceAtIndex(position->board, 59) != makePiece(WHITE, QUEEN)) { return false; }
    if (pieceAtIndex(position->board, 60) != NOPIECE) { return false; }
    if (pieceAtIndex(position->board, 61) != makePiece(WHITE, ROOK)) { return false; }
    if (pieceAtIndex(position->board, 62) != makePiece(WHITE, KING)) { return false; }
    if (pieceAtIndex(position->board, 63) != NOPIECE) { return false; }

    return true;
}

#define TEST3 ("")

bool test3(ChessPosition *position) {
    // This is unreachable
    return false;
}

#define TEST4 ("8/8/8/8/8/8/8/8 b - - 338749012 00001341293847")

bool test4(ChessPosition *position) {
    if (position->colorToGo != BLACK) { return false; }
    if (position->castlingPerm != (0)) { return false; }
    if (position->enPassantTargetSquare != 0) { return false; }
    if (position->turnsForFiftyRule != 338749012) { return false; }
    if (position->nbMoves != 1341293847) { return false; }

    for (int square = 0; square < 64; square++) {
        if (pieceAtIndex(position->board, square) != NOPIECE) { return false; }
    }

    return true;
}

#define TEST5 ("8/8/8/8/8/8/8/8 t")

bool test5(ChessPosition *position) {
    // This is unreachable
    return false;
}

#define TEST6 ("8/8/8/8/8/8/8/8 b - f7 0 1")

bool test6(ChessPosition *position) {
    if (position->colorToGo != BLACK) { return false; }
    if (position->castlingPerm != (0)) { return false; }
    if (position->enPassantTargetSquare != 13) { return false; }
    if (position->turnsForFiftyRule != 0) { return false; }
    if (position->nbMoves != 1) { return false; }

    for (int square = 0; square < 64; square++) {
        if (pieceAtIndex(position->board, square) != NOPIECE) { return false; }
    }

    return true;
}

typedef struct TestCase {
    char *fen;
    bool (*test_func)(ChessPosition*);
    bool setPositionReturnValue;
} TestCase;

#define NB_TEST (6)
TestCase tests[NB_TEST] = {
    (TestCase) {
        .fen = TEST_1,
        .test_func = &test1,
        .setPositionReturnValue = true
    }, 
    (TestCase) {
        .fen = TEST_2,
        .test_func = &test2,
        .setPositionReturnValue = true
    },
    (TestCase) {
        .fen = TEST3,
        .test_func = &test3,
        .setPositionReturnValue = false
    }, 
    (TestCase) {
        .fen = TEST4,
        .test_func = &test4,
        .setPositionReturnValue = true
    },     
    (TestCase) {
        .fen = TEST5,
        .test_func = &test5,
        .setPositionReturnValue = false
    }, 
    (TestCase) {
        .fen = TEST6,
        .test_func = &test6,
        .setPositionReturnValue = true
    },
};

// gcc testing/testFenString.c testing/logChessStructs.c src/utils/utils.c src/utils/fenString.c src/utils/charBuffer.c src/state/zobristKey.c src/state/board.c && ./a.out
int main(int argc, char const *argv[]) {

    ChessPosition position = { 0 };
    for (int testIndex = 0; testIndex < NB_TEST; testIndex++) {
        TestCase testCase = tests[testIndex];
        bool returnValue = setChessPositionFromFenString(testCase.fen, &position);

        if (returnValue == false) {
            if (testCase.setPositionReturnValue == returnValue) {
                printf("TEST #%d passed!\n", testIndex + 1);
            } else {
                printf("TEST #%d failed\n", testIndex + 1);
                printf("\tReturn value is false when it should be true\n");
            }
            continue;
        }

        if (testCase.test_func(&position)) {
            printf("TEST #%d passed!\n", testIndex + 1);
        } else {
            printf("TEST #%d failed!\n", testIndex + 1);
            printf("\tColor to go: %s\n", position.colorToGo == WHITE ? "white" : "black");
            printf("\tCastling perm: 0b%d%d%d%d\n", position.castlingPerm >> 3 & 1, position.castlingPerm >> 2 & 1, position.castlingPerm >> 1 & 1, position.castlingPerm & 1);
            printf("\tFifty move rule: %d\n", position.turnsForFiftyRule);
            printf("\tNb moves: %d\n", position.nbMoves);
            printf("Board:\n");
            printBoard(position.board);
        }
    }

    return 0;
}
