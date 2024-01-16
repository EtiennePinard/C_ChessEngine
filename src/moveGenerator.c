// This file is heavily inspired by this file from Sebastian Lague
// https://github.com/SebLague/Chess-Coding-Adventure/blob/Chess-V2-UCI/Chess-Coding-Adventure/src/Core/Move%20Generation/MoveGenerator.cs
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "MoveGenerator.h"
#include "magicBitBoard/MagicBitBoard.h"

GameState currentState;

Move* validMoves;
int currentMoveIndex;

PieceCharacteristics opponentColor;
int friendlyKingIndex;

// For O(1) .contains call
bool attackedSquares[BOARD_SIZE];

bool inDoubleCheck;
bool inCheck;
bool enPassantWillRemoveTheCheck;

u64 checkBitBoard;
u64 pinMasks[BOARD_SIZE];

void init() {
    opponentColor = currentState.colorToGo == WHITE ? BLACK : WHITE;
    friendlyKingIndex = trailingZeros_64(bitBoardForPiece(currentState.board, makePiece(currentState.colorToGo, KING)));
    inCheck = false;
    inDoubleCheck = false;
    enPassantWillRemoveTheCheck = false;

    memset(attackedSquares, false, sizeof(bool) * BOARD_SIZE);

    checkBitBoard = ~((u64) 0); // There is no check (for now), so every square is valid, thus every bit is set
    memset(pinMasks, 0xFF, sizeof(u64) * BOARD_SIZE);
}

void appendMove(int startSquare, int targetSquare, int flag) {
    Move move = startSquare;
    move |= (targetSquare << 6);
    move |= (flag << 12);
    validMoves[currentMoveIndex] = move;
    (currentMoveIndex)++;
}

bool isIndexValidForKnight(int currentIndex, int targetSquare, bool goDownOne) {
    int y = targetSquare / 8;
    bool wrappingCondition = (goDownOne) ? 
        abs(currentIndex / 8 - y) == 1 : 
        abs(currentIndex / 8 - y) == 2;
    return targetSquare >= 0 && targetSquare <= 63 && wrappingCondition;
}

bool isIndexPseudoLegalForKnight(int currentIndex, int targetSquare, bool goDownOne) {
    return isIndexValidForKnight(currentIndex, targetSquare, goDownOne) && pieceColor(pieceAtIndex(currentState.board, targetSquare)) != currentState.colorToGo;
}

u64 getKnightValidMovesBitBoard(int currentIndex) {
    u64 result = (u64) 0;
    u64 toggle = (u64) 1;
    if (isIndexValidForKnight(currentIndex, currentIndex - 6 , true )) result ^= toggle << (currentIndex - 6 );
    if (isIndexValidForKnight(currentIndex, currentIndex - 15, false)) result ^= toggle << (currentIndex - 15);
    if (isIndexValidForKnight(currentIndex, currentIndex - 10, true )) result ^= toggle << (currentIndex - 10);
    if (isIndexValidForKnight(currentIndex, currentIndex - 17, false)) result ^= toggle << (currentIndex - 17);
    if (isIndexValidForKnight(currentIndex, currentIndex + 6 , true )) result ^= toggle << (currentIndex + 6 );
    if (isIndexValidForKnight(currentIndex, currentIndex + 10, true )) result ^= toggle << (currentIndex + 10);
    if (isIndexValidForKnight(currentIndex, currentIndex + 15, false)) result ^= toggle << (currentIndex + 15);
    if (isIndexValidForKnight(currentIndex, currentIndex + 17, false)) result ^= toggle << (currentIndex + 17);
    return result;
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

u64 getKingMoves(int currentIndex) {
    u64 result = (u64) 0;
    u64 toggle = (u64) 1;
    if (currentIndex % 8 != 0 && currentIndex / 8 != 0) { result ^= toggle << (currentIndex - 9); }
    if (currentIndex / 8 != 0                         ) { result ^= toggle << (currentIndex - 8); }
    if (currentIndex % 8 != 7 && currentIndex / 8 != 0) { result ^= toggle << (currentIndex - 7); }
    if (currentIndex % 8 != 0                         ) { result ^= toggle << (currentIndex - 1); }
    if (currentIndex % 8 != 7                         ) { result ^= toggle << (currentIndex + 1); }
    if (currentIndex % 8 != 0 && currentIndex / 8 != 7) { result ^= toggle << (currentIndex + 7); }
    if (currentIndex / 8 != 7                         ) { result ^= toggle << (currentIndex + 8); }
    if (currentIndex % 8 != 7 && currentIndex / 8 != 7) { result ^= toggle << (currentIndex + 9); }
    return result;
}

void addBitBoardToAttackedSquares(u64 bitBoard) {
    // Turning the bitboard into our move objects
    while (bitBoard) {
        // Extract the position of the least significant bit
        int to = trailingZeros_64(bitBoard);
        if (attackedSquares[to] && to == friendlyKingIndex) {
            inDoubleCheck = true;
        }
        attackedSquares[to] = true;
        
        // Clearing the least significant bit to get the position of the next bit
        bitBoard &= bitBoard - 1;
    }
}

void rookAttackedSquares(int from) {
    // Obtaining the blockingBitBoard
    u64 friendlyKingBitBoard = bitBoardForPiece(currentState.board, makePiece(currentState.colorToGo, KING));
    u64 allPieceNoFriendlyKingBB = allPiecesBitBoard(currentState.board) ^ friendlyKingBitBoard;
    u64 blockingBitBoard = (allPieceNoFriendlyKingBB & rookMovementMask[from]);
    // Getting the attacked bitboard, which will go pass the king if it checks him
    // Also note that this move bit boards hits friendly pieces, which is what we need
    u64 attackedBitBoard = getRookPseudoLegalMovesBitBoard(from, blockingBitBoard);
    addBitBoardToAttackedSquares(attackedBitBoard);
}

void bishopAttackedSquares(int from) {
    // Obtaining the blockingBitBoard
    u64 friendlyKingBitBoard = bitBoardForPiece(currentState.board, makePiece(currentState.colorToGo, KING));
    u64 allPieceNoFriendlyKingBB = allPiecesBitBoard(currentState.board) ^ friendlyKingBitBoard;
    u64 blockingBitBoard = (allPieceNoFriendlyKingBB & bishopMovementMask[from]);
    // Getting the attacked bitboard, which will go pass the king if it checks him
    // Also note that this move bit boards hits friendly pieces, which is what we need
    u64 attackedBitBoard = getBishopPseudoLegalMovesBitBoard(from, blockingBitBoard);
    addBitBoardToAttackedSquares(attackedBitBoard);
}

void queenAttackedSquares(int from) {
    // Obtaining the blockingBitBoard
    u64 friendlyKingBitBoard = bitBoardForPiece(currentState.board, makePiece(currentState.colorToGo, KING));
    u64 allPieceNoFriendlyKingBB = allPiecesBitBoard(currentState.board) ^ friendlyKingBitBoard;
    u64 rookBlockingBitBoard = (allPieceNoFriendlyKingBB & rookMovementMask[from]);
    u64 bishopBlockingBitBoard = (allPieceNoFriendlyKingBB & bishopMovementMask[from]);
    // Getting the attacked bitboard, which will go pass the king if it checks him
    // Also note that this move bit boards hits friendly pieces, which is what we need
    u64 rookAttackedBitBoard = getRookPseudoLegalMovesBitBoard(from, rookBlockingBitBoard);
    u64 bishopAttackedBitBoard = getBishopPseudoLegalMovesBitBoard(from, bishopBlockingBitBoard);
    u64 attackedBitBoard = rookAttackedBitBoard | bishopAttackedBitBoard;
    addBitBoardToAttackedSquares(attackedBitBoard);
}

void pawnAttackingSquares(int from) {
    int forwardIndex = opponentColor == WHITE ? from - 8 : from + 8;
    // The forward index is always valid because no pawn can be on the first or last rank (they get promoted)
    if (forwardIndex % 8 != 7) {
        int square = forwardIndex + 1;
        if (attackedSquares[square] && square == friendlyKingIndex) {
            inDoubleCheck = true;
        }
        attackedSquares[square] = true;
    }
    if (forwardIndex % 8 != 0) { 
        int square = forwardIndex - 1; 
        if (attackedSquares[square] && square == friendlyKingIndex) {
            inDoubleCheck = true;
        }
        attackedSquares[square] = true;
    }
}

void calculateAttackSquares() {
    u64 bitBoard;
    for (int currentIndex = 0; currentIndex < BOARD_SIZE; currentIndex++) {
        const Piece piece = pieceAtIndex(currentState.board, currentIndex);
        if (piece == NOPIECE || pieceColor(piece) != opponentColor) continue;
        switch (pieceType(piece)) {
        case ROOK: 
            rookAttackedSquares(currentIndex);
            break;
        case BISHOP:
            bishopAttackedSquares(currentIndex);
            break;
        case QUEEN:
            queenAttackedSquares(currentIndex);
            break;
        case KNIGHT:
            bitBoard = getKnightValidMovesBitBoard(currentIndex);
            addBitBoardToAttackedSquares(bitBoard);
            break;
        case PAWN:
            pawnAttackingSquares(currentIndex);
            break;
        case KING:
            bitBoard = getKingMoves(currentIndex);
            addBitBoardToAttackedSquares(bitBoard);
            break;
        default:
            break;
        }
    }
}

bool isKingIndexLegal(int targetSquare) {
    // King does not land on a square which he will be eaten or does not eat one of his own square
    return !attackedSquares[targetSquare] && pieceColor(pieceAtIndex(currentState.board, targetSquare)) != currentState.colorToGo;
}

// I am aware and I do not like the 6 deep indentation in this function. Will refactor later (lol)
void handlePinAndCheckForDirection(int increment, u64 directionMask, PieceCharacteristics dangerousSlidingPiece) {
    // Note that we are guaranteed to not be at a square where adding the increment would give an invalid square in the direction
    Piece firstPiece = NOPIECE;
    int firstPieceIndex;
    Piece secondPiece = NOPIECE;

    u64 toggle = (u64) 1;
    u64 rayMask = (u64) 0;
    bool hitEdge = false;
    int currentIndex = friendlyKingIndex + increment;

    // I need to do the hitEdge shenanigans to avoid to pass a "predicate" in the arguments
    // The last valid square in each direction is not set in the directionMask
    // We can then know that we are on the edge of the direction when we hit that unset bit
    // We then set hitEdge to true and do one last pass, which we know is valid
    // If we did not have this feature, we could potentialy include invalid squares for the direction that we are handling
    while (true) {
        if (((directionMask >> currentIndex) & 1) == 0) {
            hitEdge = true;
        }

        rayMask ^= toggle << currentIndex;

        if (firstPiece == NOPIECE) {
            if (pieceAtIndex(currentState.board, currentIndex) != NOPIECE) {
                firstPiece = pieceAtIndex(currentState.board, currentIndex);
                firstPieceIndex = currentIndex;

                if (pieceColor(firstPiece) == opponentColor) {
                    if (pieceType(firstPiece) == dangerousSlidingPiece || pieceType(firstPiece) == QUEEN) {
                        // This piece is checking the king
                        checkBitBoard |= rayMask;
                    }

                    break;
                }
            }
        } else if (secondPiece == NOPIECE) {

            if (pieceAtIndex(currentState.board, currentIndex) != NOPIECE) {
                secondPiece = pieceAtIndex(currentState.board, currentIndex);

                if (secondPiece == makePiece(opponentColor, dangerousSlidingPiece) || secondPiece == makePiece(opponentColor, QUEEN)) {
                    // The first piece is pinned!
                    // Creating the pin mask for this square
                    pinMasks[firstPieceIndex] = rayMask;
                }
                break;
            }
        }

        if (hitEdge) {
            break;
        }
        currentIndex += increment;
    }
}

void handlePinsAndChecksFromSlidingPieces() {
    int kingX = friendlyKingIndex % 8;
    int kingY = friendlyKingIndex / 8;
    // Doing another implementation of this function using precomputed bitboards
    // Here a "hack" is used to get a bitboard that has the "edge" bits set
    u64 orthogonalMask = rookMovementMask[friendlyKingIndex];
    if (kingX < 7) handlePinAndCheckForDirection(1, orthogonalMask, ROOK);
    if (kingX > 0) handlePinAndCheckForDirection(-1, orthogonalMask, ROOK);
    if (kingY < 7) handlePinAndCheckForDirection(8, orthogonalMask, ROOK);
    if (kingY > 0) handlePinAndCheckForDirection(-8, orthogonalMask, ROOK);
    
    u64 diagonalMask = bishopMovementMask[friendlyKingIndex];
    if (kingX < 7 && kingY < 7) handlePinAndCheckForDirection(9, diagonalMask, BISHOP);
    if (kingX > 0 && kingY > 0) handlePinAndCheckForDirection(-9, diagonalMask, BISHOP);
    if (kingX > 0 && kingY < 7) handlePinAndCheckForDirection(7, diagonalMask, BISHOP);
    if (kingX < 7 && kingY > 0) handlePinAndCheckForDirection(-7, diagonalMask, BISHOP);
}

void generateCastle() {
    const int defaultKingIndex = currentState.colorToGo == WHITE ? 60 : 4;
    if (friendlyKingIndex != defaultKingIndex || inCheck) { return; }

    const int castlingBits = currentState.colorToGo == WHITE ? currentState.castlingPerm >> 2 : currentState.castlingPerm & 0b11;
    if (castlingBits >> 1) { // Can castle king side
        const int rookIndex = friendlyKingIndex + 3;
        // Trusting the caller that the the king nor the rook has moved
        // Note the first check is redundant, if we assume 
        // that the caller has done some proper checks to set the castling perm, which I do not
        if (pieceAtIndex(currentState.board, rookIndex) == makePiece(currentState.colorToGo, ROOK) &&
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
        if (pieceAtIndex(currentState.board, rookIndex) == makePiece(currentState.colorToGo, ROOK) &&
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
    if (attackedSquares[friendlyKingIndex]) { inCheck = true; checkBitBoard = (u64) 0; }

    u64 bitboard;
    bitboard = getKingMoves(friendlyKingIndex);
    while (bitboard) {
        int targetSquare = trailingZeros_64(bitboard);
        if (isKingIndexLegal(targetSquare)) {
            appendMove(friendlyKingIndex, targetSquare, NOFlAG);
        }
        bitboard &= bitboard - 1;
    }

    if (inDoubleCheck) { return; } // Only king moves are valid
    
    generateCastle();
    handlePinsAndChecksFromSlidingPieces();

    u64 toggle = (u64) 1;
    // Handling checks from pawns and knights
    const int delta = currentState.colorToGo == WHITE ? -1 : 1;
    int potentialPawn;
    // For loop is remove copy pasting. I know sometimes I copy paste a lot but I was fed up this time
    for (int i = 0; i < 2; i++) {
        potentialPawn = friendlyKingIndex + (i == 0 ? 7 : 9) * delta;
        if (pieceAtIndex(currentState.board, potentialPawn) == makePiece(opponentColor, PAWN)) {
            checkBitBoard |= toggle << potentialPawn;

            // Adding this condition if en-passant were to remove the check
            if (potentialPawn + 8 * delta == currentState.enPassantTargetSquare) {
                // Eating this pawn by en-passant would remove the check
                // I cannot set the en-passant bit in the checkBitBoard else non-pawn pieces would try to do en-passant
                enPassantWillRemoveTheCheck = true;
            }

            return; // Can return without checking the others since there is no double check
        }
    }
    
    // Checking for knights 
    bitboard = getKnightValidMovesBitBoard(friendlyKingIndex);
    while (bitboard) {
        int index = trailingZeros_64(bitboard);
        if (pieceAtIndex(currentState.board, index) == makePiece(opponentColor, KNIGHT)) {
            checkBitBoard |= toggle << index;
            return; // Can return without checking the others since there is no double check
        }
        bitboard &= bitboard - 1;
    }
}

u64 checkEnPassantPinned(int from, u64 bitBoard) {
    if (!bitBoard) {
        // Piece is pinned and it cannot do en-passant
        return bitBoard;
    }
    int enPassantRank = from / 8;
    if ((friendlyKingIndex / 8 )!= enPassantRank) { 
        return bitBoard; // The king is not on the same rank as the from pawn, so oawn cannot be en-passant pinned
    }

    // To determine the correct direction to look for pinned en-passant
    int increment = friendlyKingIndex > from ? -1 : 1;

    // Messy logic to determine the starting index to check if the pawn is en-passant pinned
    int indexToSearchForThreateningPiece;
    int indexToCheckIfNoPieceIsBetweenKingAndPawn;
    int enPassantIndex = opponentColor == BLACK ? currentState.enPassantTargetSquare + 8 : currentState.enPassantTargetSquare - 8;
    if (friendlyKingIndex > from) { // This condition is true is the position: 8/8/8/r1pP1K2/8/8/8/8 w
        if (from < enPassantIndex) {
            indexToSearchForThreateningPiece = from;
            indexToCheckIfNoPieceIsBetweenKingAndPawn = enPassantIndex;
        } else {
            indexToSearchForThreateningPiece = enPassantIndex;
            indexToCheckIfNoPieceIsBetweenKingAndPawn = from;
        }
    } else { // This else is true is the position 8/8/8/K1Pp1r2/8/8/8/8 w
        if (from > enPassantIndex) {
            indexToSearchForThreateningPiece = from;
            indexToCheckIfNoPieceIsBetweenKingAndPawn = enPassantIndex;
        } else {
            indexToSearchForThreateningPiece = enPassantIndex;
            indexToCheckIfNoPieceIsBetweenKingAndPawn = from;
        }
    }

    // Checking that there is not any piece between the pawn trying to do en-passant and king
    indexToCheckIfNoPieceIsBetweenKingAndPawn -= increment; // I do a minus cause I want to go in the opposite direction
    while (indexToCheckIfNoPieceIsBetweenKingAndPawn != friendlyKingIndex) {
        if (pieceAtIndex(currentState.board, indexToCheckIfNoPieceIsBetweenKingAndPawn) == NOPIECE) {
            indexToCheckIfNoPieceIsBetweenKingAndPawn -= increment;
        } else {
            // There is a piece between king and pawn, so pawn is not en-passant pinned
            return bitBoard;
        }
    }
    Piece threateningPiece;
    do {
        indexToSearchForThreateningPiece += increment;
        threateningPiece = pieceAtIndex(currentState.board, indexToSearchForThreateningPiece);
    } while (threateningPiece == NOPIECE && indexToSearchForThreateningPiece / 8 == enPassantRank);
    
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
            canEnPassant &= ~toggle; // The bits are inverted to set the enPassant bit to 0
        }
    } else {
        canEnPassant = checkEnPassantPinned(from, canEnPassant);
    }

    if (canEnPassant) {
        appendMove(from, currentState.enPassantTargetSquare, EN_PASSANT);
    }
}

void generatePawnDoublePush(int from, int increment) {
    int toSquareFromDoublePush = from + 2 * increment;
    if (pieceAtIndex(currentState.board, from + increment) != NOPIECE ||
        pieceAtIndex(currentState.board, toSquareFromDoublePush) != NOPIECE) { return; }
    
    u64 canDoublePawnPush = (u64) 1;
    canDoublePawnPush <<= toSquareFromDoublePush;

    // Accounting for pins
    canDoublePawnPush &= pinMasks[from];

    // Accounting for checks
    canDoublePawnPush &= checkBitBoard;

    if (canDoublePawnPush) {
        appendMove(from, toSquareFromDoublePush, DOUBLE_PAWN_PUSH);
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
    // We assume that the array is 0 initialized, so we do not need to add a 0 entry
}