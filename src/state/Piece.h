#ifndef C61A945A_18DA_4DBE_85F9_7784EBEDF6AE
#define C61A945A_18DA_4DBE_85F9_7784EBEDF6AE

/**
 * A piece is char that contains the color and the type of the piece.
 * The piece type is in the first 3 least significant bits.
 * The piece color is the next 2 bits that follow the piece type bits
 * The first 3 most significant bits of the char are unused.
 * Use the pieceColor(piece) function or pieceType(piece) function to extract those information
 * Use the makePiece(color, type) function to create a valid Piece
*/
typedef char Piece;

typedef enum PieceCharacteristics {
    NOPIECE = 0, 
    KING    = 1, 
    KNIGHT  = 2, 
    BISHOP  = 3, 
    QUEEN   = 4, 
    ROOK    = 5, 
    PAWN    = 6, 
            
    WHITE   = 8, 
    BLACK   = 16
} PieceCharacteristics;

#define MAX_PIECE_INDEX (BLACK | PAWN)

#define pieceColor(piece) (PieceCharacteristics) (piece & 0b11000)
#define pieceType(piece) (PieceCharacteristics) (piece & 0b111)
#define makePiece(color, type) (Piece) (color | type)

#endif /* C61A945A_18DA_4DBE_85F9_7784EBEDF6AE */
