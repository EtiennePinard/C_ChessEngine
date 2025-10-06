// TODO: Use SDL's file api instead of stdio
#include <stdio.h>

#include "../../engine/src/utils/CharBuffer.h"

#include "gameScene/GameScene.h"

#include "Config.h"

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

bool parseTimeControl(TimeControl* timecontrol, Tokens tokens) {
    // We need at least three tokens for a time control line
    if (tokens.nbTokens < 3) return false;

    int timeleft = string_parseNumber(tokens.tokens[1]);
    int increment = string_parseNumber(tokens.tokens[2]);
    if (timeleft == -1 || increment == -1) return false;
    timecontrol->timeLeft = timeleft;
    timecontrol->increment = increment;
    return true;
}

bool parseIsEngine(bool* isEngine, Tokens tokens) {
    // We need at least two tokens for an isEngine line
    if (tokens.nbTokens < 2) return false;
    if (string_compareStrings("true", tokens.tokens[1])) *isEngine = true;
    else if (string_compareStrings("false", tokens.tokens[1])) *isEngine = false;
    else return false; // Is not "true" nor "false" so we cannot parse it

    return true;
}

bool parseEnginePath(char** engineConfigPath, Tokens tokens, size_t read) {
    // We need at least two tokens for an engine path line
    if (tokens.nbTokens < 2) return false;

    if (string_compareStrings(tokens.tokens[1], "NULL")) {
        // The engine path is NULL
        *engineConfigPath = NULL;
        return true;
    }

    // length of <b | w>EnginePath<space> is 12
    size_t enginePathLength = read - 12 + tokens.nbTokens + 1;
    char* enginePath = SDL_calloc(enginePathLength, sizeof(char));
    size_t enginePathIndex = 0;
    for (size_t tokenIndex = 1; tokenIndex < tokens.nbTokens; tokenIndex++) {
        char* currentToken = tokens.tokens[tokenIndex];
        SDL_memcpy(enginePath + enginePathIndex, currentToken, SDL_strlen(currentToken));
        enginePathIndex += SDL_strlen(currentToken);
        enginePath[enginePathIndex++] = SPACE_CHAR;
    }
    // Removing the last space char
    enginePath[enginePathIndex - 1] = '\0';
    *engineConfigPath = enginePath;
    return true;
}

bool parseThinkTime(TimeControl_MS* timeToThink, Tokens tokens) {
    // We need at least two tokens for a time to think line
    if (tokens.nbTokens < 2) return false;

    int parsedTimeToThink = string_parseNumber(tokens.tokens[1]);
    // Either the bot decides how much it thinks (0) or it thinks for a specified amount of time
    *timeToThink = SDL_max(parsedTimeToThink, 0);

    return true;
}

bool parseStartingPosition(char** fen, Tokens tokens, size_t read) {
    // We need at least seven tokens for the startingPosition line
    if (tokens.nbTokens < 7) return false;

    // length of startingPosition<space> is 17
    size_t parsedFenLength = read - 17 + tokens.nbTokens + 1;
    char* parsedFen = SDL_calloc(parsedFenLength, sizeof(char));
    size_t parsedFenIndex = 0;
    for (size_t tokenIndex = 1; tokenIndex < tokens.nbTokens; tokenIndex++) {
        char* currentToken = tokens.tokens[tokenIndex];
        SDL_memcpy(parsedFen + parsedFenIndex, currentToken, SDL_strlen(currentToken));
        parsedFenIndex += SDL_strlen(currentToken);
        parsedFen[parsedFenIndex++] = SPACE_CHAR;
    }
    // Removing the last space char
    parsedFen[parsedFenIndex - 1] = '\0';
    *fen = parsedFen;
    return true;
}

#define WHITE_TIME_CONTROL_TOKEN ("wTimeControl")
#define WHITE_IS_ENGINE_TOKEN ("wIsEngine")
#define WHITE_ENGINE_PATH_TOKEN ("wEnginePath")
#define WHITE_THINK_TIME_TOKEN ("wThinkTime")

#define BLACK_TIME_CONTROL_TOKEN ("bTimeControl")
#define BLACK_IS_ENGINE_TOKEN ("bIsEngine")
#define BLACK_ENGINE_PATH_TOKEN ("bEnginePath")
#define BLACK_THINK_TIME_TOKEN ("bThinkTime")

#define STARTING_POSITION_TOKEN ("startingPosition")

#define TOTAL_GAME_CONFIG_TOKEN (9)

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

    size_t tokensRead = 0;

    while ((line = portableSubsetGetLine(file)) != NULL) {
        size_t read = SDL_strlen(line);
        nbTokens = string_removeUnecessarySpacesAndTabs(line);
        char* tokens_arr[nbTokens];
        tokens.nbTokens = nbTokens;
        tokens.tokens = tokens_arr;
        string_tokenizeStringBySpace(line, &tokens);

        // ignore any empty lines
        if (tokens.nbTokens == 0) continue;

        // Check if the string starts with any of the character, else we ignore the line
        if (string_compareStrings(tokens.tokens[0], WHITE_TIME_CONTROL_TOKEN)) {
            if (!parseTimeControl(&data->white.timeControl, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], WHITE_IS_ENGINE_TOKEN)) {
            if (!parseIsEngine(&data->white.engineConfig.isEngine, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], WHITE_ENGINE_PATH_TOKEN)) {
            if (!parseEnginePath(&data->white.engineConfig.enginePath, tokens, read)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], WHITE_THINK_TIME_TOKEN)) {
            if (!parseThinkTime(&data->white.engineConfig.timeToThink, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], BLACK_TIME_CONTROL_TOKEN)) {
            if (!parseTimeControl(&data->black.timeControl, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], BLACK_IS_ENGINE_TOKEN)) {
            if (!parseIsEngine(&data->black.engineConfig.isEngine, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], BLACK_ENGINE_PATH_TOKEN)) {
            if (!parseEnginePath(&data->black.engineConfig.enginePath, tokens, read)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], BLACK_THINK_TIME_TOKEN)) {
            if (!parseThinkTime(&data->black.engineConfig.timeToThink, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], STARTING_POSITION_TOKEN)) {
            if (!parseStartingPosition(&data->startingPositionFen, tokens, read)) goto end_of_parsing_file;
            tokensRead++;
        }
        SDL_free(line);
    }

    result = tokensRead == TOTAL_GAME_CONFIG_TOKEN;
end_of_parsing_file:
    SDL_free(line);
    return result;
}

void loadGameConfig(GameConfig* data) {
    char* basePath = SDL_GetPrefPath("Etienne", "ChessApp");
    if (!basePath) goto fallback;

    // Construct the full file path
    int length = SDL_snprintf(NULL, 0, "%schess.config", basePath) + 1;
    char* configPath = SDL_calloc(length, sizeof(char));
    SDL_snprintf(configPath, length, "%schess.config", basePath);
    SDL_free(basePath);

    SDL_Log("Reading config from %s\n", configPath);
    FILE* file = fopen(configPath, "r");
    if (loadMainMenuConfigFromFile(file, data)) {
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
    data->white.engineConfig.timeToThink = DEFAULT_TIME_TO_THINK;
    data->white.timeControl = DEFAULT_TIME_CONTROL;

    data->black.engineConfig.isEngine = false;
    data->black.engineConfig.enginePath = NULL;
    data->black.engineConfig.timeToThink = DEFAULT_TIME_TO_THINK;
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

bool saveGameConfig(const GameConfig* data) {
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

bool parseColor(SDL_Color* color, Tokens tokens) {
    // We need at least five tokens for a color line
    if (tokens.nbTokens < 5) return false;

    int r = string_parseNumber(tokens.tokens[1]);
    int g = string_parseNumber(tokens.tokens[2]);
    int b = string_parseNumber(tokens.tokens[3]);
    int a = string_parseNumber(tokens.tokens[4]);
    if (r == -1 || g == -1 || b == -1 || a == -1) return false;
    color->r = r;
    color->g = g;
    color->b = b;
    color->a = a;

    return true;
}

#define BG_COLOR_TOKEN ("backgroundColor")
#define MODAL_BG_COLOR_TOKEN ("modalBackgroundColor")
#define CREDITS_COLOR_TOKEN ("creditsColor")

#define SQUARE1_COLOR_TOKEN ("square1Color")
#define SQUARE2_COLOR_TOKEN ("square2Color")
#define HIGHLIGHT_SQUARE_COLOR_TOKEN ("highlightSquareColor")

#define TEXT_COLOR_TOKEN ("textColor")
#define SELECTED_TEXT_COLOR_TOKEN ("selectedTextColor")
#define SELECTED_TEXT_BG_COLOR_TOKEN ("selectedTextBgColor")

#define BUTTON_IDLE_COLOR_TOKEN ("buttonIdleColor")
#define BUTTON_BORDER_COLOR_TOKEN ("buttonBorderColor")
#define BUTTON_HOVER_COLOR_TOKEN ("buttonHoverColor")
#define BUTTON_CLICKED_COLOR_TOKEN ("buttonClickedColor")

#define CHECKBOX_BORDER_COLOR_TOKEN ("checkboxBorderColor")
#define CHECKBOX_HOVER_COLOR_TOKEN ("checkboxHoverColor")
#define CHECKBOX_CHECKED_COLOR_TOKEN ("checkboxCheckedColor")

#define SCROLLBAR_IDLE_COLOR_TOKEN ("scrollbarIdleColor")
#define SCROLLBAR_HOVER_COLOR_TOKEN ("scrollbarHoverColor")
#define SCROLLBAR_CLICKED_COLOR_TOKEN ("scrollbarClickedColor")

#define TOTAL_STYLE_TOKEN (19)
/*
Parses the save file for the app's style. Note that a color is a 4 byte struct
with the 1st byte being the red, the 2nd being green, 3rd being blue and
the 4th being the opacity. This is what a valid file would look like:

backgroundColor r g b a
modalBackgroundColor r g b a
creditsColor r g b a

square1Color r g b a
square2Color r g b a
highlightSquareColor r g b a

textColor r g b a
selectedTextColor r g b a
selectedTextBgColor r g b a

buttonIdleColor r g b a
buttonBorderColor r g b a
buttonHoverColor r g b a
buttonClickedColor r g b a

checkboxBorderColor r g b a
checkboxHoverColor r g b a
checkboxCheckedColor r g b a

scrollbarIdleColor r g b a
scrollbarHoverColor r g b a
scrollbarClickedColor r g b a

*/
bool loadAppStyleFromFile(FILE* file, AppStyle* data) {
    bool result = false;
    if (!file || !data) return result;

    char* line = NULL;
    Tokens tokens;
    size_t nbTokens;

    size_t tokensRead = 0;
    while ((line = portableSubsetGetLine(file)) != NULL) {
        nbTokens = string_removeUnecessarySpacesAndTabs(line);
        char* tokens_arr[nbTokens];
        tokens.nbTokens = nbTokens;
        tokens.tokens = tokens_arr;
        string_tokenizeStringBySpace(line, &tokens);

        // ignore any empty lines
        if (tokens.nbTokens == 0) continue;

        // Check if the string starts with any of the character, else we ignore the line
        if (string_compareStrings(tokens.tokens[0], BG_COLOR_TOKEN)) {
            if (!parseColor(&data->backgroundColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], MODAL_BG_COLOR_TOKEN)) {
            if (!parseColor(&data->modalBgColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], CREDITS_COLOR_TOKEN)) {
            if (!parseColor(&data->creditsColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }

        else if (string_compareStrings(tokens.tokens[0], SQUARE1_COLOR_TOKEN)) {
            if (!parseColor(&data->chessboardStyle.square1Color, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], SQUARE2_COLOR_TOKEN)) {
            if (!parseColor(&data->chessboardStyle.square2Color, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], HIGHLIGHT_SQUARE_COLOR_TOKEN)) {
            if (!parseColor(&data->chessboardStyle.highlightSquareColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }

        else if (string_compareStrings(tokens.tokens[0], TEXT_COLOR_TOKEN)) {
            if (!parseColor(&data->textStyle.textColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], SELECTED_TEXT_COLOR_TOKEN)) {
            if (!parseColor(&data->textStyle.selectedTextColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], SELECTED_TEXT_BG_COLOR_TOKEN)) {
            if (!parseColor(&data->textStyle.selectedTextBgColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }

        else if (string_compareStrings(tokens.tokens[0], BUTTON_IDLE_COLOR_TOKEN)) {
            if (!parseColor(&data->buttonStyle.idleColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], BUTTON_BORDER_COLOR_TOKEN)) {
            if (!parseColor(&data->buttonStyle.borderColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], BUTTON_HOVER_COLOR_TOKEN)) {
            if (!parseColor(&data->buttonStyle.hoverColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], BUTTON_CLICKED_COLOR_TOKEN)) {
            if (!parseColor(&data->buttonStyle.clickedColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }

        else if (string_compareStrings(tokens.tokens[0], CHECKBOX_BORDER_COLOR_TOKEN)) {
            if (!parseColor(&data->checkboxStyle.borderColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], CHECKBOX_HOVER_COLOR_TOKEN)) {
            if (!parseColor(&data->checkboxStyle.hoverColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], CHECKBOX_CHECKED_COLOR_TOKEN)) {
            if (!parseColor(&data->checkboxStyle.checkedColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }

        else if (string_compareStrings(tokens.tokens[0], SCROLLBAR_IDLE_COLOR_TOKEN)) {
            if (!parseColor(&data->scrollbarStyle.idleColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], SCROLLBAR_HOVER_COLOR_TOKEN)) {
            if (!parseColor(&data->scrollbarStyle.hoverColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }
        else if (string_compareStrings(tokens.tokens[0], SCROLLBAR_CLICKED_COLOR_TOKEN)) {
            if (!parseColor(&data->scrollbarStyle.clickedColor, tokens)) goto end_of_parsing_file;
            tokensRead++;
        }

        SDL_free(line);
    }
    result = tokensRead == TOTAL_STYLE_TOKEN;
end_of_parsing_file:
    SDL_free(line);
    return result;
}

void loadAppStyle(AppStyle* data) {
    char* basePath = SDL_GetPrefPath("Etienne", "ChessApp");
    if (!basePath) goto fallback;

    // Construct the full file path
    int length = SDL_snprintf(NULL, 0, "%sstyle.config", basePath) + 1;
    char* configPath = SDL_calloc(length, sizeof(char));
    SDL_snprintf(configPath, length, "%sstyle.config", basePath);
    SDL_free(basePath);

    SDL_Log("Reading config from %s\n", configPath);
    FILE* file = fopen(configPath, "r");
    if (loadAppStyleFromFile(file, data)) {
        fclose(file);
        SDL_free(configPath);
        return;
    }

    if (file) fclose(file);

fallback:
    // Fallback to default style
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not load style from file\n");
    SDL_memcpy(data, &defaultStyle, sizeof(AppStyle));
}

bool writeColorBlock(FILE* file, const char* blockName, SDL_Color colorToWrite) {
    return fprintf(file, "%s %u %u %u %u\n",
        blockName,
        colorToWrite.r, colorToWrite.g, colorToWrite.b, colorToWrite.a) > 0;
}

bool saveAppStyle(const AppStyle* data) {
    if (!data) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not save style data is NULL\n");
        return false;
    }

    char* basePath = SDL_GetPrefPath("Etienne", "ChessApp");
    if (!basePath) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not save style SDL_GetPrefPath returns NULL\n");
        return false;
    }

    // Construct the full file path
    size_t length = SDL_snprintf(NULL, 0, "%sstyle.config", basePath) + 1;
    char configPath[length];
    SDL_snprintf(configPath, length, "%sstyle.config", basePath);
    SDL_free(basePath);

    FILE* file = fopen(configPath, "w");
    if (!file) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not save config since we could not open the config file\n");
        return false;
    }

    bool success = true;
    success &= writeColorBlock(file, BG_COLOR_TOKEN, data->backgroundColor);
    success &= writeColorBlock(file, MODAL_BG_COLOR_TOKEN, data->modalBgColor);
    success &= writeColorBlock(file, CREDITS_COLOR_TOKEN, data->creditsColor);

    success &= writeColorBlock(file, SQUARE1_COLOR_TOKEN, data->chessboardStyle.square1Color);
    success &= writeColorBlock(file, SQUARE2_COLOR_TOKEN, data->chessboardStyle.square2Color);
    success &= writeColorBlock(file, HIGHLIGHT_SQUARE_COLOR_TOKEN, data->chessboardStyle.highlightSquareColor);

    success &= writeColorBlock(file, TEXT_COLOR_TOKEN, data->textStyle.textColor);
    success &= writeColorBlock(file, SELECTED_TEXT_COLOR_TOKEN, data->textStyle.selectedTextColor);
    success &= writeColorBlock(file, SELECTED_TEXT_BG_COLOR_TOKEN, data->textStyle.selectedTextBgColor);

    success &= writeColorBlock(file, BUTTON_IDLE_COLOR_TOKEN, data->buttonStyle.idleColor);
    success &= writeColorBlock(file, BUTTON_BORDER_COLOR_TOKEN, data->buttonStyle.borderColor);
    success &= writeColorBlock(file, BUTTON_HOVER_COLOR_TOKEN, data->buttonStyle.hoverColor);
    success &= writeColorBlock(file, BUTTON_CLICKED_COLOR_TOKEN, data->buttonStyle.clickedColor);

    success &= writeColorBlock(file, CHECKBOX_BORDER_COLOR_TOKEN, data->checkboxStyle.borderColor);
    success &= writeColorBlock(file, CHECKBOX_HOVER_COLOR_TOKEN, data->checkboxStyle.hoverColor);
    success &= writeColorBlock(file, CHECKBOX_CHECKED_COLOR_TOKEN, data->checkboxStyle.checkedColor);

    success &= writeColorBlock(file, SCROLLBAR_IDLE_COLOR_TOKEN, data->scrollbarStyle.idleColor);
    success &= writeColorBlock(file, SCROLLBAR_HOVER_COLOR_TOKEN, data->scrollbarStyle.hoverColor);
    success &= writeColorBlock(file, SCROLLBAR_CLICKED_COLOR_TOKEN, data->scrollbarStyle.clickedColor);

    fclose(file);
    if (success) SDL_Log("Saved file to %s\n", configPath);
    else SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not save file to %s\n", configPath);

    return success;
}
