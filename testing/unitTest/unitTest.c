#include "TestCharBuffer.h"
#include "TestFenString.h"
#include "TestMath.h"
#include "testBoard.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef bool (*TestFunction)(void);

typedef struct Test {
    TestFunction testFunction;
    char* name;
} Test;


void runTest(Test test) {
    printf("Starting %s tests...\n", test.name);
    if (!test.testFunction()) {
        printf("ERROR: %s tests failed :(\n", test.name);
        exit(EXIT_FAILURE);
    }
}

Test tests[] = {
    { Test_CharBuffer, "Char Buffer" },
    { Test_FenString, "Fen String" },
    { Test_Math, "Math" },
    { Test_Board, "Board" }
};

// To run: ./unit
int main(void) {

    size_t nbTests = sizeof(tests) / sizeof(Test);

    clock_t begin = clock();

    for (int testIndex = 0; testIndex < nbTests; testIndex++) {
        runTest(tests[testIndex]);
    }

    clock_t end = clock();
    double fullTestTimeSpent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("\nAll unit tests passed in %fms\n", fullTestTimeSpent * 1000);
    return 0;
}
