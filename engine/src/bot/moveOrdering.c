#include <string.h>

#include "../state/Board.h"

#include "Bot.h"
#include "MoveOrdering.h"

#define BASE_CAPTURE_SCORE (1000)
#define BASE_PROMOTION_SCORE (500)

// This is a simple piece value table
// The king is assigned a value of 999 so that capturing
// a pawn with a king is 1, the smallest positive result 
const int moveOrderingPieceValue[6] = {
    100, // Pawn is utter trash except when it's good
    300, // Knight is just a happy gal doing its thing
    325, // Bishop is slightly better than knight in most situations
    500, // Rook is based af
    900, // Queen is slightly better than knight in more than most situations
    999  // King is a freeloader that usually runs away from his problems
};

int scoreMove(Move move, Move ttMove, const Board currentBoard) {
    if (move == ttMove) return BOT_INFINITY; // Highest priority: TT move

    Piece victim = Board_pieceAtIndex(currentBoard, Move_toSquare(move));
    Piece attacker = Board_pieceAtIndex(currentBoard, Move_fromSquare(move));

    if (victim != NOPIECE) {
        // MVV-LVA: capture score based on victim and attacker value
        return BASE_CAPTURE_SCORE + 10 * moveOrderingPieceValue[Piece_type(victim) - 1] - moveOrderingPieceValue[Piece_type(attacker) - 1];
    }

    switch (Move_flag(move)) {
    case PROMOTE_TO_QUEEN: return BASE_PROMOTION_SCORE + moveOrderingPieceValue[QUEEN - 1];
    case PROMOTE_TO_ROOK: return BASE_PROMOTION_SCORE + moveOrderingPieceValue[ROOK - 1];
    case PROMOTE_TO_KNIGHT: return BASE_PROMOTION_SCORE + moveOrderingPieceValue[KNIGHT - 1];
    case PROMOTE_TO_BISHOP: return BASE_PROMOTION_SCORE + moveOrderingPieceValue[BISHOP - 1];
    default: break;
    }

    return 0;
}

void MoveOrdering_orderMoves(Move moves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES], int numMoves, Move ttMove, const Board currentBoard) {
    
    int scores[numMoves];
    
    // This is insertion sort but we compute the elements when accessing them
    for (int index = 0; index < numMoves; index++) {
        int score = scoreMove(moves[index], ttMove, currentBoard);
        Move currentMove = moves[index]; 

        int sortedIndex = index;
        // Shifting all the elements smaller than score to make room for it
        while (sortedIndex > 0 && scores[sortedIndex - 1] < score) {
            scores[sortedIndex] = scores[sortedIndex - 1];
            moves[sortedIndex] = moves[sortedIndex - 1];
            sortedIndex--;
        }
        // Placing score for its correct index to keep scores sorted
        scores[sortedIndex] = score;
        moves[sortedIndex] = currentMove;
    }

}
