#include "move_gen.hpp"
#include "lookup_tables.hpp"
#include "util.hpp"

Bitboard MoveGen::genPawnAttacks() {
	if (m_board.sideToMove == WHITE) {
		Bitboard pawns = m_board.pieces[BLACK][PAWN];
		return ((pawns & ~hFile) >> 7) | ((pawns & ~aFile) >> 9);
	} else {
		Bitboard pawns = m_board.pieces[WHITE][PAWN];
		return ((pawns & ~aFile) << 7) | ((pawns & ~hFile) << 9);
	}
}

Bitboard MoveGen::genKnightAttacks() {
	Bitboard knights = m_board.pieces[!m_board.sideToMove][KNIGHT];
	if (!knights) {
		return 0;
	}
	Bitboard attacks = 0;
	do {
		attacks |= LookupTables::s_knightAttacks[bitscan(knights)];
	} while (removeLS1B(knights));
	return attacks;
}

Bitboard MoveGen::genKingAttacks() {
	Bitboard king = m_board.pieces[!m_board.sideToMove][KING];
	return LookupTables::s_kingAttacks[bitscan(king)];
}

Bitboard MoveGen::genSlidingPiecesAttacks(Bitboard pieces, SlidingPieceDirectionFlags direction) {
	if (!pieces) {
		return 0;
	}
	Bitboard allPieces = m_board.whitePieces() | m_board.blackPieces();
	Bitboard attacks   = 0;

	do {
		Bitboard straightRays = 0;
		Bitboard diagonalRays = 0;
		Square pieceSquare	  = (Square)bitscan(pieces);
		if (direction & SlidingPieceDirectionFlags::STRAIGHT) {
			for (DirectionStraight direction : {NORTH, SOUTH, EAST, WEST}) {
				Bitboard directionalRay = LookupTables::s_straightRayTable[pieceSquare][direction];
				Bitboard blockers		= directionalRay & allPieces;
				Bitboard blockedOffMask = 0;
				if (blockers) {
					if (direction == NORTH || direction == EAST) {
						blockedOffMask = LookupTables::s_straightRayTable[bitscan(blockers)][direction];
					} else {
						blockedOffMask = LookupTables::s_straightRayTable[reverseBitscan(blockers)][direction];
					}
				}
				straightRays |= directionalRay & ~blockedOffMask;
			}
		}
		if (direction & SlidingPieceDirectionFlags::DIAGONAL) {
			for (DirectionDiagonal direction : {NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST}) {
				Bitboard directionalRay = LookupTables::s_diagonalRayTable[pieceSquare][direction];
				Bitboard blockers		= directionalRay & allPieces;
				Bitboard blockedOffMask = 0;
				if (blockers) {
					if (direction == NORTHEAST || direction == NORTHWEST) {
						blockedOffMask = LookupTables::s_diagonalRayTable[bitscan(blockers)][direction];
					} else {
						blockedOffMask = LookupTables::s_diagonalRayTable[reverseBitscan(blockers)][direction];
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
	Bitboard bishops = m_board.pieces[!m_board.sideToMove][BISHOP];
	return genSlidingPiecesAttacks(bishops, SlidingPieceDirectionFlags::DIAGONAL);
}
Bitboard MoveGen::genRookAttacks() {
	Bitboard rooks = m_board.pieces[!m_board.sideToMove][ROOK];
	return genSlidingPiecesAttacks(rooks, SlidingPieceDirectionFlags::STRAIGHT);
}
Bitboard MoveGen::genQueenAttacks() {
	Bitboard queens = m_board.pieces[!m_board.sideToMove][QUEEN];
	return genSlidingPiecesAttacks(queens, SlidingPieceDirectionFlags(DIAGONAL | STRAIGHT));
}

Bitboard MoveGen::genAttacks() {
	return genPawnAttacks() | genKnightAttacks() | genKingAttacks() | genBishopAttacks() | genRookAttacks() | genQueenAttacks();
}

Bitboard MoveGen::getAttacks() {
	return m_attacks;
}
