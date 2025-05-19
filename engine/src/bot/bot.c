// This file uses the smol.cs entry in the TinyChessBot Sebastian Lague tournament as some kind of tutorial
// https://github.com/GediminasMasaitis/Chess-Challenge-Submission/blob/submission/Chess-Challenge/src/My%20Bot/MyBot.cs
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "Bot.h"
#include "PieceSquareTable.h"
#include "RepetitionTable.h"
#include "TranspositionTable.h"

#include "../magicBitBoard/MagicBitBoard.h"
#include "../moveHandler/MoveGenerator.h"
#include "../moveHandler/MovePlayer.h"
#include "../utils/Math.h"

#define INFINITY 2000000
#define MINUS_INFINITY -INFINITY

bool useTranspositionTable = true;
volatile bool endSearch = false;

ChessPosition currentPosition;

void Bot_provideGameStateForBot(ChessPosition state) {
    currentPosition = state;
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
    { S(-62,-81), S(-53,-56), S(-12,-30), S(-4,-14), S(3,  8), S(13, 15), // Knights
      S(22, 23), S(28, 27), S(33, 33) },
    { S(-48,-59), S(-20,-23), S(16, -3), S(26, 13), S(38, 24), S(51, 42), // Bishops
      S(55, 54), S(63, 57), S(63, 65), S(68, 73), S(81, 78), S(81, 86),
      S(91, 88), S(98, 97) },
    { S(-58,-76), S(-27,-18), S(-15, 28), S(-10, 55), S(-5, 69), S(-2, 82), // Rooks
      S(9,112), S(16,118), S(30,132), S(29,142), S(32,155), S(38,165),
      S(46,166), S(48,169), S(58,171) },
    { S(-39,-36), S(-21,-15), S(3,  8), S(3, 18), S(14, 34), S(22, 54), // Queens
      S(28, 61), S(41, 73), S(43, 79), S(48, 92), S(56, 94), S(60,104),
      S(60,113), S(66,120), S(67,123), S(70,126), S(71,133), S(73,136),
      S(79,140), S(88,143), S(88,148), S(99,166), S(102,170), S(102,175),
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

int Bot_staticEvaluation() {

    GamePhase phase = TOTAL_PHASE;

    // We start the score with a small advantage for white    
    int score = 15;

    // bitmask for a file
    u64 file = 0x101010101010101UL;
    u64 bitboard, attack, currentFileWithoutPiece, kingAttacks, friendlyBitBoard;

    // Checking for mate:
    Move validMoves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    int numMoves;
    MoveHandler_getValidMoves(validMoves, &numMoves, currentPosition);

    bool inCheck = MoveHandler_isKingInCheck() || MoveHandler_isKingInDoubleCheck();
    if (numMoves == 0) {
        if (inCheck) {
            // The player has lost
            return currentPosition.colorToGo == BLACK ? INFINITY : MINUS_INFINITY;
        }
        // It is a stalemate
        return 0;
    }

    for (int i = 0; i < 2; i++) {
        PieceCharacteristics color = WHITE * (i + 1);

        int view = color == WHITE ? 1 : -1;
        int opposingKingIndex = trailingZeros_64(Board_bitBoardForPiece(currentPosition.board, Piece_makePiece(BLACK / (i + 1), KING)));
        kingAttacks = kingMovementMask[opposingKingIndex];
        friendlyBitBoard = Board_specificColorBitBoard(currentPosition.board, color);

        for (PieceCharacteristics type = PAWN; type <= KING; type++) {

            bitboard = Board_bitBoardForPiece(currentPosition.board, Piece_makePiece(color, type));

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

                if ((currentFileWithoutPiece & Board_bitBoardForPiece(currentPosition.board, Piece_makePiece(color, PAWN))) == 0) {
                    score += openFilesAndDoublePawns[type - 1] * view;
                }

                switch (type) {
                case KNIGHT:
                    attack = knightMovementMask[square] & (~friendlyBitBoard);
                    score += mobilityBonus[type - 2][numBitSet_64(attack)] * view;
                    break;
                case BISHOP:
                    attack = MagicBitBoard_getBishopPseudoLegalMovesBitBoard(
                        square, Board_allPiecesBitBoard(currentPosition.board) & bishopMovementMask[square]
                    ) & (~friendlyBitBoard);
                    score += mobilityBonus[type - 2][numBitSet_64(attack)] * view;
                    // Adding more if the piece blocks the king
                    score += reducingKingMovementBonus[type - 3] * numBitSet_64(attack & kingAttacks) * view;
                    break;

                case ROOK:
                    attack = MagicBitBoard_getRookPseudoLegalMovesBitBoard(
                        square, Board_allPiecesBitBoard(currentPosition.board) & rookMovementMask[square]
                    ) & (~friendlyBitBoard);
                    score += mobilityBonus[type - 2][numBitSet_64(attack)] * view;
                    // Adding more if the piece blocks the king
                    score += reducingKingMovementBonus[type - 3] * numBitSet_64(attack & kingAttacks) * view;
                    break;

                case QUEEN:
                    attack = (MagicBitBoard_getBishopPseudoLegalMovesBitBoard(
                        square, Board_allPiecesBitBoard(currentPosition.board) & bishopMovementMask[square]) |
                        MagicBitBoard_getRookPseudoLegalMovesBitBoard(
                            square, Board_allPiecesBitBoard(currentPosition.board) & rookMovementMask[square])) &
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
                    // Pawn mobility is not considered
                    break;
                }

                // Piece Square Table mixed with Material
                // We combined the material score and the piece square table score in the PieceSquareTable.c file
                // Note that all of the scores were taken from Stockfish 14
                score += pieceSquareTable[Piece_makePiece(color, type) - 9][square];

                bitboard &= bitboard - 1;
            }
        }
    }
    // Interpolating the midgame and endgame score, see https://www.chessprogramming.org/Tapered_Eval
    phase = (phase * 256 + (TOTAL_PHASE / 2)) / TOTAL_PHASE;
    score = ((mg_value(score) * (256 - phase)) + (eg_value(score) * phase)) / 256;
    return score;
}

// Max depth is 100 for now
#define MAXIMUM_DEPTH (100)
ChessPosition posHistory[MAXIMUM_DEPTH];
int currentDepth;

Move bestMoveFromCurrentDepthSearch;
int bestEvalFromCurrentSearch;

Move bestMoveFromFullDepthSearch;

u32 totalNodes;
u32 leafNodes;
u32 transpositionTableHits;

int alpha_beta_negamax(int alpha, int beta, int depth) {
    if (endSearch) return 0;
    
    totalNodes++;

    int startingAlpha = alpha;

    int ttEval = TranspositionTable_getEvaluationFromKey(currentPosition.key, depth, alpha, beta);
    if (ttEval != LOOKUP_FAILED) {
        transpositionTableHits++;
        return ttEval;
    }

    if (depth == 0) {
        leafNodes++;
        // Negamax needs a relative evaluation, so positive means good for color to go and vice-versa
        int whoToMove = currentPosition.colorToGo == WHITE ? 1 : -1;
        int evaluation = Bot_staticEvaluation() * whoToMove;
        return evaluation;
    }

    // We want to have the best move from this depth
    Move bestMove = NULL_MOVE;
    int bestEval = MINUS_INFINITY;

    int nbOfMoves;
    Move validMoves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    MoveHandler_getValidMoves(validMoves, &nbOfMoves, currentPosition);

    posHistory[currentDepth - depth] = currentPosition;

    for (int i = 0; i < nbOfMoves; i++) {
        Move move = validMoves[i];
        MoveHandler_playMove(move, &currentPosition, true);

        // We switch alpha and beta, because alpha is the lower bound for the color to play 
        // but it is the upper bound for the other color. Opposite is true for beta
        // so we need to switch them we it is the opposite color to play
        int eval = -alpha_beta_negamax(-beta, -alpha, depth - 1);

        memcpy(&currentPosition, &posHistory[currentDepth - depth], sizeof(ChessPosition));

        // Update the best evaluation that we found
        if (eval > bestEval) {
            bestMove = move;
            bestEval = eval;
        }

        // Update the best move that we found from the previous depth
        // or this depth if we have already updated alpha during this depth search
        if (eval > alpha) alpha = eval;

        // Fail-soft beta cutoff: This move is too good, our opponent will not allow it
        if (eval >= beta) break;

        if (endSearch) return 0;
    }

    EntryType type;
    if (bestEval <= startingAlpha) type = UPPER_BOUND;
    else if (bestEval >= beta) type = LOWER_BOUND; 
    else type = EXACT;
    
    TranspositionTable_recordEntry(currentPosition.key, currentDepth, type, bestMove, bestEval);

    return bestEval;
}

Move Bot_think() {
    RepetitionTable_setCurrentIndexAsRootPosition();

    bestMoveFromCurrentDepthSearch = NULL_MOVE;
    bestMoveFromFullDepthSearch = NULL_MOVE;

    Move moves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    int nbMoves;
    MoveHandler_getValidMoves(moves, &nbMoves, currentPosition);

    // The first element is the root position
    posHistory[0] = currentPosition;

    for (int depth = 1; depth < MAXIMUM_DEPTH; depth++) {
        currentDepth = depth;
        // At every depth, we need to reset the bestEvalFromSearch to MINUS_INFINITY
        // because the result from a lower depth are irrelevant when searching at a higher depth
        bestEvalFromCurrentSearch = MINUS_INFINITY;

        // debug information
        totalNodes = 0;
        leafNodes = 0;
        transpositionTableHits = 0;

        // Check endSearch before the long search loop
        if (endSearch) goto stop_search;

        for (int index = 0; index < nbMoves; index++) {
            Move move = moves[index];
            MoveHandler_playMove(move, &currentPosition, true);

            // We input the depth minus 1 because we already make one move with the root negamax loop
            // We need to do the negative of negamax because a good score for our opponent is a bad score for us
            int score = -alpha_beta_negamax(MINUS_INFINITY, INFINITY, depth - 1);

            // Check endSearch before using the calculated score because it could be 0 
            // if endSearch is triggered when we were searching
            if (endSearch) goto stop_search;


            if (score > bestEvalFromCurrentSearch) {
                bestEvalFromCurrentSearch = score;
                bestMoveFromCurrentDepthSearch = move;
            }

            memcpy(&currentPosition, posHistory, sizeof(ChessPosition));

        }
        RepetitionTable_returnToRootPosition();

        // We have done one full depth search and so we update the full search best move
        bestMoveFromFullDepthSearch = bestMoveFromCurrentDepthSearch;

        // Added this print statement to make it more convenient when debugging the bot
        printf("Depth %d search finished, %u leaf nodes, %u total nodes, %u ttHits, %d best eval, %c%d%c%d best move\n",
            depth,
            leafNodes,
            totalNodes,
            transpositionTableHits,
            bestEvalFromCurrentSearch,
            'a' + file(Move_fromSquare(bestMoveFromFullDepthSearch)),
            8 - rank(Move_fromSquare(bestMoveFromCurrentDepthSearch)),
            'a' + file(Move_toSquare(bestMoveFromFullDepthSearch)),
            8 - rank(Move_toSquare(bestMoveFromCurrentDepthSearch))
        );

        if (endSearch) goto stop_search;
    }

stop_search:
    RepetitionTable_returnToRootPosition();
    return bestMoveFromFullDepthSearch;
}
