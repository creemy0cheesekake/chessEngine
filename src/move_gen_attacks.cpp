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

Bitboard MoveGen::genStraightRays(Square pieceSquare, Bitboard occ) {
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

Bitboard MoveGen::genDiagonalRays(Square pieceSquare, Bitboard occ) {
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

template <SlidingPieceDirectionFlags Direction>
Bitboard MoveGen::genSlidingPiecesAttacks(Bitboard pieces) const {
	if (!pieces) {
		return 0;
	}
	Bitboard allPieces = m_board.whitePieces() | m_board.blackPieces();
	Bitboard attacks   = 0;

	do {
		int sq = bitscan(pieces);
		if constexpr (Direction & SlidingPieceDirectionFlags::STRAIGHT) {
			Bitboard Nrays	   = LookupTables::s_straightRayTable[sq][NORTH],
					 Erays	   = LookupTables::s_straightRayTable[sq][EAST],
					 Wrays	   = LookupTables::s_straightRayTable[sq][WEST],
					 Srays	   = LookupTables::s_straightRayTable[sq][SOUTH];
			Bitboard Nblockers = Nrays & allPieces;
			if (Nblockers != 0) Nrays ^= (LookupTables::s_straightRayTable[bitscan(Nblockers)][NORTH]);
			Bitboard Eblockers = Erays & allPieces;
			if (Eblockers != 0) Erays ^= (LookupTables::s_straightRayTable[bitscan(Eblockers)][EAST]);
			Bitboard Sblockers = Srays & allPieces;
			if (Sblockers != 0) Srays ^= (LookupTables::s_straightRayTable[reverseBitscan(Sblockers)][SOUTH]);
			Bitboard Wblockers = Wrays & allPieces;
			if (Wblockers != 0) Wrays ^= (LookupTables::s_straightRayTable[reverseBitscan(Wblockers)][WEST]);
			attacks |= Nrays | Erays | Wrays | Srays;
		}
		if constexpr (Direction & SlidingPieceDirectionFlags::DIAGONAL) {
			Bitboard NErays		= LookupTables::s_diagonalRayTable[sq][NORTHEAST],
					 NWrays		= LookupTables::s_diagonalRayTable[sq][NORTHWEST],
					 SWrays		= LookupTables::s_diagonalRayTable[sq][SOUTHWEST],
					 SErays		= LookupTables::s_diagonalRayTable[sq][SOUTHEAST];
			Bitboard NEblockers = NErays & allPieces;
			if (NEblockers != 0) NErays ^= (LookupTables::s_diagonalRayTable[bitscan(NEblockers)][NORTHEAST]);
			Bitboard NWblockers = NWrays & allPieces;
			if (NWblockers != 0) NWrays ^= (LookupTables::s_diagonalRayTable[bitscan(NWblockers)][NORTHWEST]);
			Bitboard SWblockers = SWrays & allPieces;
			if (SWblockers != 0) SWrays ^= (LookupTables::s_diagonalRayTable[reverseBitscan(SWblockers)][SOUTHWEST]);
			Bitboard SEblockers = SErays & allPieces;
			if (SEblockers != 0) SErays ^= (LookupTables::s_diagonalRayTable[reverseBitscan(SEblockers)][SOUTHEAST]);
			attacks |= NErays | NWrays | SWrays | SErays;
		}
	} while (removeLS1B(pieces));
	return attacks;
}

Bitboard MoveGen::genBishopAttacks() const {
	Bitboard bishops = m_board.boardState.pieces[!m_board.boardState.sideToMove][BISHOP];
	return genSlidingPiecesAttacks<SlidingPieceDirectionFlags::DIAGONAL>(bishops);
}
Bitboard MoveGen::genRookAttacks() const {
	Bitboard rooks = m_board.boardState.pieces[!m_board.boardState.sideToMove][ROOK];
	return genSlidingPiecesAttacks<SlidingPieceDirectionFlags::STRAIGHT>(rooks);
}
Bitboard MoveGen::genQueenAttacks() const {
	Bitboard queens = m_board.boardState.pieces[!m_board.boardState.sideToMove][QUEEN];
	return genSlidingPiecesAttacks<SlidingPieceDirectionFlags(DIAGONAL | STRAIGHT)>(queens);
}

Bitboard MoveGen::genAttacks() const {
	return genPawnAttacks() | genKnightAttacks() | genKingAttacks() | genBishopAttacks() | genRookAttacks() | genQueenAttacks();
}

Bitboard MoveGen::getAttacks() const {
	return m_attacks;
}
