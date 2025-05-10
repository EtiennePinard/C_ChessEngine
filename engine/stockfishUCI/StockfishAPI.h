#ifndef A3EA3C40_5803_4027_A286_6049A6D5C587
#define A3EA3C40_5803_4027_A286_6049A6D5C587

#include <stdbool.h>

#include "../src/state/Move.h"

bool stockfishInit(const char *stockfishPath);

void terminate();

void setStockfishPosition(char* fen);

float stockfishStaticEvaluation();

Move stockfishBestMove(int depth);

#endif /* A3EA3C40_5803_4027_A286_6049A6D5C587 */
