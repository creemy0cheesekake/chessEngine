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
		Xs &= theirPieces | board.enPassantSquare;
		bitboard push	 = (board.sideToMove == WHITE ? pawn << 8 : pawn >> 8) & ~(allPieces ^ pawn);
		bitboard dblPush = pawn & (secondRank | seventhRank) ? (board.sideToMove == WHITE ? push << 8 : push >> 8) & ~(allPieces ^ pawn) : 0;
		bitboard mask	 = Xs | push | dblPush;
		if (mask)
			do
				if (LS1B(mask) & (firstRank | eightRank))
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
	for (int i = 0; i < 64; i++) {
		if (!((knights >> i) & 1)) continue;

		int rank		   = i / 8;
		int moveOffsets1[] = {15, 17};
		for (int offset : moveOffsets1) {
			if (inRange(i + offset, 0, 63) && ((i + offset) / 8) - rank == 2 && ~(yourPieces >> (i + offset)) & 1)
				moves.push_back(Move(board, i, i + offset));
			if (inRange(i - offset, 0, 63) && ((i - offset) / 8) - rank == -2 && ~(yourPieces >> (i - offset)) & 1)
				moves.push_back(Move(board, i, i - offset));
		}

		int moveOffsets2[] = {6, 10};
		for (int offset : moveOffsets2) {
			if (inRange(i + offset, 0, 63) && ((i + offset) / 8) - rank == 1 && ~(yourPieces >> (i + offset)) & 1)
				moves.push_back(Move(board, i, i + offset));
			if (inRange(i - offset, 0, 63) && ((i - offset) / 8) - rank == -1 && ~(yourPieces >> (i - offset)) & 1)
				moves.push_back(Move(board, i, i - offset));
		}
	}
	return moves;
}

bool MoveGen::isntCastlingThroughCheck(bool longCastle) {
	Board cpy									  = board;
	cpy.pieces[board.sideToMove == WHITE ? 0 : 6] = 0;
	cpy.sideToMove								  = (Color)!board.sideToMove;
	for (Move m : MoveGen(cpy).genPseudoLegalMovesWithoutCastling()) {
		if (board.sideToMove == WHITE) {
			if (longCastle) {
				if (
					(m.execute().blackPieces() >> c1) & 1 ||
					(m.execute().blackPieces() >> d1) & 1 ||
					(m.execute().blackPieces() >> e1) & 1
				)
					return false;
			} else {
				if (
					(m.execute().blackPieces() >> e1) & 1 ||
					(m.execute().blackPieces() >> f1) & 1 ||
					(m.execute().blackPieces() >> g1) & 1
				)
					return false;
			}
		} else {
			if (longCastle) {
				if (
					(m.execute().whitePieces() >> c8) & 1 ||
					(m.execute().whitePieces() >> d8) & 1 ||
					(m.execute().whitePieces() >> e8) & 1
				)
					return false;
			} else {
				if (
					(m.execute().whitePieces() >> e8) & 1 ||
					(m.execute().whitePieces() >> f8) & 1 ||
					(m.execute().whitePieces() >> g8) & 1
				)
					return false;
			}
		}
	}
	return true;
}

bool MoveGen::inCheck() {
	Board cpy	   = board;
	cpy.sideToMove = (Color)!board.sideToMove;
	for (Move m : MoveGen(cpy).genPseudoLegalMovesWithoutCastling()) {
		Board b = m.execute();
		if (!*b.W_KING || !*b.B_KING) return true;
	}
	return false;
}

std::vector<Move> MoveGen::genKingMoves() {
	std::vector<Move> moves;
	int king			= bitscan(board.sideToMove == WHITE ? *board.W_KING : *board.B_KING);
	int rank			= king / 8;
	bitboard yourPieces = board.sideToMove == WHITE ? board.whitePieces() : board.blackPieces();

	int moveOffsets1[] = {7, 8, 9};
	for (int offset : moveOffsets1) {
		if (inRange(king + offset, 0, 63) && ((king + offset) / 8) - rank == 1 && ~(yourPieces >> (king + offset)) & 1)
			moves.push_back(Move(board, king, king + offset));
		if (inRange(king - offset, 0, 63) && ((king - offset) / 8) - rank == -1 && ~(yourPieces >> (king - offset)) & 1)
			moves.push_back(Move(board, king, king - offset));
	}

	if (inRange(king + 1, 0, 63) && ((king + 1) / 8) - rank == 0 && ~(yourPieces >> (king + 1)) & 1)
		moves.push_back(Move(board, king, king + 1));
	if (inRange(king - 1, 0, 63) && ((king - 1) / 8) - rank == 0 && ~(yourPieces >> (king - 1)) & 1)
		moves.push_back(Move(board, king, king - 1));
	return moves;
}

std::vector<Move> MoveGen::genCastlingMoves() {
	std::vector<Move> moves;
	if (inCheck()) return moves;
	int king					  = bitscan(board.sideToMove == WHITE ? *board.W_KING : *board.B_KING);
	unsigned short castlingRights = board.sideToMove == WHITE ? board.castlingRights >> 2 : board.castlingRights & 3;
	bitboard allPieces			  = board.whitePieces() | board.blackPieces();
	if (castlingRights & 2 && isntCastlingThroughCheck(false) && !((allPieces >> (king + 1)) & 1) && !((allPieces >> (king + 2)) & 1)) {
		moves.push_back(Move(board, king, king + 2));
	}
	if (castlingRights & 1 && isntCastlingThroughCheck(true) && !((allPieces >> (king - 1)) & 1) && !((allPieces >> (king - 2)) & 1) && !((allPieces >> (king - 3)) & 1)) {
		moves.push_back(Move(board, king, king - 2));
	}
	return moves;
}

std::vector<Move> genSlidingPieces(bitboard pieces, Board board, bool straight, bool diagonal) {
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
			while (!(Nrays & eightRank)) {
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
			while (!(NErays & (eightRank | hFile))) {
				if (yourPieces & MS1B(NErays << 9)) break;
				NErays |= NErays << 9;
				if (theirPieces & MS1B(NErays)) break;
			}

			while (!(NWrays & (eightRank | aFile))) {
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

		if (rays)
			do moves.push_back(Move(board, bitscan(piece), bitscan(LS1B(rays))));
			while (rays ^= LS1B(rays));
	} while (pieces ^= LS1B(pieces));

	return moves;
}

std::vector<Move> MoveGen::genBishopMoves() {
	bitboard bishops = *(board.sideToMove == WHITE ? board.W_BISHOP : board.B_BISHOP);
	return genSlidingPieces(bishops, board, false, true);
}

std::vector<Move> MoveGen::genRookMoves() {
	bitboard rooks = *(board.sideToMove == WHITE ? board.W_ROOK : board.B_ROOK);
	return genSlidingPieces(rooks, board, true, false);
}

std::vector<Move> MoveGen::genQueenMoves() {
	bitboard queens = *(board.sideToMove == WHITE ? board.W_QUEEN : board.B_QUEEN);
	return genSlidingPieces(queens, board, true, true);
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
