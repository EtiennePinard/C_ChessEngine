#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "UCICommandProcessing.h"
#include "utils/Utils.h"

// TODO: Make this the global current chessGame we are analyzing
// TODO: Also check if it is more performant to have a pointer or a value 
ChessGame chessgame = (ChessGame) { 0 };

// From: https://stackoverflow.com/a/2661788
static void stringToLower(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

static void sendResponse(char *response) {
    printf("%s\n", response);
}

// Here first word refers to the first letters before a space
static void firstWord(char command[BUFFER_SIZE], char buffer[BUFFER_SIZE]) {
    // Here we assume that BUFFER_SIZE == 256 which is U8_MAX
    u8 index;
    for (index = 0; command[index] != ' ' && command[index] != '\n'; index++) {
        buffer[index] = command[index];
    }
}

char pieceToFenChar(Piece piece) {
    char result;
    switch (pieceType(piece)) {
        case KING:
        result = 'k';
        break;
        case QUEEN:
        result = 'q';  
        break;
        case ROOK: 
        result = 'r';       
        break;
        case BISHOP:
        result = 'b';   
        break;
        case KNIGHT:
        result = 'n';   
        break;
        case PAWN:
        result = 'p';
        break;
        default:
        return ' ';
        break;
    }
    if (pieceColor(piece) == WHITE) {
        result = toupper(result);
    }
    return result;
}

static void processDCommand() {
  for (int index = 0; index < BOARD_SIZE; index++) {

    if ((index % 8) == 0) {
      printf("%d ", 8 - index / 8);
    }

    Piece pieceAtPosition = pieceAtIndex(chessgame.currentPosition.board, index);
    printf("[%c]", pieceToFenChar(pieceAtPosition));
    printf((index + 1) % 8 == 0 ? "\n" : " ");
  }

  printf("  ");
  for (int i = 0; i < BOARD_LENGTH; i++) {
    printf(" %c  ", 'a' + i);
  }
  printf("\n");
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
        processDCommand();
    } else {
        sendResponse("Command invalid or not supported by this engine");
    }
    
    return true;
}