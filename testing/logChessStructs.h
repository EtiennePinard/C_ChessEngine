#ifndef LOGCHESSSTRCUTS_H
#define LOGCHESSSTRCUTS_H

#include <stdio.h>
#include "../src/moveGenerator.h"

void printMove(Move move);

void printMoveToAlgebraic(Move move);

void printBoard(int board[64]);

void writeMoveToFile(Move move, FILE *file);

void writeMoveToAlgebraicToFile(Move move, FILE *file);

void writeBoardToFile(int board[64], FILE *file);
#endif