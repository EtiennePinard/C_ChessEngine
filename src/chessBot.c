// This file uses the smol.cs entry in the TinyChessBot Sebastian Lague tournament as some kind of tutorial
// https://github.com/GediminasMasaitis/Chess-Challenge-Submission/blob/submission/Chess-Challenge/src/My%20Bot/MyBot.cs
#include <stdbool.h>
#include "ChessBot.h"
#include "MoveGenerator.h"

Move think(ChessGame* game) {
    Move bestMove = (Move) 0;
    
    // Here I used 256 because it is the closest power of 2 from MAX_LEGAL_MOVES
    Move moves[256] = { [0 ... (255)] = 0 };
    
    getValidMoves(game);
    bestMove = moves[0]; // The best bot ever!
    
    return bestMove; 
}