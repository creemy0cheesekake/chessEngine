#include <iostream>
#include "board.hpp"
#include "move_gen.hpp"
#include "lookup_tables.hpp"
#include "util.hpp"

Bitboard MoveGen::genPawnAttacks() const {
	if (m_board.boardState.sideToMove == WHITE) {
		Bitboard pawns = m_board.boardState.pieces[BLACK][PAWN];
		return ((pawns & ~hFile) >> 7) | ((pawns & ~aFile) >> 9);
	} else {
		Bitboard pawns = m_board.boardState.pieces[WHITE][PAWN];
		return ((pawns & ~aFile) << 7) | ((pawns & ~hFile) << 9);
	}
}

Bitboard MoveGen::genKnightAttacks() const {
	Bitboard knights = m_board.boardState.pieces[!m_board.boardState.sideToMove][KNIGHT];
	if (!knights) {
		return 0;
	}
	Bitboard attacks = 0;
	do {
		attacks |= LookupTables::s_knightAttacks[bitscan(knights)];
	} while (removeLS1B(knights));
	return attacks;
}

Bitboard MoveGen::genKingAttacks() const {
	Bitboard king = m_board.boardState.pieces[!m_board.boardState.sideToMove][KING];
	return LookupTables::s_kingAttacks[bitscan(king)];
}

Bitboard MoveGen::genStraightRays(Board& b, Square pieceSquare, Bitboard occ) {
	Bitboard attacks = 0;
	for (DirectionStraight direction : {NORTH, SOUTH, EAST, WEST}) {
		Bitboard directionalRay = LookupTables::s_straightRayTable[pieceSquare][direction];
		Bitboard blockers		= directionalRay & occ;
		Bitboard blockedOffMask = 0;
		if (blockers) {
			if (direction == NORTH || direction == EAST) {
				blockedOffMask = LookupTables::s_straightRayTable[bitscan(blockers)][direction];
			} else {
				blockedOffMask = LookupTables::s_straightRayTable[reverseBitscan(blockers)][direction];
			}
		}
		attacks |= directionalRay & ~blockedOffMask;
	}
	return attacks;
}

Bitboard MoveGen::genDiagonalRays(Board& b, Square pieceSquare, Bitboard occ) {
	Bitboard attacks = 0;
	for (DirectionDiagonal direction : {NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST}) {
		Bitboard directionalRay = LookupTables::s_diagonalRayTable[pieceSquare][direction];
		Bitboard blockers		= directionalRay & occ;
		Bitboard blockedOffMask = 0;
		if (blockers) {
			if (direction == NORTHEAST || direction == NORTHWEST) {
				blockedOffMask = LookupTables::s_diagonalRayTable[bitscan(blockers)][direction];
			} else {
				blockedOffMask = LookupTables::s_diagonalRayTable[reverseBitscan(blockers)][direction];
			}
		}
		attacks |= directionalRay & ~blockedOffMask;
	}
	return attacks;
}

Bitboard MoveGen::genSlidingPiecesAttacks(Bitboard pieces, SlidingPieceDirectionFlags direction) const {
	if (!pieces) {
		return 0;
	}
	Bitboard allPieces = m_board.whitePieces() | m_board.blackPieces();
	Bitboard attacks   = 0;

	do {
		if (direction & SlidingPieceDirectionFlags::STRAIGHT) {
			attacks |= genStraightRays(m_board, (Square)bitscan(pieces), allPieces);
		}
		if (direction & SlidingPieceDirectionFlags::DIAGONAL) {
			attacks |= genDiagonalRays(m_board, (Square)bitscan(pieces), allPieces);
		}
	} while (removeLS1B(pieces));
	return attacks;
}

Bitboard MoveGen::genBishopAttacks() const {
	Bitboard bishops = m_board.boardState.pieces[!m_board.boardState.sideToMove][BISHOP];
	return genSlidingPiecesAttacks(bishops, SlidingPieceDirectionFlags::DIAGONAL);
}
Bitboard MoveGen::genRookAttacks() const {
	Bitboard rooks = m_board.boardState.pieces[!m_board.boardState.sideToMove][ROOK];
	return genSlidingPiecesAttacks(rooks, SlidingPieceDirectionFlags::STRAIGHT);
}
Bitboard MoveGen::genQueenAttacks() const {
	Bitboard queens = m_board.boardState.pieces[!m_board.boardState.sideToMove][QUEEN];
	return genSlidingPiecesAttacks(queens, SlidingPieceDirectionFlags(DIAGONAL | STRAIGHT));
}

Bitboard MoveGen::genAttacks() const {
	return genPawnAttacks() | genKnightAttacks() | genKingAttacks() | genBishopAttacks() | genRookAttacks() | genQueenAttacks();
}

Bitboard MoveGen::getAttacks() const {
	return m_attacks;
}
