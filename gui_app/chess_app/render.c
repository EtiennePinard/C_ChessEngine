#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

#include "../sdl_framework/Render.h"
#include "../sdl_framework/EventHandler.h"

#include "../../engine/src/moveHandler/MoveGenerator.h"
#include "../../engine/src/state/Piece.h"
#include "../../engine/src/utils/Types.h"
#include "../../engine/src/utils/Math.h"

#include "Events.h"
#include "AppStyle.h"

static void formatTime(TimeControl_MS milliseconds, char* output, size_t outputSize) {
    if (!output || outputSize < 6) { printf("Invalid output or/and outputSize in formatTime at " __FILE__); exit(EXIT_FAILURE); }

    u32 totalSeconds = milliseconds / 1000;
    u32 minutes = totalSeconds / 60;
    u32 seconds = totalSeconds % 60;
    // Format the string as "mm:ss"
    snprintf(output, outputSize, "%02u:%02u", minutes, seconds);
}

/**
 * @brief Enum created because I hate bool params
 */
typedef enum Side {
    TOP,
    BOTTOM
} Side;

static void renderTimeControl(SDL_Renderer* renderer, TTF_Font* font, char* timeText, Side side) {
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, timeText, WHITE_COLOR);
    if (textSurface == NULL) { printf("Text Surface is NULL\n"); exit(EXIT_FAILURE); }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == NULL) { printf("Text Texture is NULL\n"); exit(EXIT_FAILURE); }

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    SDL_Rect timeControlRect;
    timeControlRect.w = textWidth + BUTTON_PADDING;
    timeControlRect.h = textHeight + BUTTON_PADDING;
    timeControlRect.x = PLACEHOLDER_X + (PLACEHOLDER_WIDTH - textWidth) / 2;
    if (side == BOTTOM) {
        timeControlRect.y = PLACEHOLDER_Y + PLACEHOLDER_HEIGHT - timeControlRect.h - BUTTON_PADDING;
    } else {
        timeControlRect.y = PLACEHOLDER_Y + BUTTON_PADDING;
    }

    SDL_Rect textRect = {
        .x = timeControlRect.x + (timeControlRect.w - textWidth) / 2,
        .y = timeControlRect.y + (timeControlRect.h - textHeight) / 2,
        .w = textWidth,
        .h = textHeight
    };

    SDL_SetRenderDrawColor(renderer, BUTTON_COLOR.r, BUTTON_COLOR.g, BUTTON_COLOR.b, BUTTON_COLOR.a);
    SDL_RenderFillRect(renderer, &timeControlRect);

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

#define TIME_TEXT_LENGTH (6)

static void renderTimeControls(SDL_Renderer* renderer, TTF_Font* font, GameState* gameState) {

    // Logic for game end in here cause I don't use threads for now
    if (gameState->result == GAME_IS_NOT_DONE) {
        u64 currentTick = SDL_GetTicks64();
        if (gameState->currentPosition.colorToGo == WHITE) {
            if (gameState->whiteRemainingTime <= currentTick - gameState->turnStartTick) {
                gameState->whiteRemainingTime = 0;
                gameState->result = BLACK_WON_ON_TIME;
            }
            else {
                gameState->whiteRemainingTime -= (currentTick - gameState->turnStartTick);
            }
        }
        else {
            if (gameState->blackRemainingTime <= currentTick - gameState->turnStartTick) {
                gameState->blackRemainingTime = 0;
                gameState->result = WHITE_WON_ON_TIME;
            }
            else {
                gameState->blackRemainingTime -= (currentTick - gameState->turnStartTick);
            }
        }
        gameState->turnStartTick = currentTick;
    }

    char blackTimeText[TIME_TEXT_LENGTH];
    formatTime(gameState->blackRemainingTime, blackTimeText, TIME_TEXT_LENGTH);
    char whiteTimeText[TIME_TEXT_LENGTH];
    formatTime(gameState->whiteRemainingTime, whiteTimeText, TIME_TEXT_LENGTH);

    renderTimeControl(renderer, font, gameState->playerColor == WHITE ? blackTimeText : whiteTimeText, TOP);
    renderTimeControl(renderer, font, gameState->playerColor == WHITE ? whiteTimeText : blackTimeText, BOTTOM);
}

static SDL_Rect renderGameStateText(SDL_Renderer* renderer, TTF_Font* font, GameState* gameState) {
    // Right now, this value needs to be checked everytime we add a new 
    // game result so that we don't have a buffer overflow. So yeah, this
    // is not very optimal
    char text[31];

    switch (gameState->result) {
    case GAME_IS_NOT_DONE:
        memcpy(text, "Game is on!", 12);
        break;
    case THREE_MOVE_REPETITION:
        memcpy(text, "Draw by\n repetition", 20);
        break;
    case STALEMATE:
        memcpy(text, "Stalemate", 10);
        break;
    case INSUFFICIENT_MATERIAL:
        memcpy(text, "Draw by\ninsufficient material", 30);
        break;
    case FIFTY_MOVE_RULE:
        memcpy(text, "Draw by\nfifty move rule", 24);
        break;
    case WHITE_WON_CHECKMATE:
        memcpy(text, "White won\nby checkmate", 23);
        break;
    case BLACK_WON_CHECKMATE:
        memcpy(text, "Black won\nby checkmate", 23);
        break;
    case WHITE_WON_ON_TIME:
        memcpy(text, "White won\non time", 18);
        break;
    case BLACK_WON_ON_TIME:
        memcpy(text, "Black won\non time", 18);
        break;
    default:
        printf("Error on switch for game state text, result is %d\n", gameState->result);
        exit(EXIT_FAILURE);
        break;
    }

    SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(font, text, WHITE_COLOR, 0);
    if (textSurface == NULL) { printf("Game result Text Surface is NULL\n"); exit(EXIT_FAILURE); }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == NULL) { printf("Game result Text Texture is NULL\n"); exit(EXIT_FAILURE); }

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_Rect textRect = {
        .x = PLACEHOLDER_X + (PLACEHOLDER_WIDTH - textWidth) / 2, // Center horizontally
        .y = PLACEHOLDER_Y + (PLACEHOLDER_HEIGHT - textHeight) / 2, // Center vertically
        .w = textWidth,
        .h = textHeight
    };
    SDL_FreeSurface(textSurface);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);

    return textRect;
}

static SDL_Rect renderRestartButton(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect textRect, ClickableAreas* clickableAreas) {
    SDL_Surface* buttonTextSurface = TTF_RenderText_Solid(font, "Restart", BUTTON_TEXT_COLOR);
    if (buttonTextSurface == NULL) { printf("Restart Text Surface is NULL\n"); exit(EXIT_FAILURE); }
    SDL_Texture* buttonTextTexture = SDL_CreateTextureFromSurface(renderer, buttonTextSurface);
    if (buttonTextSurface == NULL) { printf("Restart Text Texture is NULL\n"); exit(EXIT_FAILURE); }

    int buttonTextWidth = buttonTextSurface->w;
    int buttonTextHeight = buttonTextSurface->h;

    int buttonWidth = buttonTextWidth + BUTTON_PADDING;
    int buttonHeight = buttonTextHeight + BUTTON_PADDING;
    SDL_Rect buttonRect = {
        .x = PLACEHOLDER_X + BUTTON_PADDING,
        .y = textRect.y + textRect.h + BUTTON_PADDING,
        .w = buttonWidth,
        .h = buttonHeight };

    SDL_Rect buttonTextRect = {
        .x = buttonRect.x + (buttonRect.w - buttonTextWidth) / 2,
        .y = buttonRect.y + (buttonRect.h - buttonTextHeight) / 2,
        .w = buttonTextWidth,
        .h = buttonTextHeight
    };

    // Render the button background
    SDL_SetRenderDrawColor(renderer, BUTTON_COLOR.r, BUTTON_COLOR.g, BUTTON_COLOR.b, BUTTON_COLOR.a);
    SDL_RenderFillRect(renderer, &buttonRect);

    SDL_FreeSurface(buttonTextSurface);
    SDL_RenderCopy(renderer, buttonTextTexture, NULL, &buttonTextRect);
    SDL_DestroyTexture(buttonTextTexture);

    ClickableArea area = {
        .rect = buttonRect,
        .callback = &clickedRestartButton
    };

    clickableAreas->data[RESTART_BUTTON_INDEX] = area;

    return buttonRect;
}

static SDL_Rect renderSwitchColorButton(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect restartButtonRect, ClickableAreas* clickableAreas) {
    SDL_Surface* buttonTextSurface = TTF_RenderText_Blended_Wrapped(font, "Switch", BUTTON_TEXT_COLOR, 0);
    if (buttonTextSurface == NULL) { printf("Switch Text Surface is NULL\n"); exit(EXIT_FAILURE);; }
    SDL_Texture* buttonTextTexture = SDL_CreateTextureFromSurface(renderer, buttonTextSurface);
    if (buttonTextSurface == NULL) { printf("Switch Text Texture is NULL\n"); exit(EXIT_FAILURE);; }

    int buttonTextWidth = buttonTextSurface->w;
    int buttonTextHeight = buttonTextSurface->h;

    int buttonWidth = buttonTextWidth + BUTTON_PADDING;
    int buttonHeight = buttonTextHeight + BUTTON_PADDING;
    SDL_Rect buttonRect = {
        .x = PLACEHOLDER_X + PLACEHOLDER_WIDTH - buttonWidth - BUTTON_PADDING,
        .y = restartButtonRect.y,
        .w = buttonWidth,
        .h = buttonHeight };

    SDL_Rect buttonTextRect = {
        .x = buttonRect.x + (buttonRect.w - buttonTextWidth) / 2,
        .y = buttonRect.y + (buttonRect.h - buttonTextHeight) / 2,
        .w = buttonTextWidth,
        .h = buttonTextHeight
    };

    // Render the button background
    SDL_SetRenderDrawColor(renderer, BUTTON_COLOR.r, BUTTON_COLOR.g, BUTTON_COLOR.b, BUTTON_COLOR.a);
    SDL_RenderFillRect(renderer, &buttonRect);


    SDL_FreeSurface(buttonTextSurface);
    SDL_RenderCopy(renderer, buttonTextTexture, NULL, &buttonTextRect);
    SDL_DestroyTexture(buttonTextTexture);

    ClickableArea area = {
        .rect = buttonRect,
        .callback = &clickedSwitchColorButton
    };

    clickableAreas->data[SWITCH_BUTTON_INDEX] = area;

    return buttonRect;
}

static void renderBackButton(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect switchButtonRect, ClickableAreas* clickableAreas) {
    SDL_Surface* buttonTextSurface = TTF_RenderText_Blended_Wrapped(font, "Back", BUTTON_TEXT_COLOR, 0);
    if (buttonTextSurface == NULL) { printf("Back Text Surface is NULL\n"); exit(EXIT_FAILURE); }
    SDL_Texture* buttonTextTexture = SDL_CreateTextureFromSurface(renderer, buttonTextSurface);
    if (buttonTextSurface == NULL) { printf("Back Text Texture is NULL\n"); exit(EXIT_FAILURE); }

    int buttonTextWidth = buttonTextSurface->w;
    int buttonTextHeight = buttonTextSurface->h;

    int buttonWidth = buttonTextWidth + BUTTON_PADDING;
    int buttonHeight = buttonTextHeight + BUTTON_PADDING;
    SDL_Rect buttonRect = {
        .x = switchButtonRect.x + (switchButtonRect.w - buttonWidth) / 2,
        .y = switchButtonRect.y + switchButtonRect.h + BUTTON_PADDING,
        .w = buttonWidth,
        .h = buttonHeight };

    SDL_Rect buttonTextRect = {
        .x = buttonRect.x + (buttonRect.w - buttonTextWidth) / 2,
        .y = buttonRect.y + (buttonRect.h - buttonTextHeight) / 2,
        .w = buttonTextWidth,
        .h = buttonTextHeight
    };

    // Render the button background
    SDL_SetRenderDrawColor(renderer, BUTTON_COLOR.r, BUTTON_COLOR.g, BUTTON_COLOR.b, BUTTON_COLOR.a);
    SDL_RenderFillRect(renderer, &buttonRect);


    SDL_FreeSurface(buttonTextSurface);
    SDL_RenderCopy(renderer, buttonTextTexture, NULL, &buttonTextRect);
    SDL_DestroyTexture(buttonTextTexture);

    ClickableArea area = {
        .rect = buttonRect,
        .callback = &clickedBackButton
    };

    clickableAreas->data[BACK_BUTTON_INDEX] = area;

}

static void renderCopyFenButton(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect restartButtonRect, ClickableAreas* clickableAreas) {
    SDL_Surface* buttonTextSurface = TTF_RenderText_Blended_Wrapped(font, "Copy Fen", BUTTON_TEXT_COLOR, 0);
    if (buttonTextSurface == NULL) { printf("Copy Fen Text Surface is NULL\n"); exit(EXIT_FAILURE); }
    SDL_Texture* buttonTextTexture = SDL_CreateTextureFromSurface(renderer, buttonTextSurface);
    if (buttonTextSurface == NULL) { printf("Copy Fen Text Texture is NULL\n"); exit(EXIT_FAILURE); }

    int buttonTextWidth = buttonTextSurface->w;
    int buttonTextHeight = buttonTextSurface->h;

    int buttonWidth = buttonTextWidth + BUTTON_PADDING;
    int buttonHeight = buttonTextHeight + BUTTON_PADDING;
    SDL_Rect buttonRect = {
        .x = restartButtonRect.x + (restartButtonRect.w - buttonWidth) / 2,
        .y = restartButtonRect.y + restartButtonRect.h + BUTTON_PADDING,
        .w = buttonWidth,
        .h = buttonHeight };

    SDL_Rect buttonTextRect = {
        .x = buttonRect.x + (buttonRect.w - buttonTextWidth) / 2,
        .y = buttonRect.y + (buttonRect.h - buttonTextHeight) / 2,
        .w = buttonTextWidth,
        .h = buttonTextHeight
    };

    // Render the button background
    SDL_SetRenderDrawColor(renderer, BUTTON_COLOR.r, BUTTON_COLOR.g, BUTTON_COLOR.b, BUTTON_COLOR.a);
    SDL_RenderFillRect(renderer, &buttonRect);


    SDL_FreeSurface(buttonTextSurface);
    SDL_RenderCopy(renderer, buttonTextTexture, NULL, &buttonTextRect);
    SDL_DestroyTexture(buttonTextTexture);

    ClickableArea area = {
        .rect = buttonRect,
        .callback = &clickedCopyFenButton
    };

    clickableAreas->data[COPY_FEN_BUTTON_INDEX] = area;
}

static void renderPlaceholder(SDL_Renderer* renderer, TTF_Font* font, GameState* gameState, ClickableAreas* clickableAreas) {
    SDL_Rect placeholderRect = PLACEHOLDER_RECT;
    // Draw rectangle border
    SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);
    SDL_RenderDrawRect(renderer, &placeholderRect);

    SDL_Rect textRect = renderGameStateText(renderer, font, gameState);
    SDL_Rect restartButtonRect = renderRestartButton(renderer, font, textRect, clickableAreas);
    SDL_Rect switchButtonRect = renderSwitchColorButton(renderer, font, restartButtonRect, clickableAreas);
    renderBackButton(renderer, font, switchButtonRect, clickableAreas);
    renderCopyFenButton(renderer, font, restartButtonRect, clickableAreas);

    renderTimeControls(renderer, font, gameState);
}

static void renderDraggedPiece(SDL_Renderer* renderer,
    Textures chessImages,
    DraggingState draggingState,
    int mouseX, int mouseY) {
    Piece draggedPiece = draggingState.draggedPiece;
    // This should be always false because we are already checking if isDragging is true
    if (draggedPiece == NOPIECE) { return; }

    int squareSize = CHESSBOARD_WIDTH / BOARD_LENGTH;
    int indexOffset = Piece_color(draggedPiece) == WHITE ? 9 : 11;
    TextureState chessImageData = chessImages.data[draggedPiece - indexOffset];
    SDL_Rect destRect = { mouseX - squareSize / 2, mouseY - squareSize / 2, squareSize, squareSize };
    SDL_RenderCopy(renderer, chessImageData.texture, NULL, &destRect);
}

static void renderChessboard(SDL_Renderer* renderer,
    Textures chessImages,
    GameState* gameState,
    DraggingState draggingState) {
    SDL_Rect rect = CHESSBOARD_RECT;
    int squareSize = rect.w / BOARD_LENGTH;

    bool flip = gameState->playerColor == BLACK;

    for (Square squareIndex = 0; squareIndex < BOARD_SIZE; squareIndex++) {
        int row = rank(squareIndex);
        int col = file(squareIndex);
        if (flip) {
            row = BOARD_LENGTH - 1 - row; // Flip the row index
            col = BOARD_LENGTH - 1 - col; // Flip the column index
        }

        SDL_Color color = ((row + col) % 2 == 0) ? SQUARE_COLOR_1 : SQUARE_COLOR_2;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect square = { rect.x + col * squareSize, rect.y + row * squareSize, squareSize, squareSize };
        SDL_RenderFillRect(renderer, &square);

        // Don't render the dragged pieces at their position and at the mouse coordinates
        if (draggingState.isDragging && squareIndex == draggingState.from && gameState->result == GAME_IS_NOT_DONE) { continue; }

        Piece piece = Board_pieceAtIndex(gameState->currentPosition.board, squareIndex);
        if (piece != NOPIECE) {
            SDL_Rect pieceRect = { square.x, square.y, squareSize, squareSize };
            int index = piece - (Piece_color(piece) == WHITE ? 9 : 11);
            SDL_RenderCopy(renderer, chessImages.data[index].texture, NULL, &pieceRect);
        }
    }
}

void render(App app) {
    SDL_RenderClear(app.state->sdlState.renderer);

    renderPlaceholder(app.state->sdlState.renderer, app.state->sdlState.font, &app.state->gameState, &app.events->clickableAreas);
    renderChessboard(app.state->sdlState.renderer, app.state->textures, &app.state->gameState, app.state->draggingState);
    if (app.state->draggingState.isDragging) {
        if (app.state->gameState.result == GAME_IS_NOT_DONE) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            renderDraggedPiece(app.state->sdlState.renderer, app.state->textures, app.state->draggingState, mouseX, mouseY);
        } else {
            // Resetting draggingState
            app.state->draggingState.isDragging = false;
            app.state->draggingState.from = 0;
            app.state->draggingState.to = 0;
            app.state->draggingState.draggedPiece = NOPIECE;
        }
    }

    SDL_RenderPresent(app.state->sdlState.renderer);
}
