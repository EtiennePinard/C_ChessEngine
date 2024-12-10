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

ChessPosition currentState;

Move* validMoves;
int currentMoveIndex;
#define appendMove(startSquare, targetSquare, flag) validMoves[currentMoveIndex++] = makeMove((startSquare), (targetSquare), (flag));

PieceCharacteristics opponentColor;
int friendlyKingIndex;

// For O(1) .contains call
bool attackedSquares[BOARD_SIZE];

bool inDoubleCheck;
bool inCheck;
bool enPassantWillRemoveTheCheck;

u64 checkBitBoard;
u64 pinMasks[BOARD_SIZE];
u64 friendlyPieceBitBoard;
u64 opponentBitBoard;

bool isKingInCheck() {
    return inCheck;
}
bool isKingInDoubleCheck() {
    return inDoubleCheck;
}

void init() {
    opponentColor = currentState.colorToGo == WHITE ? BLACK : WHITE;
    friendlyKingIndex = trailingZeros_64(bitBoardForPiece(currentState.board, makePiece(currentState.colorToGo, KING)));
    inCheck = false;
    inDoubleCheck = false;
    enPassantWillRemoveTheCheck = false;

    memset(attackedSquares, false, sizeof(bool) * BOARD_SIZE);

    checkBitBoard = ~((u64) 0); // There is no check (for now), so every square is valid, thus every bit is set
    memset(pinMasks, 0xFF, sizeof(u64) * BOARD_SIZE);
    friendlyPieceBitBoard = specificColorBitBoard(currentState.board, currentState.colorToGo);
    opponentBitBoard = specificColorBitBoard(currentState.board, opponentColor);
}

// Note: Could be inlined
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
    int forwardIndexX = file(forwardIndex);
    // The forward index is always valid because no pawn can be on the first or last rank (they get promoted)
    if (forwardIndexX != 7) {
        int square = forwardIndex + 1;
        if (attackedSquares[square] && square == friendlyKingIndex) {
            inDoubleCheck = true;
        }
        attackedSquares[square] = true;
    }
    if (forwardIndexX != 0) { 
        int square = forwardIndex - 1; 
        if (attackedSquares[square] && square == friendlyKingIndex) {
            inDoubleCheck = true;
        }
        attackedSquares[square] = true;
    }
}

void calculateAttackSquares() {
    u64 bitBoard;
    u64 opponentBitBoardCopy = opponentBitBoard;
    while (opponentBitBoardCopy) {
        int currentIndex = trailingZeros_64(opponentBitBoardCopy);
        switch (pieceType(pieceAtIndex(currentState.board, currentIndex))) {
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
            bitBoard = knightMovementMask[currentIndex];
            addBitBoardToAttackedSquares(bitBoard);
            break;
        case PAWN:
            pawnAttackingSquares(currentIndex);
            break;
        case KING:
            bitBoard = kingMovementMask[currentIndex];
            addBitBoardToAttackedSquares(bitBoard);
            break;
        default:
            break;
        }
       opponentBitBoardCopy &= opponentBitBoardCopy - 1; 
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
    int kingX = file(friendlyKingIndex);
    int kingY = rank(friendlyKingIndex);
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
    if (friendlyKingIndex != defaultKingIndex) { return; }

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
    bitboard = kingMovementMask[friendlyKingIndex];
    while (bitboard) {
        int targetSquare = trailingZeros_64(bitboard);
        if (isKingIndexLegal(targetSquare)) {
            appendMove(friendlyKingIndex, targetSquare, NOFlAG);
        }
        bitboard &= bitboard - 1;
    }

    if (inDoubleCheck) { return; } // Only king moves are valid
    
    handlePinsAndChecksFromSlidingPieces();

    if (!inCheck) { 
        generateCastle();
        return; // Do not need to look for checks
    } 

    u64 toggle = (u64) 1;
    // Handling checks from pawns and knights
    const int delta = currentState.colorToGo == WHITE ? -1 : 1;
    int potentialPawn;
    // For loop is remove copy pasting. I know sometimes I copy paste a lot but I was fed up this time
    for (int i = 0; i < 2; i++) {
        // A kind of hacky way to get 7 if i == 0 and 9 if i == 1
        potentialPawn = friendlyKingIndex + (2 * i + 7) * delta;
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
    bitboard = knightMovementMask[friendlyKingIndex];
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
    int enPassantRank = rank(from);
    if (rank(friendlyKingIndex) != enPassantRank) { 
        return bitBoard; // The king is not on the same rank as the from pawn, so pawn cannot be en-passant pinned
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
    } while (threateningPiece == NOPIECE && rank(indexToSearchForThreateningPiece) == enPassantRank);
    
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

    // Note that canEnPasant is never 0, because pawns can only en passant on the third or fourth rank
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

// Note: Could be inlined
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
    movesBitBoard &= ~friendlyPieceBitBoard; // We invert the bitboard so that we do not capture friendly pieces
    
    appendLegalMovesFromPseudoLegalMovesBitBoard(from, movesBitBoard);
}

void pawnMoves(int from) {
    char fromY = rank(from);
    bool isPawnBeforePromotion = currentState.colorToGo == WHITE ? 
        fromY == 1 : 
        fromY == 6;
    bool pawnCanEnPassant = currentState.colorToGo == WHITE ? 
        fromY == 3 : 
        fromY == 4;
    bool pawnCanDoublePush = currentState.colorToGo == WHITE ? 
        fromY == 6 : 
        fromY == 1;
    int increment = currentState.colorToGo == WHITE ? -8 : 8;

    u64 pseudoLegalMoves = (u64) 0;
    u64 toggle = (u64) 1;
    
    int forwardIndex = from + increment;
    int forwardX = file(forwardIndex);
    // The forward index is always between 0 and 63, because if a pawn is on one of the edge ranks it will get promoted
    if (forwardX != 7) { pseudoLegalMoves ^= toggle << (forwardIndex + 1); }
    if (forwardX != 0) { pseudoLegalMoves ^= toggle << (forwardIndex - 1); }
    // Only making capture moves if we actually capture a piece
    pseudoLegalMoves &= opponentBitBoard;
    
    if (pieceAtIndex(currentState.board, forwardIndex) == NOPIECE) { pseudoLegalMoves ^= toggle << forwardIndex; }
    
    // enPassantTargetSquare is 0 when there is no pawn that has double pushed
    if (pawnCanEnPassant && currentState.enPassantTargetSquare) { 
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
    u64 pseudoLegalMovesBitBoard;
    u64 piecesBitBoard = friendlyPieceBitBoard;
    while (piecesBitBoard) {
        int currentIndex = trailingZeros_64(piecesBitBoard);

        switch (pieceType(pieceAtIndex(currentState.board, currentIndex))) {
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
                pseudoLegalMovesBitBoard = knightMovementMask[currentIndex];
                pseudoLegalMovesBitBoard &= ~friendlyPieceBitBoard; // We invert the bit board so that we do not capture friendly pieces
                appendLegalMovesFromPseudoLegalMovesBitBoard(currentIndex, pseudoLegalMovesBitBoard);
                break;
            case PAWN:
                pawnMoves(currentIndex);
                break;
            default:
                break;
        }

        piecesBitBoard &= piecesBitBoard - 1;
    }
}

// TODO: Make the caller give us a fixed size of memory (arenas?) which we can then use to store our global variables
/*
We are not computing end of games in this function!!!!
They are not needed for perft and so I did not write a function to compute just yet
These function will probably in the board.c file
*/
void getValidMoves(Move result[256], int* numMoves, ChessPosition position) {

    currentState = position;

    validMoves = result;
    currentMoveIndex = 0;

    init();
    calculateAttackSquares();
    generateKingMoves();

    if (!inDoubleCheck) { 
        generateSupportingPiecesMoves();
    }
    
    *(numMoves) = currentMoveIndex;
}