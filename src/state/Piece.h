#ifndef C61A945A_18DA_4DBE_85F9_7784EBEDF6AE
#define C61A945A_18DA_4DBE_85F9_7784EBEDF6AE

#include "../utils/Utils.h"

/**
 * @brief A piece is an unsigned 8 bit integer that contains the color and the type of the piece.
 * The piece type is in the first 3 least significant bits.
 * The piece color is the next 2 bits that follow the piece type bits
 * The first 3 most significant bits are unused.
 * Use the Piece_color(piece) function or Piece_type(piece) function to extract those information
 * Use the Piece_makePiece(color, type) function to create a valid Piece
 */
typedef u8 Piece;

typedef enum PieceCharacteristics {
    NOPIECE = 0, 
    PAWN    = 1, 
    KNIGHT  = 2, 
    BISHOP  = 3, 
    ROOK    = 4, 
    QUEEN   = 5, 
    KING    = 6, 
            
    WHITE   = 8, 
    BLACK   = 16
} PieceCharacteristics;

#define MAX_PIECE_INDEX (BLACK | KING) // 22
#define MIN_PIECE_INDEX (WHITE | PAWN) // 9
#define NB_PIECE_TYPE (6)
#define NB_PIECE_COLOR (2)
#define NB_PIECES (NB_PIECE_COLOR * NB_PIECE_TYPE)

#define Piece_color(piece) ((PieceCharacteristics) (piece & 0b11000))
#define Piece_type(piece) ((PieceCharacteristics) (piece & 0b111))
#define Piece_makePiece(color, type) ((Piece) (color | type))

#endif /* C61A945A_18DA_4DBE_85F9_7784EBEDF6AE */
