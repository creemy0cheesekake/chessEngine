#include "board.hpp"
#include "consts.hpp"
#include "move.hpp"
#include "move_gen.hpp"

Board::Board() {}

std::ostream &operator<<(std::ostream &os, const Board &b) {
	std::unordered_map<int, std::string> indexToPiece = {
		{0, "♚ "},
		{1, "♛ "},
		{2, "♜ "},
		{3, "♝ "},
		{4, "♞ "},
		{5, "♟︎ "},
		{6, "♔ "},
		{7, "♕ "},
		{8, "♖ "},
		{9, "♗ "},
		{10, "♘ "},
		{11, "♙ "},
	};

	std::string board = "";
	for (int rank = 7; rank >= 0; rank--) {
		for (int squareIndex = rank * 8; squareIndex < (rank + 1) * 8; squareIndex++) {
			std::string pieceString = ". ";
			for (int pieceIndex = 0; pieceIndex < 12; pieceIndex++)
				if ((b.pieces[pieceIndex] >> squareIndex) & 1)
					pieceString = indexToPiece[pieceIndex];
			board += pieceString;
		}
		board = board + '\n';
	}
	os << board;
	return os;
}

void Board::setToFen(std::string fen) {
	std::unordered_map<char, int> charToIndex = {
		{'K', 0},
		{'Q', 1},
		{'R', 2},
		{'B', 3},
		{'N', 4},
		{'P', 5},
		{'k', 6},
		{'q', 7},
		{'r', 8},
		{'b', 9},
		{'n', 10},
		{'p', 11},
	};
	for (int i = 0; i < 12; i++) pieces[i] = 0;
	int writeIndex = 56;
	char *ptr	   = &fen[0];
	do {
		if (*ptr == ' ') break;

		if (*ptr == '/') writeIndex -= 16;
		else if (inRange(*ptr, '1', '9')) writeIndex += *ptr - 48;
		else pieces[charToIndex[*ptr]] |= (1UL << writeIndex++);
	} while (*ptr++);
	sideToMove = *(++ptr) == 'w' ? WHITE : BLACK;
	ptr += 2;
	castlingRights = 0;
	while (*ptr != ' ') {
		switch (*ptr) {
			case 'K':
				castlingRights |= 8;
				break;
			case 'Q':
				castlingRights |= 4;
				break;
			case 'k':
				castlingRights |= 2;
				break;
			case 'q':
				castlingRights |= 1;
				break;
		}
		ptr++;
	}
	if (*(++ptr) != '-') {
		enPassantSquare = *ptr++ - 97;
		enPassantSquare += 8 * (*ptr - 49);
		enPassantSquare = 1UL << enPassantSquare;
	}
	ptr += 2;
	hmClock = *ptr - 48;
	ptr += 2;
	fmClock = *ptr - 48;
}

bool Board::inIllegalCheck() {
	Board b		  = *this;
	b.sideToMove  = (Color)!b.sideToMove;
	bitboard king = sideToMove == WHITE ? b.pieces[B_KING] : b.pieces[W_KING];
	return king & MoveGen(b).getAttacks();
}

bitboard Board::whitePieces() {
	return pieces[W_PAWN] | pieces[W_KNIGHT] | pieces[W_BISHOP] | pieces[W_ROOK] | pieces[W_QUEEN] | pieces[W_KING];
}

bitboard Board::blackPieces() {
	return pieces[B_PAWN] | pieces[B_KNIGHT] | pieces[B_BISHOP] | pieces[B_ROOK] | pieces[B_QUEEN] | pieces[B_KING];
}

void Board::reset() {
	pieces[W_KING]	 = 0x10;
	pieces[W_QUEEN]	 = 0x8;
	pieces[W_ROOK]	 = 0x81;
	pieces[W_BISHOP] = 0x24;
	pieces[W_KNIGHT] = 0x42;
	pieces[W_PAWN]	 = 0xff00;
	pieces[B_KING]	 = 0x1000000000000000;
	pieces[B_QUEEN]	 = 0x800000000000000;
	pieces[B_ROOK]	 = 0x8100000000000000;
	pieces[B_BISHOP] = 0x2400000000000000;
	pieces[B_KNIGHT] = 0x4200000000000000;
	pieces[B_PAWN]	 = 0xff000000000000;
}
