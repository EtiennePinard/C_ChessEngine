#ifndef A97C26AB_5900_466A_9103_AC0FB8AA1ABF
#define A97C26AB_5900_466A_9103_AC0FB8AA1ABF

#include "GameScene.h"

typedef struct GameEndedModalData {
    GameResult result;
} GameEndedModalData;

typedef struct PromotionModalData {
    Square promotionSquareTo;
    Square promotionSquareFrom;
} PromotionModalData;

typedef enum GameModalId {
    PROMOTION_MODAL_ID,
    GAME_ENDED_MODAL_ID
} GameModal;

void setPromotionModalActive(App* app, int promotionSquareTo);
void setGameEndedModalActive(App* app);

SDL_FRect calculatePromotionRect(GameSceneData* data, int promotionSquareTo, SDL_FRect boardRect);
SDL_FRect calculateGameEndedRect(SDL_FRect boardRect);

#endif /* A97C26AB_5900_466A_9103_AC0FB8AA1ABF */
