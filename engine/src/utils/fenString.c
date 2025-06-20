#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../state/ZobristKey.h"
#include "Constants.h"
#include "Math.h"

#include "FenString.h"

static int getCastlingPermFromFenString(const char* castlingPerm) {
    int result = 0;

    size_t lengthOfCastlingPerm = strlen(castlingPerm);
    if (lengthOfCastlingPerm == 0) { return -1; }
    if (castlingPerm[0] == '-') { return 0; }
    if (lengthOfCastlingPerm > 4) { return -1; }

    // We only take characters 'K' = 75, 'Q' = 81, 'k' = 107 and 'q' = 113
    // The shift amounts are:  'K' -> 3, 'Q' -> 2, 'k' -> 1 and 'q' -> 0
    // The hash function is f(c) = (113 - c) / 6
    // We get these results
    // f('K') = 6, f('Q') = 5, f('k') = 1, f('q') = 0
    // This justifies the weird shape of shiftAmounts
    int shiftAmounts[7] = { 0, 1, 0, 0, 0, 2, 3 };
    for (size_t index = 0; index < lengthOfCastlingPerm; index++) {
        int hashedCharacter = (113 - castlingPerm[index]) / 6;
        result |= (1 << shiftAmounts[hashedCharacter]);
    }
    return result;
}

static bool setBoardArrayFromFenString(const char* fenBoard, Piece board[BOARD_SIZE]) {
    // The minimum length for a board string is 15 with this string "8/8/8/8/8/8/8/8"
    size_t lengthOfBoardFen = strlen(fenBoard);
    if (lengthOfBoardFen < 15) { return false; }

    int index = 0;
    for (size_t str_index = 0; str_index < lengthOfBoardFen; str_index++) {
        char fenChar = fenBoard[str_index];

        switch (fenChar) {
            // Saw this type of trick in https://youtu.be/zGWj7Qo_POY?si=ie9cEXp5p_59KLfU&t=799
        case '8': board[index++] = NO_PIECE; __attribute__((fallthrough));
        case '7': board[index++] = NO_PIECE; __attribute__((fallthrough));
        case '6': board[index++] = NO_PIECE; __attribute__((fallthrough));
        case '5': board[index++] = NO_PIECE; __attribute__((fallthrough));
        case '4': board[index++] = NO_PIECE; __attribute__((fallthrough));
        case '3': board[index++] = NO_PIECE; __attribute__((fallthrough));
        case '2': board[index++] = NO_PIECE; __attribute__((fallthrough));
        case '1': board[index++] = NO_PIECE;
            break;

        case 'P':
            board[index++] = Piece_makePiece(WHITE, PAWN);
            break;
        case 'N':
            board[index++] = Piece_makePiece(WHITE, KNIGHT);
            break;
        case 'B':
            board[index++] = Piece_makePiece(WHITE, BISHOP);
            break;
        case 'R':
            board[index++] = Piece_makePiece(WHITE, ROOK);
            break;
        case 'Q':
            board[index++] = Piece_makePiece(WHITE, QUEEN);
            break;
        case 'K':
            board[index++] = Piece_makePiece(WHITE, KING);
            break;
        case 'p':
            board[index++] = Piece_makePiece(BLACK, PAWN);
            break;
        case 'n':
            board[index++] = Piece_makePiece(BLACK, KNIGHT);
            break;
        case 'b':
            board[index++] = Piece_makePiece(BLACK, BISHOP);
            break;
        case 'r':
            board[index++] = Piece_makePiece(BLACK, ROOK);
            break;
        case 'q':
            board[index++] = Piece_makePiece(BLACK, QUEEN);
            break;
        case 'k':
            board[index++] = Piece_makePiece(BLACK, KING);
            break;
        default:
            // Note: do not increment index on '/' character
            break;
        }
    }

    return index == 64;
}

static PieceCharacteristics getColorToGo(const char* fenColor) {
    if (strlen(fenColor) != 1) { return false; }
    if (fenColor[0] == 'w') {
        return WHITE;
    }
    else if (fenColor[0] == 'b') {
        return BLACK;
    }
    else {
        return -1;
    }
}

bool FenString_setChessPositionFromCopiedFenString(const char* fen, ChessPosition* position) {
    if (fen == NULL) { return false; }

    size_t sizeOfString = strlen(fen);
    char* copied = malloc(sizeOfString + 1); // We need to add 1 for the null byte
    assert(copied != NULL && "copied is NULL, buy more RAM LOL");
    memcpy(copied, fen, sizeOfString + 1);

    bool returnValue = FenString_setChessPositionFromFenString(copied, position);

    free(copied);

    return returnValue;
}

bool FenString_setChessPositionFromFenString(char* fen, ChessPosition* position) {
    if (fen == NULL) { return false; }

    Tokens tokens;
    tokens.length = string_removeUnecessarySpacesAndTabs(fen);
    char* uniqueName[tokens.length];
    tokens.tokens = uniqueName;
    string_tokenizeStringBySpace(fen, &tokens);

    return FenString_setChessPositionFromTokens(&tokens, position);
}

bool FenString_setChessPositionFromTokens(Tokens* fenTokenized, ChessPosition* position) {
    if (position == NULL || fenTokenized == NULL) return false;

    if (fenTokenized->length != 6) return false;

    // Setting up the board
    Piece boardArray[BOARD_SIZE] = { 0 };
    if (!setBoardArrayFromFenString(fenTokenized->tokens[0], boardArray)) return false;
    Board board = { 0 };
    Board_fromArray(&board, boardArray);
    position->board = board;

    // Setting the color to go
    position->colorToGo = getColorToGo(fenTokenized->tokens[1]);
    if ((int)position->colorToGo == -1) return false;

    // Setting the castling perm
    int num = getCastlingPermFromFenString(fenTokenized->tokens[2]);
    if (num == -1) return false;
    position->castlingPerm = num;

    // Setting the en passant target square
    num = string_algebraicToIndex(fenTokenized->tokens[3]);
    if (num == -1) return false;
    position->enPassantTargetSquare = num;

    // Setting the fifty rule turns
    num = string_parseNumber(fenTokenized->tokens[4]);
    if (num == -1) return false;
    position->turnsForFiftyRule = num;

    // Setting the number of moves
    num = string_parseNumber(fenTokenized->tokens[5]);
    if (num == -1) return false;
    position->nbMoves = num;

    ZobristKey_calculateInitialKey(position);

    return true;
}

int FenString_chessPositionToFenString(ChessPosition position, char fen[MAX_FEN_STRING_SIZE]) {
    int fenIndex = 0;

    int emptySquare = 0;
    for (Square square = A8; square <= H1; square++) {
        Piece piece = Board_pieceAtIndex(position.board, square);
        char pieceChar = '\0';
        switch (Piece_type(piece)) {
        case PAWN: pieceChar = 'p'; break;
        case KNIGHT: pieceChar = 'n'; break;
        case BISHOP: pieceChar = 'b'; break;
        case ROOK: pieceChar = 'r'; break;
        case QUEEN: pieceChar = 'q'; break;
        case KING: pieceChar = 'k'; break;
        default: break;
        }

        if (Piece_color(piece) == WHITE) {
            // This offset will make the letter uppercase
            pieceChar -= 32;
        }

        if (piece == NO_PIECE) {
            emptySquare++;
        }
        else {
            if (emptySquare != 0) {
                fen[fenIndex++] = '0' + emptySquare;
                emptySquare = 0;
            }
            fen[fenIndex++] = pieceChar;
        }


        if (file(square) == 7 && square != H1) {
            if (emptySquare != 0) {
                fen[fenIndex++] = '0' + emptySquare;
                emptySquare = 0;
            }
            fen[fenIndex++] = '/';
        }
    }
    if (emptySquare != 0) {
        // Writing the remaining empty squares
        fen[fenIndex++] = '0' + emptySquare;
    }

    fen[fenIndex++] = ' ';
    if (position.colorToGo == WHITE) {
        fen[fenIndex++] = 'w';
    }
    else {
        fen[fenIndex++] = 'b';
    }

    fen[fenIndex++] = ' ';
    if (position.castlingPerm & 0b1000) fen[fenIndex++] = 'K';
    if (position.castlingPerm & 0b0100) fen[fenIndex++] = 'Q';
    if (position.castlingPerm & 0b0010) fen[fenIndex++] = 'k';
    if (position.castlingPerm & 0b0001) fen[fenIndex++] = 'q';
    if (!position.castlingPerm) fen[fenIndex++] = '-';

    fen[fenIndex++] = ' ';
    if (position.enPassantTargetSquare == 0) fen[fenIndex++] = '-';
    else {
        fen[fenIndex++] = 'a' + file(position.enPassantTargetSquare);
        fen[fenIndex++] = '1' + (7 - rank(position.enPassantTargetSquare));
    }

    fen[fenIndex++] = ' ';
    // The longest game of chess can have 8849 which has 4 digits
    char digits[4];
    int digitLength = 0;
    int num = position.turnsForFiftyRule;
    do {
        digits[digitLength++] = '0' + (num % 10);
        num /= 10;
    } while (num);
    do {
        fen[fenIndex++] = digits[--digitLength];
    } while (digitLength);


    fen[fenIndex++] = ' ';
    num = position.nbMoves;
    do {
        digits[digitLength++] = '0' + (num % 10);
        num /= 10;
    } while (num);
    do {
        fen[fenIndex++] = digits[--digitLength];
    } while (digitLength);

    fen[fenIndex] = '\0';
    return fenIndex;
}
