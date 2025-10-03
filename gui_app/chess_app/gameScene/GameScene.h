#ifndef B8A31D74_EDB3_4C47_A179_16C371768CA7
#define B8A31D74_EDB3_4C47_A179_16C371768CA7

#include "../AppState.h"
#include "../uciEngineCommunication/UCIEngineCommunication.h"

typedef struct UndoGameState {
    ChessPosition position;
    TimeControl whiteTimeControl;
    TimeControl blackTimeControl;
} UndoGameState;

typedef struct UndoGameStates_da {
    UndoGameState* data;
    size_t capacity;
    size_t count;
} UndoGameStates_da;

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

typedef struct Moves_da {
    Move* data;
    size_t capacity;
    size_t count;
} Moves_da;

typedef struct MoveListInfo {
    Moves_da movesPlayed;

    SDL_FRect scrollbarFRect;
    bool isScrolling;
    float startingDragOffset;
    float scrollRatio;

    int hoveredMoveIndex;
} MoveListInfo;

#define NO_SQUARE_SELECTED ((Square)-1)

typedef struct SelectedSquareInfo {
    Square selectedSquare;
} SelectedSquareInfo;


typedef struct GameSceneData {
    GameState state;
    
    bool flipBoard;
    SelectedSquareInfo selectedSquare;
    MoveListInfo moveListInfo;
    UndoGameStates_da undoGameStates;
    
    GameConfig gameInfo;
    Textures textures;
} GameSceneData;

#endif /* B8A31D74_EDB3_4C47_A179_16C371768CA7 */
