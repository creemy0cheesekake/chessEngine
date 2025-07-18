#include "move_gen.hpp"
#include "lookup_tables.hpp"
#include "util.hpp"

Bitboard MoveGen::genPawnAttacks() {
	if (board.sideToMove == WHITE) {
		Bitboard pawns = board.pieces[BLACK][PAWN];
		return ((pawns & ~hFile) >> 7) | ((pawns & ~aFile) >> 9);
	} else {
		Bitboard pawns = board.pieces[WHITE][PAWN];
		return ((pawns & ~aFile) << 7) | ((pawns & ~hFile) << 9);
	}
}

Bitboard MoveGen::genKnightAttacks() {
	Bitboard knights = board.pieces[!board.sideToMove][KNIGHT];
	if (!knights) {
		return 0;
	}
	Bitboard attacks = 0;
	do {
		attacks |= LookupTables::knightAttacks[bitscan(knights)];
	} while (removeLS1B(knights));
	return attacks;
}

Bitboard MoveGen::genKingAttacks() {
	Bitboard king = board.pieces[!board.sideToMove][KING];
	return LookupTables::kingAttacks[bitscan(king)];
}

Bitboard MoveGen::genSlidingPiecesAttacks(Bitboard pieces, SlidingPieceDirectionFlags direction) {
	if (!pieces) {
		return 0;
	}
	Bitboard allPieces = board.whitePieces() | board.blackPieces();
	Bitboard attacks   = 0;

	do {
		Bitboard straightRays = 0;
		Bitboard diagonalRays = 0;
		Square pieceSquare	  = (Square)bitscan(pieces);
		if (direction & SlidingPieceDirectionFlags::STRAIGHT) {
			for (DirectionStraight direction : {NORTH, SOUTH, EAST, WEST}) {
				Bitboard directionalRay = LookupTables::straightRayTable[pieceSquare][direction];
				Bitboard blockers		= directionalRay & allPieces;
				Bitboard blockedOffMask = 0;
				if (blockers) {
					if (direction == NORTH || direction == EAST) {
						blockedOffMask = LookupTables::straightRayTable[bitscan(blockers)][direction];
					} else {
						blockedOffMask = LookupTables::straightRayTable[reverseBitscan(blockers)][direction];
					}
				}
				straightRays |= directionalRay & ~blockedOffMask;
			}
		}
		if (direction & SlidingPieceDirectionFlags::DIAGONAL) {
			for (DirectionDiagonal direction : {NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST}) {
				Bitboard directionalRay = LookupTables::diagonalRayTable[pieceSquare][direction];
				Bitboard blockers		= directionalRay & allPieces;
				Bitboard blockedOffMask = 0;
				if (blockers) {
					if (direction == NORTHEAST || direction == NORTHWEST) {
						blockedOffMask = LookupTables::diagonalRayTable[bitscan(blockers)][direction];
					} else {
						blockedOffMask = LookupTables::diagonalRayTable[reverseBitscan(blockers)][direction];
					}
				}
				diagonalRays |= directionalRay & ~blockedOffMask;
			}
		}

		attacks |= (straightRays | diagonalRays);
	} while (removeLS1B(pieces));
	return attacks;
}

Bitboard MoveGen::genBishopAttacks() {
	Bitboard bishops = board.pieces[!board.sideToMove][BISHOP];
	return genSlidingPiecesAttacks(bishops, SlidingPieceDirectionFlags::DIAGONAL);
}
Bitboard MoveGen::genRookAttacks() {
	Bitboard rooks = board.pieces[!board.sideToMove][ROOK];
	return genSlidingPiecesAttacks(rooks, SlidingPieceDirectionFlags::STRAIGHT);
}
Bitboard MoveGen::genQueenAttacks() {
	Bitboard queens = board.pieces[!board.sideToMove][QUEEN];
	return genSlidingPiecesAttacks(queens, SlidingPieceDirectionFlags(DIAGONAL | STRAIGHT));
}

Bitboard MoveGen::genAttacks() {
	return genPawnAttacks() | genKnightAttacks() | genKingAttacks() | genBishopAttacks() | genRookAttacks() | genQueenAttacks();
}

Bitboard MoveGen::getAttacks() {
	return attacks;
}
