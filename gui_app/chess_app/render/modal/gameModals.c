#include <stdlib.h>

#include "../../../../engine/src/utils/Math.h"

#include "../../../sdl_framework/CommonEvents.h"

#include "../../AppStyle.h"
#include "../../events/GameEvents.h"

#include "../scene/GameScene.h"
#include "../RenderUtils.h"

#include "GameModals.h"

SDL_FRect calculatePromotionRect(GameSceneData* data, int promotionSquareTo, SDL_FRect boardRect) {
    // Chessboard dimensions
    float squareSize = boardRect.w / BOARD_LENGTH;

    // Calculate position of the promotion square
    int promotionFile = file(promotionSquareTo);
    int promotionRank = rank(promotionSquareTo);
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

SDL_AppResult renderPromotionModal(SDL_FRect rect, App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    PromotionModalData* modalData = (PromotionModalData*)app->events.modal.data;

    float squareSize = rect.w / 2.0;

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    SDL_SetRenderDrawColor(renderer, OVERLAY_COLOR.r, OVERLAY_COLOR.g, OVERLAY_COLOR.b, OVERLAY_COLOR.a);
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, BLACK_COLOR.r, BLACK_COLOR.g, BLACK_COLOR.b, BLACK_COLOR.a);
    SDL_RenderRect(renderer, &rect);

    // Render piece textures
    PieceCharacteristics colorToPromote = Piece_color(Board_pieceAtIndex(data->state.position.board, modalData->promotionSquareFrom));
    int indexOffSet = colorToPromote == WHITE ? 9 : 11;
    SDL_Texture* textures[NB_PROMOTION_TYPE];
    textures[0] = data->textures.data[Piece_makePiece(colorToPromote, QUEEN) - indexOffSet].texture;
    textures[1] = data->textures.data[Piece_makePiece(colorToPromote, KNIGHT) - indexOffSet].texture;
    textures[2] = data->textures.data[Piece_makePiece(colorToPromote, ROOK) - indexOffSet].texture;
    textures[3] = data->textures.data[Piece_makePiece(colorToPromote, BISHOP) - indexOffSet].texture;

    for (int i = 0; i < NB_PROMOTION_TYPE; i++) {
        int pieceCol = i % 2; // Column within the grid 
        int pieceRow = i / 2; // Row within the grid
        SDL_FRect pieceRect = {
            rect.x + pieceCol * squareSize,
            rect.y + pieceRow * squareSize,
            squareSize,
            squareSize
        };
        SDL_RenderTexture(renderer, textures[i], NULL, &pieceRect);

        if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &pieceRect)) {

            SDL_Color highlightColor = PROMOTION_HIGHLIGHT_COLOR;

            // Enable alpha blending for transparency
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);

            const float thickness = rect.w / 16;

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

SDL_AppResult clickedDownPromotionModal(SDL_Event* event, SDL_FRect rect, App* app) {

    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    PromotionModalData* modalData = (PromotionModalData*)app->events.modal.data;

    SDL_FPoint mousePoint = { event->button.x, event->button.y };
    // If we are not in the promotion overlay simply continue the app
    if (!SDL_PointInRectFloat(&mousePoint, &rect)) return SDL_APP_CONTINUE;

    Move move = NULL_MOVE;
    int squareSize = (STARTING_WINDOW_WIDTH * 2 / 3) / BOARD_LENGTH;
    int relativeX = ((int)event->button.x) - rect.x;
    int relativeY = ((int)event->button.y) - rect.y;

    int colIndex = relativeX / squareSize; // 0 or 1
    int rowIndex = relativeY / squareSize; // 0 or 1

    // Map the row and column to a piece
    int pieceIndex = rowIndex * 2 + colIndex;
    Square from = modalData->promotionSquareFrom;
    Square to = modalData->promotionSquareTo;
    switch (pieceIndex) {
    case 0: move = Move_makeMove(from, to, PROMOTE_TO_QUEEN); break;
    case 1: move = Move_makeMove(from, to, PROMOTE_TO_KNIGHT); break;
    case 2: move = Move_makeMove(from, to, PROMOTE_TO_ROOK); break;
    case 3: move = Move_makeMove(from, to, PROMOTE_TO_BISHOP); break;
    default: break;
    }

    if (move == NULL_MOVE) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not select the promotion move\n");
        return SDL_APP_FAILURE;
    }

    // Cleaning up the modal
    app->events.modal.isActive = false;
    free(modalData);

    playMoveOnBoard(data, move);
    // We played a move so we reset the selected square
    data->selectedSquare.selectedSquare = NO_SQUARE_SELECTED;
    app->state.currentScene.selectedRenderBoxIndex = CHESSBOARD;

    if (data->state.result != GAME_IS_NOT_DONE) {
        setGameEndedModalActive(app);
    }

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

void setPromotionModalActive(App* app, int promotionSquareTo) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    PromotionModalData* modalData = calloc(1, sizeof(PromotionModalData));
    SDL_assert(modalData);

    modalData->promotionSquareFrom = data->selectedSquare.selectedSquare;
    modalData->promotionSquareTo = promotionSquareTo;

    // Initializing the modal
    app->events.modal.modalRender.renderRect = calculatePromotionRect(data, promotionSquareTo, app->state.currentScene.sceneRender.renderBoxes[CHESSBOARD].renderRect);
    app->events.modal.modalRender.renderFunction = &renderPromotionModal;
    app->events.modal.modalRender.onMouseButtonDown = &clickedDownPromotionModal;
    app->events.modal.modalRender.onMouseButtonUp = NULL;
    app->events.modal.modalRender.onMouseWheelScrolled = NULL;
    app->events.modal.modalRender.onMouseEntered = NULL;
    app->events.modal.modalRender.onMouseExited = NULL;

    app->events.modal.canOnlyInteractWithModal = true;
    app->events.modal.data = modalData;
    app->events.modal.onEscape = NULL;
    app->events.modal.onReturn = NULL;

    app->events.modal.modalId = PROMOTION_MODAL_ID;
    app->events.modal.isActive = true;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
}


#define GAME_ENDED_SIZE (0.35f)

SDL_FRect calculateGameEndedRect(SDL_FRect boardRect) {
    const float gameEndedSize = GAME_ENDED_SIZE * boardRect.w;
    return (SDL_FRect) {
        boardRect.x + (boardRect.w - gameEndedSize) / 2.0,
            boardRect.y + (boardRect.w - gameEndedSize) / 2.0,
            gameEndedSize,
            gameEndedSize
    };
}

SDL_AppResult renderGameEndedModal(SDL_FRect rect, App* app) {
    GameEndedModalData* modalData = (GameEndedModalData*)app->events.modal.data;

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    SDL_SetRenderDrawColor(renderer, OVERLAY_COLOR.r, OVERLAY_COLOR.g, OVERLAY_COLOR.b, OVERLAY_COLOR.a);
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, BLACK_COLOR.r, BLACK_COLOR.g, BLACK_COLOR.b, BLACK_COLOR.a);
    SDL_RenderRect(renderer, &rect);

    char text[31];

    switch (modalData->result) {
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error on switch for a GameResult value of: %d\n", modalData->result);
        return SDL_APP_FAILURE;
    }

    const float overlayPadding = rect.w / 16;
    SDL_FRect textRect = {
        rect.x + overlayPadding,
        rect.y + overlayPadding,
        rect.w - 2 * overlayPadding,
        rect.h - 2 * overlayPadding
    };

    return renderTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, text, true, BLACK_COLOR, textRect, NULL);
}

void setGameEndedModalActive(App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    GameEndedModalData* modalData = calloc(1, sizeof(GameEndedModalData));
    SDL_assert(modalData);

    modalData->result = data->state.result;

    // Initializing the modal
    app->events.modal.modalRender.renderRect = calculateGameEndedRect(app->state.currentScene.sceneRender.renderBoxes[CHESSBOARD].renderRect);
    app->events.modal.modalRender.renderFunction = &renderGameEndedModal;
    app->events.modal.modalRender.onMouseButtonDown = &closeModalEventCallbackBox;
    app->events.modal.modalRender.onMouseButtonUp = NULL;
    app->events.modal.modalRender.onMouseWheelScrolled = NULL;
    app->events.modal.modalRender.onMouseEntered = NULL;
    app->events.modal.modalRender.onMouseExited = NULL;

    app->events.modal.canOnlyInteractWithModal = false;
    app->events.modal.data = modalData;
    app->events.modal.onEscape = &closeModalEventCallback;
    app->events.modal.onReturn = &closeModalEventCallback;

    app->events.modal.modalId = GAME_ENDED_MODAL_ID;
    app->events.modal.isActive = true;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
}
