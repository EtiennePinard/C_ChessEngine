#ifndef A3EA3C40_5803_4027_A286_6049A6D5C587
#define A3EA3C40_5803_4027_A286_6049A6D5C587

#include <stdbool.h>

#include "../src/state/Move.h"

bool init(const char *stockfishPath);

void terminate();

void setPosition(char* fen);

float staticEvaluationOfCurrentPosition();

Move bestMoveOfCurrentPosition(int depth);

#endif /* A3EA3C40_5803_4027_A286_6049A6D5C587 */
