#include "move_gen.hpp"
#include "lookup_tables.hpp"
#include "util.cpp"

bitboard MoveGen::genPawnAttacks() {
	bool whiteToMove = board.sideToMove == WHITE;
	bitboard pawns	 = whiteToMove ? board.pieces[B_PAWN] : board.pieces[W_PAWN];
	return (!whiteToMove ? ((pawns & ~aFile) << 7) | ((pawns & ~hFile) << 9) : ((pawns & ~hFile) >> 7) | ((pawns & ~aFile) >> 9));
}

bitboard MoveGen::genKnightAttacks() {
	bitboard knights = board.sideToMove == WHITE ? board.pieces[B_KNIGHT] : board.pieces[W_KNIGHT];
	if (!knights) return 0;
	bitboard atts = 0;
	do {
		bitboard knight = LS1B(knights);
		atts |= LookupTables::knightAttacks[bitscan(knight)];
	} while (knights &= knights - 1);
	return atts;
}

bitboard MoveGen::genKingAttacks() {
	bitboard king = board.sideToMove == WHITE ? board.pieces[B_KING] : board.pieces[W_KING];
	return LookupTables::kingAttacks[bitscan(king)];
}

bitboard MoveGen::genSlidingPiecesAttacks(bitboard pieces, bool straight, bool diagonal) {
	if (!pieces) return 0;
	bitboard allPieces = board.whitePieces() | board.blackPieces();
	bitboard atts	   = 0;

	do {
		bitboard piece		  = LS1B(pieces);
		bitboard straightRays = 0;
		bitboard diagonalRays = 0;
		if (straight) {
			bitboard Nrays = piece, Erays = piece, Wrays = piece, Srays = piece;
			while (!(Nrays & eighthRank)) {
				Nrays |= Nrays << 8;
				if (allPieces & MS1B(Nrays)) break;
			}

			while (!(Erays & hFile)) {
				Erays |= Erays << 1;
				if (allPieces & MS1B(Erays)) break;
			}

			while (!(Wrays & aFile)) {
				Wrays |= Wrays >> 1;
				if (allPieces & LS1B(Wrays)) break;
			}

			while (!(Srays & firstRank)) {
				Srays |= Srays >> 8;
				if (allPieces & LS1B(Srays)) break;
			}
			straightRays = Nrays | Erays | Wrays | Srays;
		}
		if (diagonal) {
			bitboard NErays = piece, NWrays = piece, SWrays = piece, SErays = piece;
			while (!(NErays & (eighthRank | hFile))) {
				NErays |= NErays << 9;
				if (allPieces & MS1B(NErays)) break;
			}

			while (!(NWrays & (eighthRank | aFile))) {
				NWrays |= NWrays << 7;
				if (allPieces & MS1B(NWrays)) break;
			}

			while (!(SWrays & (firstRank | aFile))) {
				SWrays |= SWrays >> 9;
				if (allPieces & LS1B(SWrays)) break;
			}

			while (!(SErays & (firstRank | hFile))) {
				SErays |= SErays >> 7;
				if (allPieces & LS1B(SErays)) break;
			}
			diagonalRays = NErays | NWrays | SWrays | SErays;
		}

		atts |= (straightRays | diagonalRays) ^ piece;
	} while (pieces &= pieces - 1);
	return atts;
}

bitboard MoveGen::genBishopAttacks() {
	bitboard bishops = board.sideToMove == BLACK ? board.pieces[W_BISHOP] : board.pieces[B_BISHOP];
	return genSlidingPiecesAttacks(bishops, false, true);
};
bitboard MoveGen::genRookAttacks() {
	bitboard rooks = board.sideToMove == BLACK ? board.pieces[W_ROOK] : board.pieces[B_ROOK];
	return genSlidingPiecesAttacks(rooks, true, false);
};
bitboard MoveGen::genQueenAttacks() {
	bitboard queens = board.sideToMove == BLACK ? board.pieces[W_QUEEN] : board.pieces[B_QUEEN];
	return genSlidingPiecesAttacks(queens, true, true);
}

bitboard MoveGen::genAttacks() {
	return genPawnAttacks() | genKnightAttacks() | genKingAttacks() | genBishopAttacks() | genRookAttacks() | genQueenAttacks();
}

bitboard MoveGen::getAttacks() {
	return attacks;
}
