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
			if (((*board).blackPieces() >> (i - 7)) & 1) {
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
			if (((*board).blackPieces() >> (i - 9)) & 1) {
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
