#ifndef A457A96D_10A5_43F0_96FF_BF739498588A
#define A457A96D_10A5_43F0_96FF_BF739498588A

#include "AppState.h"

void renderPromotionOverlay(SDL_Renderer* renderer, 
                            Textures chessImages,
                            PieceCharacteristics colorToPromote,
                            int promotionSquare,
                            bool flip,
                            Popup *popup);

#endif /* A457A96D_10A5_43F0_96FF_BF739498588A */
