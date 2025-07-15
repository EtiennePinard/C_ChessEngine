#ifndef E50A5778_B8CC_4205_8BEF_5FD650592497
#define E50A5778_B8CC_4205_8BEF_5FD650592497

#include "../sdl_framework/State.h"

#include "../../engine/src/state/ChessPosition.h"
#include "../../engine/src/state/Move.h"
#include "uciEngineCommunication/UCIEngineCommunication.h"

#define APP_VERSION "dev"
#define APP_AUTHOR "Etienne Pinard"
#define CREDIT_TEXT ("Version " APP_VERSION " by " APP_AUTHOR)

#define WINDOW_TITLE ("Chess")
#define STARTING_WINDOW_WIDTH 900
#define STARTING_WINDOW_HEIGHT 850

#define DEFAULT_FONT_SIZE (20.0f)
#define FONT_PATH ("./assets/font/Edwin-Roman.ttf")

#define BASE_IMG_PATH "./assets/img"

#define BASE_CHESS_PIECES_IMG_PATH BASE_IMG_PATH "/chess_pieces"
#define WHITE_PAWN_IMG_PATH (BASE_CHESS_PIECES_IMG_PATH "/white_pawn.png")
#define WHITE_KNIGHT_IMG_PATH (BASE_CHESS_PIECES_IMG_PATH "/white_knight.png")
#define WHITE_BISHOP_IMG_PATH (BASE_CHESS_PIECES_IMG_PATH "/white_bishop.png")
#define WHITE_ROOK_IMG_PATH (BASE_CHESS_PIECES_IMG_PATH "/white_rook.png")
#define WHITE_QUEEN_IMG_PATH (BASE_CHESS_PIECES_IMG_PATH "/white_queen.png") 
#define WHITE_KING_IMG_PATH (BASE_CHESS_PIECES_IMG_PATH "/white_king.png")
#define BLACK_PAWN_IMG_PATH (BASE_CHESS_PIECES_IMG_PATH "/black_pawn.png")
#define BLACK_KNIGHT_IMG_PATH (BASE_CHESS_PIECES_IMG_PATH "/black_knight.png")
#define BLACK_BISHOP_IMG_PATH (BASE_CHESS_PIECES_IMG_PATH "/black_bishop.png")
#define BLACK_ROOK_IMG_PATH (BASE_CHESS_PIECES_IMG_PATH "/black_rook.png")
#define BLACK_QUEEN_IMG_PATH (BASE_CHESS_PIECES_IMG_PATH "/black_queen.png") 
#define BLACK_KING_IMG_PATH (BASE_CHESS_PIECES_IMG_PATH "/black_king.png")

#define BASE_ICON_PATH BASE_IMG_PATH "/icons"
#define HUMAN_ICON_PATH (BASE_ICON_PATH "/human_icon.png")
#define COMPUTER_ICON_PATH (BASE_ICON_PATH "/computer_icon.png")

#define DEFAULT_TIME_CONTROL ((TimeControl) { .timeLeft = (TimeControl_MS) (5 * 60 * 1000), .increment = (TimeControl_MS) (0) })

typedef struct TimeControl {
    TimeControl_MS timeLeft;
    TimeControl_MS increment;
} TimeControl;

typedef struct UndoGameState {
    ChessPosition position;
    TimeControl whiteTimeControl;
    TimeControl blackTimeControl;
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

typedef struct GameState {
    ChessPosition position;
    Player white;
    Player black;

    GameResult result;

    u64 previousTick;
} GameState;

typedef struct Moves {
    Move* data;
    size_t capacity;
    size_t count;
} Moves;

typedef struct MoveListInfo {
    Moves movesPlayed;

    SDL_FRect scrollbarFRect;
    bool isScrolling;
    float startingDragOffset;
    float scrollRatio;

    int hoveredMoveIndex;
} MoveListInfo;

typedef struct GameEndedModalData {
    GameResult result;
} GameEndedModalData;

typedef struct PromotionModalData {
    Square promotionSquareTo;
    Square promotionSquareFrom;
} PromotionModalData;

#define NO_SQUARE_SELECTED ((Square)-1)

typedef struct SelectedSquareInfo {
    Square selectedSquare;
} SelectedSquareInfo;

typedef struct EngineConfig {
    bool isEngine;
    TimeControl_MS timeToThink; // 0 if the bot thinks by itself
    char* enginePath;
} EngineConfig;

typedef struct PlayerConfig {
    TimeControl timeControl;
    EngineConfig engineConfig;
} PlayerConfig;

typedef struct GameConfig {
    PlayerConfig white;
    PlayerConfig black;
    char* startingPositionFen;
} GameConfig;

typedef struct GameSceneData {
    GameState state;
    
    bool flipBoard;
    SelectedSquareInfo selectedSquare;
    MoveListInfo moveListInfo;
    UndoGameStates undoGameStates;
    
    GameConfig gameInfo;
    Textures textures;
} GameSceneData;

typedef struct TimeControlModalData {
    TimeControl hovered;
    PieceCharacteristics playerColor;
} TimeControlModalData;

typedef struct EngineConfigModalData {
    EngineConfig currentConfig;
    PieceCharacteristics playerColor;

    SDL_FRect checkboxRect;
    SDL_FRect enginePathRect;
    SDL_FRect thinkTimeRect;
    SDL_FRect okButtonRect;
    SDL_FRect cancelButtonRect;
} EngineConfigModalData;

typedef struct MainMenuSceneData {
    GameConfig gameInfo;

    Textures textures;
} MainMenuSceneData;

#define GAME_SCENE_ID (0)
#define MAIN_MENU_SCENE_ID (1)

#endif /* E50A5778_B8CC_4205_8BEF_5FD650592497 */
