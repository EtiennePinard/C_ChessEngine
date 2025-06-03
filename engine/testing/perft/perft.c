#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <inttypes.h>

#include "../../src/moveHandler/MoveGenerator.h"
#include "../../src/moveHandler/MovePlayer.h"
#include "../../src/utils/FenString.h"
#include "../../src/magicBitBoard/MagicBitBoard.h"
#include "../../src/state/ZobristKey.h"
#include "../../src/utils/CharBuffer.h"
#include "../../src/utils/Constants.h"

#include "PerftTranspositionTable.h"
#include "../LogChessStructs.h"

// This is because we don't use the repetition table so we need
// RepetitionTable_storeKey to be define for the MoveHandler_playMove 
// function to compile
void RepetitionTable_storeKey(void) {}

#define TEST_ITERATION 100

int maximumDepth;
ChessPosition currentPosition = { 0 };

bool divide = false;
u32 hashHits = 0;

int exitCode = 0;

u64 perft(u8 depth) {
    if (depth == 0) { return 1; }

    int nbOfMoves;
    Move validMoves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    MoveHandler_getValidMoves(validMoves, &nbOfMoves, currentPosition); // We do not care about draw by repetition
    u64 nodes = 0;

    // Note that here we do not take into account the fifty move rule
    // This could make it so that our perft result differ from other engine
    // Thus let us be aware of this potential bug
    if (nbOfMoves == 0) {
        // There is a checkmate or a draw, continuing to next move
        return 0;
    }

    if (depth == 1 && (!divide || maximumDepth > 1)) {
        return nbOfMoves;
    }

    ChessPosition previousPos = currentPosition;

    for (int moveIndex = 0; moveIndex < nbOfMoves; moveIndex++) {

        Move move = validMoves[moveIndex];

        MoveHandler_playMove(move, &currentPosition, false);

        u64 moveOutput = PerftTranspositionTable_getPerftFromKey(currentPosition.key, depth);

        if (moveOutput == LOOKUP_FAILED) {
            moveOutput = perft(depth - 1); // We generate the moves for the next perft

            PerftTranspositionTable_recordPerft((PerftTranspositionTable) {
                .key = currentPosition.key,
                    .depth = depth,
                    .perft = moveOutput
            });
        }
        else {
            hashHits++;
        }

        if (divide && depth == maximumDepth) {
            printMoveToAlgebraic(move);
            printf(": %" PRIu64 "\n", moveOutput);
        }
        nodes += moveOutput;

        currentPosition = previousPos;
    }

    return nodes;
}

/**
 * @brief Represents a position to perform a perft test on.
 * The nbTest parameter indicates for how many depths there is a results
 * perftResults returns the expected number of moves for a depth (the indices of said int)
*/
typedef struct testPosition {
    char* fenString;
    int nbTest;
    int* perftResults;
} TestPosition;

// These are the 8 ANSI color types
#define RED   "\033[31m"
#define GRN   "\033[32m"
#define YEL   "\033[33m"
#define BLU   "\033[34m"
#define MAG   "\033[35m"
#define CYN   "\033[36m"
#define WHT   "\033[37m"
#define RESET "\033[0m"

#define NUM_TEST_POSITIONS (17)

const char* testPassed = GRN ":)" RESET;
const char* testFailedPrefix = RED ":(" RESET " Test failed (expected ";

void test() {
    if (!MagicBitBoard_init() || !ZobristKey_init() || !PerftTranspositionTable_init()) {
        printf("ERROR: Failure to properly initialize, exiting program\n");
        exit(EXIT_FAILURE);
    }

    int startingPosResults[6] = { 1, 20, 400, 8902, 197281, 4865609 };
    TestPosition startingPositionTests = {
      .fenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
      .nbTest = 6,
      .perftResults = startingPosResults
    };

    int pos2Result[5] = { 1, 48, 2039, 97862, 4085603 };
    TestPosition pos2 = {
      .fenString = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
      .nbTest = 5,
      .perftResults = pos2Result
    };

    int pos3Result[7] = { 1, 14, 191, 2812, 43238, 674624, 11030083 };
    TestPosition pos3 = {
      .fenString = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
      .nbTest = 7,
      .perftResults = pos3Result
    };

    int pos4Result[6] = { 1, 6, 264, 9467, 422333, 15833292 };
    TestPosition pos4 = {
      .fenString = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
      .nbTest = 6,
      .perftResults = pos4Result
    };

    int pos5Result[5] = { 1, 44, 1486, 62379, 2103487 };
    TestPosition pos5 = {
      .fenString = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
      .nbTest = 5,
      .perftResults = pos5Result
    };

    int pos6Result[5] = { 1, 46, 2079, 89890, 3894594 };
    TestPosition pos6 = {
      .fenString = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
      .nbTest = 5,
      .perftResults = pos6Result
    };

    int pos7Result[6] = { 1, 6, 156, 2588, 59162, 1187831 };
    TestPosition pos7 = {
        .fenString = "8/pp3ppk/q3p3/P7/8/2PQ4/1P4PP/7K b - - 0 29",
        .nbTest = 6,
        .perftResults = pos7Result
    };

    int pos8Result[7] = { 1, 3, 42, 219, 2680, 16769, 213505 };
    TestPosition pos8 = {
        .fenString = "7k/8/8/4b3/8/8/3P4/K7 w - - 0 1",
        .nbTest = 7,
        .perftResults = pos8Result
    };

    int pos9Result[7] = { 1, 6, 95, 564, 9330, 56825, 977910 };
    TestPosition pos9 = {
        .fenString = "2r4k/8/8/8/3p4/8/2P5/2K5 w - - 0 1",
        .nbTest = 7,
        .perftResults = pos9Result
    };

    int pos10Result[7] = { 1, 7, 90, 595, 9958, 68200, 1194348 };
    TestPosition pos10 = {
        .fenString = "7k/8/8/8/3p4/8/r1P1K3/8 w - - 0 1",
        .nbTest = 7,
        .perftResults = pos10Result
    };

    int pos11Result[7] = { 1, 7, 113, 625, 9930, 57267, 899530 };
    TestPosition pos11 = {
        .fenString = "k6r/8/8/K1Pp4/8/8/8/8 w - d6 0 1",
        .nbTest = 7,
        .perftResults = pos11Result
    };

    int pos12Result[7] = { 1, 2, 40, 248, 4496, 28197, 514111 };
    TestPosition pos12 = {
        .fenString = "r7/7k/8/K1pP4/8/8/8/8 w - c6 0 1",
        .nbTest = 7,
        .perftResults = pos12Result
    };

    int pos13Result[7] = { 1, 1, 34, 83, 3026, 12617, 470572 };
    TestPosition pos13 = {
        .fenString = "r3b3/7k/8/2pP4/1K6/8/8/2rb4 w - c6 0 1",
        .nbTest = 7,
        .perftResults = pos13Result
    };

    int pos14Result[8] = { 1, 5, 29, 208, 1809, 12925, 136091, 939821 };
    TestPosition pos14 = {
        .fenString = "k7/b7/8/2Pp4/3K4/8/8/8 w - d6 0 1",
        .nbTest = 8,
        .perftResults = pos14Result
    };

    int pos15Result[7] = { 1, 6, 79, 448, 7015, 39768, 634167 };
    TestPosition pos15 = {
        .fenString = "k7/8/8/K1Pp3r/8/8/8/8 w - d6 0 1",
        .nbTest = 7,
        .perftResults = pos15Result
    };

    int pos16Result[6] = { 1, 16, 193, 2900, 37622, 588991 };
    TestPosition pos16 = {
        .fenString = "8/8/3p4/KPp4r/1R2PpPk/8/8/8 b - e3 0 1",
        .nbTest = 6,
        .perftResults = pos16Result
    };

    int pos17Result[6] = { 1, 17, 181, 2686, 37581, 577002 };
    TestPosition pos17 = {
        .fenString = "8/8/8/KPpP3r/1R3p1k/8/6P1/8 w - c6 1 3",
        .nbTest = 6,
        .perftResults = pos17Result
    };

    TestPosition testPositions[NUM_TEST_POSITIONS] = {
      startingPositionTests,
      pos2,
      pos3,
      pos4,
      pos5,
      pos6,
      pos7,
      pos8,
      pos9,
      pos10,
      pos11,
      pos12,
      pos13,
      pos14,
      pos15,
      pos16,
      pos17
    };

    ChessPosition startingState;
    u64 perftResult;
    double timeSpent;
    clock_t begin, end, fullTestBegin = clock();

    for (int i = 0; i < NUM_TEST_POSITIONS; i++) {
        TestPosition testPosition = testPositions[i];

        if (!FenString_setChessPositionFromCopiedFenString(testPosition.fenString, &currentPosition)) {
            printf("ERROR: Unable to set the chess position from the fen string %s, exiting program\n", testPosition.fenString);
            exit(EXIT_FAILURE);
        }

        startingState = currentPosition;

        printf("Running test for fen string: %s\n", testPosition.fenString);

        for (int depth = 0; depth < testPosition.nbTest; depth++) {
            maximumDepth = depth;
            hashHits = 0;
            begin = clock();
            perftResult = perft(depth);
            end = clock();
            timeSpent = (double)(end - begin) / CLOCKS_PER_SEC;

            printf("Depth: " GRN "%d " RESET "ply " RESET "Result: " CYN "%" PRIu64 RESET " HashHits: " YEL "%u" RESET "  Time: " MAG "%.0f " RESET "ms ", depth, perftResult, hashHits, timeSpent * 1000);
            if (perftResult == (u64)testPosition.perftResults[depth]) {
                printf("%s" RESET "\n", testPassed);
            }
            else {
                printf("%s " RED "%d" RESET ")\n", testFailedPrefix, testPosition.perftResults[depth]);
                exitCode++;
            }

            currentPosition = startingState;
        }

        PerftTranspositionTable_clear(); // We don't want the perft information from a different test influence the next test
        printf("\n");
    }

    clock_t fullTestEnd = clock();
    double fullTestTimeSpent = (double)(fullTestEnd - fullTestBegin) / CLOCKS_PER_SEC;
    printf("The full test took " CYN "%.0f " RESET "ms" RESET "\n", fullTestTimeSpent * 1000);

    if (exitCode == 0) printf("All test passed! %s\n", testPassed);
    else printf("%d/%d test failed %s\n", exitCode, NUM_TEST_POSITIONS, testFailedPrefix);

    MagicBitBoard_terminate();
    PerftTranspositionTable_terminate();
}

void usage(char* programName) {
    printf("Usage: %s <mode (divide, time, test)> [position (fen string)] [depth (positive integer)]\n", programName);
    printf("\tIf `mode` is not provided it will default to divide mode\n");
    printf("\tIf `position` is not provided it will default to the starting position\n");
    printf("\tThe `position` and `depth` argument only apply for the divide and time mode\n");
    printf("\t`depth` needs to be provided for the modes it applies to\n");
}

// To compile and run the program: ./perft
// To check for memory leaks: valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./perftTesting <args>
int main(int argc, char* argv[]) {
    if (argc == 1) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    char* fenString = INITIAL_FEN;
    maximumDepth = -1;

    { // I am wrapping this code in a scope to not "leak out" the firstArg variable  
        char* firstArg = argv[1];
        if (string_compareStrings(firstArg, "test")) {
            test();
            return exitCode;
        }
        if (string_compareStrings(firstArg, "divide")) {
            divide = true;
        }
        else if (!string_compareStrings(firstArg, "time")) {
            // No mode parameter is provided, so the mode is divide and the first argument is either a fen string of a depth
            divide = true;
            maximumDepth = string_parseNumber(firstArg);
            if (maximumDepth == -1) {
                // The first argument is probably a fen string
                fenString = firstArg;
            }
        }
    }

    if (argc >= 3) {
        char* secondArg = argv[2];
        maximumDepth = string_parseNumber(secondArg);
        if (maximumDepth == -1) {
            // The second argument is probably a fen string
            fenString = secondArg;
        }
    }

    if (argc >= 4) {
        char* thirdArgument = argv[3];
        maximumDepth = string_parseNumber(thirdArgument);
        // This argument needs to be the depth
        if (maximumDepth == -1) {
            printf("The argument %s is not a valid perft number\n", thirdArgument);
            exit(EXIT_FAILURE);
        }
    }

    if (maximumDepth < 0) {
        printf("You did not provide a valid depth for the mode `%s`\n", divide ? "divide" : "time");
        exit(EXIT_FAILURE);
    }

    if (!MagicBitBoard_init() || !ZobristKey_init() || !PerftTranspositionTable_init()) {
        printf("ERROR: Failure to properly initialize, exiting program\n");
        exit(EXIT_FAILURE);
    }

    if (!FenString_setChessPositionFromCopiedFenString(fenString, &currentPosition)) {
        printf("ERROR while setup of chess game state\n Exiting\n");
        exit(EXIT_FAILURE);
    }

    printBoard_stockfish(currentPosition.board);

    u64 perftResult;
    clock_t begin, end;

    hashHits = 0;
    begin = clock();
    perftResult = perft(maximumDepth);
    end = clock();

    double timeSpent_ms = (double)(end - begin) / CLOCKS_PER_SEC * 1000;

    printf("Perft depth %d returned a total number of moves of %" PRIu64 ", had %u hash hits and took %.0fms\n", maximumDepth, perftResult, hashHits, timeSpent_ms);

    MagicBitBoard_terminate();
    PerftTranspositionTable_terminate();

    return exitCode;
}
