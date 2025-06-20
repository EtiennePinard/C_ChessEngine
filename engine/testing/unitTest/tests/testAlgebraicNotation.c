#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../src/utils/AlgebraicNotation.h"
#include "../../../src/utils/Math.h"
#include "../../../src/magicBitBoard/MagicBitBoard.h"
#include "../../../src/utils/FenString.h"

#include "TestAlgebraicNotation.h"

// This is because we don't use the repetition table so we need
// RepetitionTable_storeKey to be define for the MoveHandler_playMove 
// function to compile
void RepetitionTable_storeKey(void) {}

bool test_moveToAlgebraic(const char* fen, Move move, const char* expected) {
    ChessPosition pos;

    if (!FenString_setChessPositionFromCopiedFenString(fen, &pos)) {
        fprintf(stderr, "Could not set chess position from fen string `%s`\n", fen);
        exit(EXIT_FAILURE);
    }
    char buffer[32];
    moveToStandardAlgebraic(pos, move, buffer);

    if (strcmp(buffer, expected) != 0) {
        printf("test_moveToAlgebraic failed with test case: moveToStandardAlgebraic(pos, %c%d%c%d, buffer)\n",
            'a' + file(Move_fromSquare(move)),
            8 - rank(Move_fromSquare(move)),
            'a' + file(Move_toSquare(move)),
            8 - rank(Move_toSquare(move)));
        printf("\tExpected: %s\n", expected);
        printf("\tActual: %s\n", buffer);
        return false;
    }
    return true;
}

bool Test_AlgebraicNotation() {
    MagicBitBoard_init();

    if (!test_moveToAlgebraic("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        Move_makeMove(E7, E5, DOUBLE_PAWN_PUSH), "e5")) return false;

    if (!test_moveToAlgebraic("rnbqkbnr/pppppppp/8/8/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 1",
        Move_makeMove(G8, F6, NO_FLAG), "Nf6")) return false;

    if (!test_moveToAlgebraic("rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 3",
        Move_makeMove(F3, E5, NO_FLAG), "Nxe5")) return false;

    if (!test_moveToAlgebraic("8/5k2/8/8/8/8/4P3/4K2R w K - 0 1",
        Move_makeMove(E1, G1, KING_SIDE_CASTLING), "O-O+")) return false;

    if (!test_moveToAlgebraic("1k6/8/8/8/8/8/4P3/R3K2R w KQ - 0 1",
        Move_makeMove(E1, C1, QUEEN_SIDE_CASTLING), "O-O-O")) return false;

    if (!test_moveToAlgebraic("8/2k1P3/4K3/8/8/8/8/8 w - - 2 9",
        Move_makeMove(E7, E8, PROMOTE_TO_ROOK), "e8R")) return false;

    if (!test_moveToAlgebraic("rnbqkbnr/ppp2ppp/8/3pP3/3p4/8/PPP2PPP/RNBQKBNR w KQkq d6 0 4",
        Move_makeMove(E5, D6, EN_PASSANT), "exd6")) return false;

    if (!test_moveToAlgebraic("rnb1kbnr/pppp1ppp/8/4N1q1/4PP2/8/PPPP2PP/RNBQKB1R b KQkq - 0 4",
        Move_makeMove(G5, H4, NO_FLAG), "Qh4+")) return false;

    if (!test_moveToAlgebraic("7K/8/8/8/4Q2Q/8/8/k3n2Q w - - 0 1",
        Move_makeMove(H4, E1, NO_FLAG), "Qh4xe1+")) return false;

    if (!test_moveToAlgebraic("7K/8/8/8/3B4/2R5/8/k7 w - - 0 1",
        Move_makeMove(C3, C1, NO_FLAG), "Rc1++")) return false;

    if (!test_moveToAlgebraic("7K/8/8/8/2QB4/2R5/8/k7 w - - 0 1",
        Move_makeMove(C4, B3, NO_FLAG), "Qb3=")) return false;

    if (!test_moveToAlgebraic("k3n3/rp3P2/8/8/8/8/8/K7 w - - 0 1",
        Move_makeMove(F7, E8, PROMOTE_TO_ROOK), "fxe8R#")) return false;

    MagicBitBoard_terminate();
    return true;
}
