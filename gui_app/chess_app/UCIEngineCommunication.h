#ifndef C52171AC_10DB_4019_994F_E4AC71EEB71E
#define C52171AC_10DB_4019_994F_E4AC71EEB71E

#include <stdbool.h>

#include "AppState.h"

#include "../../engine/src/state/ChessPosition.h"
#include "../../engine/src/state/Move.h"

/**
 * @brief Initializes the communication with the engine
 * 
 * IMPORTANT: The engine needs to be flushing its ouput after every message
 * so that we can communication properly with it.
 * 
 * @param enginePath The path to the engine
 * @return true If everything was initialized properly
 * @return false If an error occurred
 */
bool UCIEngine_initialize(const char *enginePath);

/**
 * @brief Terminates the communication with the uci engine
 * 
 */
void UCIEngine_terminate();

/**
 * @brief Sends a command to the engine. The command does not need 
 * to finish with a '\n' character, the function sends one after 
 * sending the command string.
 * 
 * @param command The command to send to the engine
 */
void UCIEngine_sendCommand(const char* command);

/**
 * @brief Reads a reponse from the bot. Will halt if there was no data sent 
 * so be careful when calling it. The data string will be overwritten by the 
 * response. It also needs to be a heap allocated string since the function will
 * be calling realloc on it.
 * 
 * @param data A heap allocated string which will contain the response
 * @param capacity The capacity of this heap allocated string
 * @return char* A heap allocated string containing the response. It is the caller's duty to free it
 */
char* UCIEngine_readResponse(char* data, int capacity);

/**
 * @brief Get the best move according to the engine. 
 * 
 * @param startingPosition The position the game started with
 * @param movesPlayed The moves played in this game
 * @param numMoves The number of moves played in the game
 * @param wtime The amount of time white has in milliseconds
 * @param btime The amount of time black has in milliseconds
 * @param winc The time increment white has per move in milliseconds
 * @param binc The time increment black has per move in milliseconds
 * @param movesToGo The amount of moves to go before the next time control. 
 * If this value is negative then it will not be sent to the engine
 * @return Move The best move according to the bot
 */
Move UCIEngine_bestMoveFromTimeControls(ChessPosition startingPosition, Move* movesPlayed, int numMoves, TimeControl_MS wtime, TimeControl_MS btime, TimeControl_MS winc, TimeControl_MS binc, int movesToGo);

/**
 * @brief Get the best move according to the engine with a limited think time.
 * 
 * @param startingPosition The position the game started with
 * @param movesPlayed The moves played in this game
 * @param numMoves The number of moves played in the game
 * @param timeToThink The amount of time in milliseconds the bot can think
 * @return Move The best move according to the bot
 */
Move UCIEngine_bestMoveTimed(ChessPosition startingPosition, Move* movesPlayed, int numMoves, TimeControl_MS timeToThink);

#endif /* C52171AC_10DB_4019_994F_E4AC71EEB71E */
