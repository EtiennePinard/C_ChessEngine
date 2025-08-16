#include "../../../sdl_framework/CommonEvents.h"
#include "../../../sdl_framework/CommonRenderFunctions.h"

#include "../../AppStyle.h"

#include "../GameRender.h"
#include "GameModals.h"

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
        SDL_memcpy(text, "Game is on!", 12);
        break;
    case THREE_MOVE_REPETITION:
        SDL_memcpy(text, "Draw by\nrepetition", 20);
        break;
    case STALEMATE:
        SDL_memcpy(text, "Stalemate", 10);
        break;
    case INSUFFICIENT_MATERIAL:
        SDL_memcpy(text, "Draw by\ninsufficient material", 30);
        break;
    case FIFTY_MOVE_RULE:
        SDL_memcpy(text, "Draw by\nfifty move rule", 24);
        break;
    case WHITE_WON_CHECKMATE:
        SDL_memcpy(text, "White won\nby checkmate", 23);
        break;
    case BLACK_WON_CHECKMATE:
        SDL_memcpy(text, "Black won\nby checkmate", 23);
        break;
    case WHITE_WON_ON_TIME:
        SDL_memcpy(text, "White won\non time", 18);
        break;
    case BLACK_WON_ON_TIME:
        SDL_memcpy(text, "Black won\non time", 18);
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

    return renderMultilineTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, text, BLACK_COLOR, textRect);
}

void setGameEndedModalActive(App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    GameEndedModalData* modalData = SDL_calloc(1, sizeof(GameEndedModalData));
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
