#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>

#include "StockfishAPI.h"

#define PIPE_READ_INDEX 0
#define PIPE_WRITE_INDEX 1

#define BUFFER_SIZE 256

#define UCIOK_LENGTH 5

#define WHILE_LOOP_SAFEGUARD 1000

#define returnOnFail(condition, message) \
    if (condition)                       \
    {                                    \
        fprintf(stderr, message);        \
        return false;                    \
    }

typedef int FileDescriptor;

typedef struct {
    FileDescriptor outPipe;
    FileDescriptor inPipe;
    pid_t stockfishPid;
} StockfishUCIState;

StockfishUCIState stockfishUCIState;

#define initResponse(response) \
char buf[BUFFER_SIZE] = { 0 }; \
response = (StockfishResponse) { \
        .buffer = buf, \
        .size = 0 \
    };\

// Note: The capacity of the buffer is always BUFFER_SIZE = 256
typedef struct StockfishResponse {
    char* buffer;
    int size;
} StockfishResponse;

void sendCommand(const char *command) {
    write(stockfishUCIState.outPipe, command, strlen(command));
    write(stockfishUCIState.outPipe, "\n", 1); // End each command with a newline
}

// We have a buffer overflow if the line length is larger than 256 characters
void readResponse(StockfishResponse* response) {
    int offset = 0;
    while (read(stockfishUCIState.inPipe, response->buffer + offset, 1) == 1) {
        if (response->buffer[offset] == '\n') break;
        offset += 1;
    }
    response->size = offset + 1;
}

/**
 * @brief Returns the line which starts with responseStart
 * IMPORTANT: The function will hang if it does not find your reponse... so yeah, though luck
 * @param responseStart The start of the response
 * @param response The reponse from stockfish
 * @return true if the response is found
 * @return false if the response is not found
 */
bool findResponse(char* responseStart, StockfishResponse* response) {
    int safeGuard = 0;
    int beginningLength = strlen(responseStart);
    bool beginningMatch = false;

    do {
        memset(response->buffer, 0, response->size);
        readResponse(response);

        if (response->size < beginningLength) {
            beginningMatch = false;
        } else {
            beginningMatch = true;
            for (int index = 0; index < beginningLength; index++) {
                if (response->buffer[index] != responseStart[index]) {
                    beginningMatch = false;
                    break;
                }
            }
        }

        safeGuard++;
    } while(!beginningMatch && safeGuard < WHILE_LOOP_SAFEGUARD);

    return safeGuard < WHILE_LOOP_SAFEGUARD;
}

bool sendUCICommand() {
    sendCommand("uci");

    StockfishResponse response;
    initResponse(response);

    return findResponse("uciok", &response);
}

bool stockfishInit(const char *stockfishPath) {

    pid_t pid = 0;
    FileDescriptor inpipefd[2];
    FileDescriptor outpipefd[2];

    // Create pipes for communication
    returnOnFail(pipe(inpipefd), "Input pipe failed\n");
    returnOnFail(pipe(outpipefd), "Output pipe failed\n");

    pid = fork();
    returnOnFail(pid == -1, "Fork failed\n");

    if (pid == 0)
    {
        // Child process: set up to run Stockfish
        dup2(outpipefd[PIPE_READ_INDEX], STDIN_FILENO);  // Read from parent's outpipefd
        dup2(inpipefd[PIPE_WRITE_INDEX], STDOUT_FILENO); // Write to parent's inpipefd
        dup2(inpipefd[PIPE_WRITE_INDEX], STDERR_FILENO); // Redirect stderr as well

        // Ensure the child receives SIGTERM if the parent dies
        prctl(PR_SET_PDEATHSIG, SIGTERM);

        // Close unused pipe ends in the child process
        close(outpipefd[PIPE_WRITE_INDEX]);
        close(inpipefd[PIPE_READ_INDEX]);

        // Launch Stockfish (ensure Stockfish path is correct)
        execl(stockfishPath, "stockfish", (char *)NULL);

        // If execl fails, exit the child process
        perror("execl failed");
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(outpipefd[PIPE_READ_INDEX]);
    close(inpipefd[PIPE_WRITE_INDEX]);

    stockfishUCIState = (StockfishUCIState){
        .inPipe = inpipefd[PIPE_READ_INDEX],
        .outPipe = outpipefd[PIPE_WRITE_INDEX],
        .stockfishPid = pid};

    return sendUCICommand();
}

void terminate() {
    // Terminate the child process
    int status;
    pid_t pid = stockfishUCIState.stockfishPid;
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0); // Wait for the child process to terminate
    printf("Stockfish terminated.\n");
}

#define POSITION_FEN_LENGTH 13
void setStockfishPosition(char* fen) {
    sendCommand("ucinewgame");

    int fenLength = strlen(fen);
    char command[POSITION_FEN_LENGTH + fenLength + 1];
    memcpy(command, "position fen ", POSITION_FEN_LENGTH);
    memcpy(command + POSITION_FEN_LENGTH, fen, fenLength);
    command[POSITION_FEN_LENGTH + fenLength] = '\0';

    sendCommand(command);
}

float stockfishStaticEvaluation() {
    sendCommand("eval");
    StockfishResponse response;
    initResponse(response);
    findResponse("Final", &response);
    
    // String has format: Final evaluation       (+ or -)xx.xx (who's side)
    // where x are digits
    int evalStartIndex = 0;
    char currentChar = response.buffer[evalStartIndex];
    while (currentChar != '-' && currentChar != '+') {
        evalStartIndex++;
        currentChar = response.buffer[evalStartIndex];
    }

    // Do not need the end index, it is handled in the strtof function
    float eval = strtof(response.buffer + evalStartIndex, NULL);

    return eval;
}

int get_square_from_algebraic(const char *square) {
    if (square[0] < 'a' || square[0] > 'h' ||
        square[1] < '1' || square[1] > '8') {
        return -1; // Invalid square
    }
    
    int file = square[0] - 'a';
    int rank = 7 - (square[1] - '1');
    
    return rank * 8 + file;
}

Move moveFromAlgebraic(const char *move) {
    if (strlen(move) != 4) {
        return 0; // Invalid move format
    }
    
    char start[3] = { move[0], move[1], '\0' };
    char target[3] = { move[2], move[3], '\0' };
    
    int from = get_square_from_algebraic(start);
    int to = get_square_from_algebraic(target);
    
    if (from == -1 || to == -1) {
        return (Move) -1; // Invalid move, return max move value
    }

    return makeMove(from, to, NOFlAG);
}

#define GO_DEPTH_LENGTH 9
Move stockfishBestMove(int depth) {
    int depthLength = (int) ceilf(log10f((float) depth)) + (depth % 10 == 0 ? 1 : 0);
    char depthString[depthLength];
    int index = depthLength - 1;
    while (depth) {
        depthString[index] = '0' + (depth % 10);
        depth /= 10;
        index--;
    }
    char command[GO_DEPTH_LENGTH + depthLength + 1];
    memcpy(command, "go depth ", GO_DEPTH_LENGTH);
    memcpy(command + GO_DEPTH_LENGTH, depthString, depthLength);
    command[GO_DEPTH_LENGTH + depthLength] = '\0';

    sendCommand(command);

    StockfishResponse response;
    initResponse(response);
    findResponse("bestmove", &response);
    
    char* split;
    split = strtok(response.buffer, " "); // bestmove
    split = strtok(NULL, " "); // the algebraic move

    return moveFromAlgebraic(split);
}