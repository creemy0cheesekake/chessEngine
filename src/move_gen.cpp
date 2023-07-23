#include "consts.hpp"
#include "move_gen.hpp"
#include <algorithm>
#include <bitset>
#include <iostream>
#include <string>
#include <unordered_map>

MoveGen::MoveGen(Board b) {
	board = b;
}

std::vector<Move> MoveGen::genPawnMoves() {
	std::vector<Move> moves;
	bitboard pawns		 = *(board.sideToMove == WHITE ? board.W_PAWN : board.B_PAWN);
	bitboard allPieces	 = board.whitePieces() | board.blackPieces();
	bitboard theirPieces = board.sideToMove == WHITE ? board.blackPieces() : board.whitePieces();
	do {
		bitboard pawn = LS1B(pawns);
		bitboard Xs	  = (pawn & ~aFile ? pawn << 7 : 0) | (pawn & ~hFile ? pawn << 9 : 0);
		if (board.sideToMove == BLACK) Xs >>= 16;
		attacks[board.sideToMove] |= Xs;
		Xs &= theirPieces | board.enPassantSquare;
		bitboard push	 = (board.sideToMove == WHITE ? pawn << 8 : pawn >> 8) & ~(allPieces ^ pawn);
		bitboard dblPush = pawn & (secondRank | seventhRank) ? (board.sideToMove == WHITE ? push << 8 : push >> 8) & ~(allPieces ^ pawn) : 0;
		bitboard mask	 = Xs | push | dblPush;
		if (mask)
			do
				if (LS1B(mask) & (firstRank | eighthRank))
					for (int i = 1; i <= 4; i++)
						moves.push_back(Move(board, bitscan(pawn), bitscan(LS1B(mask)), i));
				else
					moves.push_back(Move(board, bitscan(pawn), bitscan(LS1B(mask))));
			while (mask ^= LS1B(mask));
	} while (pawns ^= LS1B(pawns));

	return moves;
}

std::vector<Move> MoveGen::genKnightMoves() {
	std::vector<Move> moves;
	bitboard knights	= *(board.sideToMove == WHITE ? board.W_KNIGHT : board.B_KNIGHT);
	bitboard yourPieces = board.sideToMove == WHITE ? board.whitePieces() : board.blackPieces();
	do {
		bitboard knight = LS1B(knights);
		bitboard mask	= 0;
		if (!(knight & aFile || knight & (seventhRank | eighthRank))) mask |= knight << 15;
		if (!(knight & hFile || knight & (seventhRank | eighthRank))) mask |= knight << 17;
		if (!(knight & aFile || knight & (firstRank | secondRank))) mask |= knight >> 17;
		if (!(knight & hFile || knight & (firstRank | secondRank))) mask |= knight >> 15;
		if (!(knight & eighthRank || knight & (aFile | bFile))) mask |= knight << 6;
		if (!(knight & eighthRank || knight & (gFile | hFile))) mask |= knight << 10;
		if (!(knight & firstRank || knight & (aFile | bFile))) mask |= knight >> 10;
		if (!(knight & firstRank || knight & (gFile | hFile))) mask |= knight >> 6;
		attacks[board.sideToMove] |= mask;
		mask &= ~yourPieces;

		if (mask)
			do moves.push_back(Move(board, bitscan(knight), bitscan(LS1B(mask))));
			while (mask ^= LS1B(mask));
	} while (knights ^= LS1B(knights));
	return moves;
}

bool MoveGen::isntCastlingThroughCheck(bool longCastle) {
	MoveGen mg			= MoveGen(board);
	mg.board.sideToMove = (Color)!mg.board.sideToMove;
	mg.genPseudoLegalMovesWithoutCastling();
	bitboard theirAttacks = mg.attacks[mg.board.sideToMove];

	if (board.sideToMove == WHITE && longCastle && theirAttacks & (1UL << c1 | 1UL << d1)) return false;
	if (board.sideToMove == WHITE && !longCastle && theirAttacks & (1UL << f1 | 1UL << g1)) return false;
	if (board.sideToMove == BLACK && longCastle && theirAttacks & (1UL << c8 | 1UL << d8)) return false;
	if (board.sideToMove == BLACK && !longCastle && theirAttacks & (1UL << f8 | 1UL << g8)) return false;
	return true;
}

bool MoveGen::inCheck() {
	return attacks[!board.sideToMove] & *(board.sideToMove == WHITE ? board.W_KING : board.B_KING);
}

std::vector<Move> MoveGen::genKingMoves() {
	std::vector<Move> moves;
	bitboard yourPieces = board.sideToMove == WHITE ? board.whitePieces() : board.blackPieces();
	bitboard king		= *(board.sideToMove == WHITE ? board.W_KING : board.B_KING);
	bitboard mask		= 0;
	if (!(king & eighthRank)) mask |= king << 8;
	if (!(king & firstRank)) mask |= king >> 8;
	if (!(king & hFile)) mask |= king << 1;
	if (!(king & aFile)) mask |= king >> 1;
	if (!(king & (aFile | eighthRank))) mask |= king << 7;
	if (!(king & (hFile | eighthRank))) mask |= king << 9;
	if (!(king & (hFile | firstRank))) mask |= king >> 7;
	if (!(king & (aFile | firstRank))) mask |= king >> 9;
	attacks[board.sideToMove] |= mask;
	mask &= ~yourPieces;
	if (mask)
		do moves.push_back(Move(board, bitscan(king), bitscan(LS1B(mask))));
		while (mask ^= LS1B(mask));
	return moves;
}

std::vector<Move> MoveGen::genCastlingMoves() {
	std::vector<Move> moves;
	if (inCheck()) return moves;
	bitboard king				  = board.sideToMove == WHITE ? *board.W_KING : *board.B_KING;
	unsigned short kingIndex	  = bitscan(king);
	unsigned short castlingRights = board.sideToMove == WHITE ? board.castlingRights >> 2 : board.castlingRights & 3;
	bitboard allPieces			  = board.whitePieces() | board.blackPieces();
	if (castlingRights & 2 && isntCastlingThroughCheck(false) && !(allPieces & (king << 1 | king << 2)))
		moves.push_back(Move(board, kingIndex, kingIndex + 2));
	if (castlingRights & 1 && isntCastlingThroughCheck(true) && !(allPieces & (king >> 1 | king >> 2 | king >> 3)))
		moves.push_back(Move(board, kingIndex, kingIndex - 2));
	return moves;
}

std::vector<Move> MoveGen::genSlidingPieces(bitboard pieces, bool straight, bool diagonal) {
	std::vector<Move> moves;
	if (!pieces) return moves;
	bitboard yourPieces	 = board.sideToMove == WHITE ? board.whitePieces() : board.blackPieces();
	bitboard theirPieces = board.sideToMove == BLACK ? board.whitePieces() : board.blackPieces();

	do {
		bitboard piece		  = LS1B(pieces);
		bitboard straightRays = 0;
		bitboard diagonalRays = 0;
		if (straight) {
			bitboard Nrays = piece, Erays = piece, Wrays = piece, Srays = piece;
			while (!(Nrays & eighthRank)) {
				if (yourPieces & MS1B(Nrays << 8)) break;
				Nrays |= Nrays << 8;
				if (theirPieces & MS1B(Nrays)) break;
			}

			while (!(Erays & hFile)) {
				if (yourPieces & MS1B(Erays << 1)) break;
				Erays |= Erays << 1;
				if (theirPieces & MS1B(Erays)) break;
			}

			while (!(Wrays & aFile)) {
				if (yourPieces & LS1B(Wrays >> 1)) break;
				Wrays |= Wrays >> 1;
				if (theirPieces & LS1B(Wrays)) break;
			}

			while (!(Srays & firstRank)) {
				if (yourPieces & LS1B(Srays >> 8)) break;
				Srays |= Srays >> 8;
				if (theirPieces & LS1B(Srays)) break;
			}
			straightRays = Nrays | Erays | Wrays | Srays;
		}
		if (diagonal) {
			bitboard NErays = piece, NWrays = piece, SWrays = piece, SErays = piece;
			while (!(NErays & (eighthRank | hFile))) {
				if (yourPieces & MS1B(NErays << 9)) break;
				NErays |= NErays << 9;
				if (theirPieces & MS1B(NErays)) break;
			}

			while (!(NWrays & (eighthRank | aFile))) {
				if (yourPieces & MS1B(NWrays << 7)) break;
				NWrays |= NWrays << 7;
				if (theirPieces & MS1B(NWrays)) break;
			}

			while (!(SWrays & (firstRank | aFile))) {
				if (yourPieces & LS1B(SWrays >> 9)) break;
				SWrays |= SWrays >> 9;
				if (theirPieces & LS1B(SWrays)) break;
			}

			while (!(SErays & (firstRank | hFile))) {
				if (yourPieces & LS1B(SErays >> 7)) break;
				SErays |= SErays >> 7;
				if (theirPieces & LS1B(SErays)) break;
			}
			diagonalRays = NErays | NWrays | SWrays | SErays;
		}

		bitboard rays = (straightRays | diagonalRays) ^ piece;

		attacks[board.sideToMove] |= rays;
		if (rays)
			do moves.push_back(Move(board, bitscan(piece), bitscan(LS1B(rays))));
			while (rays ^= LS1B(rays));
	} while (pieces ^= LS1B(pieces));

	return moves;
}

std::vector<Move> MoveGen::genBishopMoves() {
	bitboard bishops = *(board.sideToMove == WHITE ? board.W_BISHOP : board.B_BISHOP);
	return genSlidingPieces(bishops, false, true);
}

std::vector<Move> MoveGen::genRookMoves() {
	bitboard rooks = *(board.sideToMove == WHITE ? board.W_ROOK : board.B_ROOK);
	return genSlidingPieces(rooks, true, false);
}

std::vector<Move> MoveGen::genQueenMoves() {
	bitboard queens = *(board.sideToMove == WHITE ? board.W_QUEEN : board.B_QUEEN);
	return genSlidingPieces(queens, true, true);
}

std::vector<Move> MoveGen::genPseudoLegalMovesWithoutCastling() {
	std::vector<Move> moves;
	std::vector<Move> allMoves[] = {genPawnMoves(), genKnightMoves(), genKingMoves(), genBishopMoves(), genRookMoves(), genQueenMoves()};
	for (auto v : allMoves) moves.insert(moves.end(), v.begin(), v.end());
	return moves;
}

std::vector<Move> MoveGen::genPseudoLegalMoves() {
	std::vector<Move> moves;
	std::vector<Move> allMoves[] = {genPawnMoves(), genKnightMoves(), genKingMoves(), genBishopMoves(), genRookMoves(), genQueenMoves(), genCastlingMoves()};
	for (auto v : allMoves) moves.insert(moves.end(), v.begin(), v.end());
	return moves;
}

std::vector<Move> MoveGen::genMoves() {
	std::vector<Move> moves;
	for (Move m : genPseudoLegalMoves())
		if (!m.execute().inIllegalCheck()) moves.push_back(m);
	return moves;
}
