#ifndef E8619F73_510A_4F85_B1F4_77DA454B0DE5
#define E8619F73_510A_4F85_B1F4_77DA454B0DE5

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

#endif /* E8619F73_510A_4F85_B1F4_77DA454B0DE5 */
