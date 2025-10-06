#ifndef A97C26AB_5900_466A_9103_AC0FB8AA1ABF
#define A97C26AB_5900_466A_9103_AC0FB8AA1ABF

#include "../GameScene.h"

typedef struct GameEndedModalData {
    GameResult result;
} GameEndedModalData;

typedef struct PromotionModalData {
    Square promotionSquareTo;
    Square promotionSquareFrom;
} PromotionModalData;

typedef struct SettingsData {
    GameConfig gameInfo;
    PieceCharacteristics currentColor;
    AppStyle currentStyle;

    SDL_FRect colorToggle;
    SDL_FRect engineConfig;
    SDL_FRect timeControl;
    SDL_FRect startingPosition;
    SDL_FRect styleButton;

    SDL_FRect saveButton;
    SDL_FRect cancelButton;

    Textures textures;

    bool wasInStartingPositionBefore;
    bool wasTextInputExited;
} SettingsData;

typedef struct TimeControlModalData {
    TimeControl hovered;

    SettingsData* savedSettingsData;
} TimeControlModalData;

typedef struct EngineConfigModalData {
    EngineConfig currentConfig;

    SDL_FRect checkboxRect;
    SDL_FRect enginePathRect;
    SDL_FRect thinkTimeRect;
    SDL_FRect okButtonRect;
    SDL_FRect cancelButtonRect;
    SDL_FRect thinkByHimselfRect;

    bool wasTextInputExited;

    SettingsData* savedSettingsData;
} EngineConfigModalData;

typedef struct StyleModalData {
    AppStyle* hovered;
    bool isStyleHovered;

    SettingsData* savedSettingsData;
} StyleModalData;

typedef enum GameModalId {
    PROMOTION_MODAL_ID,
    GAME_ENDED_MODAL_ID,
    SETTINGS_MODAL_ID,
    TIME_CONTROL_MODAL_ID,
    ENGINE_CONFIG_MODAL_ID,
    STYLE_MODAL_ID
} GameModal;

void setPromotionModalActive(App* app, int promotionSquareTo);
void setGameEndedModalActive(App* app);
SDL_AppResult setSettingsModalActive(App* app);
SDL_AppResult setSettingsModalActiveFromCopy(App* app, SettingsData* settingsData);
void setTimeControlModalActive(App* app);
void setEngineConfigModalActive(App* app);
void setStyleModalActive(App* app);

SDL_FRect calculatePromotionRect(GameSceneData* data, int promotionSquareTo, SDL_FRect boardRect);
SDL_FRect calculateGameEndedRect(SDL_FRect boardRect);
SDL_FRect calculateSettingsRect(App* app);

#endif /* A97C26AB_5900_466A_9103_AC0FB8AA1ABF */
