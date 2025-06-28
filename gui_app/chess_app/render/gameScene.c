#include <stdio.h>
#include <stdlib.h>

#include "../../../engine/src/utils/Math.h"
#include "../../../engine/src/utils/AlgebraicNotation.h"

#include "../../sdl_framework/Render.h"
#include "../../sdl_framework/EventHandler.h"
#include "../../sdl_framework/AppCleanup.h"

#include "../events/CommonEvents.h"
#include "../events/GameEvents.h"
#include "../AppStyle.h"

#include "RenderUtils.h"
#include "GameScene.h"

void renderDraggedPiece(SDL_Renderer* renderer, const Textures* textures, Piece selectedPiece, float squareSize, float mouseX, float mouseY) {
    int indexOffset = Piece_color(selectedPiece) == WHITE ? 9 : 11;
    TextureState chessImageData = textures->data[selectedPiece - indexOffset];
    SDL_FRect destRect = { mouseX - squareSize / 2, mouseY - squareSize / 2, squareSize, squareSize };
    SDL_RenderTexture(renderer, chessImageData.texture, NULL, &destRect);
}

SDL_AppResult renderChessboard(SDL_FRect boardRect, App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    SDL_Renderer* renderer = app->state.sdlState.renderer;

    const float squareSize = boardRect.w / BOARD_LENGTH;

    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_FPoint mousePoint = { mouseX, mouseY };
    if (!SDL_PointInRectFloat(&mousePoint, &boardRect)) {
        // If the mouse is not in the board rect reset selected piece state
        data->selectedSquare.selectedSquare = NO_SQUARE_SELECTED;
    }

    bool doRenderDraggedPiece = data->selectedSquare.selectedSquare != NO_SQUARE_SELECTED && app->events.mouseState.holdingLeftMouseButton;

    for (Square squareIndex = 0; squareIndex < BOARD_SIZE; squareIndex++) {
        int row = rank(squareIndex);
        int col = file(squareIndex);
        if (data->flipBoard) {
            row = BOARD_LENGTH - 1 - row; // Flip the row index
            col = BOARD_LENGTH - 1 - col; // Flip the column index
        }

        SDL_FRect squareRect = { boardRect.x + col * squareSize, boardRect.y + row * squareSize, squareSize, squareSize };

        bool previousMoveSquare = false;
        if (data->moveListInfo.movesPlayed.count > 0) {
            Move previousMove = data->moveListInfo.movesPlayed.data[data->moveListInfo.movesPlayed.count - 1];
            previousMoveSquare = (squareIndex == Move_fromSquare(previousMove) || squareIndex == Move_toSquare(previousMove));
        }

        bool isSquareSelected = squareIndex == data->selectedSquare.selectedSquare;

        if (previousMoveSquare) {
            // Change the color of the square if it was part of the previous move
            SDL_Color color = CLICKED_SQUARE_COLOR;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &squareRect);
        }
        else if (isSquareSelected) {
            // Only change the border of the square if it selected

            // Rendering the complete rectangle with the border color
            SDL_Color color = CLICKED_SQUARE_COLOR;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &squareRect);

            // Render the inner rectangle with the square color
            color = ((row + col) % 2 == 0) ? SQUARE_COLOR_1 : SQUARE_COLOR_2;
            const int borderThickness = squareSize / 8;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_FRect innerRect = {
                squareRect.x + borderThickness,
                squareRect.y + borderThickness,
                squareRect.w - 2 * borderThickness,
                squareRect.h - 2 * borderThickness
            };
            SDL_RenderFillRect(renderer, &RECT_TO_FRECT(innerRect));
        }
        else {
            SDL_Color color = ((row + col) % 2 == 0) ? SQUARE_COLOR_1 : SQUARE_COLOR_2;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &squareRect);
        }

        // Don't render the dragged pieces at their position and at the mouse coordinates
        if (doRenderDraggedPiece && isSquareSelected && data->gameEndedInfo.result == GAME_IS_NOT_DONE) continue;

        Piece piece = Board_pieceAtIndex(data->state.position.board, squareIndex);
        if (piece != NO_PIECE) {
            SDL_FRect pieceRect = { squareRect.x, squareRect.y, squareSize, squareSize };
            SDL_FRect pieceFRect = RECT_TO_FRECT(pieceRect);
            int index = piece - (Piece_color(piece) == WHITE ? 9 : 11);
            SDL_RenderTexture(renderer, data->textures.data[index].texture, NULL, &pieceFRect);
        }
    }

    if (doRenderDraggedPiece) {
        if (data->gameEndedInfo.result == GAME_IS_NOT_DONE) {
            Piece selectedPiece = Board_pieceAtIndex(data->state.position.board, data->selectedSquare.selectedSquare);
            if (selectedPiece != NO_PIECE) {
                renderDraggedPiece(app->state.sdlState.renderer, &data->textures, selectedPiece, squareSize, mouseX, mouseY);
            }
        }
        else {
            // Resetting selectedPiece
            data->selectedSquare.selectedSquare = NO_SQUARE_SELECTED;
        }
    }
    return SDL_APP_CONTINUE;
}

#define CLOCK_BUFFER_SIZE (16)

SDL_AppResult renderBlackClock(SDL_FRect blackClockRect, App* app) {
    char buffer[CLOCK_BUFFER_SIZE];

    if (formatTime(((GameSceneData*)app->state.currentScene.data)->state.black.timeControl.timeLeft, buffer, CLOCK_BUFFER_SIZE) != SDL_APP_CONTINUE) return SDL_APP_FAILURE;
    return renderTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, buffer, false, WHITE_COLOR, blackClockRect);
}

SDL_AppResult renderWhiteClock(SDL_FRect whiteClockRect, App* app) {
    char buffer[CLOCK_BUFFER_SIZE];

    if (formatTime(((GameSceneData*)app->state.currentScene.data)->state.white.timeControl.timeLeft, buffer, CLOCK_BUFFER_SIZE) != SDL_APP_CONTINUE) return SDL_APP_FAILURE;
    return renderTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, buffer, false, WHITE_COLOR, whiteClockRect);
}

#define SCROLL_BAR_SIZE_PERCENT (0.1f)

SDL_AppResult renderMoveListScrollbar(SDL_FRect rect, App* app) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    SDL_Color scrollBarColor = { 75, 75, 75, 255 };
    SDL_Color backgroundColor = SEMI_TRANSPARENT_BACKGROUND_COLOR;

    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &RECT_TO_FRECT(rect));

    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;

    // Drawing the bar
    float scrollbarX = (float)rect.x;

    float scrollbarHeight = SCROLL_BAR_SIZE_PERCENT * (float)rect.h;
    const float maxScrollY = (float)rect.h - scrollbarHeight;
    float scrollbarY = (float)rect.y + maxScrollY * data->moveListInfo.scrollRatio;

    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    if (data->moveListInfo.isScrolling && app->events.mouseState.holdingLeftMouseButton) {
        // Render the scrollbar over the mouse
        scrollbarY = SDL_clamp(mouseY - data->moveListInfo.startingDragOffset, (float)rect.y, (float)(rect.y + maxScrollY));
        // Update the scroll ratio
        data->moveListInfo.scrollRatio = SDL_clamp((mouseY - data->moveListInfo.startingDragOffset - rect.y) / maxScrollY, 0.0, 1.0);
        // Bright highlight the scrollbar
        scrollBarColor = (SDL_Color){ 140, 140, 140, 255 };
    }
    else {
        data->moveListInfo.isScrolling = false;
    }

    float scrollbarWidth = (float)rect.w;
    float radius = scrollbarWidth / 2.0f;

    // The movelist scrollbar frect contains the semi circles
    data->moveListInfo.scrollbarFRect = (SDL_FRect){
        scrollbarX,
        scrollbarY,
        scrollbarWidth,
        scrollbarHeight
    };
    SDL_FPoint mousePoint = { mouseX, mouseY };
    if (app->events.mouseState.hoveredIndex == MOVE_LIST_SCROLLBAR &&
        SDL_PointInRectFloat(&mousePoint, &data->moveListInfo.scrollbarFRect) &&
        !app->events.mouseState.holdingLeftMouseButton) {

        // Small highlight of the scrollbar
        scrollBarColor = (SDL_Color){ 111, 111, 111, 255 };
    }

    SDL_FRect scrollbarFRect = (SDL_FRect){
        scrollbarX,
        scrollbarY + radius,
        scrollbarWidth,
        scrollbarHeight - 2 * radius
    };

    SDL_SetRenderDrawColor(renderer, scrollBarColor.r, scrollBarColor.g, scrollBarColor.b, scrollBarColor.a);
    SDL_RenderFillRect(renderer, &scrollbarFRect);

    float cx = scrollbarFRect.x + radius;
    float cy = scrollbarY + radius;
    if (drawFilledCircle(renderer, cx, cy, radius) != SDL_APP_CONTINUE) return SDL_APP_FAILURE;
    cy = scrollbarY + scrollbarHeight - radius;
    if (drawFilledCircle(renderer, cx, cy, radius) != SDL_APP_CONTINUE) return SDL_APP_FAILURE;

    return SDL_APP_CONTINUE;
}

SDL_AppResult renderMoveList(SDL_FRect rect, App* app) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

    SDL_Color borderColor = BUTTON_BORDER_COLOR;
    SDL_Color textColor = BUTTON_TEXT_COLOR;
    SDL_Color highlightColor = BUTTON_HIGHLIGHT_COLOR;
    SDL_Color backgroundColor = SEMI_TRANSPARENT_BACKGROUND_COLOR;

    // Draw semi-transparent background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &RECT_TO_FRECT(rect));

    // Draw border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &RECT_TO_FRECT(rect));

    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;

    const size_t numRows = data->moveListInfo.movesPlayed.count / 2 + data->moveListInfo.movesPlayed.count % 2;
    // If we have no moves to render simply return
    if (numRows == 0) return SDL_APP_CONTINUE;

    const float padding = rect.w / 16;
    const int lineHeightPerMove = TTF_GetFontLineSkip(font);
    const float spaceTakenUpByOneMove = lineHeightPerMove + padding;

    // Define 3 column widths
    const float column1Width = rect.w / 6; // Move number
    const float columnWidth = (rect.w - column1Width - 4 * padding) / 2;

    const float maxY = rect.y + rect.h - padding;
    const float scrollRatio = data->moveListInfo.scrollRatio;
    const size_t scrollY = numRows * spaceTakenUpByOneMove * scrollRatio;
    float y = rect.y + padding - scrollY;

    ChessPosition position;
    Board board;

    char moveText[16];
    SDL_FRect whiteMoveRect;

    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_FPoint mousePoint = { mouseX, mouseY };

    // So that the move info will not be out of the rectangle
    // if the scroll index is a fraction of a line height
    SDL_Rect renderClipRect = { (int)rect.x, (int)rect.y, (int)rect.w, (int)rect.h };
    SDL_SetRenderClipRect(renderer, &renderClipRect);
    for (size_t index = 0; index < data->moveListInfo.movesPlayed.count; index += 2) {
        // If we are completely below the move list stop rendering
        if (y >= maxY) break;
        // If we are completely above the move list skip to the next move to render
        if (y + lineHeightPerMove < rect.y) {
            y += spaceTakenUpByOneMove;
            continue;
        }

        // --- Move number ---
        char numberBuffer[8];
        snprintf(numberBuffer, sizeof(numberBuffer), "%zu.", index / 2 + 1);

        SDL_FRect moveNumberRect = {
            .x = rect.x + padding,
            .y = y,
            .w = column1Width,
            .h = lineHeightPerMove
        };

        SDL_AppResult result = renderTextCenteredToFit(renderer, font, numberBuffer, false, textColor, moveNumberRect);
        if (result != SDL_APP_CONTINUE) return result;

        if (index < data->moveListInfo.movesPlayed.count) {
            // We need to do this so that we don't accidentally modify the bitboards of the previous position
            position = data->undoGameStates.data[index].position;
            memcpy(&board, position.board.bitboards, 14 * sizeof(BitBoard));
            position.board = board;

            moveToStandardAlgebraic(position, data->moveListInfo.movesPlayed.data[index], moveText);

            whiteMoveRect = (SDL_FRect){
                .x = moveNumberRect.x + moveNumberRect.w + padding,
                .y = y,
                .w = columnWidth,
                .h = lineHeightPerMove
            };

            if (app->events.mouseState.hoveredIndex == MOVE_LIST &&
                SDL_PointInRectFloat(&mousePoint, &whiteMoveRect)) {
                SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
                SDL_RenderFillRect(renderer, &whiteMoveRect);
                data->moveListInfo.hoveredMoveIndex = index;
            }

            result = renderTextCenteredToFit(renderer, font, moveText, false, textColor, whiteMoveRect);
            if (result != SDL_APP_CONTINUE) return result;
        }

        if (index + 1 < data->moveListInfo.movesPlayed.count) {
            // We need to do this so that we don't accidentally modify the bitboards of the previous position
            position = data->undoGameStates.data[index + 1].position;
            memcpy(&board, position.board.bitboards, 14 * sizeof(BitBoard));
            position.board = board;

            moveToStandardAlgebraic(position, data->moveListInfo.movesPlayed.data[index + 1], moveText);

            SDL_FRect blackMoveRect = {
                .x = whiteMoveRect.x + whiteMoveRect.w + padding,
                .y = y,
                .w = columnWidth,
                .h = lineHeightPerMove
            };

            if (app->events.mouseState.hoveredIndex == MOVE_LIST &&
                SDL_PointInRectFloat(&mousePoint, &blackMoveRect)) {
                SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
                SDL_RenderFillRect(renderer, &blackMoveRect);
                data->moveListInfo.hoveredMoveIndex = index + 1;
            }

            result = renderTextCenteredToFit(renderer, font, moveText, false, textColor, blackMoveRect);
            if (result != SDL_APP_CONTINUE) return result;
        }

        y += spaceTakenUpByOneMove;
    }

    // Resetting the clip rect
    SDL_SetRenderClipRect(renderer, NULL);
    return SDL_APP_CONTINUE;
}


SDL_AppResult renderRestartButton(SDL_FRect rect, App* app) {
    return renderButton(rect, app, RESTART_BUTTON, "Restart");
}

SDL_AppResult renderBackButton(SDL_FRect rect, App* app) {
    return renderButton(rect, app, BACK_BUTTON, "Back");
}

SDL_AppResult renderFlipBoardButton(SDL_FRect rect, App* app) {
    return renderButton(rect, app, FLIP_BOARD_BUTTON, "Flip board");
}

SDL_FRect calculatePromotionRect(GameSceneData* data, SDL_FRect boardRect) {
    // Chessboard dimensions
    float squareSize = boardRect.w / BOARD_LENGTH;

    // Calculate position of the promotion square
    int promotionSquare = data->promotionInfo.promotionSquareTo;
    int promotionFile = file(promotionSquare);
    int promotionRank = rank(promotionSquare);
    if (data->flipBoard) {
        promotionFile = BOARD_LENGTH - 1 - promotionFile;
        promotionRank = BOARD_LENGTH - 1 - promotionRank;
    }

    float squareX = boardRect.x + promotionFile * squareSize;
    float squareY = boardRect.y + promotionRank * squareSize;

    // Overlay dimensions
    float overlayWidth = 2 * squareSize; // Two options per row
    float overlayHeight = 2 * squareSize; // Two rows of options
    float overlayX = squareX - squareSize; // Centered relative to the square
    float overlayY = (promotionRank == 0) ? squareY : squareY - overlayHeight; // Adjust for promotion direction
    return (SDL_FRect) { overlayX, overlayY, overlayWidth, overlayHeight };
}

#define NB_PROMOTION_TYPE (4)

SDL_AppResult renderPromotionOverlay(SDL_FRect boardRect, App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    if (!data->promotionInfo.renderPromotionOverlay) return SDL_APP_CONTINUE;

    float squareSize = boardRect.w / BOARD_LENGTH;
    SDL_FRect overlayRect = calculatePromotionRect(data, boardRect);
    data->promotionInfo.overlayRect = overlayRect;

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    SDL_SetRenderDrawColor(renderer, OVERLAY_COLOR.r, OVERLAY_COLOR.g, OVERLAY_COLOR.b, OVERLAY_COLOR.a);
    SDL_RenderFillRect(renderer, &overlayRect);

    SDL_SetRenderDrawColor(renderer, BLACK_COLOR.r, BLACK_COLOR.g, BLACK_COLOR.b, BLACK_COLOR.a);
    SDL_RenderRect(renderer, &overlayRect);

    // Render piece textures
    PieceCharacteristics colorToPromote = Piece_color(Board_pieceAtIndex(data->state.position.board, data->promotionInfo.promotionSquareFrom));
    int indexOffSet = colorToPromote == WHITE ? 9 : 11;
    SDL_Texture* textures[NB_PROMOTION_TYPE];
    textures[0] = data->textures.data[Piece_makePiece(colorToPromote, QUEEN) - indexOffSet].texture;
    textures[1] = data->textures.data[Piece_makePiece(colorToPromote, KNIGHT) - indexOffSet].texture;
    textures[2] = data->textures.data[Piece_makePiece(colorToPromote, ROOK) - indexOffSet].texture;
    textures[3] = data->textures.data[Piece_makePiece(colorToPromote, BISHOP) - indexOffSet].texture;

    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_FPoint mousePoint = { mouseX, mouseY };
    for (int i = 0; i < NB_PROMOTION_TYPE; i++) {
        int pieceCol = i % 2; // Column within the grid 
        int pieceRow = i / 2; // Row within the grid
        SDL_FRect pieceRect = {
            overlayRect.x + pieceCol * squareSize,
            overlayRect.y + pieceRow * squareSize,
            squareSize,
            squareSize
        };
        SDL_RenderTexture(renderer, textures[i], NULL, &RECT_TO_FRECT(pieceRect));

        if (app->events.mouseState.hoveredIndex == PROMOTION_OVERLAY &&
            SDL_PointInRectFloat(&mousePoint, &pieceRect)) {

            SDL_Color highlightColor = PROMOTION_HIGHLIGHT_COLOR;

            // Enable alpha blending for transparency
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);

            const float thickness = overlayRect.w / 16;

            SDL_FRect top = { pieceRect.x, pieceRect.y, pieceRect.w, thickness };
            SDL_FRect bottom = { pieceRect.x, pieceRect.y + pieceRect.h - thickness, pieceRect.w, thickness };
            SDL_FRect left = { pieceRect.x, pieceRect.y + top.h, thickness, pieceRect.h - top.h - bottom.h };
            SDL_FRect right = { pieceRect.x + pieceRect.w - thickness, pieceRect.y + top.h, thickness, pieceRect.h - top.h - bottom.h };

            SDL_RenderFillRect(renderer, &top);
            SDL_RenderFillRect(renderer, &bottom);
            SDL_RenderFillRect(renderer, &left);
            SDL_RenderFillRect(renderer, &right);
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult renderGameEndedOverlay(SDL_FRect overlayRect, App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    if (!data->gameEndedInfo.renderOverlay) return SDL_APP_CONTINUE;

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    SDL_SetRenderDrawColor(renderer, OVERLAY_COLOR.r, OVERLAY_COLOR.g, OVERLAY_COLOR.b, OVERLAY_COLOR.a);
    SDL_RenderFillRect(renderer, &overlayRect);

    SDL_SetRenderDrawColor(renderer, BLACK_COLOR.r, BLACK_COLOR.g, BLACK_COLOR.b, BLACK_COLOR.a);
    SDL_RenderRect(renderer, &overlayRect);

    char text[31];

    switch (data->gameEndedInfo.result) {
    case GAME_IS_NOT_DONE:
        memcpy(text, "Game is on!", 12);
        break;
    case THREE_MOVE_REPETITION:
        memcpy(text, "Draw by\nrepetition", 20);
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error on switch for a GameResult value of: %d\n", data->gameEndedInfo.result);
        return SDL_APP_FAILURE;
    }

    const float overlayPadding = overlayRect.w / 16;
    SDL_FRect textRect = {
        overlayRect.x + overlayPadding,
        overlayRect.y + overlayPadding,
        overlayRect.w - 2 * overlayPadding,
        overlayRect.h - 2 * overlayPadding
    };

    return renderTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, text, true, BLACK_COLOR, textRect);
}

#define BOARD_SIZE_PERCENT (0.75f)
#define CLOCK_HEIGHT_PERCENT (0.07f)
#define CLOCK_WIDTH_PERCENT (0.1f)
#define GAME_BUTTON_HEIGHT_PERCENT CLOCK_HEIGHT_PERCENT
#define GAME_BUTTON_WIDTH_PERCENT CLOCK_WIDTH_PERCENT
#define SCROLL_BAR_WIDTH_PERCENT (0.02f)
#define GAME_ENDED_SIZE (0.35f)

void computeGameSceneRender(SDL_Window* window, Scene* scene) {
    SceneRender* sceneRender = &scene->sceneRender;
    GameSceneData* data = (GameSceneData*)scene->data;

    // Setting in the background color of the scene
    sceneRender->renderDrawColor = BACKGROUND_COLOR;

    sceneRender->numRenderBox = TOTAL_GAME_SCENE_RENDER_BOX;
    // We assume that sceneRender->renderBoxes is always NULL
    sceneRender->renderBoxes = calloc(sceneRender->numRenderBox, sizeof(RenderBox));

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    const float padding = PADDING_PERCENT * windowHeight;
    const float clockHeight = CLOCK_HEIGHT_PERCENT * windowHeight;
    const float clockWidth = CLOCK_WIDTH_PERCENT * windowWidth;
    const float boardSize = min(windowHeight, windowWidth) * BOARD_SIZE_PERCENT;

    // Left side layout (board and clocks)
    float boardX = padding;
    float boardY = padding + clockHeight + padding;

    SDL_FRect boardRect = (SDL_FRect){ boardX, boardY, boardSize, boardSize };

    sceneRender->renderBoxes[CHESSBOARD].renderRect = boardRect;
    sceneRender->renderBoxes[CHESSBOARD].renderFunction = &renderChessboard;
    sceneRender->renderBoxes[CHESSBOARD].onMouseButtonDown = &chessBoardMouseButtonDown;
    sceneRender->renderBoxes[CHESSBOARD].onMouseButtonUp = &chessBoardMouseButtonUp;

    // Adding the gameEnded overlay
    const float gameEndedSize = GAME_ENDED_SIZE * boardSize;
    SDL_FRect gameEndedRect = (SDL_FRect){
        boardX + (boardSize - gameEndedSize) / 2,
        boardY + (boardSize - gameEndedSize) / 2,
        gameEndedSize,
        gameEndedSize
    };
    sceneRender->renderBoxes[GAME_ENDED_OVERLAY].renderRect = gameEndedRect;
    sceneRender->renderBoxes[GAME_ENDED_OVERLAY].renderFunction = &renderGameEndedOverlay;

    const float clockX = boardX + (boardSize - clockWidth) / 2;
    const float blackClockY = data->flipBoard ? boardY + boardSize + padding : padding;
    SDL_FRect blackClockRect = (SDL_FRect){
        clockX,
        blackClockY,
        clockWidth,
        clockHeight
    };
    sceneRender->renderBoxes[BLACK_CLOCK].renderRect = blackClockRect;
    sceneRender->renderBoxes[BLACK_CLOCK].renderFunction = &renderBlackClock;

    const float whiteClockY = data->flipBoard ? padding : boardY + boardSize + padding;
    SDL_FRect whiteClockRect = (SDL_FRect){
        clockX,
        whiteClockY,
        clockWidth,
        clockHeight
    };
    sceneRender->renderBoxes[WHITE_CLOCK].renderRect = whiteClockRect;
    sceneRender->renderBoxes[WHITE_CLOCK].renderFunction = &renderWhiteClock;

    // Movelist scrollbar
    float moveListScrollWidth = SCROLL_BAR_WIDTH_PERCENT * windowWidth;
    float moveListScrollX = windowWidth - moveListScrollWidth;
    SDL_FRect moveListScrollRect = (SDL_FRect){
        moveListScrollX,
        boardY,
        moveListScrollWidth,
        boardRect.h
    };
    sceneRender->renderBoxes[MOVE_LIST_SCROLLBAR].renderRect = moveListScrollRect;
    sceneRender->renderBoxes[MOVE_LIST_SCROLLBAR].renderFunction = &renderMoveListScrollbar;
    sceneRender->renderBoxes[MOVE_LIST_SCROLLBAR].onMouseButtonDown = &clickedDownScrollbar;
    sceneRender->renderBoxes[MOVE_LIST_SCROLLBAR].onMouseHovered = &rerenderScene;
    sceneRender->renderBoxes[MOVE_LIST_SCROLLBAR].onMouseWheelScrolled = &movelistMouseWheelScrolled;

    // Right side layout (move list)
    float moveListX = boardX + boardSize + padding;
    float moveListWidth = windowWidth - moveListX - moveListScrollWidth;

    SDL_FRect moveListRect = (SDL_FRect){
        moveListX,
        boardY,
        moveListWidth,
        boardRect.h
    };
    sceneRender->renderBoxes[MOVE_LIST].renderRect = moveListRect;
    sceneRender->renderBoxes[MOVE_LIST].renderFunction = &renderMoveList;
    sceneRender->renderBoxes[MOVE_LIST].onMouseWheelScrolled = &movelistMouseWheelScrolled;
    sceneRender->renderBoxes[MOVE_LIST].onMouseHovered = &rerenderScene;
    sceneRender->renderBoxes[MOVE_LIST].onMouseButtonDown = &clickedDownMoveList;

    // Buttons
    const float buttonWidth = GAME_BUTTON_WIDTH_PERCENT * windowWidth;
    const float buttonHeight = GAME_BUTTON_HEIGHT_PERCENT * windowHeight;

    // Back button
    SDL_FRect backButtonRect = (SDL_FRect){
        boardX,
        padding,
        buttonWidth,
        buttonHeight
    };
    sceneRender->renderBoxes[BACK_BUTTON].renderRect = backButtonRect;
    sceneRender->renderBoxes[BACK_BUTTON].renderFunction = &renderBackButton;
    sceneRender->renderBoxes[BACK_BUTTON].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[BACK_BUTTON].onMouseExited = &rerenderScene;
    sceneRender->renderBoxes[BACK_BUTTON].onMouseButtonDown = &clickedDownBackButton;

    SDL_FRect restartButtonRect = (SDL_FRect){
        boardX,
        boardY + boardSize + padding,
        buttonWidth * 2,
        buttonHeight
    };
    sceneRender->renderBoxes[RESTART_BUTTON].renderRect = restartButtonRect;
    sceneRender->renderBoxes[RESTART_BUTTON].renderFunction = &renderRestartButton;
    sceneRender->renderBoxes[RESTART_BUTTON].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[RESTART_BUTTON].onMouseExited = &rerenderScene;
    sceneRender->renderBoxes[RESTART_BUTTON].onMouseButtonDown = &clickedDownRestartButton;


    SDL_FRect flipBoardButton = (SDL_FRect){
        moveListX + moveListWidth / 2,
        restartButtonRect.y,
        buttonWidth,
        buttonHeight
    };
    sceneRender->renderBoxes[FLIP_BOARD_BUTTON].renderRect = flipBoardButton;
    sceneRender->renderBoxes[FLIP_BOARD_BUTTON].renderFunction = &renderFlipBoardButton;
    sceneRender->renderBoxes[FLIP_BOARD_BUTTON].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[FLIP_BOARD_BUTTON].onMouseExited = &rerenderScene;
    sceneRender->renderBoxes[FLIP_BOARD_BUTTON].onMouseButtonDown = &clickedDownFlipBoardButton;

    // Rectangle for the promotion overlay will be calculated when it is rendered
    // The only information needed to create this rectangle is the size of the board 
    // and the promotion square which are provided to the render function via the
    // rect parameter and the pointer to the app
    sceneRender->renderBoxes[PROMOTION_OVERLAY].renderRect = boardRect;
    sceneRender->renderBoxes[PROMOTION_OVERLAY].renderFunction = &renderPromotionOverlay;
    sceneRender->renderBoxes[PROMOTION_OVERLAY].onMouseHovered = &rerenderScene;
    sceneRender->renderBoxes[PROMOTION_OVERLAY].onMouseButtonDown = &promotionOverlayMouseButtonDown;
}

void terminateGameScene(void* data) {
    GameSceneData* gameData = (GameSceneData*)data;
    if (gameData->state.white.engineCommunication) UCIEngine_terminate(gameData->state.white.engineCommunication);
    if (gameData->state.black.engineCommunication) UCIEngine_terminate(gameData->state.black.engineCommunication);

    free(gameData->undoGameStates.data);
    free(gameData->moveListInfo.movesPlayed.data);
    cleanupTextures(gameData->textures);
    free(gameData->textures.data);

    free(gameData);
}
