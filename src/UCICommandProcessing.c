#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>

#include "UCICommandProcessing.h"
#include "engine/ChessGameEmulator.h"
#include "engine/MoveGenerator.h"
#include "state/Move.h"
#include "utils/Utils.h"

#include "../testing/LogChessStructs.h"

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
    switch (Piece_type(piece)) {
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
    if (Piece_color(piece) == WHITE) {
        result = toupper(result);
    }
    return result;
}

static void processDCommand() {
  for (int index = 0; index < BOARD_SIZE; index++) {

    if ((index % 8) == 0) {
      printf("%d ", 8 - index / 8);
    }

    Piece pieceAtPosition = Board_pieceAtIndex(chessgame.currentPosition.board, index);
    printf("[%c]", pieceToFenChar(pieceAtPosition));
    printf((index + 1) % 8 == 0 ? "\n" : " ");
  }

  printf("  ");
  for (int i = 0; i < BOARD_LENGTH; i++) {
    printf(" %c  ", 'a' + i);
  }
  printf("\n");
}

static Move findMatchingMove(int startSquare, int endSquare, Flag moveFlag) {
    Move moveToMake = (Move) 0;
    Move moves[256];
    int moveCount;
    Engine_getValidMoves(moves, &moveCount, chessgame.currentPosition);
    for (int index = 0; index < moveCount; index++) {
        Move move = moves[index];
        if (Move_fromSquare(move) == startSquare && 
            Move_toSquare(move) == endSquare && 
            (moveFlag == NOFLAG || Move_flag(move) == moveFlag)) {
            moveToMake = move;
            break;
        }
    }
    return moveToMake;
}

#define STARTPOS_LENGTH (9)
// Format: 'position startpos moves e2e4 e7e5'
// Or: 'position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves e2e4 e7e5'
// Note: 'moves' section is optional
static void processPositionCommand(char *command, size_t firstSpaceIndex) {
    size_t commandLength = strlen(command);
    // Smallest valid position command is: position startpos, with 17 non-zero characters
    // If we have a fen it is: position fen 8/8/8/8/8/8/8/8 w - - 0 1 with 38 non-zero characters 
    if (commandLength < 17) {
        sendResponse("ERROR: Invalid position command `%s`\n", command);
        return;
    }

    // Note: splitData is big enough to hold all notated moves
    char splitData[STARTPOS_LENGTH] = { 0 };
    size_t spaceIndex = string_nextSpaceCharacterFromIndex(command, firstSpaceIndex + 1); // After startpos or fen
    memcpy(splitData, command + firstSpaceIndex + 1, min(spaceIndex - (firstSpaceIndex + 1), (size_t) STARTPOS_LENGTH));
    string_toLower(splitData);

    if (string_compareStrings(splitData, "startpos")) {
        if (!Game_setupChesGame(&chessgame, &chessgame.currentPosition, INITIAL_FEN, 0, 0)) {
            sendResponse("ERROR: While initializing the starting position");
            return;
        }
    } else if (string_compareStrings(splitData, "fen")) {
        if (!Game_setupChesGame(&chessgame, &chessgame.currentPosition, command + spaceIndex + 1, 0, 0)) {
            sendResponse("ERROR: While initializing the position fen string");
            return;
        }
        // Advancing the space index after the fen string
        spaceIndex = string_nextSpaceCharacterFromIndex(command, spaceIndex + 1); // after fen board
        spaceIndex = string_nextSpaceCharacterFromIndex(command, spaceIndex + 1); // after color to go
        spaceIndex = string_nextSpaceCharacterFromIndex(command, spaceIndex + 1); // after castling perm
        spaceIndex = string_nextSpaceCharacterFromIndex(command, spaceIndex + 1); // after en passant target square
        spaceIndex = string_nextSpaceCharacterFromIndex(command, spaceIndex + 1); // after fifty move rule
        spaceIndex = string_nextSpaceCharacterFromIndex(command, spaceIndex + 1); // after moves made
    } else {
        sendResponse("ERROR: Invalid option `%s` in position command\n", splitData);
        return;
    }
    if (spaceIndex == commandLength) {
        // There is no `moves` part to this position command
        return;
    }

    // Handle moves command
    size_t oldSpaceIndex = spaceIndex;
    spaceIndex = string_nextSpaceCharacterFromIndex(command, oldSpaceIndex + 1); // after moves string
    memcpy(splitData, command + oldSpaceIndex + 1, min(spaceIndex - (oldSpaceIndex + 1), (size_t) 5));
    splitData[5] = '\0';
    string_toLower(splitData);
    if (!string_compareStrings(splitData, "moves")) { 
        sendResponse("ERROR: Invalid `moves` option of position command `%s`\n", splitData);
        return;
    }

    while (spaceIndex < commandLength) {
        oldSpaceIndex = spaceIndex;
        spaceIndex = string_nextSpaceCharacterFromIndex(command, oldSpaceIndex + 1); // after move 
        size_t algebraicMoveLength = spaceIndex - (oldSpaceIndex + 1);
        if (algebraicMoveLength != 4 && algebraicMoveLength != 5) {
            command[oldSpaceIndex + 1 + algebraicMoveLength] = '\0';
            sendResponse("ERROR: Invalid move `%s`, aborting position command\n", command + oldSpaceIndex + 1);
            break;
        }

        int startSquare = string_algebraicToIndex((char[]) { command[oldSpaceIndex + 1], command[oldSpaceIndex + 2], '\0' });
        int endSquare = string_algebraicToIndex((char[]) { command[oldSpaceIndex + 3], command[oldSpaceIndex + 4], '\0' });
        if (startSquare == -1 || endSquare == -1) {
            command[oldSpaceIndex + 1 + algebraicMoveLength] = '\0';
            sendResponse("ERROR: Invalid move `%s`, aborting position command\n", command + oldSpaceIndex + 1);
            break;
        }
        Flag moveFlag = NOFLAG;
        if (algebraicMoveLength == 5) {
            char fenCharToPromote = command[oldSpaceIndex + 5];
            if (fenCharToPromote == 'q') {
                moveFlag = PROMOTE_TO_QUEEN;
            } else if (fenCharToPromote == 'n') {
                moveFlag = PROMOTE_TO_KNIGHT;
            } else if (fenCharToPromote == 'r') {
                moveFlag = PROMOTE_TO_ROOK;
            } else if (fenCharToPromote == 'b') {
                moveFlag = PROMOTE_TO_BISHOP;
            } else {
                command[oldSpaceIndex + 1 + algebraicMoveLength] = '\0';
                sendResponse("ERROR: Invalid move `%s`, aborting position command\n", command + oldSpaceIndex + 1);
                break;
            }
        }
        Move moveToMake = findMatchingMove(startSquare, endSquare, moveFlag);

        if (moveToMake == (Move) 0) {
            command[oldSpaceIndex + 1 + algebraicMoveLength] = '\0';
            sendResponse("The move `%s` cannot be made from the current position, aborting position command\n", command + oldSpaceIndex + 1);
            break;
        }
        Engine_playMove(moveToMake, &chessgame);
    }

}

/*
These are all the uci commands this engine supports:
    uci (done)
    isready (done)
    ucinewgame 
    position (done)
    go
    stop
    quit
    d
*/
#define MAX_UCI_COMMAND_WITH_OPTION_SIZE (9)
bool processUCICommand(char *command) {
    size_t firstSpaceIndex = string_nextSpaceCharacterFromIndex(command, 0);
    char messageType[MAX_UCI_COMMAND_WITH_OPTION_SIZE] = { 0 };
    memcpy(messageType, command, min((size_t) MAX_UCI_COMMAND_WITH_OPTION_SIZE, firstSpaceIndex));
    string_toLower(messageType);

    if (string_compareStrings(messageType, "quit")) {
        return false;
    }

    if (string_compareStrings(messageType, "uci")) {
        sendResponse("uciok");
    
    } else if (string_compareStrings(messageType, "isready")) {
        sendResponse("readyok");

    } else if (string_compareStrings(messageType, "ucinewgame")) {
        // I guess we gonna handle uci new game when it is necessary   
    
    } else if (string_compareStrings(messageType, "position")) {
        processPositionCommand(command, firstSpaceIndex);
    
    } else if (string_compareStrings(messageType, "go")) {
        sendResponse("Command is: %s\n", command);
    
    } else if (string_compareStrings(messageType, "stop")) {
        // Stop the bot from thinking
    
    } else if (string_compareStrings(messageType, "d")) {
        processDCommand();
    } else {
        sendResponse("ERROR: Command `%s` invalid or not supported by this engine\n", command);
    }
    
    return true;
}