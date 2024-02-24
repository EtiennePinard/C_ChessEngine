// This file uses the smol.cs entry in the TinyChessBot Sebastian Lague tournament as some kind of tutorial
// https://github.com/GediminasMasaitis/Chess-Challenge-Submission/blob/submission/Chess-Challenge/src/My%20Bot/MyBot.cs
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "ChessBot.h"
#include "../magicBitBoard/MagicBitBoard.h"
#include "PieceSquareTable.h"
#include "../MoveGenerator.h"
#include "../ChessGameEmulator.h"

#define INFINITY 2000000
#define MINUS_INFINITY -INFINITY

// Piece order influenced
/*
This array maps the piece index to a number, which represents how much
a piece is worth for a phase of the game
*/
char phaseWeight[6] = { 
    0, // Pawn
    1, // Knight
    1, // Bishop
    2, // Rook
    4, // Queen
    0, // King
};

// Piece order influenced
int openFilesAndDoublePawns[6] = { 
    S(16, 26), // Pawn
    S(5, -4), // Knight
    S(1, 4), // Bishop
    S(31, 13), // Rook
    S(3, 21), // Queen
    S(-30, 4), // King
};

// Gotten from https://github.com/official-stockfish/Stockfish/blob/b4c239b625285307c5871f1104dc3fb80aa6d5d2/src/evaluate.cpp#L102
// MobilityBonus[PieceType - 2][num square attacked] contains bonuses for middle and end game,
// indexed by piece type and number of attacked squares in the mobility area.
int mobilityBonus[][32] = {
    { S(-62,-81), S(-53,-56), S(-12,-30), S( -4,-14), S(  3,  8), S( 13, 15), // Knights
      S( 22, 23), S( 28, 27), S( 33, 33) },
    { S(-48,-59), S(-20,-23), S( 16, -3), S( 26, 13), S( 38, 24), S( 51, 42), // Bishops
      S( 55, 54), S( 63, 57), S( 63, 65), S( 68, 73), S( 81, 78), S( 81, 86),
      S( 91, 88), S( 98, 97) },
    { S(-58,-76), S(-27,-18), S(-15, 28), S(-10, 55), S( -5, 69), S( -2, 82), // Rooks
      S(  9,112), S( 16,118), S( 30,132), S( 29,142), S( 32,155), S( 38,165),
      S( 46,166), S( 48,169), S( 58,171) },
    { S(-39,-36), S(-21,-15), S(  3,  8), S(  3, 18), S( 14, 34), S( 22, 54), // Queens
      S( 28, 61), S( 41, 73), S( 43, 79), S( 48, 92), S( 56, 94), S( 60,104),
      S( 60,113), S( 66,120), S( 67,123), S( 70,126), S( 71,133), S( 73,136),
      S( 79,140), S( 88,143), S( 88,148), S( 99,166), S(102,170), S(102,175),
      S(106,184), S(109,191), S(113,206), S(116,212) },
    { S(-30, 4), S(-30, 4), S(-30, 4), S(-30, 4), S(-30, 4), S(-30, 4), S(-30, 4), S(-30, 4),  } // King
  };

int reducingKingMovementBonus[] = {
    S(9, -2),   // Bishop
    S(16, 0),   // Rook
    S(36, -10), // Queen
    S(23, 18)   // King
};

u64 friendlyBitBoard;

bool isPosititionARepeatition(ChessGame* game) {
    for (int i = 0; i < game->previousStatesCount; i++) {
        if (game->previousStates[i] == game->currentState->key) {
            return true;
        }
    }
    return false; 
}

#define TOTAL_PHASE 24

int staticEvaluation(ChessGame* game) {

    int phase = TOTAL_PHASE;
    int score = 15;
    u64 file = 0x101010101010101UL;
    u64 bitboard, attack, currentFileWithoutPiece, kingAttacks;    

    for (int i = 0; i < 2; i++) {
        PieceCharacteristics color = WHITE * (i + 1);
        
        int view = color == WHITE ? 1 : -1;
        int opposingKingIndex = trailingZeros_64(bitBoardForPiece(game->currentState->board, makePiece(BLACK / (i + 1), KING)));
        kingAttacks = kingMovementMask[opposingKingIndex];
        friendlyBitBoard = specificColorBitBoard(game->currentState->board, color);
        
        for (PieceCharacteristics type = PAWN; type <= KING; type++) {

            bitboard = bitBoardForPiece(game->currentState->board, makePiece(color, type));

            while (bitboard) {
                int square = trailingZeros_64(bitboard);

                // Adding the phase value to calculate at which phase of the game we are
                phase -= phaseWeight[type - 1];

                // Checking for open files and double pawns
                // We want to give higher scores to position where pawns aren't doubled
                // and pieces are put in an open file.
                // An open file is just a file where there are no of the own color pawns blocking the file
                // We do not include the piece that we are currently looking in the file bitboard
                currentFileWithoutPiece = (file << file(square)) & ~(1UL << square);
                
                if ((currentFileWithoutPiece & bitBoardForPiece(game->currentState->board, makePiece(color, PAWN))) == 0) {
                    score += openFilesAndDoublePawns[type - 1] * view;
                }
                
                switch (type) {
                case KNIGHT:
                    attack = knightMovementMask[square] & (~friendlyBitBoard);
                    score += mobilityBonus[type - 2][numBitSet_64(attack)] * view;
                    break;
                case BISHOP:
                    attack = getBishopPseudoLegalMovesBitBoard(
                                 square, allPiecesBitBoard(game->currentState->board) & bishopMovementMask[square]
                            ) & (~friendlyBitBoard);
                    score += mobilityBonus[type - 2][numBitSet_64(attack)] * view;
                    // Adding more if the piece blocks the king
                    score += reducingKingMovementBonus[type - 3] * numBitSet_64(attack & kingAttacks) * view;
                    break;

                case ROOK:
                    attack = getRookPseudoLegalMovesBitBoard(
                                 square, allPiecesBitBoard(game->currentState->board) & rookMovementMask[square]
                            ) & (~friendlyBitBoard);
                    score += mobilityBonus[type - 2][numBitSet_64(attack)] * view;
                    // Adding more if the piece blocks the king
                    score += reducingKingMovementBonus[type - 3] * numBitSet_64(attack & kingAttacks) * view;
                    break;

                case QUEEN:
                    attack = (getBishopPseudoLegalMovesBitBoard(
                                  square, allPiecesBitBoard(game->currentState->board) & bishopMovementMask[square]) |
                              getRookPseudoLegalMovesBitBoard(
                                  square, allPiecesBitBoard(game->currentState->board) & rookMovementMask[square])) &
                             (~friendlyBitBoard);
                    score += mobilityBonus[type - 2][numBitSet_64(attack)] * view;
                    // Adding more if the piece blocks the king
                    score += reducingKingMovementBonus[type - 3] * numBitSet_64(attack & kingAttacks) * view;
                    break;

                case KING:
                    attack = kingMovementMask[square] & (~friendlyBitBoard);
                    score += mobilityBonus[type - 2][numBitSet_64(attack)] * view;
                    // Adding more if the piece blocks the king
                    score += reducingKingMovementBonus[type - 3] * numBitSet_64(attack & kingAttacks) * view;
                    break;
                default:
                    // Pawn mobilty is not considered
                    break;
                }

                // Piece Square Table mixed with Material
                // We combined the material score and the piece square table score in the PieceSquareTable.c file
                // Note that all of the scores were taken from Stockfish 14
                score += pieceSquareTable[makePiece(color, type) - 9][square];

                bitboard &= bitboard - 1;   
            }
        }
    }
    // Interpolating the midgame and endgame score, see https://www.chessprogramming.org/Tapered_Eval
    phase = (phase * 256 + (TOTAL_PHASE / 2)) / TOTAL_PHASE;
    score = ((mg_value(score) * (256 - phase)) + (eg_value(score) * phase)) / 256;
    return score;
}

int perspective;

int search(ChessGame* game, int depth) {
    if (depth == 0) {
        return staticEvaluation(game) * perspective;
    }

    /* Later
    // If repetition is the best course of action, then no need to check for 3 fold repetition before exiting
    if (isPosititionARepeatition(game)) {
        return 0;
    }

    // Check extension
    // Note: Check if this return a valid value (did we call generateValidMoves yet)
    bool inCheck = isKingInCheck(); 
    if (inCheck) {
        depth++;
    }
    */

   return search(game, depth - 1);
}

Move think(ChessGame* game) {
    
    Move bestMove = (Move) 0;
    int bestEval = MINUS_INFINITY;

    // Here I used 256 because it is the closest power of 2 from MAX_LEGAL_MOVES
    Move moves[256] = { [0 ... (255)] = 0 };
    int numMoves;

    getValidMoves(moves, &numMoves, game);

    ChessPosition lastPos = *game->currentState;

    perspective = game->currentState->colorToGo == WHITE ? 1 : -1;
    for (int i = 0; i < numMoves; i++) {
        Move move = moves[i];

        playMove(move, game);
        int score = search(game, 0);
        if (score >= bestEval) {
            bestEval = score;
            bestMove = move;
        }

        game->previousStatesCount--; // Removing last zobrist key compute
        memcpy(game->currentState, &lastPos, sizeof(ChessPosition));
    }

    return bestMove; 
}