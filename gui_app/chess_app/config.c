#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include "../../engine/src/utils/CharBuffer.h"
#include "Config.h"

bool parsePlayer(PlayerConfig* player, Tokens tokens, size_t read) {
    // We need at least two tokens for a player line
    if (tokens.length < 2) return false;

    if (string_compareStrings(tokens.tokens[1], "false")) {
        player->isEngine = false;
        player->enginePath = NULL;
    }
    else if (string_compareStrings(tokens.tokens[1], "true")) {
        player->isEngine = true;
        // We need at least some engine path
        if (tokens.length < 3) return false;

        size_t enginePathLength = read - 11 + tokens.length + 1;
        char* enginePath = calloc(enginePathLength, sizeof(char));
        size_t enginePathIndex = 0;
        for (size_t tokenIndex = 2; tokenIndex < tokens.length; tokenIndex++) {
            char* currentToken = tokens.tokens[tokenIndex];
            memcpy(enginePath + enginePathIndex, currentToken, strlen(currentToken));
            enginePathIndex += strlen(currentToken);
            enginePath[enginePathIndex++] = SPACE_CHAR;
        }
        enginePath[enginePathIndex - 1] = '\0';
        player->enginePath = enginePath;
    }
    else return false; // If it is not "true" nor "false" so we cannot parse the config file
    return true;
}

bool parseTimeControl(TimeControl* timecontrol, Tokens tokens) {
    // We need at least two tokens for a time control line
    if (tokens.length < 2) return false;

    int timeleft = string_parseNumber(tokens.tokens[1]);
    int increment = string_parseNumber(tokens.tokens[2]);
    if (timeleft == -1 || increment == -1) return false;
    timecontrol->timeLeft = timeleft;
    timecontrol->increment = increment;
    return true;
}

/*
Parses the save file for the game setting. This is what a valid file would look like:

white true /Chess Engine/nameOfTheEngine.elf
black false
timeControl 180000 2000

*/
bool loadMainMenuConfigFromFile(FILE* file, MainMenuSceneData* data) {
    bool result = false;
    if (!file || !data) return result;

    char* line = NULL;
    size_t size = 0;
    ssize_t read;
    Tokens tokens;
    size_t nbTokens;

    while ((read = getline(&line, &size, file)) != -1) {
        line[read - 1] = '\0';
        nbTokens = string_removeUnecessarySpacesAndTabs(line);
        char* tokens_arr[nbTokens];
        tokens.length = nbTokens;
        tokens.tokens = tokens_arr;
        string_tokenizeStringBySpace(line, &tokens);

        // ignore any empty lines
        if (tokens.length == 0) continue;

        // Check if the string starts with any of the character, else we ignore the line
        if (string_compareStrings(tokens.tokens[0], "white")) {
            if (!parsePlayer(&data->white, tokens, read)) goto end_of_parsing_file;
        }
        else if (string_compareStrings(tokens.tokens[0], "black")) {
            if (!parsePlayer(&data->black, tokens, read)) goto end_of_parsing_file;
        }
        else if (string_compareStrings(tokens.tokens[0], "timecontrol")) {
            if (!parseTimeControl(&data->timeControl.selected, tokens)) goto end_of_parsing_file;
        }
    }

    result = true;
end_of_parsing_file:
    free(line);
    return result;
}

void loadMainMenuConfig(MainMenuSceneData* data) {
    char* basePath = SDL_GetPrefPath("Etienne", "ChessApp");
    if (!basePath) goto fallback;

    // Construct the full file path
    size_t length = snprintf(NULL, 0, "%schess.config", basePath) + 1;
    char* configPath = calloc(length, sizeof(char));
    snprintf(configPath, length, "%schess.config", basePath);
    SDL_free(basePath);

    FILE* file = fopen(configPath, "r");
    if (file && loadMainMenuConfigFromFile(file, data)) {
        fclose(file);
        return;
    }

    if (file) fclose(file);
    free(configPath);

fallback:
    // Fallback to default settings
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not load settings from file\n");
    data->white.isEngine = false;
    data->white.enginePath = NULL;

    data->black.isEngine = false;
    data->black.enginePath = NULL;

    data->timeControl.selected = DEFAULT_TIME_CONTROL;
}

// Helper function
bool writePlayerBlock(FILE* file, const char* label, bool isEngine, const char* path) {
    if (isEngine) {
        if (fprintf(file, "%s true %s\n", label, path) < 0) return false;
    }
    else {
        if (fprintf(file, "%s false\n", label) < 0) return false;
    }
    return true;
}

bool saveMainMenuConfig(const MainMenuSceneData* data) {
    if (!data) return false;

    char* basePath = SDL_GetPrefPath("Etienne", "ChessApp");
    if (!basePath) return false;

    // Construct the full file path
    size_t length = snprintf(NULL, 0, "%schess.config", basePath) + 1;
    char configPath[length];
    snprintf(configPath, length, "%schess.config", basePath);
    SDL_free(basePath);

    FILE* file = fopen(configPath, "w");
    if (!file) return false;

    bool success = true;
    if (!writePlayerBlock(file, "white", data->white.isEngine, data->white.enginePath)) success = false;
    else if (!writePlayerBlock(file, "black", data->black.isEngine, data->black.enginePath)) success = false;
    else if (fprintf(file, "timecontrol %u %u\n",
        data->timeControl.selected.timeLeft,
        data->timeControl.selected.increment) < 0)
        success = false;

    fclose(file);
    SDL_Log("Saved file to %s\n", configPath);
    return success;
}
