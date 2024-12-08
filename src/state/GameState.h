#ifndef BDF83061_7504_461E_BCDD_602085692048
#define BDF83061_7504_461E_BCDD_602085692048

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "Board.h"

#define PREVIOUS_STATE_CAPACITY (1 << 9)

typedef u64 ZobristKey;

typedef struct ChessPosition {
    Board board;
    PieceCharacteristics colorToGo;
    char castlingPerm; // 4 bits are used. The first bit is for white king side, second bit is for white queen side and pattern continues but for black
    int enPassantTargetSquare; 
    int turnsForFiftyRule; 
    int nbMoves;
    ZobristKey key;
} ChessPosition;

/**
 * Represents a game of chess.
 * currentState holds all information of a position (same information that is in a fen string)
 * Zobrist keys are used to quickly compute repetitions
*/
typedef struct ChessGame {
    ChessPosition currentPosition;

    /**
     * The longest game possible will be 2^32 milliseconds, which is
     * 49 days 17 hours 2 minutes and 47.3 seconds (thanks WolframAlpha).
     * This should be plently of time to play a pretty long game.
     */
    u32 whiteTimeMs;
    u32 blackTimeMs;

    /*
        IMPORTANT! Notice that previousStates is initialized to 512. 
        This means that we only support games which are 512 moves long. 
        This could be like a huge flaw in the engine, since legal games 
        can theoriticaly go to 6000 moves. 
        
        However it is important to note that 6000 * 64 bit * 1kb / 1024 bit = 375kb, 
        which is a bit too much memory to store on the stack for my taste.
        With 512 moves, we only store 32 kb, which is a lot, I guess, but it's fine by me
        
        Also like the average chess game is about 40 moves, so its not
        like were gonna run into this issue anytime soon (I really hope so)
     */
    ZobristKey previousPositions[PREVIOUS_STATE_CAPACITY];
    u16 previousPositionsCount;
} ChessGame;

/**
 * Setups a chess game from the position specified by the fen string
*/
bool setupChesGame(ChessGame *result,
                   ChessPosition *currentPosition,
                   const char *fenString,
                   u32 whiteTimeInMs, u32 blackTimeInMs);

/**
 * @brief Returns true if the current position aleady happened twice in the game
 * 
 * @param game The game to look for repeated positions
 * @return true If the current position already happened twice
 * @return false If the current position did not already happened twice
 */
bool isThereThreeFoldRepetition(ChessGame* game);

#endif /* BDF83061_7504_461E_BCDD_602085692048 */
