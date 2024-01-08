#include "Piece.h"

PieceCharacteristics pieceColor(Piece piece) {
    return (PieceCharacteristics) (piece & 0b11000);
}

PieceCharacteristics pieceType(Piece piece) {
    return (PieceCharacteristics) (piece & 0b111);
}

Piece makePiece(PieceCharacteristics color, PieceCharacteristics type) {
    return (Piece) (color | type);
}
