#ifdef __linux__

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <signal.h>

#include "UCIEngineCommunication.h"

#define PIPE_READ_INDEX (0)
#define PIPE_WRITE_INDEX (1)

typedef int FileDescriptor;

struct EngineCommunication {
    FileDescriptor outPipe;
    FileDescriptor inPipe;
    pid_t enginePid;
    FILE* logFile;
};

#define returnOnFail(condition, message) \
    if (condition) {                     \
        fprintf(stderr, message);        \
        free(engineCommunication);       \
        return NULL;                     \
    }                                    \

EngineCommunication* UCIEngine_initialize_posix(const char* enginePath, const char* logFilePath) {
    EngineCommunication* engineCommunication = malloc(sizeof(EngineCommunication));
    pid_t pid = 0;
    FileDescriptor inpipefd[2];
    FileDescriptor outpipefd[2];
    FileDescriptor execStatusPipe[2]; // New pipe to detect exec failure

    returnOnFail(pipe(inpipefd), "Input pipe failed\n");
    returnOnFail(pipe(outpipefd), "Output pipe failed\n");
    returnOnFail(pipe(execStatusPipe), "Exec status pipe failed\n");

    pid = fork();
    returnOnFail(pid == -1, "Fork failed\n");

    if (pid == 0) {
        // Child process
        close(execStatusPipe[0]); // Close read end

        // Set close-on-exec on the write end
        fcntl(execStatusPipe[1], F_SETFD, FD_CLOEXEC);

        dup2(outpipefd[PIPE_READ_INDEX], STDIN_FILENO);
        dup2(inpipefd[PIPE_WRITE_INDEX], STDOUT_FILENO);
        dup2(inpipefd[PIPE_WRITE_INDEX], STDERR_FILENO);

        prctl(PR_SET_PDEATHSIG, SIGTERM);

        close(outpipefd[PIPE_WRITE_INDEX]);
        close(inpipefd[PIPE_READ_INDEX]);

        execl(enginePath, enginePath, (char*)NULL);

        // If we reach here, execl failed
        perror("execl failed");
        write(execStatusPipe[1], "x", 1); // Write a byte to indicate failure
        close(execStatusPipe[1]);
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(execStatusPipe[1]); // Close write end

    // Check if child reported exec failure
    char buf;
    ssize_t n = read(execStatusPipe[0], &buf, 1);
    close(execStatusPipe[0]);

    if (n > 0) {
        // Child failed to exec
        fprintf(stderr, "ERROR: Child process failed to exec the engine\n");
        close(inpipefd[PIPE_READ_INDEX]);
        close(outpipefd[PIPE_WRITE_INDEX]);
        waitpid(pid, NULL, 0); // Reap zombie child
        free(engineCommunication);
        return NULL;
    }

    // Continue as normal
    close(outpipefd[PIPE_READ_INDEX]);
    close(inpipefd[PIPE_WRITE_INDEX]);

    engineCommunication->inPipe = inpipefd[PIPE_READ_INDEX];
    engineCommunication->outPipe = outpipefd[PIPE_WRITE_INDEX];
    engineCommunication->enginePid = pid;

    engineCommunication->logFile = fopen(logFilePath, "w");
    returnOnFail(engineCommunication->logFile == NULL, "Log file is NULL\n");

    return engineCommunication;
}

void UCIEngine_sendCommand_posix(EngineCommunication* engineCommunication, const char* command) {
    write(engineCommunication->outPipe, command, strlen(command));
    write(engineCommunication->outPipe, "\n", 1); // End each command with a newline
    fprintf(engineCommunication->logFile, "%s\n", command);
}

#define data_resize(size)                                        \
    if (numBytesRead >= capacity) {                              \
        capacity += size;                                        \
        data = realloc(data, capacity * sizeof(char));           \
        assert(data != NULL && "Buy more ram lol at " __FILE__); \
    }                                                            \

#define DEFAULT_BUF_SIZE (128)

char* UCIEngine_readResponse_posix(EngineCommunication* engineCommunication, char* data, int capacity) {
    assert(data != NULL && "Data is NULL at " __FILE__);
    int numBytesRead = 0;
    if (capacity == 0) data_resize(DEFAULT_BUF_SIZE);

    while (read(engineCommunication->inPipe, data + numBytesRead, 1) == 1) {
        if (data[numBytesRead] == '\n') break;
        numBytesRead++;
        data_resize(DEFAULT_BUF_SIZE);
    }
    // Setting the next character to be 0
    numBytesRead++;
    data_resize(1); // We just need to add one more character
    data[numBytesRead] = '\0';
    fprintf(engineCommunication->logFile, "%s", data);
    return data;
}

void UCIEngine_terminate_posix(EngineCommunication* engineCommunication) {
    // Terminate the child process
    int status;
    pid_t pid = engineCommunication->enginePid;
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0); // Wait for the child process to terminate
    fflush(engineCommunication->logFile);
    fclose(engineCommunication->logFile);
    free(engineCommunication);
}

#endif
