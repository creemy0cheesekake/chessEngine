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
		Bitboard knight = LS1B(knights);
		attacks |= LookupTables::knightAttacks[bitscan(knight)];
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
		Bitboard piece		  = LS1B(pieces);
		Bitboard straightRays = 0;
		Bitboard diagonalRays = 0;
		if (direction & SlidingPieceDirectionFlags::STRAIGHT) {
			Bitboard Nrays = piece, Erays = piece, Wrays = piece, Srays = piece;
			while (!(Nrays & eighthRank)) {
				Nrays |= Nrays << 8;
				if (allPieces & MS1B(Nrays)) {
					break;
				}
			}
			while (!(Erays & hFile)) {
				Erays |= Erays << 1;
				if (allPieces & MS1B(Erays)) {
					break;
				}
			}
			while (!(Wrays & aFile)) {
				Wrays |= Wrays >> 1;
				if (allPieces & LS1B(Wrays)) {
					break;
				}
			}
			while (!(Srays & firstRank)) {
				Srays |= Srays >> 8;
				if (allPieces & LS1B(Srays)) {
					break;
				}
			}
			straightRays = Nrays | Erays | Wrays | Srays;
		}
		if (direction & SlidingPieceDirectionFlags::DIAGONAL) {
			Bitboard NErays = piece, NWrays = piece, SWrays = piece, SErays = piece;
			while (!(NErays & (eighthRank | hFile))) {
				NErays |= NErays << 9;
				if (allPieces & MS1B(NErays)) {
					break;
				}
			}
			while (!(NWrays & (eighthRank | aFile))) {
				NWrays |= NWrays << 7;
				if (allPieces & MS1B(NWrays)) {
					break;
				}
			}
			while (!(SWrays & (firstRank | aFile))) {
				SWrays |= SWrays >> 9;
				if (allPieces & LS1B(SWrays)) {
					break;
				}
			}
			while (!(SErays & (firstRank | hFile))) {
				SErays |= SErays >> 7;
				if (allPieces & LS1B(SErays)) {
					break;
				}
			}
			diagonalRays = NErays | NWrays | SWrays | SErays;
		}

		attacks |= (straightRays | diagonalRays) ^ piece;
	} while (removeLS1B(pieces));
	return attacks;
}

Bitboard MoveGen::genBishopAttacks() {
	Bitboard bishops = board.pieces[!board.sideToMove][BISHOP];
	return genSlidingPiecesAttacks(bishops, SlidingPieceDirectionFlags::DIAGONAL);
};
Bitboard MoveGen::genRookAttacks() {
	Bitboard rooks = board.pieces[!board.sideToMove][ROOK];
	return genSlidingPiecesAttacks(rooks, SlidingPieceDirectionFlags::STRAIGHT);
};
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
