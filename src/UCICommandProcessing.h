#ifndef BE5C3626_D8A7_4BB7_B7D4_8F70EF4329DB
#define BE5C3626_D8A7_4BB7_B7D4_8F70EF4329DB

#include <stdbool.h>
#include "state/GameState.h"
#include "utils/CharBuffer.h"

/**
 * @brief Processes a uci command
 * 
 * @param command The uci command
 * @return true if no error and no quit command occurred
 * @return false if an error or a quit command occurred  
 */
bool processUCICommand(char *command);

/**
 * @brief Send a message to the user using the printf API
 * 
 * @param format The format string 
 * @param ... The values to be put in the format string
 */
void sendResponse(const char *format, ...);

#endif /* BE5C3626_D8A7_4BB7_B7D4_8F70EF4329DB */
