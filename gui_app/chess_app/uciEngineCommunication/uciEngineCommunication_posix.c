#ifdef __linux__
#include <sys/wait.h>
#include <sys/prctl.h>

#define PIPE_READ_INDEX (0)
#define PIPE_WRITE_INDEX (1)

typedef int FileDescriptor;

typedef struct EngineCommunication {
    FileDescriptor outPipe;
    FileDescriptor inPipe;
    pid_t enginePid;
} EngineCommunication;

EngineCommunication engineCommunication;

#define returnOnFail(condition, message) \
    if (condition) {                     \
        fprintf(stderr, message);        \
        return false;                    \
    }                                    \

bool UCIEngine_initialize_posix(const char* enginePath) {
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

    return true;
}

void UCIEngine_sendCommand_posix(const char* command) {
    write(engineCommunication.outPipe, command, strlen(command));
    write(engineCommunication.outPipe, "\n", 1); // End each command with a newline
}

#define data_resize(size)                                        \
    if (numBytesRead >= capacity) {                              \
        capacity += size;                                        \
        data = realloc(data, capacity * sizeof(char));           \
        assert(data != NULL && "Buy more ram lol at " __FILE__); \
    }                                                            \

char* UCIEngine_readResponse_posix(char* data, int capacity) {
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
    return data;
}

void UCIEngine_terminate_posix() {
    // Terminate the child process
    int status;
    pid_t pid = engineCommunication.enginePid;
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0); // Wait for the child process to terminate
}

#endif
