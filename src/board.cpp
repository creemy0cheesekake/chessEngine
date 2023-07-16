#include "board.hpp"
#include "consts.hpp"
#include <iostream>

Board::Board() {}

std::string Board::stringBoard() {
	std::unordered_map<int, char> indexToChar = {
		{0, 'K'},
		{1, 'Q'},
		{2, 'R'},
		{3, 'B'},
		{4, 'N'},
		{5, 'P'},
		{6, 'k'},
		{7, 'q'},
		{8, 'r'},
		{9, 'b'},
		{10, 'n'},
		{11, 'p'},
	};

	std::string board = "";
	for (int rank = 7; rank >= 0; rank--) {
		for (int squareIndex = rank * 8; squareIndex < (rank + 1) * 8; squareIndex++) {
			char pieceChar = '.';
			for (int pieceIndex = 0; pieceIndex < 12; pieceIndex++)
				// if pieces[pieceIndex] has a piece at squareIndex
				if ((pieces[pieceIndex] >> squareIndex) & 1)
					pieceChar = indexToChar[pieceIndex];
			board = (board + pieceChar) + ' ';
		}
		board = board + '\n';
	}
	return board;
}
std::string Board::stringBoard(bool flipped) {
	std::unordered_map<int, char> indexToChar = {
		{0, 'K'},
		{1, 'Q'},
		{2, 'R'},
		{3, 'B'},
		{4, 'N'},
		{5, 'P'},
		{6, 'k'},
		{7, 'q'},
		{8, 'r'},
		{9, 'b'},
		{10, 'n'},
		{11, 'p'},
	};

	std::string board = "";
	if (flipped) {
		for (int rank = 0; rank <= 7; rank++) {
			for (int squareIndex = rank * 8 + 7; squareIndex >= rank * 8; squareIndex--) {
				char pieceChar = '.';
				for (int pieceIndex = 0; pieceIndex < 12; pieceIndex++)
					// if pieces[pieceIndex] has a piece at squareIndex
					if ((pieces[pieceIndex] >> squareIndex) & 1)
						pieceChar = indexToChar[pieceIndex];
				board = (board + pieceChar) + ' ';
			}
			board = board + '\n';
		}

	} else {
		for (int rank = 7; rank >= 0; rank--) {
			for (int squareIndex = rank * 8; squareIndex < (rank + 1) * 8; squareIndex++) {
				char pieceChar = '.';
				for (int pieceIndex = 0; pieceIndex < 12; pieceIndex++)
					// if pieces[pieceIndex] has a piece at squareIndex
					if ((pieces[pieceIndex] >> squareIndex) & 1)
						pieceChar = indexToChar[pieceIndex];
				board = (board + pieceChar) + ' ';
			}
			board = board + '\n';
		}
	}
	return board;
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
		else if (*ptr >= '1' and *ptr <= '9') writeIndex += *ptr - 48;
		else pieces[charToIndex[*ptr]] |= (1UL << writeIndex++);
	} while (*ptr++);
	sideToMove = *(++ptr) == 'w' ? WHITE : BLACK;
	ptr += 2;
	castlingRights = 0;
	while (*ptr != ' ') {
		switch (*ptr) {
			case 'K':
				castlingRights |= 1 << 3;
				break;
			case 'Q':
				castlingRights |= 1 << 2;
				break;
			case 'k':
				castlingRights |= 1 << 1;
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

bitboard Board::whitePieces() {
	return *W_PAWN | *W_KNIGHT | *W_BISHOP | *W_ROOK | *W_QUEEN | *W_KING;
}

bitboard Board::blackPieces() {
	return *B_PAWN | *B_KNIGHT | *B_BISHOP | *B_ROOK | *B_QUEEN | *B_KING;
}

void Board::reset() {
	*W_KING	  = 0x10;
	*W_QUEEN  = 0x8;
	*W_ROOK	  = 0x81;
	*W_BISHOP = 0x24;
	*W_KNIGHT = 0x42;
	*W_PAWN	  = 0xff00;
	*B_KING	  = 0x1000000000000000;
	*B_QUEEN  = 0x800000000000000;
	*B_ROOK	  = 0x8100000000000000;
	*B_BISHOP = 0x2400000000000000;
	*B_KNIGHT = 0x4200000000000000;
	*B_PAWN	  = 0xff000000000000;
}
