#include "Move.h"

char fromSquare(Move move) {
    return move & 0b111111;
}

char toSquare(Move move) {
    return (move >> 6) & 0b111111;
}

Flag flagFromMove(Move move) {
    return (Flag) (move >> 12);
}

Move createMove(int from, int to, Flag flag) {
    Move move = from;
    move |= (to << 6);
    move |= (flag << 12);
    return move;
}