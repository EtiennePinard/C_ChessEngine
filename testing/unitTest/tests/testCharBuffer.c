#include "../../../src/utils/CharBuffer.h"
#include "../../../src/utils/Constants.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

bool test_string_compareStrings() {
    bool actual;
    bool expected;

    // Test case: "hello", "hello"
    expected = true;
    actual = string_compareStrings("hello", "hello");

    if (expected != actual) {
        printf("ERROR: string_compareStrings(\"hello\", \"hello\")\n");
        printf("\tExpected: %s\n", expected ? "true" : "false");
        printf("\tActual: %s\n", actual ? "true" : "false");
        return false;
    }

    // Test case: "Hello", "hello"
    expected = false;
    actual = string_compareStrings("Hello", "hello");

    if (expected != actual) {
        printf("ERROR: string_compareStrings(\"Hello\", \"hello\")\n");
        printf("\tExpected: %s\n", expected ? "true" : "false");
        printf("\tActual: %s\n", actual ? "true" : "false");
        return false;
    }

    // Test case: "", ""
    expected = true;
    actual = string_compareStrings("", "");

    if (expected != actual) {
        printf("ERROR: string_compareStrings(\"\", \"\")\n");
        printf("\tExpected: %s\n", expected ? "true" : "false");
        printf("\tActual: %s\n", actual ? "true" : "false");
        return false;
    }

    return true;
}

bool test_string_nextSpaceCharacterFromIndex() {
    size_t actual;
    size_t expected;

    // Test case: "hello world", 0
    expected = 5;
    actual = string_nextSpaceCharacterFromIndex("hello world", 0);

    if (expected != actual) {
        printf("ERROR: string_nextSpaceCharacterFromIndex(\"hello world\", 0)\n");
        printf("\tExpected: %zu\n", expected);
        printf("\tActual: %zu\n", actual);
        return false;
    }

    // Test case: "hello world", 6
    expected = 11;
    actual = string_nextSpaceCharacterFromIndex("hello world", 6);

    if (expected != actual) {
        printf("ERROR: string_nextSpaceCharacterFromIndex(\"hello world\", 6)\n");
        printf("\tExpected: %zu\n", expected);
        printf("\tActual: %zu\n", actual);
        return false;
    }

    // Test case: "helloworld", 0 (no spaces)
    expected = 10;
    actual = string_nextSpaceCharacterFromIndex("helloworld", 0);

    if (expected != actual) {
        printf("ERROR: string_nextSpaceCharacterFromIndex(\"helloworld\", 0)\n");
        printf("\tExpected: %zu\n", expected);
        printf("\tActual: %zu\n", actual);
        return false;
    }

    // Test case: " ", 0 (single space character)
    expected = 0;
    actual = string_nextSpaceCharacterFromIndex(" ", 0);

    if (expected != actual) {
        printf("ERROR: string_nextSpaceCharacterFromIndex(\" \", 0)\n");
        printf("\tExpected: %zu\n", expected);
        printf("\tActual: %zu\n", actual);
        return false;
    }

    // Test case: "hello world", 20 (out of bounds)
    expected = 11;
    actual = string_nextSpaceCharacterFromIndex("hello world", 20);

    if (expected != actual) {
        printf("ERROR: string_nextSpaceCharacterFromIndex(\"hello world\", 20)\n");
        printf("\tExpected: %zu\n", expected);
        printf("\tActual: %zu\n", actual);
        return false;
    }

    return true;
}

bool test_string_toLower() {
    char test1[] = "HELLO";
    char expected1[] = "hello";
    string_toLower(test1);

    if (strcmp(test1, expected1) != 0) {
        printf("ERROR: string_toLower(\"HELLO\")\n");
        printf("\tExpected: \"%s\"\n", expected1);
        printf("\tActual: \"%s\"\n", test1);
        return false;
    }

    char test2[] = "HeLLo WoRLD";
    char expected2[] = "hello world";
    string_toLower(test2);

    if (strcmp(test2, expected2) != 0) {
        printf("ERROR: string_toLower(\"HeLLo WoRLD\")\n");
        printf("\tExpected: \"%s\"\n", expected2);
        printf("\tActual: \"%s\"\n", test2);
        return false;
    }

    char test3[] = "";
    char expected3[] = "";
    string_toLower(test3);

    if (strcmp(test3, expected3) != 0) {
        printf("ERROR: string_toLower(\"\")\n");
        printf("\tExpected: \"%s\"\n", expected3);
        printf("\tActual: \"%s\"\n", test3);
        return false;
    }

    return true;
}

bool test_string_removeUnecessarySpaces() {

    char test1[] = "      test    case      1     ";
    char expected1[] = "test case 1";
    string_removeUnecessarySpaces(test1);

    if (strcmp(test1, expected1) != 0) {
        printf("ERROR: string_removeUnecessarySpaces(\"      test    case      1     \")\n");
        printf("\tExpected: \"%s\"\n", expected1);
        printf("\tActual: \"%s\"\n", test1);
        return false;
    }

    char test2[] = "test_case_2";
    char expected2[] = "test_case_2";
    string_removeUnecessarySpaces(test2);

    if (strcmp(test2, expected2) != 0) {
        printf("ERROR: string_removeUnecessarySpaces(\"test_case_2\")\n");
        printf("\tExpected: \"%s\"\n", expected2);
        printf("\tActual: \"%s\"\n", test2);
        return false;
    }

    char test3[] = "";
    char expected3[] = "";
    string_removeUnecessarySpaces(test3);

    if (strcmp(test3, expected3) != 0) {
        printf("ERROR: string_removeUnecessarySpaces(\"\")\n");
        printf("\tExpected: \"%s\"\n", expected3);
        printf("\tActual: \"%s\"\n", test3);
        return false;
    }

    return true;
}

bool test_string_parseNumber() {
    int actual;
    int expected;

    // Test case: "123"
    expected = 123;
    actual = string_parseNumber("123");
    if (expected != actual) {
        printf("ERROR: string_parseNumber(\"123\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: "0"
    expected = 0;
    actual = string_parseNumber("0");
    if (expected != actual) {
        printf("ERROR: string_parseNumber(\"0\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: "99999"
    expected = 99999;
    actual = string_parseNumber("99999");
    if (expected != actual) {
        printf("ERROR: string_parseNumber(\"99999\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: "42abc"
    expected = -1;
    actual = string_parseNumber("42abc");
    if (expected != actual) {
        printf("ERROR: string_parseNumber(\"42abc\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: "abc42"
    expected = -1;
    actual = string_parseNumber("abc42");
    if (expected != actual) {
        printf("ERROR: string_parseNumber(\"abc42\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: "-42"
    expected = -1;
    actual = string_parseNumber("-42");
    if (expected != actual) {
        printf("ERROR: string_parseNumber(\"-42\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: " 42"
    expected = -1;
    actual = string_parseNumber(" 42");
    if (expected != actual) {
        printf("ERROR: string_parseNumber(\" 42\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    return true;
}

bool test_string_algebraicToIndex() {
    int actual;
    int expected;

    // Test case: "a1"
    expected = 56;
    actual = string_algebraicToIndex("a1");
    if (expected != actual) {
        printf("ERROR: string_algebraicToIndex(\"a1\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: "h1"
    expected = 63;
    actual = string_algebraicToIndex("h1");
    if (expected != actual) {
        printf("ERROR: string_algebraicToIndex(\"h1\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: "a8"
    expected = 0;
    actual = string_algebraicToIndex("a8");
    if (expected != actual) {
        printf("ERROR: string_algebraicToIndex(\"a8\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: "h8"
    expected = 7;
    actual = string_algebraicToIndex("h8");
    if (expected != actual) {
        printf("ERROR: string_algebraicToIndex(\"h8\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: "d4"
    expected = 35;
    actual = string_algebraicToIndex("d4");
    if (expected != actual) {
        printf("ERROR: string_algebraicToIndex(\"d4\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: "-"
    expected = 0;
    actual = string_algebraicToIndex("-");
    if (expected != actual) {
        printf("ERROR: string_algebraicToIndex(\"-\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: "j9" (Invalid square)
    expected = -1;
    actual = string_algebraicToIndex("j9");
    if (expected != actual) {
        printf("ERROR: string_algebraicToIndex(\"j9\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    // Test case: "a0" (Invalid square)
    expected = -1;
    actual = string_algebraicToIndex("a0");
    if (expected != actual) {
        printf("ERROR: string_algebraicToIndex(\"a0\")\n");
        printf("\tExpected: %d\n", expected);
        printf("\tActual: %d\n", actual);
        return false;
    }

    return true;
}

bool test_string_moveToLongAlgebraic() {
    char expected[6];
    char actual[6];

    // Test case 1: Move from e2 to e4
    Move move1 = Move_makeMove(E2, E4, NOFLAG);
    strcpy(expected, "e2e4");
    string_moveToLongAlgebraic(move1, actual);

    if (strcmp(expected, actual) != 0) {
        printf("ERROR: string_moveToLongAlgebraic(move1) // Move is from e2 to e4\n");
        printf("\tExpected: %s\n", expected);
        printf("\tActual: %s\n", actual);
        return false;
    }

    // Test case 2: Move from g1 to f3
    Move move2 = Move_makeMove(G1, F3, NOFLAG);
    strcpy(expected, "g1f3");
    string_moveToLongAlgebraic(move2, actual);

    if (strcmp(expected, actual) != 0) {
        printf("ERROR: string_moveToLongAlgebraic(move2) // Move is from g1 to f3\n");
        printf("\tExpected: %s\n", expected);
        printf("\tActual: %s\n", actual);
        return false;
    }

    // Test case 3: Pawn promotion to Queen at h8
    Move move3 = Move_makeMove(H7, H8, PROMOTE_TO_QUEEN);
    strcpy(expected, "h7h8q");
    string_moveToLongAlgebraic(move3, actual);

    if (strcmp(expected, actual) != 0) {
        printf("ERROR: string_moveToLongAlgebraic(move3) // Move is from h7 to h8\n");
        printf("\tExpected: %s\n", expected);
        printf("\tActual: %s\n", actual);
        return false;
    }

    // Test case 4: Pawn promotion to Knight at a8
    Move move4 = Move_makeMove(A7, A8, PROMOTE_TO_KNIGHT);
    strcpy(expected, "a7a8n");
    string_moveToLongAlgebraic(move4, actual);

    if (strcmp(expected, actual) != 0) {
        printf("ERROR: string_moveToLongAlgebraic(move4) // Move is from a7 to a8\n");
        printf("\tExpected: %s\n", expected);
        printf("\tActual: %s\n", actual);
        return false;
    }

    // Test case 5: Castling king-side for White
    Move move5 = Move_makeMove(E1, G1, NOFLAG);
    strcpy(expected, "e1g1");
    string_moveToLongAlgebraic(move5, actual);

    if (strcmp(expected, actual) != 0) {
        printf("ERROR: string_moveToLongAlgebraic(move5) // Move is from e1 to eg1\n");
        printf("\tExpected: %s\n", expected);
        printf("\tActual: %s\n", actual);
        return false;
    }

    // Test case 6: Castling queen-side for Black
    Move move6 = Move_makeMove(E8, C8, NOFLAG);
    strcpy(expected, "e8c8");
    string_moveToLongAlgebraic(move6, actual);

    if (strcmp(expected, actual) != 0) {
        printf("ERROR: string_moveToLongAlgebraic(move6) // Move is from e8 to c8\n");
        printf("\tExpected: %s\n", expected);
        printf("\tActual: %s\n", actual);
        return false;
    }

    return true;
}

bool Test_CharBuffer() {
    if (!test_string_compareStrings()) return false;
    if (!test_string_compareStrings()) return false;
    if (!test_string_nextSpaceCharacterFromIndex()) return false;
    if (!test_string_toLower()) return false;
    if (!test_string_removeUnecessarySpaces()) return false;
    if (!test_string_parseNumber()) return false;
    if (!test_string_algebraicToIndex()) return false;
    if (!test_string_moveToLongAlgebraic()) return false;
    return true;
}
