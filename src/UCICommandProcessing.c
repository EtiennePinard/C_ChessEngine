#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "UCICommandProcessing.h"
#include "utils/Utils.h"

// TODO: Make this the global current chessGame we are analyzing
// TODO: Also check if it is more performant to have a pointer or a value 
ChessGame chessgame = (ChessGame) { 0 };

// Thank you to https://stackoverflow.com/a/1516384
static void sendResponse(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

static char pieceToFenChar(Piece piece) {
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

// Format: 'position startpos moves e2e4 e7e5'
// Or: 'position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves e2e4 e7e5'
// Note: 'moves' section is optional
static void processPositionCommand(String command, size_t firstSpaceIndex) {
    size_t commandLength = string_length(command);
    // Smallest valid position command is: position startpos, with 17 non-zero characters
    // If we have a fen it is: position fen 8/8/8/8/8/8/8/8 w - - 0 1 with 38 non-zero characters 
    if (commandLength < 17) {
        sendResponse("Invalid position command %s\n", command);
        return;
    }

    String buffer;
    size_t spaceIndex = string_nextSpaceTokenStartingAtIndex(command, firstSpaceIndex + 1, buffer);
    

    if (string_equalsCharArray(buffer, "startpos")) {
        setupChesGame(&chessgame, &chessgame.currentPosition, INITIAL_FEN, 0, 0);
    } else if (string_equalsCharArray(buffer, "fen")) {
        if (!setupChesGame(&chessgame, &chessgame.currentPosition, command + spaceIndex + 1, 0, 0)) {
            sendResponse("ERROR: While initializing the position fen string");
        }
    } else {
        sendResponse("Invalid position command %s\n", command);
        return;
    }
    
    if (spaceIndex == commandLength - 1) {
        // There is no `move` part to this position command
        return;
    }

    // Handle move command

    // Note: We can only handle about 26 moves in our 256 length string
    // so like this might be an issue later. Just keep that in mind
    

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
bool processUCICommand(String command) {
    string_toLower(command);

    // I know this is too much but we are safe from any overflow with this
    String messageType;
    size_t firstSpaceIndex = string_nextSpaceTokenStartingAtIndex(command, 0, messageType);

    if (string_equalsCharArray(command, "quit")) {
        return false;

    } else if (string_equalsCharArray(command, "uci")) {
        sendResponse("uciok");
    
    } else if (string_equalsCharArray(command, "isready")) {
        sendResponse("readyok");

    } else if (string_equalsCharArray(command, "ucinewgame")) {
        // I guess we gonna handle uci new game when it is necessary   
    
    } else if (string_equalsCharArray(messageType, "position")) {
        processPositionCommand(command, firstSpaceIndex);
    
    } else if (string_equalsCharArray(messageType, "go")) {
        printf("Command is: %s\n", command);
    
    } else if (string_equalsCharArray(command, "stop")) {
        // Stop the bot from thinking
    
    } else if (string_equalsCharArray(command, "d")) {
        processDCommand();
    } else {
        sendResponse("Command `%s` invalid or not supported by this engine\n", command);
    }
    
    return true;
}