#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#include "../../engine/src/utils/Math.h"
#include "../../engine/src/utils/FenString.h"
#include "../../engine/src/utils/CharBuffer.h"
#include "../../engine/src/bot/Evaluation.h"
#include "../../engine/src/bot/PieceSquareTable.h"
#include "../../engine/src/magicBitBoard/MagicBitBoard.h"

#include "../sdl_framework/State.h"
#include "../sdl_framework/AppRunner.h"
#include "../sdl_framework/AppInit.h"
#include "../sdl_framework/Render.h"
#include "../sdl_framework/AppCleanup.h"

#define WINDOWN_TITLE ("Evaluation Visualization")
#define WINDOW_WIDTH (640)
#define WINDOW_HEIGHT (640 + BUTTON_HEIGHT)
#define SQUARE_SIZE (WINDOW_WIDTH / BOARD_LENGTH) // Size of each square

#define CHESSBOARD_RECT ((SDL_Rect) {0, 0, WINDOW_WIDTH, WINDOW_WIDTH})

#define BUTTON_HEIGHT 80
#define BUTTON_RECT ((SDL_Rect) {0, WINDOW_HEIGHT - BUTTON_HEIGHT, WINDOW_WIDTH, BUTTON_HEIGHT})

#define WINDOW_RECT ((SDL_Rect) {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT})

#define SQUARE_COLOR_1 ((SDL_Color) {100, 100, 100, 255})  // Medium gray
#define SQUARE_COLOR_2 ((SDL_Color) {50, 50, 50, 255})     // Darker gray
#define TEXT_COLOR ((SDL_Color) {0, 0, 0, 255})            // black
#define BUTTON_COLOR ((SDL_Color) {200, 200, 200, 255})    // gray-ish white

#define FONT_SIZE 24
#define FONT_PATH ("./assets/font/cmunbl.ttf")

#define POSITIONS_PATH ("./assets/chessPositions.txt")

#define MAX_NB_LENGTH (12)

typedef struct {
    SDL_Color color;
    char* text;
} Button;

typedef struct FenStringFileData {
    // This is about 237 kb, which is fine
    Tokens* lines;
    int currentLineIndex;
    int totalLinesInFile;
} FenStringFileData;

typedef struct BotEvaluation {
    ChessPosition position;
    int staticEvaluation;
} BotEvaluation;

struct AppState {
    SDL_State sdlState;
    Textures textures;
    BotEvaluation evaluation;
    FenStringFileData fenStringFileData;
    Button button;
};

void drawChessBoard(AppState* state) {
    SDL_Rect rect = CHESSBOARD_RECT;
    int squareSize = rect.w / BOARD_LENGTH;

    for (int squareIndex = 0; squareIndex < BOARD_SIZE; squareIndex++) {
        int row = rank(squareIndex);
        int col = file(squareIndex);

        SDL_Color color = ((row + col) % 2 == 0) ? SQUARE_COLOR_1 : SQUARE_COLOR_2;
        SDL_SetRenderDrawColor(state->sdlState.renderer, color.r, color.g, color.b, color.a);
        SDL_Rect square = { rect.x + col * squareSize, rect.y + row * squareSize, squareSize, squareSize };
        SDL_RenderFillRect(state->sdlState.renderer, &square);

        Piece piece = Board_pieceAtIndex(state->evaluation.position.board, squareIndex);
        if (piece != NOPIECE) {
            SDL_Rect pieceRect = { square.x, square.y, squareSize, squareSize };
            int index = piece - (Piece_color(piece) == WHITE ? 9 : 11);
            SDL_RenderCopy(state->sdlState.renderer, state->textures.data[index].texture, NULL, &pieceRect);
        }
    }
}

void drawButton(AppState* state) {
    // Set button text
    snprintf(state->button.text, MAX_NB_LENGTH, "%d", state->evaluation.staticEvaluation);

    // Draw button background
    SDL_SetRenderDrawColor(state->sdlState.renderer, state->button.color.r, state->button.color.g, state->button.color.b, state->button.color.a);
    SDL_RenderFillRect(state->sdlState.renderer, &BUTTON_RECT);

    // Draw button border
    SDL_SetRenderDrawColor(state->sdlState.renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(state->sdlState.renderer, &BUTTON_RECT);

    SDL_Surface* textSurface = TTF_RenderText_Blended(state->sdlState.font, state->button.text, TEXT_COLOR);
    if (textSurface != NULL) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(state->sdlState.renderer, textSurface);
        if (textTexture != NULL) {
            SDL_Rect textRect;
            textRect.w = textSurface->w;
            textRect.h = textSurface->h;
            textRect.x = BUTTON_RECT.x + (BUTTON_RECT.w - textRect.w) / 2; // Center text horizontally
            textRect.y = BUTTON_RECT.y + (BUTTON_RECT.h - textRect.h) / 2; // Center text vertically
            SDL_RenderCopy(state->sdlState.renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
}

void nextLineIndex(FenStringFileData* fenStringFileData) {
    fenStringFileData->currentLineIndex++;
}

void changePosition(AppState* state) {
    nextLineIndex(&state->fenStringFileData);
    Tokens fen = state->fenStringFileData.lines[state->fenStringFileData.currentLineIndex];
    if (!FenString_setChessPositionFromTokens(&fen, &state->evaluation.position)) {
        fprintf(stderr, "Failed to initialize the position, exiting...\n");
        exit(EXIT_FAILURE);
    }
    state->evaluation.staticEvaluation = Bot_staticEvaluation(state->evaluation.position);
}

void clickedButton(SDL_Event event, App app) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        changePosition(app.state);
        break;
    default:
        break;
    }
}

#define BUF_SIZE (1 << 8)

FenStringFileData initializeFenStringFileData(FILE* file) {
    Tokens* fens = malloc(BUF_SIZE * sizeof(Tokens));
    char fen[MAX_FEN_STRING_SIZE] = { 0 };

    int capacity = BUF_SIZE;
    int counter = 0;
    while (true) {
        if (fgets(fen, MAX_FEN_STRING_SIZE, file) == NULL) {
            if (ferror(file)) {
                fprintf(stderr, "Error when reading the file, exiting...\n");
                exit(EXIT_FAILURE);
            }
            else {
                // we have reached the end of the file
                break;
            }
        }
        
        // We ignore empty lines
        if (fen[0] == '\n') { continue; }

        size_t fenLength = 0;
        while (fen[fenLength] != '\n' && fen[fenLength] != '#') {
            fenLength++;
        }
        // If we have reached the end of fen string or we have encountered a comment
        for (; fen[fenLength] != '\n' && fen[fenLength] != '#'; fenLength++);

        fen[fenLength] = '\0';

        // We ignore lines which are only comments
        if (fenLength == 0) { continue; }

        fens[counter].length = string_removeUnecessarySpaces(fen);
        if (fens[counter].length != 6) {
            fprintf(stderr, "A fen string is composed of exactly 6 strings separated by spaces. This supposed fen string does not do that: `%s`\nExiting...", fen);
            exit(EXIT_FAILURE);
        }

        fens[counter].tokens = malloc(fens[counter].length * sizeof(char*));
        char* newFen = malloc(fenLength * sizeof(char));
        memcpy(newFen, fen, fenLength);
        string_tokenizeStringBySpace(newFen, fens + counter);
        
        counter++;

        if (counter == capacity) {
            capacity *= 2;
            fens = realloc(fens, sizeof(Tokens) * capacity);
        }
    }

    // We start the index at -1 because we increment before we display the position
    return (FenStringFileData) { .lines = fens, .currentLineIndex = -1, .totalLinesInFile = counter };
}

static const char* PIECE_NAMES[NB_PIECES] = {
    "./assets/png/white_pawn.png", "./assets/png/white_knight.png", "./assets/png/white_bishop.png", "./assets/png/white_rook.png", "./assets/png/white_queen.png", "./assets/png/white_king.png",
    "./assets/png/black_pawn.png", "./assets/png/black_knight.png", "./assets/png/black_bishop.png", "./assets/png/black_rook.png", "./assets/png/black_queen.png", "./assets/png/black_king.png"
};

bool initializeApp(App app) {
    if (!initializeSDlLibraries(SDL_INIT_VIDEO, IMG_INIT_TIF) ||
        !initializeSDLState(&app.state->sdlState,
            WINDOWN_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
            FONT_PATH, FONT_SIZE) ||
        !initializeTextures(&app.state->textures) ||
        !initializeClickableArea(app.events, 1) ||
        !loadImageFromFilePath(&app.state->sdlState, &app.state->textures, PIECE_NAMES, NB_PIECES)) {
        return false;
    }

    app.events->clickableAreas.data[0] = (ClickableArea) { .rect = WINDOW_RECT, .callback = &clickedButton };

    char* text = calloc(MAX_NB_LENGTH, sizeof(char));
    app.state->button = (Button){
        .color = BUTTON_COLOR,
        .text = text
    };

    FILE* positions = fopen(POSITIONS_PATH, "r");
    if (positions == NULL) {
        fprintf(stderr, "Unable to open the positions file\n");
        return false;
    }
    app.state->fenStringFileData = initializeFenStringFileData(positions);
    fclose(positions);

    if (!PieceSquareTable_init() || !MagicBitBoard_init()) {
        fprintf(stderr, "Error while initializing the chess engine\n");
        return false;
    }

    // Setting the initial position
    changePosition(app.state);

    app.events->hasQuitEventHappened = false;
    return true;
}

void render(App app) {
    SDL_RenderClear(app.state->sdlState.renderer);

    drawChessBoard(app.state);
    drawButton(app.state);

    SDL_RenderPresent(app.state->sdlState.renderer);
}

void cleanupApp(App app) {
    MagicBitBoard_terminate();

    free(app.state->button.text);

    for (int index = 0; index < app.state->fenStringFileData.totalLinesInFile; index++) {
        // Please note that since the tokens were created with the string_tokenizeStringBySpace
        // function than the elements of the char** tokens array are actually the same string
        // that had its space replace by null terminator. This means that we can simply free the
        // first element of the array and then the array itself
        free(app.state->fenStringFileData.lines[index].tokens[0]);
        free(app.state->fenStringFileData.lines[index].tokens);
    }

    free(app.state->fenStringFileData.lines);

    cleanupClickableAreas(app.events);
    cleanupTextures(app.state->textures);
    cleanupSDL_State(app.state->sdlState);
    quitSDL();
}

// make visualizeEval
int main() {
    AppState appState = { 0 };
    AppEvents appEvents = { 0 };
    App app = { .events = &appEvents, .state = &appState };

    if (!runApp(app)) {
        fprintf(stderr, "An error occurred while running the app.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
