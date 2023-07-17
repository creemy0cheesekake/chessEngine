#include "consts.hpp"
#include "move_gen.hpp"
#include <algorithm>
#include <bitset>
#include <iostream>
#include <string>
#include <unordered_map>

MoveGen::MoveGen(Board *b)
	: board(b) {
	moves = std::vector<Move>();
}

std::vector<Move> MoveGen::genPawnMoves() {
	std::vector<Move> moves;
	bitboard pawns	   = *((*board).sideToMove == WHITE ? (*board).W_PAWN : (*board).B_PAWN);
	bitboard allPieces = (*board).whitePieces() | (*board).blackPieces();
	if ((*board).sideToMove == WHITE) {
		for (int i = 8; i < 64; i++) {
			bool pawnOnSquare = (pawns >> i) & 1;
			if (!pawnOnSquare) continue;

			// if theres no piece on the square directly in front of the pawn
			if (!((((*board).whitePieces() | (*board).blackPieces()) >> (i + 8)) & 1)) {
				Move m = Move(board, i, i + 8);

				// if target square is on last rank
				if ((i + 8) >= 56)
					for (int i = 1; i <= 4; i++) {
						m.setPromoPiece(i);
						moves.push_back(m);
					}
				else
					moves.push_back(m);
			}

			// if the pawn starts on its home square and theres no piece on the square 2 squares in front of the pawn
			if (inRange(i, 8, 15) && !(((allPieces >> (i + 8)) & 1) || ((allPieces >> (i + 16)) & 1)))
				moves.push_back(Move(board, i, i + 16));

			// if theres a capturable piece to the right
			if (((*board).blackPieces() >> (i + 9)) & 1) {
				// if its an h pawn
				if ((i + 1) % 8 == 0) continue;

				Move m = Move(board, i, i + 9);

				// if target square is on last rank
				if ((i + 9) >= 56)
					for (int i = 1; i <= 4; i++) {
						m.setPromoPiece(i);
						moves.push_back(m);
					}
				else
					moves.push_back(m);
			}
			// if theres a capturable piece to the left
			if (((*board).blackPieces() >> (i + 7)) & 1) {
				// if its an a pawn
				if (i % 8 == 0) continue;

				Move m = Move(board, i, i + 7);

				// if target square is on last rank
				if ((i + 7) >= 56)
					for (int i = 1; i <= 4; i++) {
						m.setPromoPiece(i);
						moves.push_back(m);
					}
				else
					moves.push_back(m);
			}
			if (((*board).enPassantSquare >> (i + 7)) & 1 && i % 8)
				moves.push_back(Move(board, i, i + 7));
			if (((*board).enPassantSquare >> (i + 9)) & 1 && (i + 1) % 8)
				moves.push_back(Move(board, i, i + 9));
		}
	} else {
		for (int i = 0; i < 56; i++) {
			bool pawnOnSquare = (pawns >> i) & 1;
			if (!pawnOnSquare) continue;

			// if theres no piece on the square directly in front of the pawn
			if (!((((*board).whitePieces() | (*board).blackPieces()) >> (i - 8)) & 1)) {
				Move m = Move(board, i, i - 8);

				// if target square is on last rank
				if ((i - 8) < 8)
					for (int i = 1; i <= 4; i++) {
						m.setPromoPiece(i);
						moves.push_back(m);
					}
				else
					moves.push_back(m);
			}

			// if the pawn starts on its home square and theres no piece on the square 2 squares in front of the pawn
			if (inRange(i, 48, 55) && !(((allPieces >> (i - 8)) & 1) || ((allPieces >> (i - 16)) & 1)))
				moves.push_back(Move(board, i, i - 16));

			// if theres a capturable piece to the right
			if (((*board).whitePieces() >> (i - 7)) & 1) {
				// if its an h pawn
				if ((i + 1) % 8 == 0) continue;

				Move m = Move(board, i, i - 7);

				// if target square is on last rank
				if ((i - 7) >= 56)
					for (int i = 1; i <= 4; i++) {
						m.setPromoPiece(i);
						moves.push_back(m);
					}
				else
					moves.push_back(m);
			}
			// if theres a capturable piece to the left
			if (((*board).whitePieces() >> (i - 9)) & 1) {
				// if its an a pawn
				if (i % 8 == 0) continue;

				Move m = Move(board, i, i - 9);

				// if target square is on last rank
				if ((i - 9) >= 56)
					for (int i = 1; i <= 4; i++) {
						m.setPromoPiece(i);
						moves.push_back(m);
					}
				else
					moves.push_back(m);
			}
			if (((*board).enPassantSquare >> (i - 9)) & 1 && i % 8)
				moves.push_back(Move(board, i, i - 9));
			if (((*board).enPassantSquare >> (i - 7)) & 1 && (i + 1) % 8)
				moves.push_back(Move(board, i, i - 7));
		}
	}
	return moves;
}

std::vector<Move> MoveGen::genKnightMoves() {
	std::vector<Move> moves;
	bitboard knights	= *((*board).sideToMove == WHITE ? (*board).W_KNIGHT : (*board).B_KNIGHT);
	bitboard yourPieces = (*board).sideToMove == WHITE ? (*board).whitePieces() : (*board).blackPieces();
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

std::vector<Move> MoveGen::genKingMoves() {
	std::vector<Move> moves;
	// index of king
	int king			= bitscan((*board).sideToMove == WHITE ? *(*board).W_KING : *(*board).B_KING);
	int rank			= king / 8;
	bitboard yourPieces = (*board).sideToMove == WHITE ? (*board).whitePieces() : (*board).blackPieces();

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

std::vector<Move> genSlidingPieces(bitboard pieces, Board *board, bool straight, bool diagonal) {
	std::vector<Move> moves;
	if (!pieces) return moves;
	bitboard yourPieces	 = (*board).sideToMove == WHITE ? (*board).whitePieces() : (*board).blackPieces();
	bitboard theirPieces = (*board).sideToMove == BLACK ? (*board).whitePieces() : (*board).blackPieces();

	do {
		bitboard piece		  = LS1B(pieces);
		bitboard straightRays = 0;
		bitboard diagonalRays = 0;
		if (straight) {
			bitboard Nrays = piece, Erays = piece, Wrays = piece, Srays = piece;
			while (!(Nrays & 0xff00000000000000)) {
				if (yourPieces & MS1B(Nrays << 8)) break;
				Nrays |= Nrays << 8;
				if (theirPieces & MS1B(Nrays)) break;
			}

			while (!(Erays & 0x8080808080808080)) {
				if (yourPieces & MS1B(Erays << 1)) break;
				Erays |= Erays << 1;
				if (theirPieces & MS1B(Erays)) break;
			}

			while (!(Wrays & 0x101010101010101)) {
				if (yourPieces & LS1B(Wrays >> 1)) break;
				Wrays |= Wrays >> 1;
				if (theirPieces & LS1B(Wrays)) break;
			}

			while (!(Srays & 0xff)) {
				if (yourPieces & LS1B(Srays >> 8)) break;
				Srays |= Srays >> 8;
				if (theirPieces & LS1B(Srays)) break;
			}
			straightRays = Nrays | Erays | Wrays | Srays;
		}
		if (diagonal) {
			bitboard NErays = piece, NWrays = piece, SWrays = piece, SErays = piece;
			while (!(NErays & 0xff80808080808080)) {
				if (yourPieces & MS1B(NErays << 9)) break;
				NErays |= NErays << 9;
				if (theirPieces & MS1B(NErays)) break;
			}

			while (!(NWrays & 0xff01010101010101)) {
				if (yourPieces & MS1B(NWrays << 7)) break;
				NWrays |= NWrays << 7;
				if (theirPieces & MS1B(NWrays)) break;
			}

			while (!(SWrays & 0x1010101010101ff)) {
				if (yourPieces & LS1B(SWrays >> 9)) break;
				SWrays |= SWrays >> 9;
				if (theirPieces & LS1B(SWrays)) break;
			}

			while (!(SErays & 0x80808080808080ff)) {
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
	bitboard bishops = *((*board).sideToMove == WHITE ? (*board).W_BISHOP : (*board).B_BISHOP);
	return genSlidingPieces(bishops, board, false, true);
}

std::vector<Move> MoveGen::genRookMoves() {
	bitboard rooks = *((*board).sideToMove == WHITE ? (*board).W_ROOK : (*board).B_ROOK);
	return genSlidingPieces(rooks, board, true, false);
}

std::vector<Move> MoveGen::genQueenMoves() {
	bitboard queens = *((*board).sideToMove == WHITE ? (*board).W_QUEEN : (*board).B_QUEEN);
	return genSlidingPieces(queens, board, true, true);
}

std::vector<Move> MoveGen::genMoves() {
	std::vector<Move> allMoves[] = {genPawnMoves(), genKnightMoves(), genKingMoves(), genBishopMoves(), genRookMoves(), genQueenMoves()};
	for (auto v : allMoves) moves.insert(moves.end(), v.begin(), v.end());
	return moves;
}
