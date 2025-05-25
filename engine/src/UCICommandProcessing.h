#ifndef BE5C3626_D8A7_4BB7_B7D4_8F70EF4329DB
#define BE5C3626_D8A7_4BB7_B7D4_8F70EF4329DB

#include <stdbool.h>
#include "state/GameState.h"
#include "utils/CharBuffer.h"

/**
 * @brief Processes a uci command
 * 
 * These are all the supported uci commands:
 *   uci
 *   isready
 *   ucinewgame
 *   position 
 *   go 
 *   stop
 *   quit
 *   d
 * 
 * @param command The uci command
 * @return true if no error and no quit command occurred
 * @return false if an error or a quit command occurred  
 * 
 * @see https://gist.github.com/DOBRO/2592c6dad754ba67e6dcaec8c90165bf
 */
bool UCI_processUCICommand(char *command);

/**
 * @brief Send a message to the user using the printf API
 * 
 * @param format The format string 
 * @param ... The values to be put in the format string
 */
void UCI_sendResponse(const char *format, ...);

/**
 * @brief Cleans up the data used for the uci portion of the engine
 * 
 */
void UCI_terminate();

#endif /* BE5C3626_D8A7_4BB7_B7D4_8F70EF4329DB */
