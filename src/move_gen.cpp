#include "move_gen.hpp"
#include "board.hpp"
#include "lookup_tables.hpp"
#include "util.hpp"

MoveGen::MoveGen(Board b) {
	board	= b;
	attacks = genAttacks();
}

void MoveGen::genPawnMoves() {
	bitboard pawns = board.pieces[board.sideToMove][PAWN];
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
						_moves.push_back(Move(board, bitscan(pawn), bitscan(LS1B(mask)), PAWN, i));
				else
					_moves.push_back(Move(board, bitscan(pawn), bitscan(LS1B(mask)), PAWN));
			while (mask &= mask - 1);
	} while (pawns &= pawns - 1);
}

void MoveGen::genKnightMoves() {
	bitboard knights = board.pieces[board.sideToMove][KNIGHT];
	if (!knights) return;
	bitboard yourPieces = board.sideToMove == WHITE ? board.whitePieces() : board.blackPieces();
	do {
		bitboard knight = LS1B(knights);
		bitboard mask	= LookupTables::knightAttacks[bitscan(knight)] & ~yourPieces;
		if (mask)
			do _moves.push_back(Move(board, bitscan(knight), bitscan(LS1B(mask)), KNIGHT));
			while (mask &= mask - 1);
	} while (knights &= knights - 1);
}

bool MoveGen::inCheck() {
	return attacks & board.pieces[board.sideToMove][KING];
}

void MoveGen::genKingMoves() {
	bitboard yourPieces = board.sideToMove == WHITE ? board.whitePieces() : board.blackPieces();
	bitboard king		= board.pieces[board.sideToMove][KING];
	bitboard mask		= LookupTables::kingAttacks[bitscan(king)] & ~yourPieces;
	if (mask)
		do _moves.push_back(Move(board, bitscan(king), bitscan(LS1B(mask)), KING));
		while (mask &= mask - 1);
}

void MoveGen::genCastlingMoves() {
	if (inCheck()) return;
	bitboard king				  = board.pieces[board.sideToMove][KING];
	unsigned short kingIndex	  = bitscan(king);
	unsigned short castlingRights = board.sideToMove == WHITE ? board.castlingRights >> 2 : board.castlingRights & 3;
	bitboard allPieces			  = board.whitePieces() | board.blackPieces();
	if (castlingRights & 2 && !((attacks | allPieces) & (king << 1 | king << 2)))
		_moves.push_back(Move(board, kingIndex, kingIndex + 2, KING));
	if (castlingRights & 1 && !((attacks | allPieces) & (king >> 1 | king >> 2)) && !(allPieces & king >> 3))
		_moves.push_back(Move(board, kingIndex, kingIndex - 2, KING));
}

void MoveGen::genSlidingPieces(Piece p, bitboard pieces, bool straight, bool diagonal) {
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
			do _moves.push_back(Move(board, bitscan(piece), bitscan(LS1B(rays)), p));
			while (rays &= rays - 1);
	} while (pieces &= pieces - 1);
}

void MoveGen::genBishopMoves() {
	bitboard bishops = board.pieces[board.sideToMove][BISHOP];
	genSlidingPieces(BISHOP, bishops, false, true);
}

void MoveGen::genRookMoves() {
	bitboard rooks = board.pieces[board.sideToMove][ROOK];
	genSlidingPieces(ROOK, rooks, true, false);
}

void MoveGen::genQueenMoves() {
	bitboard queens = board.pieces[board.sideToMove][QUEEN];
	genSlidingPieces(QUEEN, queens, true, true);
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
