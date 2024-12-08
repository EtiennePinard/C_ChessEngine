// This file uses the smol.cs entry in the TinyChessBot Sebastian Lague tournament as some kind of tutorial
// https://github.com/GediminasMasaitis/Chess-Challenge-Submission/blob/submission/Chess-Challenge/src/My%20Bot/MyBot.cs
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ChessBot.h"
#include "../magicBitBoard/MagicBitBoard.h"
#include "PieceSquareTable.h"
#include "../MoveGenerator.h"
#include "../ChessGameEmulator.h"
#include "../../testing/LogChessStructs.h"

#define INFINITY 2000000
#define MINUS_INFINITY -INFINITY

ChessGame* game;

void provideGameStateForBot(ChessGame* state) {
    game = state;
}

// Piece order influenced
/*
This array maps the piece index to a number, which represents how much
a piece is worth for a phase of the game
*/
const char phaseWeight[6] = { 
    0, // Pawn
    1, // Knight
    1, // Bishop
    2, // Rook
    4, // Queen
    0, // King
};

// Piece order influenced
const int openFilesAndDoublePawns[6] = { 
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
const int mobilityBonus[][32] = {
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

const int reducingKingMovementBonus[] = {
    S(9, -2),   // Bishop
    S(16, 0),   // Rook
    S(36, -10), // Queen
    S(23, 18)   // King
};

#define TOTAL_PHASE 24

// TODO: Static evaluation does not give positive/negative inf if one side is checkmated
int staticEvaluation() {

    GamePhase phase = TOTAL_PHASE;
    int score = 15;
    u64 file = 0x101010101010101UL;
    u64 bitboard, attack, currentFileWithoutPiece, kingAttacks, friendlyBitBoard;  

    // Checking for mate:
    Move validMoves[256];
    int numMoves;
    getValidMoves(validMoves, &numMoves, game->currentPosition);

    bool inCheck = isKingInCheck(); 
    if (numMoves == 0) {
        if (inCheck) {
            // The player has lost
            return game->currentPosition.colorToGo == BLACK ? MINUS_INFINITY : INFINITY;
        }
        // It is a stalemate
        return 0;
    }

    for (int i = 0; i < 2; i++) {
        PieceCharacteristics color = WHITE * (i + 1);
        
        int view = color == WHITE ? 1 : -1;
        int opposingKingIndex = trailingZeros_64(bitBoardForPiece(game->currentPosition.board, makePiece(BLACK / (i + 1), KING)));
        kingAttacks = kingMovementMask[opposingKingIndex];
        friendlyBitBoard = specificColorBitBoard(game->currentPosition.board, color);
        
        for (PieceCharacteristics type = PAWN; type <= KING; type++) {

            bitboard = bitBoardForPiece(game->currentPosition.board, makePiece(color, type));

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
                
                if ((currentFileWithoutPiece & bitBoardForPiece(game->currentPosition.board, makePiece(color, PAWN))) == 0) {
                    score += openFilesAndDoublePawns[type - 1] * view;
                }
                
                switch (type) {
                case KNIGHT:
                    attack = knightMovementMask[square] & (~friendlyBitBoard);
                    score += mobilityBonus[type - 2][numBitSet_64(attack)] * view;
                    break;
                case BISHOP:
                    attack = getBishopPseudoLegalMovesBitBoard(
                                 square, allPiecesBitBoard(game->currentPosition.board) & bishopMovementMask[square]
                            ) & (~friendlyBitBoard);
                    score += mobilityBonus[type - 2][numBitSet_64(attack)] * view;
                    // Adding more if the piece blocks the king
                    score += reducingKingMovementBonus[type - 3] * numBitSet_64(attack & kingAttacks) * view;
                    break;

                case ROOK:
                    attack = getRookPseudoLegalMovesBitBoard(
                                 square, allPiecesBitBoard(game->currentPosition.board) & rookMovementMask[square]
                            ) & (~friendlyBitBoard);
                    score += mobilityBonus[type - 2][numBitSet_64(attack)] * view;
                    // Adding more if the piece blocks the king
                    score += reducingKingMovementBonus[type - 3] * numBitSet_64(attack & kingAttacks) * view;
                    break;

                case QUEEN:
                    attack = (getBishopPseudoLegalMovesBitBoard(
                                  square, allPiecesBitBoard(game->currentPosition.board) & bishopMovementMask[square]) |
                              getRookPseudoLegalMovesBitBoard(
                                  square, allPiecesBitBoard(game->currentPosition.board) & rookMovementMask[square])) &
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

// Max depth is 10 for now
ChessPosition posHistory[10];
int maximumDepth;

// A negamax search with alpha-beta pruning
// TODO: This is slow!
// Move ordering will be important to implement to speed it up
// TODO: Implement transposition table
int search(int alpha, int beta, int depth) {
    if (depth == 0) {
        // Negamax needs a relative evaluation, so positive means good for color to go and vice-versa
        int whoToMove = game->currentPosition.colorToGo == WHITE ? 1 : -1; 
        return staticEvaluation() * whoToMove; 
    } // eventually do return quiesce(alpha, beta);
    int bestValue = MINUS_INFINITY;

    int nbOfMoves;
    Move validMoves[256];
    getValidMoves(validMoves, &nbOfMoves, game->currentPosition);
    posHistory[maximumDepth - depth] = game->currentPosition;

    for (int i = 0; i < nbOfMoves; i++) {
        Move move = validMoves[i];
        playMove(move, game);
        // We switch alpha and beta, because alpha is the lower bound for the color to go 
        // but it is the upper bound for the other color. Opposite is true for beta
        // thus we switch them so that it works for the opponent
        int score = -search(-beta, -alpha, depth - 1);
        
        if (score > bestValue) {
            bestValue = score;
            if (score > alpha ) {
                alpha = score; // alpha acts like max in MiniMax
            }
        }
        game->previousPositionsCount--;
        memcpy(&game->currentPosition, &posHistory[maximumDepth - depth], sizeof(ChessPosition));
        
        // fail soft beta-cutoff, existing the loop here is also 
        if (score >= beta) { return bestValue; }  
    }
    return bestValue;
}

Move think() {

    Move bestMove = BOT_ERROR;
    int bestEval = MINUS_INFINITY;

    ChessPosition lastPos = game->currentPosition;

    maximumDepth = 2;
    if (maximumDepth >= 10) {
        printf("ERROR: Maximum is bigger than 10 in chessbot.c!\n");
        exit(EXIT_FAILURE);
    }

    // Here I used 256 because it is the closest power of 2 from MAX_LEGAL_MOVES
    Move moves[256];
    int numMoves;
    getValidMoves(moves, &numMoves, lastPos);
    if (numMoves == 0) { // Game is done!
        return BOT_ERROR; 
    }
    bestMove = moves[0]; // So that it always return a valid move if all moves have the same evaluation

    for (int i = 0; i < numMoves; i++) {
        Move move = moves[i];
        playMove(move, game);

        int score = -search(MINUS_INFINITY, INFINITY, maximumDepth);
        if (score > bestEval) {
            bestEval = score;
            bestMove = move;
        }

        game->previousPositionsCount--; // Removing last zobrist key computed
        memcpy(&game->currentPosition, &lastPos, sizeof(ChessPosition));
    }

    return bestMove; 
}