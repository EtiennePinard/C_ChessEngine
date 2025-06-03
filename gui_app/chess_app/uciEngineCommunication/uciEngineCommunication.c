#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <string.h>

#include "../../../engine/src/utils/CharBuffer.h"
#include "../../../engine/src/utils/FenString.h"
#include "../../../engine/src/utils/Math.h"
#include "../../../engine/src/moveHandler/MoveGenerator.h"

#include "UCIEngineCommunication.h"

// Including .c files because we are already inside a .c file with a .h header so it doesn't matter
#ifdef __linux__
#include "uciEngineCommunication_posix.c"
#elif _WIN32
#include "uciEngineCommunication_windows.c"
#endif

#define WHILE_LOOP_SAFEGUARD (1000)

FILE* logFile = NULL;

void UCIEngine_sendCommand(const char* command) {
#ifdef __linux__
    UCIEngine_sendCommand_posix(command);
#elif _WIN32
    UCIEngine_sendCommand_windows(command);
#else
    assert(false && "Invalid architecture, this program only supports POSIX or windows");
#endif
    printf("%s\n", command);
    fprintf(logFile, "%s\n", command);
}

#define DEFAULT_BUF_SIZE (128)

char* UCIEngine_readResponse(char* data, int capacity) {
    char* returnValue;
#ifdef __linux__
    returnValue = UCIEngine_readResponse_posix(data, capacity);
#elif _WIN32
    returnValue = UCIEngine_readResponse_windows(data, capacity);
#else
    assert(false && "Invalid architecture, this program only supports POSIX or windows");
#endif
    printf("%s", data);
    fprintf(logFile, "%s", data);
    return returnValue;
}

bool sendUCICommand() {
    UCIEngine_sendCommand("uci");
    int iterationCount = 0;
    int capacity = DEFAULT_BUF_SIZE;
    char* data = malloc(sizeof(char) * capacity);
    while (true) {
        data = UCIEngine_readResponse(data, DEFAULT_BUF_SIZE);
        capacity = strlen(data);
        string_removeUnecessarySpacesAndTabs(data);
        if (string_compareStrings(data, "uciok\n") || iterationCount >= WHILE_LOOP_SAFEGUARD) break;
    }
    free(data);
    return iterationCount < WHILE_LOOP_SAFEGUARD;
}

bool sendIsReadyCommand() {
    UCIEngine_sendCommand("isready");
    int iterationCount = 0;
    int capacity = DEFAULT_BUF_SIZE;
    char* data = malloc(sizeof(char) * capacity);
    while (true) {
        data = UCIEngine_readResponse(data, DEFAULT_BUF_SIZE);
        capacity = strlen(data);
        string_removeUnecessarySpacesAndTabs(data);
        if (string_compareStrings(data, "readyok\n") || iterationCount >= WHILE_LOOP_SAFEGUARD) break;
    }
    free(data);
    return iterationCount < WHILE_LOOP_SAFEGUARD;
}

bool UCIEngine_initialize(const char* enginePath) {
    bool init;
#ifdef __linux__
    init = UCIEngine_initialize_posix(enginePath);
#elif _WIN32
    init = UCIEngine_initialize_windows(enginePath);
#else
    assert(false && "Invalid architecture, this program only supports POSIX or windows");
#endif
    logFile = fopen("uci_engine_log.txt", "w");
    if (logFile == NULL) {
        fprintf(stderr, "Log file is NULL\n");
        return false;
    }

    return init && sendUCICommand() && sendIsReadyCommand();
}

void UCIEngine_terminate() {
    // quitting the process by itself with the quit command
    UCIEngine_sendCommand("quit");
#ifdef __linux__
    UCIEngine_terminate_posix();
#elif _WIN32
    UCIEngine_terminate_windows();
#else
    assert(false && "Invalid architecture, this program only supports POSIX or windows");
#endif

    fflush(logFile);
    fclose(logFile);
}

#define POSITION_LENGTH (8)
#define FEN_LENGTH (3)
#define MOVES_LENGTH (5)
#define LONG_ALGEBRAIC_LENGTH (6)
#define SPACE_LENGTH (1)

void sendPositionCommand(ChessPosition startingPosition, Move* movesPlayed, int numMoves) {
    char fen[MAX_FEN_STRING_SIZE];
    int fenLength = FenString_chessPositionToFenString(startingPosition, fen);

    int firstPartLength = POSITION_LENGTH + SPACE_LENGTH +
        FEN_LENGTH + SPACE_LENGTH +
        fenLength + SPACE_LENGTH +
        MOVES_LENGTH;

    int positionCommandSize = firstPartLength + (LONG_ALGEBRAIC_LENGTH + SPACE_LENGTH) * numMoves;

    // Variable length arrays for life!
    char positionCommand[positionCommandSize];

    snprintf(positionCommand, firstPartLength + 1, "position fen %s moves", fen);

    int positionCommandIndex = firstPartLength;
    char longAlgebraicMove[6];
    for (int index = 0; index < numMoves; index++) {
        Move move = movesPlayed[index];
        int moveLength = string_moveToLongAlgebraic(move, longAlgebraicMove);

        positionCommand[positionCommandIndex++] = SPACE_CHAR;
        memcpy(positionCommand + positionCommandIndex, longAlgebraicMove, moveLength);
        positionCommandIndex += moveLength;
    }
    positionCommand[positionCommandIndex] = '\0';
    UCIEngine_sendCommand(positionCommand);
}

Move bestMoveFromOptions(ChessPosition startingPosition, Move* movesPlayed, int numMoves, char* goOptions) {
    sendPositionCommand(startingPosition, movesPlayed, numMoves);

    int goCommandSize = snprintf(NULL, 0, "go %s", goOptions) + 1;
    char goCommand[goCommandSize];
    snprintf(goCommand, goCommandSize, "go %s", goOptions);

    UCIEngine_sendCommand(goCommand);

    int capacity = DEFAULT_BUF_SIZE;
    char* data = malloc(sizeof(char) * capacity);
    Tokens engineResponse;
    do {
        data = UCIEngine_readResponse(data, capacity);
        capacity = strlen(data);
        engineResponse.length = string_removeUnecessarySpacesAndTabs(data);
    } while (
        data[0] != 'b' ||
        data[1] != 'e' ||
        data[2] != 's' ||
        data[3] != 't' ||
        data[4] != 'm' ||
        data[5] != 'o' ||
        data[6] != 'v' ||
        data[7] != 'e'
        );

    char* tokens[engineResponse.length];
    engineResponse.tokens = tokens;
    string_tokenizeStringBySpace(data, &engineResponse);
    // The second tokens should have the best move from the engine
    Move move = string_longAlgebraicToMove(engineResponse.tokens[1]);
    assert(move != NULL_MOVE && "Move is NULL_MOVE at " __FILE__);

    free(data);

    return move;
}

Move UCIEngine_bestMoveTimed(ChessPosition startingPosition, Move* movesPlayed, int numMoves, TimeControl_MS timeToThink) {
    size_t length = snprintf(NULL, 0, "movetime %u", timeToThink) + 1;
    char movetimeOption[length];
    snprintf(movetimeOption, length, "movetime %u", timeToThink);

    return bestMoveFromOptions(startingPosition, movesPlayed, numMoves, movetimeOption);
}

Move UCIEngine_bestMoveFromTimeControls(ChessPosition startingPosition, Move* movesPlayed, int numMoves,
    TimeControl_MS wtime, TimeControl_MS btime, TimeControl_MS winc, TimeControl_MS binc, int movesToGo) {
    size_t length = snprintf(NULL, 0, "wtime %u btime %u winc %u binc %u", wtime, btime, winc, binc) + 1;
    if (movesToGo >= 0) length += snprintf(NULL, 0, " movestogo %d", movesToGo);

    char goOptions[length];
    size_t bytesWritten = snprintf(goOptions, length, "wtime %u btime %u winc %u binc %u", wtime, btime, winc, binc);
    if (movesToGo >= 0) snprintf(goOptions + bytesWritten, length - bytesWritten, " movestogo %d", movesToGo);

    return bestMoveFromOptions(startingPosition, movesPlayed, numMoves, goOptions);
}
