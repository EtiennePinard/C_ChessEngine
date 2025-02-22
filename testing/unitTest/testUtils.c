#include <stdio.h>
#include <stdbool.h>

#include "../../src/utils/Utils.h"

// Test file() macro
bool test_file() {
    int expected, actual;

    expected = 0; actual = file(0);
    if (expected != actual) {
        printf("test_file failed with test case: file(0)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    expected = 7; actual = file(7);
    if (expected != actual) {
        printf("test_file failed with test case: file(7)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    expected = 3; actual = file(11);
    if (expected != actual) {
        printf("test_file failed with test case: file(11)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    return true;
}

// Test rank() macro
bool test_rank() {
    int expected, actual;

    expected = 0; actual = rank(0);
    if (expected != actual) {
        printf("test_rank failed with test case: rank(0)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    expected = 7; actual = rank(56);
    if (expected != actual) {
        printf("test_rank failed with test case: rank(56)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    expected = 1; actual = rank(9);
    if (expected != actual) {
        printf("test_rank failed with test case: rank(9)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    return true;
}

// Test max() macro
bool test_max() {
    int expected, actual;

    expected = 5; actual = max(3, 5);
    if (expected != actual) {
        printf("test_max failed with test case: max(3, 5)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    expected = -2; actual = max(-5, -2);
    if (expected != actual) {
        printf("test_max failed with test case: max(-5, -2)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    expected = 10; actual = max(10, 10);
    if (expected != actual) {
        printf("test_max failed with test case: max(10, 10)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    return true;
}

// Test min() macro
bool test_min() {
    int expected, actual;

    expected = 3; actual = min(3, 5);
    if (expected != actual) {
        printf("test_min failed with test case: min(3, 5)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    expected = -5; actual = min(-5, -2);
    if (expected != actual) {
        printf("test_min failed with test case: min(-5, -2)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    expected = 10; actual = min(10, 10);
    if (expected != actual) {
        printf("test_min failed with test case: min(10, 10)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    return true;
}

// Test trailingZeros_64()
bool test_trailingZeros_64() {
    int expected, actual;

    expected = 5; actual = trailingZeros_64(96);
    if (expected != actual) {
        printf("test_trailingZeros_64 failed with test case: trailingZeros_64(96)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    expected = 0; actual = trailingZeros_64(431);
    if (expected != actual) {
        printf("test_trailingZeros_64 failed with test case: trailingZeros_64(431)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    expected = 13; actual = trailingZeros_64(8192);
    if (expected != actual) {
        printf("test_trailingZeros_64 failed with test case: trailingZeros_64(8192)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    return true;
}

// Test numBitSet_64()
bool test_numBitSet_64() {
    int expected, actual;

    expected = 2; actual = numBitSet_64(96);
    if (expected != actual) {
        printf("test_numBitSet_64 failed with test case: numBitSet_64(96)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    expected = 7; actual = numBitSet_64(431);
    if (expected != actual) {
        printf("test_numBitSet_64 failed with test case: numBitSet_64(431)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    expected = 1; actual = numBitSet_64(8192);
    if (expected != actual) {
        printf("test_numBitSet_64 failed with test case: numBitSet_64(8192)\n\tExpected: %d\n\tActual: %d\n", expected, actual);
        return false;
    }

    return true;
}

bool Test_Utils() {
    if (!test_file()) return false;
    if (!test_rank()) return false;
    if (!test_max()) return false;
    if (!test_min()) return false;
    if (!test_trailingZeros_64()) return false;
    if (!test_numBitSet_64()) return false;

    return true;
}
