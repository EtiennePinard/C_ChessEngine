#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include "../src/moveGenerator.h"
#include "../src/fenString.h"
#include "../src/chessComputer.h"
#include "../src/chessGameEmulator.h"
#include "logChessStructs.h"

GameState* makeMoveFromStartingState(int nbMoveToDo, GameStates *previousStates) {
    GameState *state = setGameStateFromFenString(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", NULL);
    for (int i = 0; i < nbMoveToDo; i++) {
        Moves *moves = getValidMoves(state, previousStates);
        Move move = moves->items[0]; // There will always be a move right?
        da_append(previousStates, copyState(*state));
        makeMove(move, state);
        free(moves->items);
        free(moves);
    }
    return state;
}

// To compile: gcc -g -o test testing/testChessComputer.c src/chessGameEmulator.c testing/logChessStructs.c src/fenString.c src/moveGenerator.c src/chessComputer.c
// To run: ./test <number of tests iterations> <depth of computer>
// valgrind --leak-check=full --track-origins=yes ./test
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: ./test <number of tests iterations> <depth of computer>\n");
        return 0;
    }

    int nbTest = atoi(argv[1]);
    
    if (nbTest <= 0) {
        printf("Invalid integer for number of test iteration %d\n", nbTest);
        return 0;
    }

    int depth = atoi(argv[2]);
    
    if (depth < 0) {
        printf("Invalid integer for depth of computer %d\n", depth);
        return 0;
    }
    
    printf("Performing %d iterations of the test with depth %d\n", nbTest, depth);
    FILE *file = fopen("testing/testChessComputer.txt","w");
    for (int i = 0; i < nbTest; i++) {
        GameStates *previousStates = malloc(sizeof(GameStates));
        previousStates->capacity = 0;
        previousStates->count = 0;
        previousStates->items = NULL;
        GameState *state = makeMoveFromStartingState(i, previousStates);
        Moves* moves = bestMovesAccordingToComputer(depth, state, previousStates);
        writeBoardToFile(state->boardArray, file);
        fprintf(file, "\n");
        for (int j = 0; j < moves->count; j++) {
            Move move = moves->items[j];
            writeMoveToFile(move, file);
        }
        fprintf(file, "\n\n");
        for (int k = 0; k < previousStates->count; k++) {
            free(previousStates->items[k].boardArray);
        }
        free(previousStates->items);
        free(previousStates);
        free(state->boardArray);
        free(state);
        free(moves->items);
        free(moves);
        printf("Done with test iteration %d\n", i + 1);
    }
    fclose(file);
    printf("Test are done!\n");
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Test result at %s/testing/testChessComputer.txt\n", cwd);
    }
    return 0;
}