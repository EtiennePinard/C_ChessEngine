#ifndef E50A5778_B8CC_4205_8BEF_5FD650592497
#define E50A5778_B8CC_4205_8BEF_5FD650592497

#include "../sdl_framework/State.h"

#include "../../engine/src/state/ChessPosition.h"
#include "../../engine/src/state/Move.h"
#include "uciEngineCommunication/UCIEngineCommunication.h"

#define APP_VERSION "dev"
#define APP_AUTHOR "Etienne Pinard"
#define CREDIT_TEXT ("Version " APP_VERSION " by " APP_AUTHOR)

#define STARTING_WINDOW_WIDTH 900
#define STARTING_WINDOW_HEIGHT 850
#define DEFAULT_FONT_SIZE (20.0f)
#define FONT_PATH ("./assets/font/Edwin-Roman.ttf")
#define TITLE ("Chess")

#define WHITE_PAWN_IMG_PATH ("./assets/png/white_pawn.png")
#define WHITE_KNIGHT_IMG_PATH ("./assets/png/white_knight.png")
#define WHITE_BISHOP_IMG_PATH ("./assets/png/white_bishop.png")
#define WHITE_ROOK_IMG_PATH ("./assets/png/white_rook.png")
#define WHITE_QUEEN_IMG_PATH ("./assets/png/white_queen.png") 
#define WHITE_KING_IMG_PATH ("./assets/png/white_king.png")
#define BLACK_PAWN_IMG_PATH ("./assets/png/black_pawn.png")
#define BLACK_KNIGHT_IMG_PATH ("./assets/png/black_knight.png")
#define BLACK_BISHOP_IMG_PATH ("./assets/png/black_bishop.png")
#define BLACK_ROOK_IMG_PATH ("./assets/png/black_rook.png")
#define BLACK_QUEEN_IMG_PATH ("./assets/png/black_queen.png") 
#define BLACK_KING_IMG_PATH ("./assets/png/black_king.png")

#define DEFAULT_TIME_CONTROL ((TimeControl) { .increment = (TimeControl_MS) (5 * 60 * 1000), .timeLeft = (TimeControl_MS) (0) })

typedef struct TimeControl {
    TimeControl_MS timeLeft;
    TimeControl_MS increment;
} TimeControl;

typedef struct UndoGameState {
    ChessPosition position;
    TimeControl playerToGoTimeControl;
} UndoGameState;

typedef struct UndoGameStates {
    UndoGameState* data;
    size_t capacity;
    size_t count;
} UndoGameStates;

typedef struct Player {
    TimeControl timeControl;

    struct EngineCommunication* engineCommunication;
    SDL_AtomicInt isBotThinking;
} Player;

typedef struct GameState {
    ChessPosition position;
    Player white;
    Player black;

    u64 previousTick;
} GameState;

typedef struct Moves {
    Move* data;
    size_t capacity;
    size_t count;
} Moves;

typedef struct MoveListInfo {
    Moves movesPlayed;

    size_t moveListScrollY;
} MoveListInfo;

typedef enum GameResult {
    GAME_IS_NOT_DONE,
    THREE_MOVE_REPETITION,
    STALEMATE,
    INSUFFICIENT_MATERIAL,
    FIFTY_MOVE_RULE,
    WHITE_WON_CHECKMATE,
    BLACK_WON_CHECKMATE,
    WHITE_WON_ON_TIME,
    BLACK_WON_ON_TIME
} GameResult;

typedef struct GameEndedInfo {
    bool renderOverlay;
    GameResult result;
} GameEndedInfo;

typedef struct PromotionInfo {
    bool renderPromotionOverlay;
    Square promotionSquareTo;
    Square promotionSquareFrom;
    SDL_Rect overlayRect;
} PromotionInfo;

typedef struct SelectedSquareInfo {
    Square selectedSquare;
} SelectedSquareInfo;

typedef struct PlayerConfig {
    bool isEngine;
    char* enginePath;
} PlayerConfig;

typedef struct GameConfig {
    PlayerConfig white;
    PlayerConfig black;
    TimeControl timeControl;
} GameConfig;

typedef struct GameSceneData {
    GameState state;
    
    bool flipBoard;
    SelectedSquareInfo selectedSquare;
    PromotionInfo promotionInfo;
    MoveListInfo moveListInfo;
    GameEndedInfo gameEndedInfo;
    UndoGameStates undoGameStates;
    
    GameConfig gameInfo;
    Textures textures;
} GameSceneData;

typedef struct TimeControlSettings {
    TimeControl hovered;
    bool selectModalVisible;
} TimeControlSettings;

typedef struct MainMenuSceneData {
    GameConfig gameInfo;
    TimeControlSettings timeControlSettings;

    Textures textures;
} MainMenuSceneData;

#define GAME_SCENE_ID (0)
#define MAIN_MENU_SCENE_ID (1)

#endif /* E50A5778_B8CC_4205_8BEF_5FD650592497 */
