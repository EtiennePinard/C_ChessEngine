#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <signal.h>
#include <assert.h>
#include <string.h>

#include "../../engine/src/utils/CharBuffer.h"
#include "../../engine/src/utils/FenString.h"

#include "UCIEngineCommunication.h"

#define WHILE_LOOP_SAFEGUARD (1000)

#define PIPE_READ_INDEX (0)
#define PIPE_WRITE_INDEX (1)

#define returnOnFail(condition, message) \
    if (condition) {                     \
        fprintf(stderr, message);        \
        return false;                    \
    }

typedef int FileDescriptor;

typedef struct EngineCommunication {
    FileDescriptor outPipe;
    FileDescriptor inPipe;
    pid_t enginePid;
} EngineCommunication;

EngineCommunication engineCommunication;

void sendCommand(const char* command) {
    printf("%s\n", command);
    write(engineCommunication.outPipe, command, strlen(command));
    write(engineCommunication.outPipe, "\n", 1); // End each command with a newline
}

#define DEFAULT_BUF_SIZE (128)

#define data_resize(size) \
    if (numBytesRead >= capacity) { \
        capacity += size; \
        data = realloc(data, capacity * sizeof(char)); \
        assert(data != NULL && "Buy more ram lol at " __FILE__); \
    } \

char* readResponse(char* data, int capacity) {
    assert(data != NULL && "Data is NULL at " __FILE__);
    int numBytesRead = 0;
    if (capacity == 0) data_resize(DEFAULT_BUF_SIZE);

    while (read(engineCommunication.inPipe, data + numBytesRead, 1) == 1) {
        if (data[numBytesRead] == '\n') break;
        numBytesRead++;
        data_resize(DEFAULT_BUF_SIZE);
    }
    // Setting the next character to be 0
    numBytesRead++;
    data_resize(1); // We just need to add one more character
    data[numBytesRead] = '\0';
    printf("%s", data);
    return data;
}

bool sendUCICommand() {
    sendCommand("uci");
    int iterationCount = 0;
    int capacity = DEFAULT_BUF_SIZE;
    char* data = malloc(sizeof(char) * capacity);
    while (true) {
        data = readResponse(data, DEFAULT_BUF_SIZE);
        capacity = strlen(data);
        string_removeUnecessarySpacesAndTabs(data);
        if (string_compareStrings(data, "uciok\n") || iterationCount >= WHILE_LOOP_SAFEGUARD) break;
    }
    free(data);
    return iterationCount < WHILE_LOOP_SAFEGUARD;
}

bool UCIEngine_initialize(const char* enginePath) {
    pid_t pid = 0;
    FileDescriptor inpipefd[2];
    FileDescriptor outpipefd[2];

    // Create pipes for communication
    returnOnFail(pipe(inpipefd), "Input pipe failed\n");
    returnOnFail(pipe(outpipefd), "Output pipe failed\n");

    pid = fork();
    returnOnFail(pid == -1, "Fork failed\n");

    if (pid == 0) {
        // Child process: set up to run the UCI engine
        dup2(outpipefd[PIPE_READ_INDEX], STDIN_FILENO);  // Read from parent's outpipefd
        dup2(inpipefd[PIPE_WRITE_INDEX], STDOUT_FILENO); // Write to parent's inpipefd
        dup2(inpipefd[PIPE_WRITE_INDEX], STDERR_FILENO); // Redirect stderr as well

        // Ensure the child receives SIGTERM if the parent dies
        prctl(PR_SET_PDEATHSIG, SIGTERM);

        // Close unused pipe ends in the child process
        close(outpipefd[PIPE_WRITE_INDEX]);
        close(inpipefd[PIPE_READ_INDEX]);

        // Launch the engine
        if (execl(enginePath, enginePath, (char*)NULL) == -1) {
            perror("execl failed");
        }

        // exit the child process when execl returns
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(outpipefd[PIPE_READ_INDEX]);
    close(inpipefd[PIPE_WRITE_INDEX]);

    engineCommunication = (EngineCommunication){
        .inPipe = inpipefd[PIPE_READ_INDEX],
        .outPipe = outpipefd[PIPE_WRITE_INDEX],
        .enginePid = pid
    };

    return sendUCICommand();
}

void UCIEngine_terminate() {
    // quitting the process by itself with the quit command
    sendCommand("quit");
    // Terminate the child process
    int status;
    pid_t pid = engineCommunication.enginePid;
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0); // Wait for the child process to terminate
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
    sendCommand(positionCommand);
}

Move bestMoveFromOptions(ChessPosition startingPosition, Move* movesPlayed, int numMoves, char* goOptions) {
    sendPositionCommand(startingPosition, movesPlayed, numMoves);

    int goCommandSize = snprintf(NULL, 0, "go %s", goOptions) + 1;
    char goCommand[goCommandSize];
    snprintf(goCommand, goCommandSize, "go %s", goOptions);

    sendCommand(goCommand);

    int capacity = DEFAULT_BUF_SIZE;
    char* data = malloc(sizeof(char) * capacity);
    Tokens engineResponse;
    do {
        data = readResponse(data, capacity);
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
    Move result = string_longAlgebraicToMove(engineResponse.tokens[1]);
    assert(result != NULL_MOVE && "Move is NULL_MOVE at " __FILE__);

    free(data);

    return result;
}

Move bestMoveTimed(ChessPosition startingPosition, Move* movesPlayed, int numMoves, u32 timeToThink) {
    size_t length = snprintf(NULL, 0, "movetime %u", timeToThink) + 1;
    char movetimeOption[length];
    snprintf(movetimeOption, length, "movetime %u", timeToThink);

    return bestMoveFromOptions(startingPosition, movesPlayed, numMoves, movetimeOption);
}

Move bestMoveFromTimeControls(ChessPosition startingPosition, Move* movesPlayed, int numMoves, u32 wtime, u32 btime, u32 winc, u32 binc, int movesToGo) {
    size_t length = snprintf(NULL, 0, "wtime %u btime %u winc %u binc %u", wtime, btime, winc, binc) + 1;
    if (movesToGo >= 0) length += snprintf(NULL, 0, " movestogo %d", movesToGo);

    char goOptions[length];
    size_t bytesWritten = snprintf(goOptions, length, "wtime %u btime %u winc %u binc %u", wtime, btime, winc, binc);
    if (movesToGo >= 0) snprintf(goOptions + bytesWritten, length - bytesWritten, " movestogo %d", movesToGo);

    return bestMoveFromOptions(startingPosition, movesPlayed, numMoves, goOptions);
}
