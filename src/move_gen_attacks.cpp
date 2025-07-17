#include "move_gen.hpp"
#include "lookup_tables.hpp"
#include "util.hpp"

bitboard MoveGen::genPawnAttacks() {
	if (board.sideToMove == WHITE) {
		bitboard pawns = board.pieces[BLACK][PAWN];
		return ((pawns & ~hFile) >> 7) | ((pawns & ~aFile) >> 9);
	} else {
		bitboard pawns = board.pieces[WHITE][PAWN];
		return ((pawns & ~aFile) << 7) | ((pawns & ~hFile) << 9);
	}
}

bitboard MoveGen::genKnightAttacks() {
	bitboard knights = board.pieces[!board.sideToMove][KNIGHT];
	if (!knights) {
		return 0;
	}
	bitboard attacks = 0;
	do {
		bitboard knight = LS1B(knights);
		attacks |= LookupTables::knightAttacks[bitscan(knight)];
	} while (removeLS1B(knights));
	return attacks;
}

bitboard MoveGen::genKingAttacks() {
	bitboard king = board.pieces[!board.sideToMove][KING];
	return LookupTables::kingAttacks[bitscan(king)];
}

bitboard MoveGen::genSlidingPiecesAttacks(bitboard pieces, SlidingPieceDirectionFlags direction) {
	if (!pieces) {
		return 0;
	}
	bitboard allPieces = board.whitePieces() | board.blackPieces();
	bitboard attacks   = 0;

	do {
		bitboard piece		  = LS1B(pieces);
		bitboard straightRays = 0;
		bitboard diagonalRays = 0;
		if (direction & SlidingPieceDirectionFlags::STRAIGHT) {
			bitboard Nrays = piece, Erays = piece, Wrays = piece, Srays = piece;
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
			bitboard NErays = piece, NWrays = piece, SWrays = piece, SErays = piece;
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

bitboard MoveGen::genBishopAttacks() {
	bitboard bishops = board.pieces[!board.sideToMove][BISHOP];
	return genSlidingPiecesAttacks(bishops, SlidingPieceDirectionFlags::DIAGONAL);
};
bitboard MoveGen::genRookAttacks() {
	bitboard rooks = board.pieces[!board.sideToMove][ROOK];
	return genSlidingPiecesAttacks(rooks, SlidingPieceDirectionFlags::STRAIGHT);
};
bitboard MoveGen::genQueenAttacks() {
	bitboard queens = board.pieces[!board.sideToMove][QUEEN];
	return genSlidingPiecesAttacks(queens, SlidingPieceDirectionFlags(DIAGONAL | STRAIGHT));
}

bitboard MoveGen::genAttacks() {
	return genPawnAttacks() | genKnightAttacks() | genKingAttacks() | genBishopAttacks() | genRookAttacks() | genQueenAttacks();
}

bitboard MoveGen::getAttacks() {
	return attacks;
}
