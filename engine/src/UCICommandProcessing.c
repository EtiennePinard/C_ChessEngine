#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "state/EngineState.h"
#include "state/Move.h"

#include "moveHandler/MovePlayer.h"
#include "moveHandler/MoveGenerator.h"

#include "bot/Bot.h"
#include "bot/RepetitionTable.h"
#include "bot/TranspositionTable.h"

#include "utils/Constants.h"
#include "utils/Math.h"
#include "utils/FenString.h"

#include "UCICommandProcessing.h"

// Thank you to https://stackoverflow.com/a/1516384
void UCI_sendResponse(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

static void processUCICommand() {
    UCI_sendResponse("id name %s %s\n", ENGINE_NAME, VERSION);
    UCI_sendResponse("id author %s\n", AUTHOR);
    UCI_sendResponse("\nuciok\n");
}

static void processIsReadyCommand() {
    UCI_sendResponse("readyok\n");
}

static void processUCINewGameCommand() {
    // Clearing the transposition table and repetition table
    RepetitionTable_clear();
    TranspositionTable_clear();
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
    UCI_sendResponse("\n");
    UCI_sendResponse(BOARD_PREFIX LINE_SEPARATOR "\n" BOARD_PREFIX);

    for (int index = 0; index < BOARD_SIZE; index++) {


        UCI_sendResponse(COLUMN_SEPARATOR " ");

        Piece pieceAtPosition = Board_pieceAtIndex(ourCurrentPosition.board, index);
        UCI_sendResponse("%c ", pieceToFenChar(pieceAtPosition));

        if (((index + 1) & 0b111) == 0) {
            UCI_sendResponse(COLUMN_SEPARATOR " %d\n", 8 - (index >> 3));
            UCI_sendResponse(BOARD_PREFIX LINE_SEPARATOR "\n" BOARD_PREFIX);
        }
    }

    for (int i = 0; i < BOARD_LENGTH; i++) {
        UCI_sendResponse(BOARD_PREFIX BOARD_PREFIX "%c" BOARD_PREFIX, 'a' + i);
    }
    UCI_sendResponse("\n");

    char fen[MAX_FEN_STRING_SIZE];
    FenString_chessPositionToFenString(ourCurrentPosition, fen);
    UCI_sendResponse("%s\n", fen);
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

static void processPlayCommand(Tokens* tokens) {
    if (tokens->length == 1) {
        // The command is just: play
        // In this case we just do nothing
        return;
    }
    size_t tokenIndex = 1;
    while (tokenIndex < tokens->length) {
        Move moveToMake = findMatchingMove(string_longAlgebraicToMove(tokens->tokens[tokenIndex]));

        if (moveToMake == NULL_MOVE) {
            UCI_sendResponse("The move `%s` cannot be made from the current position, aborting play command\n", tokens->tokens[tokenIndex]);
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
static void processPositionCommand(Tokens* tokens) {

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
            UCI_sendResponse("ERROR: While initializing the starting position\n");
            return;
        }
    }
    else if (string_compareStrings(tokens->tokens[tokenIndex], "fen")) {

        if (tokens->length < 8) {
            UCI_sendResponse("ERROR: Fen string is not long enough in the position command\n");
            return;
        }

        tokenIndex++;
        char* fenStringTokenize[6];
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
            UCI_sendResponse("ERROR: While initializing the position fen string\n");
            return;
        }
    }
    else {
        UCI_sendResponse("ERROR: Invalid option `%s` in position command\n", tokens->tokens[tokenIndex]);
        return;
    }

    // Clearing the repetition table since we will add them from the moves command
    RepetitionTable_clear();

    tokenIndex++;
    if (tokenIndex == tokens->length) {
        // There is no `moves` part to this position command
        return;
    }

    string_toLower(tokens->tokens[tokenIndex]);
    if (!string_compareStrings(tokens->tokens[tokenIndex], "moves")) {
        UCI_sendResponse("ERROR: Invalid `moves` option of position command `%s`\n", tokens->tokens[tokenIndex]);
        return;
    }

    Tokens moveTokens = {
        // We don't increment the tokenIndex because the processPlayCommand function discards the first token
        .tokens = tokens->tokens + tokenIndex,
        .length = tokens->length - tokenIndex
    };
    processPlayCommand(&moveTokens);
}

pthread_t searchThread;
pthread_t timerThread;
bool timerThreadRunning = false;
pthread_mutex_t timerThreadMutex = PTHREAD_MUTEX_INITIALIZER;

static void* searchThreadFunction(void* _) {
    (void) _;

    Move bestMove = Bot_think();

    char bestMoveStr[6];
    string_moveToLongAlgebraic(bestMove, bestMoveStr);
    UCI_sendResponse("bestmove %s\n", bestMoveStr);

    return NULL;
}

// The function that the timer thread will execute
// Please note that args is expected to be a heap allocated u64 value
// which will be freed in this function
static void* timerThreadFunction(void* arg) {
    u64 time_limit_MS = *((u64*)arg);
    free(arg);

    // TODO: Maybe usleep is not the best idea
    usleep(time_limit_MS * 1000);

    pthread_mutex_lock(&timerThreadMutex);
    if (timerThreadRunning) {
        endSearch = true;
        timerThreadRunning = false;

        // Since the timerThread ended by itself without a stop command 
        // then these threads will not be joined
        pthread_detach(searchThread);
        pthread_detach(pthread_self()); // We are detaching from ourselves cause POSIX allows it
    }
    pthread_mutex_unlock(&timerThreadMutex);

    return NULL;
}

#define MISSING_VALUE(optionName) \
    if (tokenIndex == tokens->length) { \
        UCI_sendResponse("Missing value for %s\n", optionName); \
        return; \
    } \

#define INVALID_NUM(optionName) \
    if (num == -1) {\
        UCI_sendResponse("ERROR: Invalid %s: `%s`\n", optionName, nextToken); \
        return; \
    } \

static void processGoCommand(Tokens* tokens) {
    // We cannot call another go command while one is already going on
    if (!endSearch) return;

    u64 moveTimeDuration = 0;
    u64 whiteTime = 0;
    u64 blackTime = 0;
    u64 whiteInc = 0;
    u64 blackInc = 0;
    // If we have no tokens we enter infinite search
    bool infiniteMode = tokens->length == 1;

    size_t tokenIndex = 1; // The first token is the go command
    while (tokenIndex < tokens->length) {
        const char* token = tokens->tokens[tokenIndex++];

        // There are only two options that has only one token: infinite and ponder
        const char* nextToken;
        int num;

        if (string_compareStrings(token, "movetime")) {
            MISSING_VALUE("movetime duration");
            nextToken = tokens->tokens[tokenIndex++];
            num = string_parseNumber(nextToken);
            INVALID_NUM("movetime duration");
            moveTimeDuration = (u64)num;
        }
        else if (string_compareStrings(token, "wtime")) {
            MISSING_VALUE("white time");
            nextToken = tokens->tokens[tokenIndex++];
            num = (u64)string_parseNumber(nextToken);
            INVALID_NUM("white time");
            whiteTime = (u64)num;
        }
        else if (string_compareStrings(token, "btime")) {
            MISSING_VALUE("black time");
            nextToken = tokens->tokens[tokenIndex++];
            num = (u64)string_parseNumber(nextToken);
            INVALID_NUM("black time");
            blackTime = (u64)num;
        }
        else if (string_compareStrings(token, "winc")) {
            MISSING_VALUE("white increment");
            nextToken = tokens->tokens[tokenIndex++];
            num = (u64)string_parseNumber(nextToken);
            INVALID_NUM("white increment");
            whiteInc = (u64)num;
        }
        else if (string_compareStrings(token, "binc")) {
            MISSING_VALUE("black increment");
            nextToken = tokens->tokens[tokenIndex++];
            num = (u64)string_parseNumber(nextToken);
            INVALID_NUM("black increment");
            blackInc = (u64)num;
        }
        else if (string_compareStrings(token, "infinite")) {
            infiniteMode = true;
        }

    }

    Bot_provideGameStateForBot(ourCurrentPosition);

    // Starting the search ASAP
    endSearch = false;
    if (pthread_create(&searchThread, NULL, searchThreadFunction, NULL) != 0) {
        UCI_sendResponse("ERROR: Failed to create a search thread, exiting the program\n");
        exit(EXIT_FAILURE);
    }

    // If we are not in infinite mode we need a timer thread to manage the search time
    if (!infiniteMode) {
        // Check if the movetime option was provided if not calculate optimal time
        // Note that if the wtime and btime is 0 then the think time will be the default think time
        if (!moveTimeDuration) {
            bool isWhiteToMove = ourCurrentPosition.colorToGo == WHITE;
            moveTimeDuration = Bot_calculateThinkTime(whiteTime, blackTime, whiteInc, blackInc, isWhiteToMove);
        }
        u64* durationInMilliseconds = malloc(sizeof(u64));
        *durationInMilliseconds = moveTimeDuration;

        pthread_mutex_lock(&timerThreadMutex);
        timerThreadRunning = true;
        pthread_mutex_unlock(&timerThreadMutex);

        if (pthread_create(&timerThread, NULL, timerThreadFunction, durationInMilliseconds) != 0) {
            UCI_sendResponse("ERROR: Failed to create a timer thread, exiting the program\n");
            exit(EXIT_FAILURE);
        }
    }
}

static void processStopCommand() {
    // If we are not searching then we cannot stop a search
    if (endSearch) return;
    endSearch = true;

    if (pthread_join(searchThread, NULL)) {
        UCI_sendResponse("ERROR: Failed to join the search thread, exiting the program\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&timerThreadMutex);
    if (timerThreadRunning) {
        pthread_cancel(timerThread);
        pthread_join(timerThread, NULL);
        timerThreadRunning = false;
    }
    pthread_mutex_unlock(&timerThreadMutex);
}

#define MAX_UCI_COMMAND_WITH_OPTION_SIZE (9)
bool UCI_processUCICommand(char* command) {

    Tokens tokens;
    size_t nbTokens = string_removeUnecessarySpacesAndTabs(command);
    char* tokens_arr[nbTokens];
    tokens.length = nbTokens;
    tokens.tokens = tokens_arr;
    string_tokenizeStringBySpace(command, &tokens);

    if (tokens.length == 0) {
        // user sent an empty message, returning but not exiting
        return true;
    }

    char* messageType = tokens.tokens[0];

    string_toLower(messageType);

    if (string_compareStrings(messageType, "quit")) return false;
    else if (string_compareStrings(messageType, "uci")) processUCICommand();
    else if (string_compareStrings(messageType, "isready")) processIsReadyCommand();
    else if (string_compareStrings(messageType, "ucinewgame")) processUCINewGameCommand();
    else if (string_compareStrings(messageType, "position")) processPositionCommand(&tokens);
    else if (string_compareStrings(messageType, "play")) processPlayCommand(&tokens);
    else if (string_compareStrings(messageType, "go")) processGoCommand(&tokens);
    else if (string_compareStrings(messageType, "stop")) processStopCommand();
    else if (string_compareStrings(messageType, "d")) processDCommand();
    else UCI_sendResponse("ERROR: Command `%s` invalid or not supported by this engine\n", command);

    return true;
}

void UCI_terminate() {
    pthread_mutex_destroy(&timerThreadMutex);
}
