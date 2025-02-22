#include "TestCharBuffer.h"
#include "TestFenString.h"
#include "TestUtils.h"
#include "testBoard.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// To run: ./unit
int main(void) {

    clock_t begin = clock();

    printf("Starting Char Buffer tests...\n");
    if (!Test_CharBuffer()) {
        printf("ERROR: Char Buffer tests failed :(\n");
        exit(EXIT_FAILURE);
    }

    printf("Starting Fen String test...\n");
    if (!Test_FenString()) {
        printf("ERROR: Fen String tests failed :(\n");
        exit(EXIT_FAILURE);
    }

    printf("Starting Utils test...\n");
    if (!Test_Utils()) {
        printf("ERROR: Utils tests failed :(\n");
        exit(EXIT_FAILURE);
    }

    printf("Starting Board test...\n");
    if (!Test_Board()) {
        printf("ERROR: Board tests failed :(\n");
        exit(EXIT_FAILURE);
    }

    clock_t end = clock();
    double fullTestTimeSpent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("\nAll unit tests passed in %fms\n", fullTestTimeSpent * 1000);
    return 0;
}
