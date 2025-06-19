#include <stdio.h>
#include <stdlib.h>

#include "../../../engine/src/utils/Math.h"

#include "../../sdl_framework/Render.h"
#include "../../sdl_framework/EventHandler.h"

#include "../events/CommonEvents.h"
#include "../events/GameEvents.h"
#include "../AppStyle.h"

#include "RenderUtils.h"
#include "GameScene.h"

void renderDraggedPiece(SDL_Renderer* renderer, const GameSceneData* scene, int squareSize, int mouseX, int mouseY, SDL_Rect boardRect) {
    Piece draggedPiece = scene->selectedPiece.selectedPiece;
    // This should be always false because we are already checking if isDragging is true
    if (draggedPiece == NOPIECE) return;

    int indexOffset = Piece_color(draggedPiece) == WHITE ? 9 : 11;
    TextureState chessImageData = scene->textures.data[draggedPiece - indexOffset];

    int x = SDL_clamp(mouseX, boardRect.x, boardRect.x + boardRect.w);
    int y = SDL_clamp(mouseY, boardRect.y, boardRect.y + boardRect.h);
    SDL_Rect destRect = { x - squareSize / 2, y - squareSize / 2, squareSize, squareSize };
    SDL_FRect destFRect = RECT_TO_FRECT(destRect);
    SDL_RenderTexture(renderer, chessImageData.texture, NULL, &destFRect);
}

SDL_AppResult renderChessboard(SDL_Rect boardRect, App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    SDL_Renderer* renderer = app->state.sdlState.renderer;

    const int squareSize = boardRect.w / BOARD_LENGTH;

    bool doRenderDraggedPiece = data->selectedPiece.isPieceSelected && data->selectedPiece.isDragged;

    for (Square squareIndex = 0; squareIndex < BOARD_SIZE; squareIndex++) {
        int row = rank(squareIndex);
        int col = file(squareIndex);
        if (data->flipBoard) {
            row = BOARD_LENGTH - 1 - row; // Flip the row index
            col = BOARD_LENGTH - 1 - col; // Flip the column index
        }

        SDL_Rect squareRect = { boardRect.x + col * squareSize, boardRect.y + row * squareSize, squareSize, squareSize };
        SDL_FRect squareFRect = RECT_TO_FRECT(squareRect);

        bool previousMoveSquare = false;
        if (data->state.undoStates.previousStateIndex > 0) {
            Move previousMove = data->state.movesPlayed[data->state.undoStates.previousStateIndex - 1];
            previousMoveSquare = (squareIndex == Move_fromSquare(previousMove) || squareIndex == Move_toSquare(previousMove));
        }

        bool isSquareSelected = data->selectedPiece.isPieceSelected && squareIndex == data->selectedPiece.from;

        if (previousMoveSquare) {
            // Change the color of the square if it was part of the previous move
            SDL_Color color = CLICKED_SQUARE_COLOR;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &squareFRect);
        }
        else if (isSquareSelected) {
            // Only change the border of the square if it selected

            // Rendering the complete rectangle with the border color
            SDL_Color color = CLICKED_SQUARE_COLOR;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &squareFRect);

            // Render the inner rectangle with the square color
            color = ((row + col) % 2 == 0) ? SQUARE_COLOR_1 : SQUARE_COLOR_2;
            const int borderThickness = squareSize / 8;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_FRect innerRect = {
                squareFRect.x + borderThickness,
                squareFRect.y + borderThickness,
                squareFRect.w - 2 * borderThickness,
                squareFRect.h - 2 * borderThickness
            };
            SDL_RenderFillRect(renderer, &RECT_TO_FRECT(innerRect));
        }
        else {
            SDL_Color color = ((row + col) % 2 == 0) ? SQUARE_COLOR_1 : SQUARE_COLOR_2;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &squareFRect);
        }

        // Don't render the dragged pieces at their position and at the mouse coordinates
        if (doRenderDraggedPiece && squareIndex == data->selectedPiece.from && data->state.gameEndedSettings.result == GAME_IS_NOT_DONE) continue;

        Piece piece = Board_pieceAtIndex(data->state.position.board, squareIndex);
        if (piece != NOPIECE) {
            SDL_Rect pieceRect = { squareRect.x, squareRect.y, squareSize, squareSize };
            SDL_FRect pieceFRect = RECT_TO_FRECT(pieceRect);
            int index = piece - (Piece_color(piece) == WHITE ? 9 : 11);
            SDL_RenderTexture(renderer, data->textures.data[index].texture, NULL, &pieceFRect);
        }
    }

    if (doRenderDraggedPiece) {
        if (data->state.gameEndedSettings.result == GAME_IS_NOT_DONE) {
            float mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            renderDraggedPiece(app->state.sdlState.renderer, data, squareSize, (int)mouseX, (int)mouseY, boardRect);
        }
        else {
            // Resetting selectedPiece
            memset(&data->selectedPiece, 0, sizeof(SelectedPiece));
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderBlackClock(SDL_Rect blackClockRect, App* app) {
    char buffer[16];

    if (formatTime(((GameSceneData*)app->state.currentScene.data)->state.black.timeControl.timeLeft, buffer, 16) != SDL_APP_CONTINUE) return SDL_APP_FAILURE;
    return renderTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, buffer, false, WHITE_COLOR, blackClockRect);
}

SDL_AppResult renderWhiteClock(SDL_Rect whiteClockRect, App* app) {
    char buffer[16];

    if (formatTime(((GameSceneData*)app->state.currentScene.data)->state.white.timeControl.timeLeft, buffer, 16) != SDL_APP_CONTINUE) return SDL_APP_FAILURE;
    return renderTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, buffer, false, WHITE_COLOR, whiteClockRect);
}

SDL_AppResult renderMoveList(SDL_Rect rect, App* app) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    //    TTF_Font* font = app->state.sdlState.font;

    SDL_Color borderColor = BUTTON_BORDER_COLOR;
    // SDL_Color textColor = BUTTON_TEXT_COLOR;
    // SDL_Color highlightColor = BUTTON_HIGHLIGHT_COLOR;
    SDL_Color backgroundColor = SEMI_TRANSPARENT_BACKGROUND_COLOR;

    // semi-transparent dark background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &RECT_TO_FRECT(rect));

    // --- Border ---
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &RECT_TO_FRECT(rect));

    // TODO: render move list inside layout->moveListRect

    return SDL_APP_CONTINUE;
}

SDL_AppResult renderRestartButton(SDL_Rect rect, App* app) {
    return renderButton(rect, app, RESTART_BUTTON, "Restart");
}

SDL_AppResult renderBackButton(SDL_Rect rect, App* app) {
    return renderButton(rect, app, BACK_BUTTON, "Back");
}

SDL_Rect calculatePromotionRect(GameSceneData* data, SDL_Rect boardRect) {
    // Chessboard dimensions
    int squareSize = boardRect.w / BOARD_LENGTH;

    // Calculate position of the promotion square
    int promotionSquare = data->promotionSettings.promotionSquareTo;
    int promotionFile = file(promotionSquare);
    int promotionRank = rank(promotionSquare);
    if (data->flipBoard) {
        promotionFile = BOARD_LENGTH - 1 - promotionFile;
        promotionRank = BOARD_LENGTH - 1 - promotionRank;
    }

    int squareX = boardRect.x + promotionFile * squareSize;
    int squareY = boardRect.y + promotionRank * squareSize;

    // Overlay dimensions
    int overlayWidth = 2 * squareSize; // Two options per row
    int overlayHeight = 2 * squareSize; // Two rows of options
    int overlayX = squareX - squareSize; // Centered relative to the square
    int overlayY = (promotionRank == 0) ? squareY : squareY - overlayHeight; // Adjust for promotion direction
    return (SDL_Rect) { overlayX, overlayY, overlayWidth, overlayHeight };
}

#define NB_PROMOTION_TYPE (4)

SDL_AppResult renderPromotionOverlay(SDL_Rect boardRect, App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    if (!data->promotionSettings.renderPromotionOverlay) return SDL_APP_CONTINUE;

    int squareSize = boardRect.w / BOARD_LENGTH;
    SDL_Rect overlayRect = calculatePromotionRect(data, boardRect);
    data->promotionSettings.overlayRect = overlayRect;

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    SDL_SetRenderDrawColor(renderer, OVERLAY_COLOR.r, OVERLAY_COLOR.g, OVERLAY_COLOR.b, OVERLAY_COLOR.a);
    SDL_RenderFillRect(renderer, &RECT_TO_FRECT(overlayRect));

    SDL_SetRenderDrawColor(renderer, BLACK_COLOR.r, BLACK_COLOR.g, BLACK_COLOR.b, BLACK_COLOR.a);
    SDL_RenderRect(renderer, &RECT_TO_FRECT(overlayRect));

    // Render piece textures
    PieceCharacteristics colorToPromote = Piece_color(Board_pieceAtIndex(data->state.position.board, data->promotionSettings.promotionSquareFrom));
    int indexOffSet = colorToPromote == WHITE ? 9 : 11;
    SDL_Texture* textures[NB_PROMOTION_TYPE];
    textures[0] = data->textures.data[Piece_makePiece(colorToPromote, QUEEN) - indexOffSet].texture;
    textures[1] = data->textures.data[Piece_makePiece(colorToPromote, KNIGHT) - indexOffSet].texture;
    textures[2] = data->textures.data[Piece_makePiece(colorToPromote, ROOK) - indexOffSet].texture;
    textures[3] = data->textures.data[Piece_makePiece(colorToPromote, BISHOP) - indexOffSet].texture;

    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_Point mousePoint = { (int)mouseX, (int)mouseY };
    for (int i = 0; i < NB_PROMOTION_TYPE; i++) {
        int pieceCol = i % 2; // Column within the grid 
        int pieceRow = i / 2; // Row within the grid
        SDL_Rect pieceRect = {
            overlayRect.x + pieceCol * squareSize,
            overlayRect.y + pieceRow * squareSize,
            squareSize,
            squareSize
        };
        SDL_RenderTexture(renderer, textures[i], NULL, &RECT_TO_FRECT(pieceRect));

        if (app->events.mouseState.hoveredIndex == PROMOTION_OVERLAY &&
            SDL_PointInRect(&mousePoint, &pieceRect)) {

            SDL_Color highlightColor = PROMOTION_HIGHLIGHT_COLOR;

            // Enable alpha blending for transparency
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);

            const int thickness = overlayRect.w / 16;

            SDL_Rect top = { pieceRect.x, pieceRect.y, pieceRect.w, thickness };
            SDL_Rect bottom = { pieceRect.x, pieceRect.y + pieceRect.h - thickness, pieceRect.w, thickness };
            SDL_Rect left = { pieceRect.x, pieceRect.y + top.h, thickness, pieceRect.h - top.h - bottom.h };
            SDL_Rect right = { pieceRect.x + pieceRect.w - thickness, pieceRect.y + top.h, thickness, pieceRect.h - top.h - bottom.h };

            SDL_RenderFillRect(renderer, &RECT_TO_FRECT(top));
            SDL_RenderFillRect(renderer, &RECT_TO_FRECT(bottom));
            SDL_RenderFillRect(renderer, &RECT_TO_FRECT(left));
            SDL_RenderFillRect(renderer, &RECT_TO_FRECT(right));
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult renderGameEndedOverlay(SDL_Rect overlayRect, App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    if (!data->state.gameEndedSettings.renderOverlay) return SDL_APP_CONTINUE;

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    SDL_SetRenderDrawColor(renderer, OVERLAY_COLOR.r, OVERLAY_COLOR.g, OVERLAY_COLOR.b, OVERLAY_COLOR.a);
    SDL_RenderFillRect(renderer, &RECT_TO_FRECT(overlayRect));

    SDL_SetRenderDrawColor(renderer, BLACK_COLOR.r, BLACK_COLOR.g, BLACK_COLOR.b, BLACK_COLOR.a);
    SDL_RenderRect(renderer, &RECT_TO_FRECT(overlayRect));

    char text[31];

    switch (data->state.gameEndedSettings.result) {
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
        memcpy(text, "Error on switch", 16);
        break;
    }

    const int overlayPadding = overlayRect.w / 16;
    SDL_Rect textRect = (SDL_Rect){
        overlayRect.x + overlayPadding,
        overlayRect.y + overlayPadding,
        overlayRect.w - 2 * overlayPadding,
        overlayRect.h - 2 * overlayPadding
    };

    return renderTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, text, true, BLACK_COLOR, textRect);
}

#define BOARD_SIZE_PERCENT (0.8f)
#define CLOCK_HEIGHT_PERCENT (0.07f)
#define CLOCK_WIDTH_PERCENT (0.1f)
#define GAME_BUTTON_HEIGHT_PERCENT CLOCK_HEIGHT_PERCENT
#define GAME_BUTTON_WIDTH_PERCENT CLOCK_WIDTH_PERCENT
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

    const int padding = (int)(PADDING_PERCENT * windowHeight);
    const int clockHeight = (int)(CLOCK_HEIGHT_PERCENT * windowHeight);
    const int clockWidth = (int)(CLOCK_WIDTH_PERCENT * windowWidth);
    const int boardSize = (int)(min(windowHeight, windowWidth) * BOARD_SIZE_PERCENT);

    // Left side layout (board and clocks)
    int boardX = padding;
    int boardY = padding + clockHeight + padding;

    SDL_Rect boardRect = (SDL_Rect){ boardX, boardY, boardSize, boardSize };

    sceneRender->renderBoxes[CHESSBOARD].renderRect = boardRect;
    sceneRender->renderBoxes[CHESSBOARD].renderFunction = &renderChessboard;
    sceneRender->renderBoxes[CHESSBOARD].onMouseButtonDown = &chessBoardMouseButtonDown;
    sceneRender->renderBoxes[CHESSBOARD].onMouseButtonUp = &chessBoardMouseButtonUp;

    // Adding the gameEnded overlay
    const int gameEndedSize = (int)(GAME_ENDED_SIZE * boardSize);
    SDL_Rect gameEndedRect = (SDL_Rect){
        boardX + (boardSize - gameEndedSize) / 2,
        boardY + (boardSize - gameEndedSize) / 2,
        gameEndedSize,
        gameEndedSize
    };
    sceneRender->renderBoxes[GAME_ENDED_OVERLAY].renderRect = gameEndedRect;
    sceneRender->renderBoxes[GAME_ENDED_OVERLAY].renderFunction = &renderGameEndedOverlay;

    const int clockX = boardX + (boardSize - clockWidth) / 2;
    const int blackClockY = data->flipBoard ? boardY + boardSize + padding : padding;
    SDL_Rect blackClockRect = (SDL_Rect){
        clockX,
        blackClockY,
        clockWidth,
        clockHeight
    };
    sceneRender->renderBoxes[BLACK_CLOCK].renderRect = blackClockRect;
    sceneRender->renderBoxes[BLACK_CLOCK].renderFunction = &renderBlackClock;

    const int whiteClockY = data->flipBoard ? padding : boardY + boardSize + padding;
    SDL_Rect whiteClockRect = (SDL_Rect){
        clockX,
        whiteClockY,
        clockWidth,
        clockHeight
    };
    sceneRender->renderBoxes[WHITE_CLOCK].renderRect = whiteClockRect;
    sceneRender->renderBoxes[WHITE_CLOCK].renderFunction = &renderWhiteClock;

    // Right side layout (move list)
    int moveListX = boardX + boardSize + padding;
    int moveListWidth = windowWidth - moveListX - padding;
    int moveListHeight = boardRect.h;

    SDL_Rect moveListRect = (SDL_Rect){
        moveListX,
        boardY,
        moveListWidth,
        moveListHeight
    };
    sceneRender->renderBoxes[MOVE_LIST].renderRect = moveListRect;
    sceneRender->renderBoxes[MOVE_LIST].renderFunction = &renderMoveList;

    // Buttons
    const int buttonWidth = (int)(GAME_BUTTON_WIDTH_PERCENT * windowWidth);
    const int buttonHeight = (int)(GAME_BUTTON_HEIGHT_PERCENT * windowHeight);

    // Back button
    SDL_Rect backButtonRect = (SDL_Rect){
        boardX,
        padding,
        buttonWidth,
        buttonHeight
    };
    sceneRender->renderBoxes[BACK_BUTTON].renderRect = backButtonRect;
    sceneRender->renderBoxes[BACK_BUTTON].renderFunction = &renderBackButton;
    sceneRender->renderBoxes[BACK_BUTTON].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[BACK_BUTTON].onMouseExited = &rerenderScene;

    SDL_Rect restartButtonRect = (SDL_Rect){
        boardX,
        boardY + boardSize + padding,
        buttonWidth * 2,
        buttonHeight
    };
    sceneRender->renderBoxes[RESTART_BUTTON].renderRect = restartButtonRect;
    sceneRender->renderBoxes[RESTART_BUTTON].renderFunction = &renderRestartButton;
    sceneRender->renderBoxes[RESTART_BUTTON].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[RESTART_BUTTON].onMouseExited = &rerenderScene;
    sceneRender->renderBoxes[RESTART_BUTTON].onMouseButtonUp = &clickedDownRestartButton;

    // Rectangle for the promotion overlay will be calculated when it is rendered
    // The only information needed to create this rectangle is the size of the board 
    // and the promotion square which are provided to the render function via the
    // rect parameter and the pointer to the app
    sceneRender->renderBoxes[PROMOTION_OVERLAY].renderRect = boardRect;
    sceneRender->renderBoxes[PROMOTION_OVERLAY].renderFunction = &renderPromotionOverlay;
    sceneRender->renderBoxes[PROMOTION_OVERLAY].onMouseHovered = &rerenderScene;
    sceneRender->renderBoxes[PROMOTION_OVERLAY].onMouseButtonDown = &promotionOverlayMouseButtonDown;
}
