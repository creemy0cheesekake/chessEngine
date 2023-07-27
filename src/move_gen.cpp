#include "move_gen.hpp"
#include "board.hpp"
#include "lookup_tables.hpp"
#include "util.cpp"

MoveGen::MoveGen(Board b) {
	board	= b;
	attacks = genAttacks();
}

void MoveGen::genPawnMoves() {
	bitboard pawns = board.sideToMove == WHITE ? board.pieces[W_PAWN] : board.pieces[B_PAWN];
	if (!pawns) return;
	bitboard allPieces	 = board.whitePieces() | board.blackPieces();
	bitboard theirPieces = board.sideToMove == WHITE ? board.blackPieces() : board.whitePieces();
	do {
		bitboard pawn = LS1B(pawns);
		bitboard Xs	  = (pawn & ~aFile ? pawn << 7 : 0) | (pawn & ~hFile ? pawn << 9 : 0);
		if (board.sideToMove == BLACK) Xs >>= 16;
		Xs &= theirPieces | board.enPassantSquare;
		bitboard push	 = (board.sideToMove == WHITE ? pawn << 8 : pawn >> 8) & ~(allPieces ^ pawn);
		bitboard dblPush = pawn & (secondRank | seventhRank) ? (board.sideToMove == WHITE ? push << 8 : push >> 8) & ~(allPieces ^ pawn) : 0;
		bitboard mask	 = Xs | push | dblPush;
		if (mask)
			do
				if (LS1B(mask) & (firstRank | eighthRank))
					for (int i = 1; i <= 4; i++)
						_moves.push_back(Move(board, bitscan(pawn), bitscan(LS1B(mask)), i));
				else
					_moves.push_back(Move(board, bitscan(pawn), bitscan(LS1B(mask))));
			while (mask &= mask - 1);
	} while (pawns &= pawns - 1);
}

void MoveGen::genKnightMoves() {
	bitboard knights = board.sideToMove == WHITE ? board.pieces[W_KNIGHT] : board.pieces[B_KNIGHT];
	if (!knights) return;
	bitboard yourPieces = board.sideToMove == WHITE ? board.whitePieces() : board.blackPieces();
	do {
		bitboard knight = LS1B(knights);
		bitboard mask	= LookupTables::knightAttacks[bitscan(knight)] & ~yourPieces;
		if (mask)
			do _moves.push_back(Move(board, bitscan(knight), bitscan(LS1B(mask))));
			while (mask &= mask - 1);
	} while (knights &= knights - 1);
}

bool MoveGen::inCheck() {
	return attacks & (board.sideToMove == WHITE ? board.pieces[W_KING] : board.pieces[B_KING]);
}

void MoveGen::genKingMoves() {
	bitboard yourPieces = board.sideToMove == WHITE ? board.whitePieces() : board.blackPieces();
	bitboard king		= board.sideToMove == WHITE ? board.pieces[W_KING] : board.pieces[B_KING];
	bitboard mask		= LookupTables::kingAttacks[bitscan(king)] & ~yourPieces;
	if (mask)
		do _moves.push_back(Move(board, bitscan(king), bitscan(LS1B(mask))));
		while (mask &= mask - 1);
}

void MoveGen::genCastlingMoves() {
	if (inCheck()) return;
	bitboard king				  = board.sideToMove == WHITE ? board.pieces[W_KING] : board.pieces[B_KING];
	unsigned short kingIndex	  = bitscan(king);
	unsigned short castlingRights = board.sideToMove == WHITE ? board.castlingRights >> 2 : board.castlingRights & 3;
	bitboard allPieces			  = board.whitePieces() | board.blackPieces();
	if (castlingRights & 2 && !((attacks | allPieces) & (king << 1 | king << 2)))
		_moves.push_back(Move(board, kingIndex, kingIndex + 2));
	if (castlingRights & 1 && !((attacks | allPieces) & (king >> 1 | king >> 2)) && !(allPieces & king >> 3))
		_moves.push_back(Move(board, kingIndex, kingIndex - 2));
}

void MoveGen::genSlidingPieces(bitboard pieces, bool straight, bool diagonal) {
	if (!pieces) return;
	bitboard yourPieces = board.sideToMove == WHITE ? board.whitePieces() : board.blackPieces();
	bitboard allPieces	= board.whitePieces() | board.blackPieces();

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

		bitboard rays = ((straightRays | diagonalRays) ^ piece) & ~yourPieces;

		if (rays)
			do _moves.push_back(Move(board, bitscan(piece), bitscan(LS1B(rays))));
			while (rays &= rays - 1);
	} while (pieces &= pieces - 1);
}

void MoveGen::genBishopMoves() {
	bitboard bishops = board.sideToMove == WHITE ? board.pieces[W_BISHOP] : board.pieces[B_BISHOP];
	genSlidingPieces(bishops, false, true);
}

void MoveGen::genRookMoves() {
	bitboard rooks = board.sideToMove == WHITE ? board.pieces[W_ROOK] : board.pieces[B_ROOK];
	genSlidingPieces(rooks, true, false);
}

void MoveGen::genQueenMoves() {
	bitboard queens = board.sideToMove == WHITE ? board.pieces[W_QUEEN] : board.pieces[B_QUEEN];
	genSlidingPieces(queens, true, true);
}

void MoveGen::genPseudoLegalMoves() {
	genPawnMoves();
	genKnightMoves();
	genKingMoves();
	genBishopMoves();
	genRookMoves();
	genQueenMoves();
	genCastlingMoves();
}

Moves MoveGen::genMoves() {
	Moves moves;
	moves.reserve(218);
	genPseudoLegalMoves();
	for (Move m : _moves)
		if (!m.execute().inIllegalCheck()) moves.push_back(m);
	return moves;
}

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
