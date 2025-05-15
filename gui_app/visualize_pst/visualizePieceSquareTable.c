/*
Chatgpt generated all the SLD specific code, since I was lazy
and did not want to read documentation.
*/
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>

#include "../../engine/src/utils/Math.h"
#include "../../engine/src/bot/PieceSquareTable.h"

#include "../sdl_framework/State.h"
#include "../sdl_framework/AppRunner.h"
#include "../sdl_framework/AppInit.h"
#include "../sdl_framework/Render.h"
#include "../sdl_framework/AppCleanup.h"

#define WINDOWN_TITLE ("Piece Square Table Visualization")
#define WINDOW_WIDTH (640)
#define WINDOW_HEIGHT (640 + BUTTON_HEIGHT)
#define SQUARE_SIZE (WINDOW_WIDTH / BOARD_LENGTH) // Size of each square

#define BUTTON_HEIGHT 80
#define BUTTON_RECT ((SDL_Rect) {0, WINDOW_HEIGHT - BUTTON_HEIGHT, WINDOW_WIDTH, BUTTON_HEIGHT})


#define SQUARE_COLOR ((SDL_Color) {255, 0, 0, 255}) // red
#define TEXT_COLOR ((SDL_Color) {0, 0, 0, 255}) // black
#define BUTTON_COLOR ((SDL_Color) {200, 200, 200, 255})

#define FONT_SIZE 24
#define FONT_PATH ("./assets/font/cmunbl.ttf")

typedef struct {
    SDL_Color color;
    char* text;
} Button;

typedef struct PST_State {
    PieceCharacteristics color;
    PieceCharacteristics type;
    GamePhase phase;
} PST_State;

struct AppState {
    SDL_State sdlState;
    PST_State pstState;
    Button button;
};

float normalizingFunction(int squareScore, int maxValue, int minValue) {
    return (float)(squareScore - minValue) / (maxValue - minValue);
}

void visualizePieceSquareTable(SDL_Renderer* renderer, TTF_Font* font, int pieceSquareTable[BOARD_SIZE], int maxValue, int minValue, int phase) {

    for (int row = 0; row < BOARD_LENGTH; row++) {
        for (int col = 0; col < BOARD_LENGTH; col++) {
            // Set render color
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            int squareIndex = row * 8 + col;
            int midEgMixed = pieceSquareTable[squareIndex];
            int squareScore = phase == MIDGAME ? mg_value(midEgMixed) : eg_value(midEgMixed);
            Uint8 opacity = (Uint8)(normalizingFunction(abs(squareScore), maxValue, minValue) * 255.0);
            opacity = max(100, opacity);

            SDL_SetRenderDrawColor(renderer,
                SQUARE_COLOR.r, SQUARE_COLOR.g, SQUARE_COLOR.b,
                opacity);

            // Define the rectangle for the square
            SDL_Rect square = {
                col * SQUARE_SIZE, // X position
                row * SQUARE_SIZE, // Y position
                SQUARE_SIZE,       // Width
                SQUARE_SIZE        // Height
            };

            // Render the square
            SDL_RenderFillRect(renderer, &square);

            char number[6];
            snprintf(number, sizeof(number), "%d", squareScore);

            // Render the text
            SDL_Surface* textSurface = TTF_RenderText_Blended(font, number, TEXT_COLOR);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

            // Get text dimensions
            int textWidth = textSurface->w;
            int textHeight = textSurface->h;

            SDL_FreeSurface(textSurface);

            // Define the position to center the text in the square
            SDL_Rect textRect = {
                square.x + (SQUARE_SIZE - textWidth) / 2, // Center horizontally
                square.y + (SQUARE_SIZE - textHeight) / 2, // Center vertically
                textWidth,
                textHeight
            };

            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
    }
}

void drawPieceSquareTable(SDL_Renderer* renderer, TTF_Font* font, PST_State pstState) {
    PieceCharacteristics piece = Piece_makePiece(pstState.color, pstState.type);
    int* pst = pieceSquareTable[piece - 9];
    int maxValue = INT16_MIN;
    int minValue = INT16_MAX;
    for (int square = 0; square < BOARD_SIZE; square++) {
        int score = pstState.phase == MIDGAME ? mg_value(pst[square]) : eg_value(pst[square]);
        maxValue = max(abs(score), maxValue);
        minValue = min(abs(score), minValue);
    }

    // Draw the chessboard
    visualizePieceSquareTable(renderer, font, pst, maxValue, minValue, pstState.phase);
}


void updateButtonText(char* buttonText, PST_State pstState) {
    int currentIndex = 0;
    const int colorTextLength = 6;
    switch (pstState.color) {
    case WHITE:
        memcpy(buttonText + currentIndex, "White ", currentIndex + colorTextLength);
        currentIndex += colorTextLength;
        break;
    case BLACK:
        memcpy(buttonText + currentIndex, "Black ", currentIndex + colorTextLength);
        currentIndex += colorTextLength;
        break;
    default:
        break;
    }
    switch (pstState.type) {
    case PAWN:
        memcpy(buttonText + currentIndex, "Pawn ", 5);
        currentIndex += 5;
        break;
    case KNIGHT:
        memcpy(buttonText + currentIndex, "Knight ", 7);
        currentIndex += 7;
        break;
    case BISHOP:
        memcpy(buttonText + currentIndex, "Bishop ", 7);
        currentIndex += 7;
        break;
    case ROOK:
        memcpy(buttonText + currentIndex, "Rook ", 5);
        currentIndex += 5;
        break;
    case QUEEN:
        memcpy(buttonText + currentIndex, "Queen ", 6);
        currentIndex += 6;
        break;
    case KING:
        memcpy(buttonText + currentIndex, "King ", 5);
        currentIndex += 5;
        break;
    default:
        break;
    }
    switch (pstState.phase)
    {
    case MIDGAME:
        memcpy(buttonText + currentIndex, "Midgame", 8);
        break;
    case ENDGAME:
        memcpy(buttonText + currentIndex, "Endgame", 8);
        break;
    default:
        break;
    }
}


void drawButton(SDL_Renderer* renderer, TTF_Font* font, Button button, PST_State pstState) {
    updateButtonText(button.text, pstState);

    // Draw button background
    SDL_SetRenderDrawColor(renderer, button.color.r, button.color.g, button.color.b, button.color.a);
    SDL_RenderFillRect(renderer, &BUTTON_RECT);

    // Draw button border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &BUTTON_RECT);

    SDL_Surface* textSurface = TTF_RenderText_Blended(font, button.text, TEXT_COLOR);
    if (textSurface != NULL) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (textTexture != NULL) {
            SDL_Rect textRect;
            textRect.w = textSurface->w;
            textRect.h = textSurface->h;
            textRect.x = BUTTON_RECT.x + (BUTTON_RECT.w - textRect.w) / 2; // Center text horizontally
            textRect.y = BUTTON_RECT.y + (BUTTON_RECT.h - textRect.h) / 2; // Center text vertically
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
}

void updatePSTPParam(PST_State* pstState) {
    pstState->type = pstState->type % NB_PIECE_TYPE + PAWN; // Always update the piece type
    if (pstState->type == PAWN) { // When we switch back to a pawn
        pstState->color = pstState->color % BLACK + WHITE;
        if (pstState->color == WHITE) { // When we switch back to white
            pstState->phase = (pstState->phase + 1) % NB_PHASE;
        }
    }
}

void clickedButton(SDL_Event event, App app) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        updatePSTPParam(&app.state->pstState);
        break;
    default:
        break;
    }
}

bool initializeApp(App app) {
    if (!initializeSDlLibraries(SDL_INIT_VIDEO, IMG_INIT_TIF) ||
        !initializeSDLState(&app.state->sdlState,
        WINDOWN_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
        FONT_PATH, FONT_SIZE) || 
        !initializeClickableArea(app.events, 1)) {
        return false;
    }

    app.events->clickableAreas.data[0] = (ClickableArea){ .rect = BUTTON_RECT, .callback = &clickedButton };

    char* text = calloc(21, sizeof(char));
    memcpy(text,  "White Pawn Midgame", 19);
    app.state->button = (Button){
        .color = BUTTON_COLOR,
        .text = text
    };

    app.state->pstState = (PST_State){
        .type = PAWN,
        .color = WHITE,
        .phase = MIDGAME
    };

    if (!PieceSquareTable_init()) return false;

    app.events->hasQuitEventHappened = false;
    return true;
}

void render(App app) {
    SDL_RenderClear(app.state->sdlState.renderer);

    drawPieceSquareTable(app.state->sdlState.renderer, app.state->sdlState.font, app.state->pstState);
    drawButton(app.state->sdlState.renderer, app.state->sdlState.font, app.state->button, app.state->pstState);

    SDL_RenderPresent(app.state->sdlState.renderer);
}

void cleanupApp(App app) {
    free(app.state->button.text);
    cleanupClickableAreas(app.events);
    cleanupSDL_State(app.state->sdlState);
    quitSDL();
}

// make visualizePST
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
