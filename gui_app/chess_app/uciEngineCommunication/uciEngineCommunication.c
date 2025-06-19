#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../../../engine/src/utils/CharBuffer.h"
#include "../../../engine/src/utils/FenString.h"
#include "../../../engine/src/utils/Math.h"
#include "../../../engine/src/moveHandler/MoveGenerator.h"

// Including .c files because we are already inside a .c file with a .h header so it doesn't matter
#ifdef __linux__
#include "uciEngineCommunication_posix.c"
#elif _WIN32
#include "uciEngineCommunication_windows.c"
#endif

#define WHILE_LOOP_SAFEGUARD (1000)

void UCIEngine_sendCommand(EngineCommunication* engineCommunication, const char* command) {
#ifdef __linux__
    UCIEngine_sendCommand_posix(engineCommunication, command);
#elif _WIN32
    UCIEngine_sendCommand_windows(engineCommunication, command);
#else
    assert(false && "Invalid architecture, this program only supports POSIX or windows");
#endif
    printf("%s\n", command);
}

#define DEFAULT_BUF_SIZE (128)

char* UCIEngine_readResponse(EngineCommunication* engineCommunication, char* data, int capacity) {
#ifdef __linux__
    data = UCIEngine_readResponse_posix(engineCommunication, data, capacity);
#elif _WIN32
    data = UCIEngine_readResponse_windows(engineCommunication, data, capacity);
#else
    assert(false && "Invalid architecture, this program only supports POSIX or windows");
#endif
    printf("%s", data);
    return data;
}

bool sendUCICommand(EngineCommunication* engineCommunication) {
    UCIEngine_sendCommand(engineCommunication, "uci");
    char* data = malloc(sizeof(char) * DEFAULT_BUF_SIZE);
    while (true) {
        data = UCIEngine_readResponse(engineCommunication, data, DEFAULT_BUF_SIZE);
        string_removeUnecessarySpacesAndTabs(data);
        if (string_compareStrings(data, "uciok\n")) break;
    }
    free(data);
    return true;
}

bool sendIsReadyCommand(EngineCommunication* engineCommunication) {
    UCIEngine_sendCommand(engineCommunication, "isready");
    char* data = malloc(sizeof(char) * DEFAULT_BUF_SIZE);
    
    while (true) {
        data = UCIEngine_readResponse(engineCommunication, data, DEFAULT_BUF_SIZE);
        string_removeUnecessarySpacesAndTabs(data);
        if (string_compareStrings(data, "readyok\n")) break;
    }
    free(data);
    return true;
}

EngineCommunication* UCIEngine_initialize(const char *enginePath, const char* logFilePath) {
    EngineCommunication* engineCommunication;
#ifdef __linux__
    engineCommunication = UCIEngine_initialize_posix(enginePath, logFilePath);
#elif _WIN32
    engineCommunication = UCIEngine_initialize_windows(enginePath, logFilePath);
#else
    assert(false && "Invalid architecture, this program only supports POSIX or windows");
#endif
    if (!engineCommunication) return NULL;

    if (!sendUCICommand(engineCommunication) || !sendIsReadyCommand(engineCommunication)) {
        free(engineCommunication);
        return NULL;
    } 

    return engineCommunication;
}

void UCIEngine_terminate(EngineCommunication* engineCommunication) {
    // quitting the process by itself with the quit command
    UCIEngine_sendCommand(engineCommunication, "quit");
#ifdef __linux__
    UCIEngine_terminate_posix(engineCommunication);
#elif _WIN32
    UCIEngine_terminate_windows(engineCommunication);
#else
    assert(false && "Invalid architecture, this program only supports POSIX or windows");
#endif
}

#define POSITION_LENGTH (8)
#define FEN_LENGTH (3)
#define MOVES_LENGTH (5)
#define LONG_ALGEBRAIC_LENGTH (6)
#define SPACE_LENGTH (1)

void sendPositionCommand(EngineCommunication* engineCommunication, ChessPosition startingPosition, Move* movesPlayed, int numMoves) {
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
    UCIEngine_sendCommand(engineCommunication, positionCommand);
}

Move bestMoveFromOptions(EngineCommunication* engineCommunication, ChessPosition startingPosition, Move* movesPlayed, int numMoves, char* goOptions) {
    sendPositionCommand(engineCommunication, startingPosition, movesPlayed, numMoves);

    int goCommandSize = snprintf(NULL, 0, "go %s", goOptions) + 1;
    char goCommand[goCommandSize];
    snprintf(goCommand, goCommandSize, "go %s", goOptions);

    UCIEngine_sendCommand(engineCommunication, goCommand);

    int capacity = DEFAULT_BUF_SIZE;
    char* data = malloc(sizeof(char) * capacity);
    Tokens engineResponse;
    do {
        data = UCIEngine_readResponse(engineCommunication, data, capacity);
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

Move UCIEngine_bestMoveTimed(EngineCommunication* engineCommunication,
    ChessPosition startingPosition, Move* movesPlayed, int numMoves, TimeControl_MS timeToThink) {
    size_t length = snprintf(NULL, 0, "movetime %u", timeToThink) + 1;
    char movetimeOption[length];
    snprintf(movetimeOption, length, "movetime %u", timeToThink);

    return bestMoveFromOptions(engineCommunication, startingPosition, movesPlayed, numMoves, movetimeOption);
}

Move UCIEngine_bestMoveFromTimeControls(EngineCommunication* engineCommunication,
    ChessPosition startingPosition, Move* movesPlayed, int numMoves,
    TimeControl_MS wtime, TimeControl_MS btime, TimeControl_MS winc, TimeControl_MS binc, int movesToGo) {
    size_t length = snprintf(NULL, 0, "wtime %u btime %u winc %u binc %u", wtime, btime, winc, binc) + 1;
    if (movesToGo >= 0) length += snprintf(NULL, 0, " movestogo %d", movesToGo);

    char goOptions[length];
    size_t bytesWritten = snprintf(goOptions, length, "wtime %u btime %u winc %u binc %u", wtime, btime, winc, binc);
    if (movesToGo >= 0) snprintf(goOptions + bytesWritten, length - bytesWritten, " movestogo %d", movesToGo);

    return bestMoveFromOptions(engineCommunication, startingPosition, movesPlayed, numMoves, goOptions);
}
