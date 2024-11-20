/*
Chatgpt generated all the SLD specific code, since I was lazy
and did not want to read documentation. 
*/
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>

#include "../src/utils/Utils.h"
#include "../src/chessBot/PieceSquareTable.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT (640 + BUTTON_HEIGHT)
#define SQUARE_SIZE (WINDOW_WIDTH / BOARD_LENGTH) // Size of each square
#define BUTTON_HEIGHT 80

#define SQUARE_COLOR ((SDL_Color) {255, 0, 0, 255}) // blue
#define TEXT_COLOR ((SDL_Color) {0, 0, 0, 255}) // black

typedef struct {
    SDL_Rect rect;
    SDL_Color color;
    const char *text;
} Button;

float normalizingFunction(int squareScore, int maxValue, int minValue) {
    return (float) (squareScore - minValue) / (maxValue - minValue);
}

void visualizePieceSquareTable(SDL_Renderer *renderer, TTF_Font *font, int pieceSquareTable[BOARD_SIZE], int maxValue,  int minValue, int phase) {

    for (int row = 0; row < BOARD_LENGTH; row++) {
        for (int col = 0; col < BOARD_LENGTH; col++) {
            // Set render color
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            int squareIndex = row * 8 + col;
            int midEgMixed = pieceSquareTable[squareIndex];
            int squareScore = phase == MIDGAME ? mg_value(midEgMixed) : eg_value(midEgMixed);
            Uint8 opacity = (Uint8) (normalizingFunction(abs(squareScore), maxValue, minValue) * 255);

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
            SDL_Surface *textSurface = TTF_RenderText_Blended(font, number, TEXT_COLOR);
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

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

void drawButton(SDL_Renderer* renderer, TTF_Font* font, Button button) {
    // Draw button background
    SDL_SetRenderDrawColor(renderer, button.color.r, button.color.g, button.color.b, button.color.a);
    SDL_RenderFillRect(renderer, &button.rect);

    // Draw button border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &button.rect);


    SDL_Color textColor = {0, 0, 0, 255}; // Black text
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, button.text, textColor);
    if (textSurface) {
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (textTexture) {
            SDL_Rect textRect;
            textRect.w = textSurface->w;
            textRect.h = textSurface->h;
            textRect.x = button.rect.x + (button.rect.w - textRect.w) / 2; // Center text horizontally
            textRect.y = button.rect.y + (button.rect.h - textRect.h) / 2; // Center text vertically
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
}

bool is_mouse_inside(const SDL_Rect *rect, int x, int y) {
    return x >= rect->x && x <= rect->x + rect->w && y >= rect->y && y <= rect->y + rect->h;
}

void sendPieceSquareTableForDrawing(SDL_Renderer *renderer, TTF_Font *font, Piece piece, GamePhase phase) {
    int* pawnSquareTable = pieceSquareTable[piece - 9];
    int maxValue = INT16_MIN;
    int minValue = INT16_MAX;
    for (int square = 0; square < BOARD_SIZE; square++) {
        int score = phase == MIDGAME ? mg_value(pawnSquareTable[square]) : eg_value(pawnSquareTable[square]);
        maxValue = max(abs(score), maxValue);
        minValue = min(abs(score), minValue);
    }

    // Draw the chessboard
    visualizePieceSquareTable(renderer, font, pawnSquareTable, maxValue, minValue, phase);
}

void updatePSTPParam(PieceCharacteristics* color, PieceCharacteristics* type, GamePhase* phase) {
    *type = *type % NB_PIECE_TYPE + PAWN; // Always update the piece type
    if (*type == PAWN) { // When we switch back to a pawn
        *color = *color % BLACK + WHITE;
        if (*color == WHITE) { // When we switch back to white
            *phase = (*phase + 1) % NB_PHASE;
        }
    }
}

void updateButtonText(char* buttonText, PieceCharacteristics color, PieceCharacteristics type, GamePhase phase) {
    int currentIndex = 0;
    const int colorTextLength = 6;
    switch (color) {
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
    switch (type) {
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
    switch (phase)
    {
    case MIDGAME:
        memcpy(buttonText + currentIndex, "Midgame", 8);
        break;
    case ENGGAME:
        memcpy(buttonText + currentIndex, "Endgame", 8);
        break;
    default:
        break;
    }
}

// gcc visualizePieceSquareTable.c ../src/chessBot/pieceSquareTable.c -lSDL2 -lSDL2_ttf && ./a.out
int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Piece Square Table Visualization",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH,
                                          WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_DestroyWindow(window);
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/computer-modern/cmunbl.ttf", 24); // Set path to your .ttf font file
    if (!font) {
        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    char buttonText[21] = "White Pawn Midgame";
    Button button = {
        .rect = {0, WINDOW_HEIGHT - BUTTON_HEIGHT, WINDOW_WIDTH, BUTTON_HEIGHT}, 
        .color = {200, 200, 200, 255}, 
        .text = buttonText};


    pieceSquareTableInitialize();

    PieceCharacteristics currentPieceType = PAWN;
    PieceCharacteristics currentPieceColor = WHITE;
    GamePhase currentPhase = MIDGAME;

    sendPieceSquareTableForDrawing(renderer, font, makePiece(currentPieceColor, currentPieceType), currentPhase);
    drawButton(renderer, font, button);
    SDL_RenderPresent(renderer);
    
    // Event loop to keep the window open
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false; // Exit the loop if the user closes the window
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (!is_mouse_inside(&button.rect, x, y)) { continue; }
                updatePSTPParam(&currentPieceColor, &currentPieceType, &currentPhase);
            }
        }
        // So that opacity 0 is white not black
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        sendPieceSquareTableForDrawing(renderer, font, makePiece(currentPieceColor, currentPieceType), currentPhase);
        updateButtonText(buttonText, currentPieceColor, currentPieceType, currentPhase);
        drawButton(renderer, font, button);
        SDL_RenderPresent(renderer);
    }

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();

    return 0;
}
