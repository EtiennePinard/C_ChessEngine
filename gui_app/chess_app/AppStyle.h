#ifndef FFFA9261_E3F1_49C7_A19E_16F715951AD0
#define FFFA9261_E3F1_49C7_A19E_16F715951AD0

#include <SDL3/SDL.h>

#define PADDING_PERCENT (0.03f)
#define CREDITS_HEIGHT_PERCENT (0.05f)
#define CREDITS_WIDTH_PERCENT (0.5f)

#define WHITE_COLOR ((SDL_Color) {255, 255, 255, 255})
#define BLACK_COLOR ((SDL_Color) { 0, 0, 0, 255 })

typedef struct TextStyle {
    SDL_Color textColor;
    SDL_Color selectedTextColor;
    SDL_Color selectedTextBgColor;
} TextStyle;

typedef struct CheckboxStyle {
    SDL_Color borderColor;
    SDL_Color hoverColor;
    SDL_Color checkedColor;
} CheckboxStyle;

typedef struct ButtonStyle {
    SDL_Color idleColor;
    SDL_Color borderColor;
    SDL_Color hoverColor;
    SDL_Color clickedColor;
} ButtonStyle;

typedef struct ChessboardStyle {
    SDL_Color square1Color;
    SDL_Color square2Color;
    SDL_Color highlightSquareColor;
} ChessboardStyle;

typedef struct ScrollBarStyle {
    SDL_Color idleColor;
    SDL_Color hoverColor;
    SDL_Color clickedColor;
} ScrollBarStyle;

typedef struct AppStyle {
    SDL_Color backgroundColor;
    SDL_Color modalBgColor;
    SDL_Color creditsColor;

    ChessboardStyle chessboardStyle;
    TextStyle textStyle;
    ButtonStyle buttonStyle;
    CheckboxStyle checkboxStyle;
    ScrollBarStyle scrollbarStyle;
} AppStyle;

extern AppStyle defaultStyle;

#endif /* FFFA9261_E3F1_49C7_A19E_16F715951AD0 */
