#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "../moveHandler/MoveGenerator.h"
#include "../moveHandler/MovePlayer.h"
#include "Math.h"
#include "AlgebraicNotation.h"

void moveToStandardAlgebraic(ChessPosition position, Move move, char* buffer) {
    Square from = Move_fromSquare(move);
    Square to = Move_toSquare(move);
    Flag moveFlag = Move_flag(move);
    Piece movingPiece = Board_pieceAtIndex(position.board, from);
    Piece capturedPiece = Board_pieceAtIndex(position.board, to);

    // This is for the disambiguation handling
    Move validMoves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    int numMoves;
    MoveHandler_getValidMoves(validMoves, &numMoves, position);

    // Either a piece is captured or en-passant has happened
    bool isCapture = (capturedPiece != NO_PIECE) ||
        (Piece_type(movingPiece) == PAWN && file(from) != file(to));

    int len = 0;
    // Handle castling
    if (moveFlag == KING_SIDE_CASTLING) {
        strcpy(buffer, "O-O");
        len += 3;
        goto endOfGame_handling;
    }
    if (moveFlag == QUEEN_SIDE_CASTLING) {
        strcpy(buffer, "O-O-O");
        len += 5;
        goto endOfGame_handling;
    }

    char pieceChar = Piece_type(movingPiece) == PAWN ? '\0' : " PNBRQK"[Piece_type(movingPiece)];

    char fileChar = 'a' + file(to);
    char rankChar = '1' + 7 - rank(to);

    if (pieceChar) {
        len += sprintf(buffer + len, "%c", pieceChar);

        bool needsDisambiguation = false;
        bool fileConflict = false;
        bool rankConflict = false;

        for (int i = 0; i < numMoves; i++) {
            Move candidate = validMoves[i];
            if (Move_toSquare(candidate) != to || candidate == move) continue;

            Square candidateFrom = Move_fromSquare(candidate);
            Piece candidatePiece = Board_pieceAtIndex(position.board, candidateFrom);

            if (candidatePiece == movingPiece) {
                needsDisambiguation = true;
                if (file(candidateFrom) == file(from)) fileConflict = true;
                if (rank(candidateFrom) == rank(from)) rankConflict = true;
            }
        }

        if (needsDisambiguation) {
            if (!fileConflict) {
                len += sprintf(buffer + len, "%c", 'a' + file(from));
            }
            else if (!rankConflict) {
                len += sprintf(buffer + len, "%d", 8 - rank(from));
            }
            else {
                len += sprintf(buffer + len, "%c%d", 'a' + file(from), 8 - rank(from));
            }
        }
    }
    if (isCapture) {
        if (!pieceChar) {
            // moveingPiece is a pawn so we add the file for captures
            len += sprintf(buffer + len, "%c", 'a' + file(from));
        }
        len += sprintf(buffer + len, "x");
    }
    len += sprintf(buffer + len, "%c%c", fileChar, rankChar);


    if (Move_flag(move) == PROMOTE_TO_QUEEN ||
        Move_flag(move) == PROMOTE_TO_ROOK ||
        Move_flag(move) == PROMOTE_TO_BISHOP ||
        Move_flag(move) == PROMOTE_TO_KNIGHT) {
        const char promoChar = " PNBRQ"[Move_flag(move) - PROMOTE_TO_KNIGHT + KNIGHT];
        len += sprintf(buffer + len, "%c", promoChar);
    }

endOfGame_handling:
    // Checking for checks, mates and stalemates/draws
    // We need to play the move to determine if it results in a check, double check or stalemate
    MoveHandler_playMove(move, &position, false);
    // This is for the end of game handling
    MoveHandler_getValidMoves(validMoves, &numMoves, position);
    if (MoveHandler_isKingInDoubleCheck()) {
        if (numMoves == 0) len += sprintf(buffer + len, "#");
        else len += sprintf(buffer + len, "++");
    }
    else if (MoveHandler_isKingInCheck()) {
        if (numMoves == 0) len += sprintf(buffer + len, "#");
        else len += sprintf(buffer + len, "+");
    }
    else if (numMoves == 0) {
        // It is stalemate
        len += sprintf(buffer + len, "=");
    }
    buffer[len] = '\0';
}
