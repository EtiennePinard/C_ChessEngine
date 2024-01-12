#ifndef C61A945A_18DA_4DBE_85F9_7784EBEDF6AE
#define C61A945A_18DA_4DBE_85F9_7784EBEDF6AE

/**
 * A piece is char that contains the color and the type of the piece.
 * The piece type is in the first 3 least significant bits.
 * The piece color is the next 2 bits that follow the piece type bits
 * The first 3 most significant bits of the char are unused.
 * Use the pieceColor function or pieceType function to extract those information
 * Use the makePiece(color, type) macro to create a valid Piece
*/
typedef char Piece;

/**
 * Note that the order of the pieces are not a coincidence
 * The pawn, rook and queen follow one another to use a clever trick
 * in the checkEnPassantPinned function in moveGenerator.c
*/
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

PieceCharacteristics pieceColor(Piece piece);
PieceCharacteristics pieceType(Piece piece);

Piece makePiece(PieceCharacteristics color, PieceCharacteristics type);

#endif /* C61A945A_18DA_4DBE_85F9_7784EBEDF6AE */
