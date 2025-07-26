// TODO: Use SDL's file api instead of stdio
#include <stdio.h>

#include "../../engine/src/utils/CharBuffer.h"
#include "gameScene/GameScene.h"
#include "Config.h"

bool parseTimeControl(TimeControl* timecontrol, Tokens tokens) {
    // We need at least three tokens for a time control line
    if (tokens.length < 3) return false;

    int timeleft = string_parseNumber(tokens.tokens[1]);
    int increment = string_parseNumber(tokens.tokens[2]);
    if (timeleft == -1 || increment == -1) return false;
    timecontrol->timeLeft = timeleft;
    timecontrol->increment = increment;
    return true;
}

bool parseIsEngine(bool* isEngine, Tokens tokens) {
    // We need at least two tokens for an isEngine line
    if (tokens.length < 2) return false;
    if (string_compareStrings("true", tokens.tokens[1])) *isEngine = true;
    else if (string_compareStrings("false", tokens.tokens[1])) *isEngine = false;
    else return false; // Is not "true" nor "false" so we cannot parse it

    return true;
}

bool parseEnginePath(char** engineConfigPath, Tokens tokens, size_t read) {
    // We need at least two tokens for an engine path line
    if (tokens.length < 2) return false;

    if (string_compareStrings(tokens.tokens[1], "NULL")) {
        // The engine path is NULL
        *engineConfigPath = NULL;
        return true;
    }

    // length of <b | w>EnginePath<space> is 12
    size_t enginePathLength = read - 12 + tokens.length + 1;
    char* enginePath = SDL_calloc(enginePathLength, sizeof(char));
    size_t enginePathIndex = 0;
    for (size_t tokenIndex = 2; tokenIndex < tokens.length; tokenIndex++) {
        char* currentToken = tokens.tokens[tokenIndex];
        SDL_memcpy(enginePath + enginePathIndex, currentToken, SDL_strlen(currentToken));
        enginePathIndex += SDL_strlen(currentToken);
        enginePath[enginePathIndex++] = SPACE_CHAR;
    }
    enginePath[enginePathIndex - 1] = '\0';
    *engineConfigPath = enginePath;
    return true;
}

bool parseThinkTime(TimeControl_MS* timeToThink, Tokens tokens) {
    // We need at least two tokens for a time to think line
    if (tokens.length < 2) return false;

    int parsedTimeToThink = string_parseNumber(tokens.tokens[1]);
    // Either the bot decides how much it thinks (0) or it thinks for a specified amount of time
    *timeToThink = SDL_max(parsedTimeToThink, 0);

    return true;
}

bool parseStartingPosition(char** fen, Tokens tokens, size_t read) {
    // We need at least seven tokens for the startingPosition line
    if (tokens.length < 7) return false;

    // length of startingPosition<space> is 17
    size_t parsedFenLength = read - 17 + tokens.length + 1;
    char* parsedFen = SDL_calloc(parsedFenLength, sizeof(char));
    size_t enginePathIndex = 0;
    for (size_t tokenIndex = 1; tokenIndex < tokens.length; tokenIndex++) {
        char* currentToken = tokens.tokens[tokenIndex];
        SDL_memcpy(parsedFen + enginePathIndex, currentToken, SDL_strlen(currentToken));
        enginePathIndex += SDL_strlen(currentToken);
        parsedFen[enginePathIndex++] = SPACE_CHAR;
    }
    parsedFen[enginePathIndex - 1] = '\0';
    *fen = parsedFen;

    return true;
}

#define STARTING_BUFFER_SIZE (16)
#define char_append(element) \
    if (index == capacity) { \
        capacity *= 2; \
        result = SDL_realloc(result, capacity); \
    } \
    result[index++] = element; \

char* portableSubsetGetLine(FILE* file) {
    int capacity = STARTING_BUFFER_SIZE;
    int index = 0;
    char* result = SDL_calloc(capacity, sizeof(char));
    if (result == NULL) return NULL;

    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            break;
        }

        char_append(ch);
    }

    // If nothing was read and EOF is reached, return NULL
    if (index == 0 && ch == EOF) {
        SDL_free(result);
        return NULL;
    }

    if (index <= capacity) {
        capacity++;
        result = SDL_realloc(result, capacity);
    }
    result[index] = '\0';
    return result;
}

/*
Parses the save file for the game setting. This is what a valid file would look like:

wTimeControl total_time_ms (num) increment_ms (num)
wIsEngine true_or_false (string)
wEnginepath path (string)
wThinkTime time_for_engine_to_think_ms (num) (-1 if you let the engine decide how long it thinks)

... (same thing with black)

startingPosition starting_position_fen_string (string)

*/
bool loadMainMenuConfigFromFile(FILE* file, GameConfig* data) {
    bool result = false;
    if (!file || !data) return result;

    char* line = NULL;
    Tokens tokens;
    size_t nbTokens;

    while ((line = portableSubsetGetLine(file)) != NULL) {
        size_t read = SDL_strlen(line);
        nbTokens = string_removeUnecessarySpacesAndTabs(line);
        char* tokens_arr[nbTokens];
        tokens.length = nbTokens;
        tokens.tokens = tokens_arr;
        string_tokenizeStringBySpace(line, &tokens);

        // ignore any empty lines
        if (tokens.length == 0) continue;

        // Check if the string starts with any of the character, else we ignore the line
        if (string_compareStrings(tokens.tokens[0], "wTimeControl")) {
            if (!parseTimeControl(&data->white.timeControl, tokens)) goto end_of_parsing_file;
        }
        else if (string_compareStrings(tokens.tokens[0], "wIsEngine")) {
            if (!parseIsEngine(&data->white.engineConfig.isEngine, tokens)) goto end_of_parsing_file;
        }
        else if (string_compareStrings(tokens.tokens[0], "wEnginepath")) {
            if (!parseEnginePath(&data->white.engineConfig.enginePath, tokens, read)) goto end_of_parsing_file;
        }
        else if (string_compareStrings(tokens.tokens[0], "wThinkTime")) {
            if (!parseThinkTime(&data->white.engineConfig.timeToThink, tokens)) goto end_of_parsing_file;
        }
        else if (string_compareStrings(tokens.tokens[0], "bTimeControl")) {
            if (!parseTimeControl(&data->black.timeControl, tokens)) goto end_of_parsing_file;
        }
        else if (string_compareStrings(tokens.tokens[0], "bIsEngine")) {
            if (!parseIsEngine(&data->black.engineConfig.isEngine, tokens)) goto end_of_parsing_file;
        }
        else if (string_compareStrings(tokens.tokens[0], "bEnginepath")) {
            if (!parseEnginePath(&data->black.engineConfig.enginePath, tokens, read)) goto end_of_parsing_file;
        }
        else if (string_compareStrings(tokens.tokens[0], "bThinkTime")) {
            if (!parseThinkTime(&data->black.engineConfig.timeToThink, tokens)) goto end_of_parsing_file;
        }
        else if (string_compareStrings(tokens.tokens[0], "startingPosition")) {
            if (!parseStartingPosition(&data->startingPositionFen, tokens, read)) goto end_of_parsing_file;
        }
        SDL_free(line);
    }

    result = true;
end_of_parsing_file:
    SDL_free(line);
    return result;
}

void loadMainMenuConfig(GameConfig* data) {
    char* basePath = SDL_GetPrefPath("Etienne", "ChessApp");
    if (!basePath) goto fallback;

    // Construct the full file path
    size_t length = snprintf(NULL, 0, "%schess.config", basePath) + 1;
    char* configPath = SDL_calloc(length, sizeof(char));
    snprintf(configPath, length, "%schess.config", basePath);
    SDL_free(basePath);

    SDL_Log("Reading config from %s\n", configPath);
    FILE* file = fopen(configPath, "r");
    if (file && loadMainMenuConfigFromFile(file, data)) {
        fclose(file);
        SDL_free(configPath);
        return;
    }

    if (file) fclose(file);

fallback:
    // Fallback to default settings
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not load settings from file\n");
    data->white.engineConfig.isEngine = false;
    data->white.engineConfig.enginePath = NULL;
    data->white.engineConfig.timeToThink = 0;
    data->white.timeControl = DEFAULT_TIME_CONTROL;

    data->black.engineConfig.isEngine = false;
    data->black.engineConfig.enginePath = NULL;
    data->black.engineConfig.timeToThink = 0;
    data->black.timeControl = DEFAULT_TIME_CONTROL;

    data->startingPositionFen = INITIAL_FEN;
}

// Helper function
bool writePlayerBlock(FILE* file, PlayerConfig playerConfig, PieceCharacteristics playerColor) {
    char colorPrefix = playerColor == WHITE ? 'w' : 'b';

    if (fprintf(file, "%cTimeControl %u %u\n",
        colorPrefix,
        playerConfig.timeControl.timeLeft,
        playerConfig.timeControl.increment) < 0) return false;

    if (fprintf(file, "%cIsEngine %s\n", colorPrefix, playerConfig.engineConfig.isEngine ? "true" : "false") < 0) return false;
    if (fprintf(file, "%cEnginePath %s\n", colorPrefix, playerConfig.engineConfig.enginePath ? playerConfig.engineConfig.enginePath : "NULL") < 0) return false;
    if (fprintf(file, "%cThinkTime %u\n", colorPrefix, playerConfig.engineConfig.timeToThink) < 0) return false;
    return true;
}

bool saveMainMenuConfig(const GameConfig* data) {
    if (!data) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not save config data is NULL\n");
        return false;
    }

    char* basePath = SDL_GetPrefPath("Etienne", "ChessApp");
    if (!basePath) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not save config SDL_GetPrefPath returns NULL\n");
        return false;
    }

    // Construct the full file path
    size_t length = SDL_snprintf(NULL, 0, "%schess.config", basePath) + 1;
    char configPath[length];
    SDL_snprintf(configPath, length, "%schess.config", basePath);
    SDL_free(basePath);

    FILE* file = fopen(configPath, "w");
    if (!file) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not save config since we could not open the config file\n");
        return false;
    }

    bool success = true;
    if (!writePlayerBlock(file, data->white, WHITE)) success = false;
    else if (!writePlayerBlock(file, data->black, BLACK)) success = false;
    else if (fprintf(file, "startingPosition %s\n", data->startingPositionFen) < 0) success = false;

    fclose(file);
    if (success) SDL_Log("Saved file to %s\n", configPath);
    else SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not save file to %s\n", configPath);
    
    return success;
}
