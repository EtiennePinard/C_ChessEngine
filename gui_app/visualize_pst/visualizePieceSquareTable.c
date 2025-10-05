#include <limits.h>

#include "../../engine/src/utils/Math.h"
#include "../../engine/src/bot/PieceSquareTable.h"

#include "../sdl_framework/State.h"
#include "../sdl_framework/AppInit.h"
#include "../sdl_framework/AppCleanup.h"
#include "../sdl_framework/CommonRenderFunctions.h"
#include "../sdl_framework/CommonEvents.h"

#define WINDOW_TITLE ("Piece Square Table Visualization")
#define STARTING_WINDOW_WIDTH (640)
#define STARTING_WINDOW_HEIGHT (640 + BUTTON_HEIGHT)

#define BUTTON_HEIGHT 80
#define BUTTON_RECT ((SDL_FRect) {0, STARTING_WINDOW_HEIGHT - BUTTON_HEIGHT, STARTING_WINDOW_WIDTH, BUTTON_HEIGHT})

#define BACKGROUND_COLOR ((SDL_Color) {0xD9, 0xD9, 0xD9, 255}) // light gray

#define SQUARE_COLOR ((SDL_Color) {255, 0, 0, 255}) // red
#define TEXT_COLOR ((SDL_Color) {0, 0, 0, 255}) // black

#define BUTTON_BORDER_COLOR ((SDL_Color) { 0, 0, 0, 255 })
#define BUTTON_IDLE_COLOR ((SDL_Color){ 200, 200, 200, 255 }) // Bright gray
#define BUTTON_HIGHLIGHT_COLOR ((SDL_Color) {100, 100, 100, 255}) // Dark gray
#define BUTTON_CLICKED_COLOR ((SDL_Color){ 120, 120, 120, 255 }) // Bright gray

#define DEFAULT_FONT_SIZE (20.0f)
#define FONT_PATH ("./assets/font/Edwin-Roman.ttf")

typedef struct Button {
    SDL_Color color;
    char* text;
} Button;

typedef struct PST_State {
    PieceCharacteristics color;
    PieceCharacteristics type;
    GamePhase phase;
} PST_State;

#define PST_SCENE_ID (0)
typedef enum PSTSceneRenderBoxIndex {
    BOARD_SCENE_INDEX,
    BUTTON_SCENE_INDEX,
    TOTAL_PST_RENDER_BOX
} PSTSceneRenderBoxIndex;

float normalizingFunction(int squareScore, int maxValue, int minValue) {
    return (float)(squareScore - minValue) / (maxValue - minValue);
}

SDL_AppResult visualizePieceSquareTable(SDL_Renderer* renderer, TTF_Font* font,
    SDL_FRect boardRect, int pieceSquareTable[BOARD_SIZE], int maxValue, int minValue, int phase) {

    const float squareSize = boardRect.w / BOARD_LENGTH;
    for (int row = 0; row < BOARD_LENGTH; row++) {
        for (int col = 0; col < BOARD_LENGTH; col++) {
            // Set render color
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            SDL_FRect square = {
                .x = boardRect.x + col * squareSize,
                .y = boardRect.y + row * squareSize,
                .w = squareSize,
                .h = squareSize
            };
            int squareIndex = row * 8 + col;
            int midEgMixed = pieceSquareTable[squareIndex];
            int squareScore = phase == MIDGAME ? mg_value(midEgMixed) : eg_value(midEgMixed);
            // Adjust opacity based on score
            Uint8 opacity = (Uint8)(normalizingFunction(SDL_abs(squareScore), maxValue, minValue) * 255.0);
            opacity = max(100, opacity);
            SDL_SetRenderDrawColor(renderer,
                SQUARE_COLOR.r, SQUARE_COLOR.g, SQUARE_COLOR.b,
                opacity);

            // Render square with current opacity
            SDL_RenderFillRect(renderer, &square);

            const float textPaddingPercent = 0.1f;
            const float textRectPadding = square.w * textPaddingPercent;
            SDL_FRect textRect = {
                .x = square.x + textRectPadding,
                .y = square.y + 2 * textRectPadding,
                .w = square.w - 2 * textRectPadding,
                .h = square.h - 4 * textRectPadding
            };
            char number[6];
            SDL_snprintf(number, sizeof(number), "%d", squareScore);
            renderSingleLineTextCenteredToFit(renderer, font, number, TEXT_COLOR, textRect);
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderPieceSquareTable(SDL_FRect rect, App* app) {
    PST_State* pstState = (PST_State*)app->state.currentScene.data;
    PieceCharacteristics piece = Piece_makePiece(pstState->color, pstState->type);
    int* pst = pieceSquareTable[piece - 9];
    int maxValue = INT16_MIN;
    int minValue = INT16_MAX;
    for (int square = 0; square < BOARD_SIZE; square++) {
        int score = pstState->phase == MIDGAME ? mg_value(pst[square]) : eg_value(pst[square]);
        maxValue = max(SDL_abs(score), maxValue);
        minValue = min(SDL_abs(score), minValue);
    }

    // Draw the chessboard
    return visualizePieceSquareTable(app->state.sdlState.renderer, app->state.sdlState.font,
        rect, pst, maxValue, minValue, pstState->phase);
}


void getButtonText(char* buttonText, PST_State* pstState) {
    int currentIndex = 0;
    const int colorTextLength = 6;
    switch (pstState->color) {
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
    switch (pstState->type) {
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
    switch (pstState->phase)
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

void updatePSTPParam(PST_State* pstState) {
    pstState->type = pstState->type % NB_PIECE_TYPE + PAWN; // Always update the piece type
    if (pstState->type == PAWN) { // When we switch back to a pawn
        pstState->color = pstState->color % BLACK + WHITE;
        if (pstState->color == WHITE) { // When we switch back to white
            pstState->phase = (pstState->phase + 1) % NB_PHASE;
        }
    }
}

SDL_AppResult renderSwitchButton(SDL_FRect rect, App* app) {
    SDL_SetRenderDrawBlendMode(app->state.sdlState.renderer, SDL_BLENDMODE_NONE);
    
    char text[21];
    PST_State* pstState = (PST_State*)app->state.currentScene.data;
    getButtonText(text, pstState);
    return renderButton(rect, app, BUTTON_SCENE_INDEX, text, 
        BUTTON_HIGHLIGHT_COLOR, BUTTON_CLICKED_COLOR, BUTTON_IDLE_COLOR, BUTTON_BORDER_COLOR, TEXT_COLOR);
}

SDL_AppResult clickedSwitchButton(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;
    PST_State* pstState = (PST_State*)app->state.currentScene.data;
    updatePSTPParam(pstState);
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

#define BOARD_SIZE_PERCENT (0.8f)

SDL_AppResult computePSTSceneRender(App* app) {
    app->events.onKeyDown = NULL;
    app->events.onTextInput = NULL;

    SceneRender* sceneRender = &app->state.currentScene.sceneRender;
    // Setting in the background color of the scene
    sceneRender->renderDrawColor = BACKGROUND_COLOR;

    sceneRender->numRenderBox = TOTAL_PST_RENDER_BOX;
    // We assume that sceneRender->renderBoxes is always NULL
    if (sceneRender->renderBoxes != NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "renderBoxes is not NULL when re-computing pst scene\n");
        return SDL_APP_FAILURE;
    }
    sceneRender->renderBoxes = SDL_calloc(sceneRender->numRenderBox, sizeof(RenderBox));

    int windowWidth, windowHeight;
    SDL_GetWindowSize(app->state.sdlState.window, &windowWidth, &windowHeight);

    const float boardSize = BOARD_SIZE_PERCENT * SDL_min(windowWidth, windowHeight);
    const float verticalSpaceBoardShouldReserve = BOARD_SIZE_PERCENT * windowHeight;

    SDL_FRect boardRect = {
        .x = (windowWidth - boardSize) / 2.0,
        .y = (verticalSpaceBoardShouldReserve - boardSize) / 2.0,
        .w = boardSize,
        .h = boardSize
    };
    sceneRender->renderBoxes[BOARD_SCENE_INDEX].renderRect = boardRect;
    sceneRender->renderBoxes[BOARD_SCENE_INDEX].renderFunction = &renderPieceSquareTable;

    SDL_FRect buttonRect = {
        .x = 0,
        .y = verticalSpaceBoardShouldReserve,
        .w = windowWidth,
        .h = windowHeight - verticalSpaceBoardShouldReserve
    };
    sceneRender->renderBoxes[BUTTON_SCENE_INDEX].renderRect = buttonRect;
    sceneRender->renderBoxes[BUTTON_SCENE_INDEX].renderFunction = &renderSwitchButton;
    sceneRender->renderBoxes[BUTTON_SCENE_INDEX].onMouseButtonDown = &clickedSwitchButton;
    sceneRender->renderBoxes[BUTTON_SCENE_INDEX].onMouseButtonUp = &rerenderScene;
    sceneRender->renderBoxes[BUTTON_SCENE_INDEX].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[BUTTON_SCENE_INDEX].onMouseExited = &rerenderScene;
    
    return SDL_APP_CONTINUE;
}

void terminatePSTScene(void* data) {
    SDL_free(data);
}

SDL_AppResult onWindowResize(SDL_Event* event, App* app) {
    (void)event;
    if (app->state.currentScene.sceneRender.renderBoxes != NULL) {
        SDL_free(app->state.currentScene.sceneRender.renderBoxes);
        app->state.currentScene.sceneRender.renderBoxes = NULL;
    }

    switch (app->state.currentScene.sceneId) {
    case PST_SCENE_ID:
        if (computePSTSceneRender(app) != SDL_APP_CONTINUE) return SDL_APP_FAILURE;
        break;
    default:
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Invalid scene ID %d\n", app->state.currentScene.sceneId);
        return SDL_APP_FAILURE;
    }
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

bool initializeApp(App* app) {
    SDL_Log("Initializing SDL libraries...\n");
    if (!initializeSDlLibraries(SDL_INIT_VIDEO) ||
        !initializeSDLState(&app->state.sdlState,
            WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, STARTING_WINDOW_WIDTH, STARTING_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE,
            NULL,
            FONT_PATH, DEFAULT_FONT_SIZE)) {
        return false;
    }
    SDL_Log("Done!\n");

    SDL_Log("Initializing piece square table...\n");
    if (!PieceSquareTable_init()) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize the piece square table\n");
        return false;
    }
    SDL_Log("Done!\n");

    SDL_Log("Initializing Main Menu Scene...\n");
    PST_State* pstState = SDL_malloc(sizeof(PST_State));
    pstState->type = PAWN;
    pstState->color = WHITE;
    pstState->phase = MIDGAME;

    app->state.currentScene.data = pstState;
    app->state.currentScene.sceneId = PST_SCENE_ID;
    app->state.currentScene.terminateSceneFunction = &terminatePSTScene;
    if (computePSTSceneRender(app) != SDL_APP_CONTINUE) return false;

    app->events.onWindowResize = &onWindowResize;

    SDL_Log("Done!\n");

    SDL_Log("App is initialized!\n");
    return true;
}

void cleanupApp(App* app) {
    SDL_free(app->state.currentScene.sceneRender.renderBoxes);
    cleanupSDL_State(app->state.sdlState);
    quitSDL();
}

