#ifndef E8619F73_510A_4F85_B1F4_77DA454B0DE5
#define E8619F73_510A_4F85_B1F4_77DA454B0DE5

#define VERSION ("dev")
#define ENGINE_NAME ("Proc nue")
#define AUTHOR ("Etienne Pinard")

/**
 * @brief The fen string for the initial starting position
 * IMPORTANT: You cannot pass this string directly to the FenString_setChessPositionFromFenString function
 * because this is a string literal and these are placed in read-only memory. The FenString_setChessPositionFromFenString
 * requires modification of the string and so you need to put it into a temporary variable before passing it to the function.
 * 
 * One way to put the string literal and is to copy it to a char array using memcpy. For example:
 *     char initialFen[57];
 *     memcpy(initialFen, INITIAL_FEN, 57);
 * 
 * Another way is to use the FenString_setChessPositionFromCopiedFenString function which will copy the string
*/
#define INITIAL_FEN ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")

#define BOARD_SIZE (64)
#define BOARD_LENGTH (8)

// MAX_LEGAL_MOVES happens in the position R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1
#define MAX_LEGAL_MOVES (218)
#define POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES (256)

// From http://wismuth.com/chess/longest-game.html
#define MAX_NB_LEGAL_MOVES_IN_GAME (8849)

// Number taken from https://chess.stackexchange.com/a/30006
#define MAX_FEN_STRING_SIZE (87)

typedef enum Square {
    A8, B8,  C8,  D8,  E8,  F8,  G8,  H8,
    A7, B7,  C7,  D7,  E7,  F7,  G7,  H7,
    A6, B6,  C6,  D6,  E6,  F6,  G6,  H6,
    A5, B5,  C5,  D5,  E5,  F5,  G5,  H5,
    A4, B4,  C4,  D4,  E4,  F4,  G4,  H4,
    A3, B3,  C3,  D3,  E3,  F3,  G3,  H3,
    A2, B2,  C2,  D2,  E2,  F2,  G2,  H2,
    A1, B1,  C1,  D1,  E1,  F1,  G1,  H1
} Square;

#endif /* E8619F73_510A_4F85_B1F4_77DA454B0DE5 */
