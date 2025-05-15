#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "UCICommandProcessing.h"

#include "state/EngineState.h"
#include "state/Move.h"

#include "moveHandler/MovePlayer.h"
#include "moveHandler/MoveGenerator.h"

#include "bot/Bot.h"

#include "utils/Constants.h"
#include "utils/Math.h"
#include "utils/FenString.h"

// Thank you to https://stackoverflow.com/a/1516384
void sendResponse(const char *format, ...) {
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

#define LINE_SEPARATOR "+---+---+---+---+---+---+---+---+"
#define COLUMN_SEPARATOR "|"
#define BOARD_PREFIX " "

static void processDCommand() {
    sendResponse("\n");
    sendResponse(BOARD_PREFIX LINE_SEPARATOR "\n" BOARD_PREFIX);

    for (int index = 0; index < BOARD_SIZE; index++) {

        
        sendResponse(COLUMN_SEPARATOR " ");
        
        Piece pieceAtPosition = Board_pieceAtIndex(ourCurrentPosition.board, index);
        sendResponse("%c ", pieceToFenChar(pieceAtPosition));

        if (((index + 1) & 0b111) == 0) {
            sendResponse(COLUMN_SEPARATOR " %d\n", 8 - (index >> 3));
            sendResponse(BOARD_PREFIX LINE_SEPARATOR "\n" BOARD_PREFIX);
        }
    }

    for (int i = 0; i < BOARD_LENGTH; i++) {
        sendResponse(BOARD_PREFIX BOARD_PREFIX "%c" BOARD_PREFIX, 'a' + i);
    }
    sendResponse("\n");
}

static Move findMatchingMove(Move moveToMatch) {
    Move moveToMake = NULL_MOVE;
    Move moves[256];
    int moveCount;
    MoveHandler_getValidMoves(moves, &moveCount, ourCurrentPosition);
    for (int index = 0; index < moveCount; index++) {
        Move move = moves[index];
        if (Move_fromSquare(move) == Move_fromSquare(moveToMatch) && 
            Move_toSquare(move) == Move_toSquare(moveToMatch) && 
            (Move_flag(moveToMatch) == NOFLAG || Move_flag(move) == Move_flag(moveToMatch))) {
            moveToMake = move;
            break;
        }
    }
    return moveToMake;
}

static void processPlayCommand(Tokens *tokens) {
    if (tokens->length == 1) {
        // The command is just: play
        // In this case we just do nothing
        return;
    }
    size_t tokenIndex = 1;
    while (tokenIndex < tokens->length) {        
        Move moveToMake = findMatchingMove(string_longAlgebraicToMove(tokens->tokens[tokenIndex]));

        if (moveToMake == NULL_MOVE) {
            sendResponse("The move `%s` cannot be made from the current position, aborting play command\n", tokens->tokens[tokenIndex]);
            break;
        }
        MoveHandler_playMove(moveToMake, &ourCurrentPosition, true);

        tokenIndex++;
    }

}

#define STARTPOS_LENGTH (9)
// Format: 'position startpos moves e2e4 e7e5'
// Or: 'position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves e2e4 e7e5'
// Note: 'moves' section is optional
static void processPositionCommand(Tokens *tokens) {

    if (tokens->length == 1) {
        // The command is just: position
        // In this case we just do nothing
        return;
    }
    
    size_t tokenIndex = 1;

    string_toLower(tokens->tokens[tokenIndex]);

    if (string_compareStrings(tokens->tokens[tokenIndex], "startpos")) {
        char initFenCopy[] = INITIAL_FEN;
        if (!FenString_setChessPositionFromFenString(initFenCopy, &ourCurrentPosition)) {
            sendResponse("ERROR: While initializing the starting position\n");
            return;
        }
    } else if (string_compareStrings(tokens->tokens[tokenIndex], "fen")) {

        if (tokens->length < 8) {
            sendResponse("ERROR: Fen string is not long enough in the position command\n");
            return;
        }

        tokenIndex++;
        char *fenStringTokenize[6];
        fenStringTokenize[0] = tokens->tokens[tokenIndex]; tokenIndex++; // fen board
        fenStringTokenize[1] = tokens->tokens[tokenIndex]; tokenIndex++; // color to go
        fenStringTokenize[2] = tokens->tokens[tokenIndex]; tokenIndex++; // castling perm
        fenStringTokenize[3] = tokens->tokens[tokenIndex]; tokenIndex++; // en passant target square
        fenStringTokenize[4] = tokens->tokens[tokenIndex]; tokenIndex++; // fifty move rule
        fenStringTokenize[5] = tokens->tokens[tokenIndex]; // moves made

        Tokens fenTokens = {
            .length = 6,
            .tokens = fenStringTokenize
        };

        if (!FenString_setChessPositionFromTokens(&fenTokens, &ourCurrentPosition)) {
            sendResponse("ERROR: While initializing the position fen string\n");
            return;
        }
    } else {
        sendResponse("ERROR: Invalid option `%s` in position command\n", tokens->tokens[tokenIndex]);
        return;
    }
    tokenIndex++;
    if (tokenIndex == tokens->length) {
        // There is no `moves` part to this position command
        return;
    }
    
    string_toLower(tokens->tokens[tokenIndex]);
    if (!string_compareStrings(tokens->tokens[tokenIndex], "moves")) { 
        sendResponse("ERROR: Invalid `moves` option of position command `%s`\n", tokens->tokens[tokenIndex]);
        return;
    }

    Tokens moveTokens = {
        // We don't increment the tokenIndex because the processPlayCommand function discards the first token
        .tokens = tokens->tokens + tokenIndex,
        .length =  tokens->length - tokenIndex
    };
    processPlayCommand(&moveTokens);
}

// The function that the timer thread will execute
static void* timerThread(void* arg) {
    u64 time_limit_MS = *((u64*) arg);
    usleep(time_limit_MS * 1000);
    endSearch = true;
    return NULL;
}

// Command format: go <wtime> <btime> <winc> <binc> <movestogo> <movetime>
static void processGoCommand(Tokens *tokens) {
    
    // TODO: Find a way to calculate the time to take to search for the optimal moves

    size_t tokenIndex = 0;
    while (tokenIndex < tokens->length) {
        // TODO: handle the options: <wtime> <btime> <winc> <binc> <movestogo> <movetime>
        tokenIndex++;
    }
    u64 durationInMilliseconds = 100;

    Bot_provideGameStateForBot(&ourCurrentPosition);
    
    pthread_t timer;
    if (pthread_create(&timer, NULL, timerThread, &durationInMilliseconds) != 0) {
        sendResponse("ERROR: Failed to create a timer thread, exiting the program\n");
        exit(EXIT_FAILURE);
    }

    Move bestMove = Bot_think();

    // Wait for the timer thread to finish
    pthread_join(timer, NULL);

    // Setting endsearch back for the next go command
    endSearch = false;

    char bestMoveLongAlgebraicNotation[6];
    string_moveToLongAlgebraic(bestMove, bestMoveLongAlgebraicNotation);

    // No need for a stop command since we are that cool
    sendResponse("bestmove %s\n", bestMoveLongAlgebraicNotation);
}

/*
These are all the uci commands this engine supports:
    uci (done)
    isready (done)
    ucinewgame (don't really know what to do so imma say done)
    position (done)
    go
    stop
    quit (done)
    d (done)
*/
#define MAX_UCI_COMMAND_WITH_OPTION_SIZE (9)
bool processUCICommand(char *command) {

    Tokens tokens;
    size_t nbTokens = string_removeUnecessarySpaces(command);
    char *tokens_arr[nbTokens];
    tokens.length = nbTokens;
    tokens.tokens = tokens_arr;
    string_tokenizeStringBySpace(command, &tokens);
    
    if (tokens.length == 0) {
        // user sent an empty message, returning but not exiting
        return true;
    }

    char* messageType = tokens.tokens[0];

    string_toLower(messageType);

    if (string_compareStrings(messageType, "quit")) {
        return false;
    }

    if (string_compareStrings(messageType, "uci")) {

        sendResponse("id name %s %s\n", ENGINE_NAME, VERSION);
        sendResponse("id author %s\n", AUTHOR);
        sendResponse("\nuciok\n");
    
    } else if (string_compareStrings(messageType, "isready")) {
        sendResponse("readyok\n");

    } else if (string_compareStrings(messageType, "ucinewgame")) {
        // I guess we gonna handle uci new game when it is necessary   

    } else if (string_compareStrings(messageType, "position")) {
        processPositionCommand(&tokens);
    
    } else if (string_compareStrings(messageType, "play")) {
        processPlayCommand(&tokens);

    } else if (string_compareStrings(messageType, "go")) {
        processGoCommand(&tokens);
    
    } else if (string_compareStrings(messageType, "stop")) {
        // Stop the bot from thinking
    
    } else if (string_compareStrings(messageType, "d")) {
        processDCommand();
    } else {
        sendResponse("ERROR: Command `%s` invalid or not supported by this engine\n", command);
    }
    
    return true;
}