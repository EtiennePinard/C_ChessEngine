#include "AppStyle.h"

AppStyle defaultStyle = {
    .backgroundColor = { 30, 30, 30, 255 },
    .modalBgColor = { 200, 200, 200, 255 },
    .creditsColor = { 128, 128, 128, 255 },
    .chessboardStyle = {
        .square1Color = {100, 100, 100, 255},
        .square2Color = {50, 50, 50, 255},
        .highlightSquareColor = { 80, 80, 120, 128 }
    },
    .textStyle = {
        .textColor = WHITE_COLOR,
        .selectedTextColor = WHITE_COLOR,
        .selectedTextBgColor = { 30, 120, 230, 25 }
    },
    .buttonStyle = {
        .idleColor = {40, 40, 40, 255},
        .borderColor = { 200, 200, 200, 255 },
        .hoverColor = { 80, 80, 120, 255 },
        .clickedColor = { 120, 120, 160, 255 }
    },
    .checkboxStyle = {
        .borderColor = { 80, 80, 120, 255 },
        .hoverColor = { 200, 200, 200, 255 },
        .checkedColor = WHITE_COLOR
    },
    .scrollbarStyle = {
        .idleColor = { 75, 75, 75, 255 },
        .hoverColor = { 111, 111, 111, 255 },
        .clickedColor = { 140, 140, 140, 255 }
    }
};