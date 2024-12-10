#ifndef BE5C3626_D8A7_4BB7_B7D4_8F70EF4329DB
#define BE5C3626_D8A7_4BB7_B7D4_8F70EF4329DB

#include <stdbool.h>

#define BUFFER_SIZE (256)

/**
 * @brief Processes a uci command
 * 
 * @param command The uci command
 * @return true if no error and no quit command occured
 * @return false if an error or a quit command occured  
 */
bool processUCICommand(char command[BUFFER_SIZE]);

#endif /* BE5C3626_D8A7_4BB7_B7D4_8F70EF4329DB */
