#ifndef C52171AC_10DB_4019_994F_E4AC71EEB71E
#define C52171AC_10DB_4019_994F_E4AC71EEB71E

#include <stdbool.h>

#include "../../../engine/src/state/ChessPosition.h"
#include "../../../engine/src/state/Move.h"

#include "../AppState.h"

/**
 * @brief Opaque struct which contains information
 * used for communicating with the engine
 * 
 */
typedef struct EngineCommunication EngineCommunication;

/**
 * @brief Initializes the communication with the engine
 * 
 * IMPORTANT: The engine needs to be flushing its ouput after every message
 * so that we can communication properly with it. If not this function and
 * all the other in this file will hang
 * 
 * @param enginePath The path to the engine
 * @param logFilePath The path of the log file to the engine
 * @return EngineCommunication* A heap pointer to the data needed to communicate with the engine
 */
EngineCommunication* UCIEngine_initialize(const char *enginePath, const char* logFilePath);

/**
 * @brief Terminates the communication with the uci engine
 * 
 * @param engineCommunication The communication information to the engine. 
 * Note: This function will free the engineCommunication pointer.
 */
void UCIEngine_terminate(EngineCommunication* engineCommunication);

/**
 * @brief Sends a command to the engine. The command does not need 
 * to finish with a '\n' character, the function sends one after 
 * sending the command string.
 * 
 * @param engineCommunication The communication information to the engine
 * @param command The command to send to the engine
 */
void UCIEngine_sendCommand(EngineCommunication* engineCommunication, const char* command);

/**
 * @brief Reads a response from the bot. Will halt if there was no data sent 
 * so be careful when calling it. The data string will be overwritten by the 
 * response. It also needs to be a heap allocated string since the function will
 * be calling realloc on it.
 * 
 * @param engineCommunication The communication information to the engine
 * @param data A heap allocated string which will contain the response
 * @param capacity The capacity of this heap allocated string
 * @return char* A heap allocated string containing the response. It is the caller's duty to free it
 */
char* UCIEngine_readResponse(EngineCommunication* engineCommunication, char* data, int capacity);

/**
 * @brief Get the best move according to the engine. Note that the flags
 * of the move are not correctly sent so you need to call MoveHandler_correctMoveFlag
 * with the position the move will be made in.
 * 
 * @param engineCommunication The communication information to the engine
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
Move UCIEngine_bestMoveFromTimeControls(EngineCommunication* engineCommunication,
    ChessPosition startingPosition, Move* movesPlayed, int numMoves, 
    TimeControl_MS wtime, TimeControl_MS btime, TimeControl_MS winc, TimeControl_MS binc, int movesToGo);

/**
 * @brief Get the best move according to the engine with a limited think time.
 * Note that the flags of the move are not correctly sent so you need to call 
 * MoveHandler_correctMoveFlag with the position the move will be made in.
 * 
 * @param engineCommunication The communication information to the engine
 * @param startingPosition The position the game started with
 * @param movesPlayed The moves played in this game
 * @param numMoves The number of moves played in the game
 * @param timeToThink The amount of time in milliseconds the bot can think
 * @return Move The best move according to the bot
 */
Move UCIEngine_bestMoveTimed(EngineCommunication* engineCommunication,
    ChessPosition startingPosition, Move* movesPlayed, int numMoves, TimeControl_MS timeToThink);

#endif /* C52171AC_10DB_4019_994F_E4AC71EEB71E */
