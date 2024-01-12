// This file is a port to C of this file from Sebastian Lague
// https://github.com/SebLague/Chess-AI/blob/main/Assets/Scripts/Core/MoveGenerator.cs
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "MoveGenerator.h"
#include "magicBitBoard/MagicBitBoard.h"

/**
 * Macro that you use to append items to your dynamic array.
 * Copy and pasted from https://github.com/tsoding/ded/blob/master/src/common.h#L45
*/
#define DA_INIT_CAP 256
#define da_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->count >= (da)->capacity) {                                         \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                       \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while (0)

/**
* This is a dynamic array of integers. You can use the 
* da_append macro to append integer to this list
*/
typedef struct intList {
    int* items;
    size_t count;
    size_t capacity;
} IntList;

GameState currentState;

Move* validMoves;
int currentMoveIndex;


int opponentColor;
int friendlyKingIndex;

// For O(1) .contains call
// TODO: Convert these bool* arrays to u64
bool* attackedSquares;
bool* doubleAttackedSquares;
bool* protectKingSquares;

// So that we don't need to recalculate the piece potential valid moves
IntList** isPieceAtLocationPinned;

bool inDoubleCheck;
bool inCheck;
bool enPassantWillRemoveTheCheck;

u64 checkBitBoard;
u64 pinMasks[BOARD_SIZE];

void init() {
    opponentColor = currentState.colorToGo == WHITE ? BLACK : WHITE;
    friendlyKingIndex = -1;
    inCheck = false;
    inDoubleCheck = false;
    enPassantWillRemoveTheCheck = false;
    attackedSquares = malloc(sizeof(bool) * BOARD_SIZE);
    doubleAttackedSquares = malloc(sizeof(bool) * BOARD_SIZE);
    protectKingSquares = malloc(sizeof(bool) * BOARD_SIZE);
    isPieceAtLocationPinned = malloc(sizeof(IntList*) * BOARD_SIZE);

    memset(attackedSquares, false, sizeof(bool) * BOARD_SIZE);
    memset(doubleAttackedSquares, false, sizeof(bool) * BOARD_SIZE);
    memset(protectKingSquares, false, sizeof(bool) * BOARD_SIZE);
    for (int i = 0; i < BOARD_SIZE; i++) {
        isPieceAtLocationPinned[i] = NULL;
    }

    checkBitBoard = ~((u64) 0); // There is no check (for now), so every square is valid, thus every bit is set
    memset(pinMasks, 0xFF, sizeof(u64) * BOARD_SIZE);
}

void resetRayResult(int ray[BOARD_LENGTH]) {
    for (int i = 0; i < BOARD_LENGTH; i++) {
        ray[i] = -1;
    }
}

void appendIntListToAttackedSquare(int src[BOARD_LENGTH]) {
    // Assuming that the items of the src are in the range of the index of the dest
    for (int i = 0; i < BOARD_LENGTH; i++) {
        int square = src[i];
        if (square == -1) { continue; }
        if (attackedSquares[square]) {
            doubleAttackedSquares[square] = true; // To find double checks
        }
        attackedSquares[square] = true;
    }
}

void appendMove(int startSquare, int targetSquare, int flag) {
    Move move = startSquare;
    move |= (targetSquare << 6);
    move |= (flag << 12);
    validMoves[currentMoveIndex] = move;
    (currentMoveIndex)++;
}

void getTopRightNetRay(int currentIndex, int colorToNotInclude, int result[BOARD_LENGTH]) {
    if (currentIndex % 8 == 7) return;
    int count = 0;
    for (int i = currentIndex - 7; i >= 0; i -= 7) {
        result[count++] = i;
        if (i % 8 == 7 || pieceAtIndex(currentState.board, i) != NOPIECE) {
            if (colorToNotInclude != 0 && (pieceColor(pieceAtIndex(currentState.board, i))) == colorToNotInclude) {
                result[count - 1] = -1; // This is remove the last item of the array
            }
            return; // We are at the right end of the board
        }
    }
    return;
}

void getTopLeftNetRay(int currentIndex, int colorToNotInclude, int result[BOARD_LENGTH]) {
    if (currentIndex % 8 == 0) return; 
    int count = 0;
    for (int i = currentIndex - 9; i >= 0; i -= 9) {
        result[count++] = i;
        if (i % 8 == 0 || pieceAtIndex(currentState.board, i) != NOPIECE) {
            if (colorToNotInclude != 0 && pieceColor(pieceAtIndex(currentState.board, i)) == colorToNotInclude) {
                result[count - 1] = -1; // This is remove the last item of the array
            }
            break; // We are at the left end of the board or hit an ennemy piece
        }
    }
    return;
}

void getBottomLeftNetRay(int currentIndex, int colorToNotInclude, int result[BOARD_LENGTH]) {
    if (currentIndex % 8 == 0) return;
    int count = 0;
    for (int i = currentIndex + 7; i <= 63; i += 7) {
        result[count++] = i;
        if (i % 8 == 0 || pieceAtIndex(currentState.board, i) != NOPIECE) {
            if (colorToNotInclude != 0 && pieceColor(pieceAtIndex(currentState.board, i)) == colorToNotInclude) {
                result[count - 1] = -1; // This is remove the last item of the array
            }
            break; // We are at the left end of the board or hit an ennemy piece
        }
    }
    return;
}

void getBottomRightNetRay(int currentIndex, int colorToNotInclude, int result[BOARD_LENGTH]) {
    if (currentIndex % 8 == 7) return;
    int count = 0;
    for (int i = currentIndex + 9; i <= 63; i += 9) {
        result[count++] = i;
        if (i % 8 == 7 || pieceAtIndex(currentState.board, i) != NOPIECE) {
            if (colorToNotInclude != 0 && pieceColor(pieceAtIndex(currentState.board, i)) == colorToNotInclude) {
                result[count - 1] = -1; // This is remove the last item of the array
            }
            break; // We are at the right end of the board of hit an ennemy piece
        }
    }
    return;
}

void getTopNetRay(int currentIndex, int colorToNotInclude, int result[BOARD_LENGTH]) {
    if (currentIndex / 8 == 0) return;
    int count = 0;
    for (int i = currentIndex - 8; i >= 0; i -= 8) {
        result[count++] = i;
        if (i / 8 == 0 || pieceAtIndex(currentState.board, i) != NOPIECE) {
            if (colorToNotInclude != 0 && pieceColor(pieceAtIndex(currentState.board, i)) == colorToNotInclude) {
                result[count - 1] = -1; // This is remove the last item of the array
            }    
            break; // We are at the beginning row of the board of hit an ennemy piece
        }
    }
    return;
}

void getBottomNetRay(int currentIndex, int colorToNotInclude, int result[BOARD_LENGTH]) {
    if (currentIndex / 8 == 7) return;
    int count = 0;
    for (int i = currentIndex + 8; i <= 63; i += 8) {
        result[count++] = i;
        if (i / 8 == 7 || pieceAtIndex(currentState.board, i) != NOPIECE) {
            if (colorToNotInclude != 0 && pieceColor(pieceAtIndex(currentState.board, i)) == colorToNotInclude) {
                result[count - 1] = -1; // This is remove the last item of the array
            }  
            break; // We are at the end row of the board of hit an ennemy piece
        }
    }
    return;
}

void getRightNetRay(int currentIndex, int colorToNotInclude, int result[BOARD_LENGTH]) {
    if (currentIndex % 8 == 7) return;
    int count = 0;
    for (int i = currentIndex + 1; i <= 63; i++) {
        result[count++] = i;
        if (i % 8 == 7 || pieceAtIndex(currentState.board, i) != NOPIECE) {
            if (colorToNotInclude != 0 && pieceColor(pieceAtIndex(currentState.board, i)) == colorToNotInclude) {
                result[count - 1] = -1; // This is remove the last item of the array
            }  
            break; // We are at the right end of the board of hit an ennemy piece
        }
    }
    return;
}

void getLeftNetRay(int currentIndex, int colorToNotInclude, int result[BOARD_LENGTH]) {
    if (currentIndex % 8 == 0) return;
    int count = 0;
    for (int i = currentIndex - 1; i >= 0; i--) {
        result[count++] = i;
        if (i % 8 == 0 || pieceAtIndex(currentState.board, i) != NOPIECE) {
            if (colorToNotInclude != 0 && pieceColor(pieceAtIndex(currentState.board, i)) == colorToNotInclude) {
                result[count - 1] = -1; // This is remove the last item of the array
            }  
            break; // We are at the left end of the board of hit an ennemy piece
        }
    }
    return;
}

bool isIndexValidForKnight(int currentIndex, int targetSquare, bool goDownOne) {
    int y = targetSquare / 8;
    bool wrappingCondition = (goDownOne) ? 
        abs(currentIndex / 8 - y) == 1 : 
        abs(currentIndex / 8 - y) == 2;
    return targetSquare >= 0 && targetSquare <= 63 && wrappingCondition;
}

int* getKnightMoves(int currentIndex, int result[BOARD_LENGTH]) {
    int counter = 0;
    if (isIndexValidForKnight(currentIndex, currentIndex - 6 , true )) result[counter++] = currentIndex - 6 ;
    if (isIndexValidForKnight(currentIndex, currentIndex - 10, true )) result[counter++] = currentIndex - 10;
    if (isIndexValidForKnight(currentIndex, currentIndex - 15, false)) result[counter++] = currentIndex - 15;
    if (isIndexValidForKnight(currentIndex, currentIndex - 17, false)) result[counter++] = currentIndex - 17;
    if (isIndexValidForKnight(currentIndex, currentIndex + 6 , true )) result[counter++] = currentIndex + 6 ;
    if (isIndexValidForKnight(currentIndex, currentIndex + 10, true )) result[counter++] = currentIndex + 10;
    if (isIndexValidForKnight(currentIndex, currentIndex + 15, false)) result[counter++] = currentIndex + 15;
    if (isIndexValidForKnight(currentIndex, currentIndex + 17, false)) result[counter++] = currentIndex + 17;
} 

int* getPawnAttackingSquares(int currentIndex, int color, int result[2]) {
    int counter = 0;
    int forwardIndex = color == WHITE ? currentIndex - 8 : currentIndex + 8;
    if (forwardIndex < 0 || forwardIndex > 63) { return result; }
    if (forwardIndex % 8 != 7) { result[counter++] = forwardIndex + 1; }
    if (forwardIndex % 8 != 0) { result[counter++] = forwardIndex - 1; }
    return result;
}

void getKingMoves(int currentIndex, int result[BOARD_LENGTH]) {
    int counter = 0;
    if (currentIndex % 8 != 0 && currentIndex / 8 != 0) { result[counter++] = currentIndex - 9; }
    if (currentIndex / 8 != 0                         ) { result[counter++] = currentIndex - 8; }
    if (currentIndex % 8 != 7 && currentIndex / 8 != 0) { result[counter++] = currentIndex - 7; }
    if (currentIndex % 8 != 0                         ) { result[counter++] = currentIndex - 1; }
    if (currentIndex % 8 != 7                         ) { result[counter++] = currentIndex + 1; }
    if (currentIndex % 8 != 0 && currentIndex / 8 != 7) { result[counter++] = currentIndex + 7; }
    if (currentIndex / 8 != 7                         ) { result[counter++] = currentIndex + 8; }
    if (currentIndex % 8 != 7 && currentIndex / 8 != 7) { result[counter++] = currentIndex + 9; }
}

bool isIndexPseudoLegalForKnight(int currentIndex, int targetSquare, bool goDownOne) {
    int y = targetSquare / 8;
    bool wrappingCondition = (goDownOne) ? 
        abs(currentIndex / 8 - y) == 1 : 
        abs(currentIndex / 8 - y) == 2;
    if (targetSquare < 0 || targetSquare > 63 || !wrappingCondition) return false;
    return pieceColor(pieceAtIndex(currentState.board, targetSquare)) != currentState.colorToGo;
}

u64 getKnightPseudoLegalMovesBitBoard(int currentIndex) {
    u64 result = (u64) 0;
    u64 toggle = (u64) 1;
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex - 6 , true )) result ^= toggle << (currentIndex - 6 );
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex - 15, false)) result ^= toggle << (currentIndex - 15);
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex - 10, true )) result ^= toggle << (currentIndex - 10);
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex - 17, false)) result ^= toggle << (currentIndex - 17);
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex + 6 , true )) result ^= toggle << (currentIndex + 6 );
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex + 10, true )) result ^= toggle << (currentIndex + 10);
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex + 15, false)) result ^= toggle << (currentIndex + 15);
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex + 17, false)) result ^= toggle << (currentIndex + 17);
    return result;
}

void getKnightPseudoLegalMoves(int currentIndex, int result[BOARD_LENGTH]) {
    int counter = 0;
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex - 6 , true )) result[counter++] = currentIndex - 6 ;
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex - 15, false)) result[counter++] = currentIndex - 15;
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex - 10, true )) result[counter++] = currentIndex - 10;
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex - 17, false)) result[counter++] = currentIndex - 17;
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex + 6 , true )) result[counter++] = currentIndex + 6 ;
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex + 10, true )) result[counter++] = currentIndex + 10;
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex + 15, false)) result[counter++] = currentIndex + 15;
    if (isIndexPseudoLegalForKnight(currentIndex, currentIndex + 17, false)) result[counter++] = currentIndex + 17;
}

void extendRayIfCheckKing(
    int ray[BOARD_LENGTH], 
    void (*getRay)(int index, int colorNotToInclude, int result[BOARD_LENGTH])) {
    if (ray[0] == 0) return;
    int lastIndex = -1;
    for (int i = 0; i < 8; i++) {
        if (ray[i + 1] == -1) { 
            lastIndex = ray[i];
            break;
        }
    }
    if (lastIndex == -1) return;
    Piece lastIndexPiece = pieceAtIndex(currentState.board, lastIndex);
    if (lastIndexPiece != (currentState.colorToGo | KING)) return;
    // King is in check by this piece and we need to generate the square behind the king
    int newRay[BOARD_LENGTH] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    getRay(lastIndex, 0, newRay);
    appendIntListToAttackedSquare(newRay);
}

// TODO: Use bitboards for the pieces in this function
void calculateAttackSquares() {
    int attackingSquares[BOARD_LENGTH] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    for (int currentIndex = 0; currentIndex < BOARD_SIZE; currentIndex++) {

        const Piece piece = pieceAtIndex(currentState.board, currentIndex);
        if (piece == NOPIECE) continue;
        if (pieceColor(piece) != opponentColor) {
            if (pieceType(piece) == KING) {
                friendlyKingIndex = currentIndex;
            }
            continue;
        }
        switch (pieceType(piece)) {
        // Note: The 0 means that if the ray stops at a piece it will include it, regarless of its color
        case ROOK: 
            getTopNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getTopNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);

            getBottomNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getBottomNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);

            getRightNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getRightNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);

            getLeftNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getLeftNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);
            break;
        
        case BISHOP:
            getTopRightNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getTopRightNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);

            getTopLeftNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getTopLeftNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);

            getBottomRightNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getBottomRightNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);

            getBottomLeftNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getBottomLeftNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);
            break;
        case QUEEN:
            getTopNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getTopNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);
            
            getBottomNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getBottomNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);

            getRightNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getRightNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);

            getLeftNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getLeftNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);

            getTopRightNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getTopRightNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);

            getTopLeftNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getTopLeftNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);

            getBottomRightNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getBottomRightNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);

            getBottomLeftNetRay(currentIndex, 0, attackingSquares);
            extendRayIfCheckKing(attackingSquares, getBottomLeftNetRay);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);
            break;
        case KNIGHT:
            getKnightMoves(currentIndex, attackingSquares);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);
            break;
        case PAWN:
            getPawnAttackingSquares(currentIndex, opponentColor, attackingSquares);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);
            break;
        case KING:
            getKingMoves(currentIndex, attackingSquares);
            appendIntListToAttackedSquare(attackingSquares);
            resetRayResult(attackingSquares);
            break;
        default:
            break;
        }
    }
}

bool isKingIndexLegal(int targetSquare) {
    // King does not land on a square which he will be eaten or does not eat one of his own square
    return !attackedSquares[targetSquare] && 
        pieceColor(pieceAtIndex(currentState.board, targetSquare)) != currentState.colorToGo;
}

int addSlidingPiecePinned(void (*getRay)(int index, int colorNotToInclude, int result[BOARD_LENGTH]), int dangerousSlidingPiece) {
    int rayToFriendlyBlockingPiece[BOARD_LENGTH] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    getRay(friendlyKingIndex, 0, rayToFriendlyBlockingPiece);
    if (rayToFriendlyBlockingPiece[0] == -1) { 
        return -1; 
    }

    int firstLastSquare = -1; 
    for (int i = 0; i < BOARD_LENGTH; i++) {
        if (rayToFriendlyBlockingPiece[i + 1] == -1) {
            firstLastSquare = rayToFriendlyBlockingPiece[i];
            break;
        }
    }
    int firstLastSquareContent = pieceAtIndex(currentState.board, firstLastSquare);
    if (firstLastSquareContent == NOPIECE) { 
        return -1; 
    }

    if (pieceColor(firstLastSquareContent) == currentState.colorToGo) {
        // This piece may be pinned
        int rayFromFriendlyBlockingToDangerousPiece[BOARD_LENGTH] = { -1, -1, -1, -1, -1, -1, -1, -1 };
        getRay(firstLastSquare, 0, rayFromFriendlyBlockingToDangerousPiece);
        if (rayFromFriendlyBlockingToDangerousPiece[0] == -1) { 
            return -1; 
        }
        int secondLastSquare = -1; 
            for (int i = 0; i < BOARD_LENGTH; i++) {
                if (rayFromFriendlyBlockingToDangerousPiece[i + 1] == -1) {
                    secondLastSquare = rayFromFriendlyBlockingToDangerousPiece[i];
                    break;
                }
            }

        int potentialDangerousPiece = pieceAtIndex(currentState.board, secondLastSquare);
        if (potentialDangerousPiece == dangerousSlidingPiece ||
            potentialDangerousPiece == makePiece(opponentColor, QUEEN)) {
            // The piece is pinned!
            IntList* potentialValidMoves = (IntList*) malloc(sizeof(IntList));
            potentialValidMoves->capacity = 8;
            potentialValidMoves->count = 0;
            potentialValidMoves->items = malloc(sizeof(int) * 8);

            // Creating the pin mask for this square
            // Appending the first ray and second ray to potential valid moves
            u64 pinMaskForSquare = (u64) 0;
            u64 toggle = (u64) 1;
            for (int i = 0; i < BOARD_LENGTH; i++) {
                if (rayToFriendlyBlockingPiece[i] != -1) {
                    pinMaskForSquare |= (toggle << rayToFriendlyBlockingPiece[i]);
                    da_append(potentialValidMoves, rayToFriendlyBlockingPiece[i]);
                }
            }
            for (int i = 0; i < BOARD_LENGTH; i++) {
                if (rayFromFriendlyBlockingToDangerousPiece[i] != -1) {
                    pinMaskForSquare |= (toggle << rayFromFriendlyBlockingToDangerousPiece[i]);
                    da_append(potentialValidMoves, rayFromFriendlyBlockingToDangerousPiece[i]);
                }
            }
            pinMasks[firstLastSquare] = pinMaskForSquare;
            isPieceAtLocationPinned[firstLastSquare] = potentialValidMoves;
        } 
    } else {
        // Piece is opponent color
        if (firstLastSquareContent == dangerousSlidingPiece ||
            firstLastSquareContent == (opponentColor | QUEEN)) {
            // The piece is checking the king
            return firstLastSquare;
        }  
    }
    return -1;
}

void appendIntListToBoolList(int src[BOARD_LENGTH], bool* dest) {
    // Assuming that the items of the src are in the range of the index of the dest
    for (int i = 0; i < BOARD_LENGTH; i++) {
        if (src[i] != -1) {
            dest[src[i]] = true;
        }
    }
}

void generateCastle() {
    const int defaultKingIndex = currentState.colorToGo == WHITE ? 60 : 4;
    if (friendlyKingIndex != defaultKingIndex || inCheck) { return; }

    const int castlingBits = currentState.colorToGo == WHITE ? currentState.castlingPerm >> 2 : currentState. castlingPerm & 0b11;
    if (castlingBits >> 1) { // Can castle king side
        // Trusting the caller that the the king nor the rook has moved
        // Note the first check is redundant, if we assume 
        // that the caller has done some proper checks to set the castling perm, which I do not
        const int rookIndex = friendlyKingIndex + 3;
        if (pieceAtIndex(currentState.board, rookIndex) == (currentState.colorToGo | ROOK) &&
            !attackedSquares[friendlyKingIndex + 1] &&
            !attackedSquares[friendlyKingIndex + 2] &&
            pieceAtIndex(currentState.board, friendlyKingIndex + 1) == NOPIECE &&
            pieceAtIndex(currentState.board, friendlyKingIndex + 2) == NOPIECE) {
            // Castling is valid
            appendMove(friendlyKingIndex, friendlyKingIndex + 2, KING_SIDE_CASTLING);
        }
    }
    if (castlingBits & 1) { // Can castle queen side
        const int rookIndex = friendlyKingIndex - 4;
        // Trusting the caller that the the king nor the rook has moved
        // Note the first check is redundant, if we assume 
        // that the caller has done some proper checks to set the castling perm, which I do not
        if (pieceAtIndex(currentState.board, rookIndex) == (currentState.colorToGo | ROOK) &&
            !attackedSquares[friendlyKingIndex - 1] &&
            !attackedSquares[friendlyKingIndex - 2] && 
            // The rook can pass trough an attacked square, which means that we don't need to check for the third square
            pieceAtIndex(currentState.board, friendlyKingIndex - 1) == NOPIECE &&
            pieceAtIndex(currentState.board, friendlyKingIndex - 2) == NOPIECE && 
            pieceAtIndex(currentState.board, friendlyKingIndex - 3) == NOPIECE) {
            // Castling is valid
            appendMove(friendlyKingIndex, friendlyKingIndex - 2, QUEEN_SIDE_CASTLING);
        }
    }
}

void generateKingMoves() {
    if (attackedSquares[friendlyKingIndex]) { inCheck = true; }
    if (doubleAttackedSquares[friendlyKingIndex]) { inDoubleCheck = true; }

    int pseudoLegalMoves[BOARD_LENGTH] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    getKingMoves(friendlyKingIndex, pseudoLegalMoves);
    for (int i = 0; i < 8; i++) {
        const int targetSquare = pseudoLegalMoves[i];
        if (targetSquare != -1 && isKingIndexLegal(targetSquare)) {
            appendMove(friendlyKingIndex, targetSquare, NOFlAG);
        }
    }

    generateCastle();
    // Pinning own color pieces    
    // Checking for attacking sliding pieces
    int checkingPieceIndex = -1;
    void (*checkingPieceDirection)(int index, int colorNotToInclude, int result[BOARD_LENGTH]) = NULL;
    // Note: Could refactor all the function pointer into an array and iterate over them
    // Unfortunatly, I have better things to do for now

    if (addSlidingPiecePinned(getTopNetRay, opponentColor | ROOK) != -1)
        checkingPieceDirection = getTopNetRay; 
    if (addSlidingPiecePinned(getBottomNetRay, opponentColor | ROOK) != -1) 
        checkingPieceDirection = getBottomNetRay; 
    if (addSlidingPiecePinned(getRightNetRay, opponentColor | ROOK) != -1)
        checkingPieceDirection = getRightNetRay; 
    if (addSlidingPiecePinned(getLeftNetRay, opponentColor | ROOK) != -1) 
        checkingPieceDirection = getLeftNetRay; 
    
    if (addSlidingPiecePinned(getTopRightNetRay, opponentColor | BISHOP) != -1) 
        checkingPieceDirection = getTopRightNetRay; 
    if (addSlidingPiecePinned(getTopLeftNetRay, opponentColor | BISHOP) != -1) 
        checkingPieceDirection = getTopLeftNetRay; 
    if (addSlidingPiecePinned(getBottomRightNetRay, opponentColor | BISHOP) != -1) 
        checkingPieceDirection = getBottomRightNetRay;
    if (addSlidingPiecePinned(getBottomLeftNetRay, opponentColor | BISHOP) != -1) 
        checkingPieceDirection = getBottomLeftNetRay;

    if (!inCheck || inDoubleCheck) { return; } // Either the king is safe or only king moves are valid

    checkBitBoard = (u64) 0; // The king is in check, thus not every square is valid
    u64 toggle = (u64) 1;
    if (checkingPieceDirection != NULL) {
        // A sliding piece is checking the king
        int result[BOARD_LENGTH] = { -1, -1, -1, -1, -1, -1, -1, -1 };
        checkingPieceDirection(friendlyKingIndex, 0, result);

        for (int i = 0; i < BOARD_LENGTH; i++) {
            if (result[i] != -1) {
                checkBitBoard |= toggle << result[i];
            }
        }

        appendIntListToBoolList(result, protectKingSquares);
        return;
    }
    // It is a pawn or a knight who is checking the king
    // The only valid moves for other pieces is to eat the checking piece
    
    // Checking for pawns
    const int delta = currentState.colorToGo == WHITE ? -1 : 1;
    int potentialPawn;
    // For loop is remove copy pasting. I know sometimes I copy paste a lot but I was fed up this time
    for (int i = 0; i < 2; i++) {
        potentialPawn = friendlyKingIndex + (i == 0 ? 7 : 9) * delta;
        if (pieceAtIndex(currentState.board, potentialPawn) == (opponentColor | PAWN)) {
            checkBitBoard |= toggle << potentialPawn;

            // Adding this condition if en-passant were to remove the check
            if (potentialPawn + 8 * delta == currentState.enPassantTargetSquare) {
                // Eating this pawn by en-passant would remove the check
                // I cannot add this to the checkBitBoard else other pieces than pawns would try to do "en-passant"
                enPassantWillRemoveTheCheck = true;
            }

            protectKingSquares[potentialPawn] = true;
            return; // Can return without checking the others since there is no double check
        }
    }
    
    // Checking for knights 
    int potentialKnigths[BOARD_LENGTH] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    getKnightMoves(friendlyKingIndex, potentialKnigths);
    for (int i = 0; i < BOARD_LENGTH; i++) {
        int index = potentialKnigths[i];
        if (index != -1 && pieceAtIndex(currentState.board, index) == (opponentColor | KNIGHT)) {
            checkBitBoard |= toggle << index;
            protectKingSquares[index] = true;
            return; // Can return without checking the others since there is no double check
        }
    }
}

u64 checkEnPassantPinned(int from, u64 bitBoard) {
    if (!bitBoard) {
        // Piece is pinned and it cannot do en-passant
        return bitBoard;
    }
    int enPassantRank = from / 8;
    if ((friendlyKingIndex / 8 ) != enPassantRank) { 
        return bitBoard; // The king is not on the same rank as the from pawn, so oawn cannot be en-passant pinned
    }

    // To determine the correct direction to look for pinned en-passant
    int increment = friendlyKingIndex > from ? -1 : 1;

    int currentIndex;
    int enPassantIndex = opponentColor == BLACK ? currentState.enPassantTargetSquare + 8 : currentState.enPassantTargetSquare - 8;
    if (friendlyKingIndex > from) { // This condition is true is the position: 8/8/8/r1pP1K2/8/8/8/8 w
        currentIndex = from < enPassantIndex ? from : enPassantIndex;
    } else { // This else is true is the position 8/8/8/K1Pp1r2/8/8/8/8 w
        currentIndex = from > enPassantIndex ? from : enPassantIndex;
    }

    PieceCharacteristics threateningPiece;
    do {
        currentIndex += increment;
        threateningPiece = pieceAtIndex(currentState.board, currentIndex);
    } while (threateningPiece == NOPIECE && currentIndex / 8 == enPassantRank);
    
    if (threateningPiece == makePiece(opponentColor, ROOK) || 
        threateningPiece == makePiece(opponentColor, QUEEN)) {
            return (u64) 0;
    } else {
        return bitBoard;
    }
}

void generateEnPassant(int from) {
    int difference = currentState.colorToGo == WHITE ? from - currentState.enPassantTargetSquare : currentState.enPassantTargetSquare - from;
    if ((difference != 7) && (difference != 9)) { return; }

    // Note that canEnPasant is never 0, because so can only en passant on the third or fourth rank
    u64 toggle = ((u64) 1) << currentState.enPassantTargetSquare;
    u64 canEnPassant = toggle;
    
    // Accounting for pins
    canEnPassant &= pinMasks[from];

    // Need to account for enPassantWillRemoveTheCheck if inCheck == true
    if (inCheck) {
        if (!enPassantWillRemoveTheCheck) {
            canEnPassant &= ~toggle; // I need to invert the bits to not undo what the pinmask AND did
        }
    } else {
        canEnPassant = checkEnPassantPinned(from, canEnPassant);
    }

    if (canEnPassant) {
        appendMove(from, currentState.enPassantTargetSquare, EN_PASSANT);
    }
}

// TODO: Make this function use bitboards
void generatePawnDoublePush(int from, int increment) {
    if (pieceAtIndex(currentState.board, from + increment) != NOPIECE ||
        pieceAtIndex(currentState.board, from + 2 * increment) != NOPIECE) { return; }
    IntList* pinnedLegalMoves = isPieceAtLocationPinned[from];
    if (pinnedLegalMoves != NULL && inCheck) {
        // Pawn is pinned and the king is checked
        return;
    } 
    if (pinnedLegalMoves == NULL && !inCheck) {
        appendMove(from, from + 2 * increment, DOUBLE_PAWN_PUSH);
    } else if (pinnedLegalMoves != NULL && !inCheck) {
        for (int i = 0; i < pinnedLegalMoves->count; i++) {
            if ((from + 2 * increment) == pinnedLegalMoves->items[i]) {
                appendMove(from, from + 2 * increment, DOUBLE_PAWN_PUSH);
                return;
            }
        }
    } else if (pinnedLegalMoves == NULL && inCheck) {
        if (protectKingSquares[from + 2 * increment]) {
            appendMove(from, from + 2 * increment, DOUBLE_PAWN_PUSH);
        }
    }

}

void appendLegalMovesFromPseudoLegalMovesBitBoard(int from, u64 pseudoLegalMoves) {
    // Accounting for pins
    pseudoLegalMoves &= pinMasks[from];

    // Accounting for checks
    pseudoLegalMoves &= checkBitBoard;

    // Turning the bitboard into our move objects
    while (pseudoLegalMoves) {
        // Extract the position of the least significant bit
        int to = trailingZeros_64(pseudoLegalMoves);
        
        appendMove(from, to, NOFlAG);
        
        // Clearing the least significant bit to get the position of the next bit
        pseudoLegalMoves &= pseudoLegalMoves - 1;
    }
}

// Function from https://youtu.be/_vqlIPDR2TU?si=J2UVpgrqJQ3gzqCT&t=2314
// I loved Sebastian Lague!
void rookMoves(int from) {
    // Obtaining the blockingBitBoard
    u64 allPieceBB = allPiecesBitBoard(currentState.board);
    u64 blockingBitBoard = (allPieceBB & rookMovementMask[from]);

    // Getting the pseudo legal moves bitboard from the array
    u64 movesBitBoard = getRookPseudoLegalMovesBitBoard(from, blockingBitBoard);

    // Accounting for friendly pieces
    u64 friendlyPieceBitBoard; // The ternary was too big for my liking
    if (currentState.colorToGo == WHITE) {
        friendlyPieceBitBoard = whitePiecesBitBoard(currentState.board);
    } else { 
        friendlyPieceBitBoard = blackPiecesBitBoard(currentState.board);
    }
    movesBitBoard &= ~friendlyPieceBitBoard; // We invert the bitboard so that we do not capture friendly pieces
    
    appendLegalMovesFromPseudoLegalMovesBitBoard(from, movesBitBoard);
}

void bishopMoves(int from) {
    // Obtaining the blockingBitBoard
    u64 allPieceBB = allPiecesBitBoard(currentState.board);
    u64 blockingBitBoard = (allPieceBB & bishopMovementMask[from]);
    // Getting the pseudo legal moves bitboard from the array
    u64 movesBitBoard = getBishopPseudoLegalMovesBitBoard(from, blockingBitBoard);

    // Accounting for friendly pieces
    u64 friendlyPieceBitBoard; // The ternary was too big for my liking
    if (currentState.colorToGo == WHITE) {
        friendlyPieceBitBoard = whitePiecesBitBoard(currentState.board);
    } else { 
        friendlyPieceBitBoard = blackPiecesBitBoard(currentState.board);
    }
    movesBitBoard &= ~friendlyPieceBitBoard; // We invert the bitboard so that we do not capture friendly pieces
    
    appendLegalMovesFromPseudoLegalMovesBitBoard(from, movesBitBoard);
}

void queenMoves(int from) {
    // Obtaining the blockingBitBoard
    u64 allPieceBB = allPiecesBitBoard(currentState.board);
    u64 rookBlockingBitBoard = (allPieceBB & rookMovementMask[from]);
    u64 bishopBlockingBitBoard = (allPieceBB & bishopMovementMask[from]);

    // Getting the pseudo legal moves bitboard from the array
    u64 rookMovesBitBoard = getRookPseudoLegalMovesBitBoard(from, rookBlockingBitBoard);
    u64 bishopMovesBitBoard = getBishopPseudoLegalMovesBitBoard(from, bishopBlockingBitBoard);
    u64 movesBitBoard = rookMovesBitBoard | bishopMovesBitBoard;

    // Accounting for friendly pieces
    u64 friendlyPieceBitBoard; // The ternary was too big for my liking
    if (currentState.colorToGo == WHITE) {
        friendlyPieceBitBoard = whitePiecesBitBoard(currentState.board);
    } else { 
        friendlyPieceBitBoard = blackPiecesBitBoard(currentState.board);
    }
    movesBitBoard &= ~friendlyPieceBitBoard; // We invert the bitboard so that we do not capture friendly pieces
    
    appendLegalMovesFromPseudoLegalMovesBitBoard(from, movesBitBoard);
}

void pawnMoves(int from) {
    bool isPawnBeforePromotion = currentState.colorToGo == WHITE ? 
        from / 8 == 1 : 
        from / 8 == 6;
    bool pawnCanEnPassant = currentState.colorToGo == WHITE ? 
        from / 8 == 3 : 
        from / 8 == 4;
    bool pawnCanDoublePush = currentState.colorToGo == WHITE ? 
        from / 8 == 6 : 
        from / 8 == 1;
    int increment = currentState.colorToGo == WHITE ? -8 : 8;

    u64 pseudoLegalMoves = (u64) 0;
    u64 toggle = (u64) 1;
    
    int forwardIndex = from + increment;

    if (forwardIndex >= 0 && forwardIndex <= 63) {
        if (pieceAtIndex(currentState.board, forwardIndex) == NOPIECE) {
            pseudoLegalMoves ^= toggle << forwardIndex;
        }

        if (forwardIndex % 8 != 7 && pieceColor(pieceAtIndex(currentState.board, forwardIndex + 1)) == opponentColor) {
            pseudoLegalMoves ^= toggle << (forwardIndex + 1);
        }

        if (forwardIndex % 8 != 0 && pieceColor(pieceAtIndex(currentState.board, forwardIndex - 1)) == opponentColor) {
            pseudoLegalMoves ^= toggle << (forwardIndex - 1);
        }
    }

    if (pawnCanEnPassant && currentState.enPassantTargetSquare != -1) { 
        generateEnPassant(from); 
    } else if (pawnCanDoublePush) {
        generatePawnDoublePush(from, increment);
    }

    // Accounting for pins
    pseudoLegalMoves &= pinMasks[from];

    // Accounting for checks
    pseudoLegalMoves &= checkBitBoard;

    // Turning the bitboard into our move objects
    while (pseudoLegalMoves) {
        // Extract the position of the least significant bit
        int to = trailingZeros_64(pseudoLegalMoves);
        
        if (isPawnBeforePromotion) {
            appendMove(from, to, PROMOTE_TO_QUEEN);
            appendMove(from, to, PROMOTE_TO_KNIGHT);
            appendMove(from, to, PROMOTE_TO_ROOK);
            appendMove(from, to, PROMOTE_TO_BISHOP);
        } else {
            appendMove(from, to, NOFlAG);
        }
        
        // Clearing the least significant bit to get the position of the next bit
        pseudoLegalMoves &= pseudoLegalMoves - 1;
    }
}

void generateSupportingPiecesMoves() {
    for (int currentIndex = 0; currentIndex < BOARD_SIZE; currentIndex++) {
        const int piece = pieceAtIndex(currentState.board, currentIndex);
        if (piece == NOPIECE || pieceColor(piece) == opponentColor) { continue; }
        
        u64 pseudoLegalMovesBitBoard;
        switch (pieceType(piece)) {
            case ROOK: 
                rookMoves(currentIndex);
                break;
            case BISHOP:
                bishopMoves(currentIndex);
                break;
            case QUEEN:
                queenMoves(currentIndex);
                break;
            case KNIGHT:
                pseudoLegalMovesBitBoard = getKnightPseudoLegalMovesBitBoard(currentIndex);
                appendLegalMovesFromPseudoLegalMovesBitBoard(currentIndex, pseudoLegalMovesBitBoard);
                break;
            case PAWN:
                pawnMoves(currentIndex);
                break;
            default:
                break;
        }
    }
}

bool compareGameStateForRepetition(const GameState gameStateToCompare) {
    // Comparing boards
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (pieceAtIndex(currentState.board, i) != pieceAtIndex(gameStateToCompare.board, i)) {
            return false;
        }
    }
    return currentState.castlingPerm == gameStateToCompare.castlingPerm &&
        currentState.colorToGo == gameStateToCompare.colorToGo &&
        currentState.enPassantTargetSquare == gameStateToCompare.enPassantTargetSquare;
}

bool isThereThreeFoldRepetition(const GameState* previousStates) {
    if (previousStates == NULL) { 
        return false;
    }
    bool hasOneDuplicate = false;
    int index = 0;
    bool result = false;
    GameState previousState;
    // If colorToGo == 0 then it is not a valid state
    while (true) {

        previousState = previousStates[index];
        if (previousState.colorToGo == 0) {
            break;
        }

        if (compareGameStateForRepetition(previousState)) {
            if (!hasOneDuplicate) {
                hasOneDuplicate = true;
            } else {
                // Already has a duplicate, this is the third repetition
                result = true;
                break;
            }
        }
        index++;
    }
    return result;
}

void noMemoryLeaksPlease() {
    free(attackedSquares);
    free(doubleAttackedSquares);
    free(protectKingSquares);
    for (int i = 0; i < BOARD_SIZE; i++) {
        IntList* piecePinnedAtLocation = isPieceAtLocationPinned[i];
        if (piecePinnedAtLocation != NULL) {
            free(piecePinnedAtLocation->items);
            free(piecePinnedAtLocation);
        }
    }
    free(isPieceAtLocationPinned);
}

void getValidMoves(Move results[MAX_LEGAL_MOVES + 1], const GameState currentGameState, const GameState* previousStates) {
    currentState = currentGameState;

    validMoves = results;
    currentMoveIndex = 0;
    
    if (isThereThreeFoldRepetition(previousStates) || (currentState.turnsForFiftyRule >= 50)) {
        appendMove(0, 0, DRAW); // This is the `draw` move

        // We assume that the array is 0 initialized, so we do not need to add a 0 entry
        return; 
    }

    init();
    calculateAttackSquares();
    generateKingMoves();

    if (inDoubleCheck) { 
        // Only king moves are valid when in double check
        if (currentMoveIndex == 0) { // Is there any king moves?
            // A pretty cool double checkmate
            appendMove(0, 0, CHECKMATE); // This is the `checkmate` move
        }

        noMemoryLeaksPlease();
        // We assume that the array is 0 initialized, so we do not need to add a 0 entry
        return;
    }
    
    generateSupportingPiecesMoves();
    
    if (currentMoveIndex == 0) {
        // There is no valid move
        if (inCheck) {
            appendMove(0, 0, CHECKMATE); // This is the `checkmate` move
        } else {
            appendMove(0, 0, STALEMATE); // This is the `stalemate` move
        }
    }
    noMemoryLeaksPlease();
    // We assume that the array is 0 initialized, so we do not need to add a 0 entry
}