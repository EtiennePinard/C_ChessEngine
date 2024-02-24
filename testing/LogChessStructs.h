#ifndef LOGCHESSSTRCUTS_H
#define LOGCHESSSTRCUTS_H

#include <stdio.h>
#include "../src/state/GameState.h"
#include "../src/state/Move.h"
#include "../src/state/Board.h"

void printMove(Move move);

void printMoveToAlgebraic(Move move);

void printBoard(Board board);

void writeMoveToFile(Move move, FILE *file);

void writeMoveToAlgebraicToFile(Move move, FILE *file);

void writeBoardToFile(Board board, FILE *file);

void printPosition(ChessPosition game, int score, Move bestMove);

void printBitBoard(const u64 bitboard);

void printBin(const u64 num); 

#endif