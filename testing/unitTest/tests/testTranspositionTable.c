#include "../../../src/chessBot/TranspositionTable.h"

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

bool test_TranspositionTable_getMoveFromKey() {
    ZobristKey key = 123456789; // Garbage key
    Move move = TranspositionTable_getMoveFromKey(key);
    Move expected = NULL_MOVE; // Assuming no move is stored initially

    if (expected != move) {
        printf("test_TranspositionTable_getMoveFromKey failed with test case: TranspositionTable_getMoveFromKey(%llu)\n", key);
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", move);
        return false;
    }
    return true;
}

bool test_TranspositionTable_getEvaluationFromKey() {
    ZobristKey key = 987654321; // Garbage key
    int depth = 4;
    int alpha = -100;
    int beta = 100;
    int actual = TranspositionTable_getEvaluationFromKey(key, depth, alpha, beta);
    int expected = LOOKUP_FAILED; // Assuming no evaluation is stored initially

    if (expected != actual) {
        printf("test_TranspositionTable_getEvaluationFromKey failed with test case: TranspositionTable_getEvaluationFromKey(%llu, %d, %d, %d)\n", key, depth, alpha, beta);
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
    Move move = 42;
    int evaluation = 20;

    TranspositionTable_recordEntry(key, depth, type, move, evaluation);
    int retrievedEvaluation = TranspositionTable_getEvaluationFromKey(key, depth, -100, 100);
    
    if (retrievedEvaluation != evaluation) {
        printf("test_TranspositionTable_recordEntry failed with test case: TranspositionTable_recordEntry(%llu, %d, %d, %d, %d)\n", key, depth, type, move, evaluation);
        printf("\tExpected: %d\n", evaluation);
        printf("\tActual: %d\n", retrievedEvaluation);
        return false;
    }

    Move retrievedMove = TranspositionTable_getMoveFromKey(key);
    if (retrievedMove != move) {
        printf("test_TranspositionTable_recordEntry failed with test case: TranspositionTable_recordEntry(%llu, %d, %d, %d, %d)\n", key, depth, type, move, evaluation);
        printf("\tExpected Move: %d\n", move);
        printf("\tActual Move: %d\n", retrievedMove);
        return false;
    }

    return true;
}

bool Test_TranspositionTable() {
    if (!test_TranspositionTable_init()) return false;
    if (!test_TranspositionTable_getMoveFromKey()) return false;
    if (!test_TranspositionTable_getEvaluationFromKey()) return false;
    if (!test_TranspositionTable_recordEntry()) return false;
    
    // We like to cleanup after ourselves
    TranspositionTable_terminate();

    return true;
}
