#include <stdio.h>
#include <inttypes.h>

#include "../../../src/bot/TranspositionTable.h"

bool test_TranspositionTable_init() {
    bool actual = TranspositionTable_init();
    bool expected = true;

    if (expected != actual) {
        printf("test_TranspositionTable_init failed with test case: TranspositionTable_init()\n");
        printf("\tExpected: %s\n", expected ? "true" : "false");
        printf("\tActual: %s\n", actual ? "true" : "false");
        return false;
    }
    return true;
}

bool test_TranspositionTable_getPvLineFromKey() {
    ZobristKey key = 123456789; // Garbage key
    TranspositionTable_getPvLineFromKey(key, NULL);
    // We expect no segfault

    Move expected[2] = { Move_makeMove(E2, E4, DOUBLE_PAWN_PUSH), Move_makeMove(E7, E5, DOUBLE_PAWN_PUSH) };
    TranspositionTable_recordEntry(key, 0, EXACT, 0, 0, expected, 2);
    Move actual[2];
    TranspositionTable_getPvLineFromKey(key, actual);

    if (expected[0] != actual[0] || expected[1] != actual[1]) {
        printf("test_TranspositionTable_getMoveFromKey failed with test case: TranspositionTable_getPvLineFromKey(%" PRIu64 ")\n", key);
        printf("\tExpected: {%d, %d}\n", expected[0], expected[1]);
        printf("\tActual: {%d, %d}\n", actual[0], actual[1]);
        return false;
    }
    return true;
}

bool test_TranspositionTable_getEvaluationFromKey() {
    ZobristKey key = 987654321; // Garbage key
    int depth = 4;
    int alpha = -100;
    int beta = 100;
    int plyFromRoot = 1;
    EntryType type;
    int actual = TranspositionTable_getEvaluationFromKey(key, depth, alpha, beta, plyFromRoot, &type);
    int expected = LOOKUP_FAILED; // Assuming no evaluation is stored initially

    if (expected != actual) {
        printf("test_TranspositionTable_getEvaluationFromKey failed with test case: TranspositionTable_getEvaluationFromKey(%" PRIu64 ", %d, %d, %d)\n", key, depth, alpha, beta);
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }
    return true;
}

bool test_TranspositionTable_recordEntry() {
    ZobristKey key = 111111111;
    u8 depth = 5;
    EntryType type = EXACT;
    int evaluation = 20;
    int plyFromRoot = 0;

    EntryType actualType;
    TranspositionTable_recordEntry(key, depth, type, evaluation, 0, NULL, 0);
    int retrievedEvaluation = TranspositionTable_getEvaluationFromKey(key, depth, -100, 100, plyFromRoot, &actualType);
    
    if (retrievedEvaluation != evaluation) {
        printf("test_TranspositionTable_recordEntry failed with test case: TranspositionTable_recordEntry(%" PRIu64 ", %d, %d, %d, %d)\n", key, depth, type, evaluation, plyFromRoot);
        printf("\tExpected: %d\n", evaluation);
        printf("\tActual: %d\n", retrievedEvaluation);
        return false;
    }

    if (actualType != type) {
        printf("test_TranspositionTable_recordEntry failed with test case: TranspositionTable_recordEntry(%" PRIu64 ", %d, %d, %d, %d)\n", key, depth, type, evaluation, plyFromRoot);
        printf("\tExpected: %d\n", type);
        printf("\tActual: %d\n", actualType);
        return false;
    }

    return true;
}

bool Test_TranspositionTable() {
    if (!test_TranspositionTable_init()) return false;
    if (!test_TranspositionTable_getPvLineFromKey()) return false;
    if (!test_TranspositionTable_getEvaluationFromKey()) return false;
    if (!test_TranspositionTable_recordEntry()) return false;
    
    // We like to cleanup after ourselves
    TranspositionTable_terminate();

    return true;
}
