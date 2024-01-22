#include "Piece.h"

// TODO: Make this a macro
PieceCharacteristics pieceColor(Piece piece) {
    return (PieceCharacteristics) (piece & 0b11000);
}

// TODO: Make this a macro
PieceCharacteristics pieceType(Piece piece) {
    return (PieceCharacteristics) (piece & 0b111);
}

// TODO: Make this a macro
Piece makePiece(PieceCharacteristics color, PieceCharacteristics type) {
    return (Piece) (color | type);
}
