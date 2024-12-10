#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "UCICommandProcessing.h"
#include "utils/Utils.h"

// From: https://stackoverflow.com/a/2661788
void stringToLower(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

void sendResponse(char *response) {
    printf("%s\n", response);
}

// Here first word refers to the first letters before a space
void firstWord(char command[BUFFER_SIZE], char buffer[BUFFER_SIZE]) {
    // Here we assume that BUFFER_SIZE == 256 which is U8_MAX
    u8 index;
    for (index = 0; command[index] != ' ' && command[index] != '\n'; index++) {
        buffer[index] = command[index];
    }
}

/*
These are all the uci commands this engine supports:
    uci
    isready
    ucinewgame
    position
    go
    stop
    quit
    d
*/
bool processUCICommand(char command[BUFFER_SIZE]) {
    stringToLower(command);

    // I know this is too much but we are safe from any overflow with this
    char messageType[BUFFER_SIZE] = { 0 };
    firstWord(command, messageType);

    if (strcmp(command, "quit") == 0) {
        return false;

    } else if (strcmp(command, "uci") == 0) {
        sendResponse("uciok");
    
    } else if (strcmp(command, "isready") == 0) {
        sendResponse("readyok");

    } else if (strcmp(command, "ucinewgame") == 0) {
        // I guess we gonna handle new game when it is necessary   
    
    } else if (strcmp(messageType, "position") == 0) {
        printf("Command is: %s\n", command);
    
    } else if (strcmp(messageType, "go") == 0) {
        printf("Command is: %s\n", command);
    
    } else if (strcmp(command, "stop") == 0) {
        // Stop the bot from thinking
    
    } else if (strcmp(command, "d") == 0) {
        printf("Command is: %s\n", command);
    
    } else {
        sendResponse("Command invalid or not supported by this engine");
    }
    

    return true;
}