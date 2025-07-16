#include "board.hpp"
#include "move_gen.hpp"
#include "util.hpp"
#include <unordered_map>

Board::Board() {}

std::string Board::printBoard() const {
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
			for (int colorIndex = 0; colorIndex < 2; colorIndex++)
				for (int pieceIndex = 0; pieceIndex < 6; pieceIndex++)
					if ((pieces[colorIndex][pieceIndex] >> squareIndex) & 1)
						pieceString = indexToPiece[6 * colorIndex + pieceIndex];
			board += pieceString;
		}
		board = board + '\n';
	}
	return board;
}

std::ostream &operator<<(std::ostream &os, const Board &b) {
	os << b.printBoard();
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
	};
	for (int i = 0; i < 6; i++) pieces[WHITE][i] = 0;
	for (int i = 0; i < 6; i++) pieces[BLACK][i] = 0;
	int writeIndex = 56;
	char *ptr	   = &fen[0];
	do {
		if (*ptr == ' ') break;

		if (*ptr == '/') writeIndex -= 16;
		else if (inRange(*ptr, '1', '9')) writeIndex += *ptr - 48;
		else pieces[!!islower(*ptr)][charToIndex[toupper(*ptr)]] |= (1UL << writeIndex++);
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
	bitboard king = b.pieces[b.sideToMove][KING];
	return king & MoveGen(b).getAttacks();
}

bitboard Board::whitePieces() {
	return pieces[WHITE][PAWN] | pieces[WHITE][KNIGHT] | pieces[WHITE][BISHOP] | pieces[WHITE][ROOK] | pieces[WHITE][QUEEN] | pieces[WHITE][KING];
}

bitboard Board::blackPieces() {
	return pieces[BLACK][PAWN] | pieces[BLACK][KNIGHT] | pieces[BLACK][BISHOP] | pieces[BLACK][ROOK] | pieces[BLACK][QUEEN] | pieces[BLACK][KING];
}

void Board::reset() {
	pieces[WHITE][KING]	  = 0x10;
	pieces[WHITE][QUEEN]  = 0x8;
	pieces[WHITE][ROOK]	  = 0x81;
	pieces[WHITE][BISHOP] = 0x24;
	pieces[WHITE][KNIGHT] = 0x42;
	pieces[WHITE][PAWN]	  = 0xff00;
	pieces[BLACK][KING]	  = 0x1000000000000000;
	pieces[BLACK][QUEEN]  = 0x800000000000000;
	pieces[BLACK][ROOK]	  = 0x8100000000000000;
	pieces[BLACK][BISHOP] = 0x2400000000000000;
	pieces[BLACK][KNIGHT] = 0x4200000000000000;
	pieces[BLACK][PAWN]	  = 0xff000000000000;
}
