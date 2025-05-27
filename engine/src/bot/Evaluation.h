#ifndef FB19040F_C714_43C1_A4F7_A0380FD02F7C
#define FB19040F_C714_43C1_A4F7_A0380FD02F7C

#include "../state/ChessPosition.h"

/**
 * @brief Returns a score that was calculated by statically analyzing the position.
 * Static analysis means only looking at information currently available, not considering 
 * potential good moves in the score.
*/
int Bot_staticEvaluation(ChessPosition position);

#endif /* FB19040F_C714_43C1_A4F7_A0380FD02F7C */
