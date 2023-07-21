#include "consts.hpp"
#include "util.cpp"
#include "move.hpp"
#include <algorithm>
#include <bitset>
#include <iostream>
#include <string>
#include <unordered_map>

Move::Move(Board b, unsigned int f, unsigned int t) {
	from  = f;
	to	  = t;
	board = b;

	flags = genFlags(board);
}

Move::Move(Board b, unsigned int f, unsigned int t, unsigned int pP) {
	from	   = f;
	to		   = t;
	promoPiece = pP;
	board	   = b;

	flags = genFlags(board);
}

unsigned int Move::genFlags(Board board) {
	unsigned int flags = 0;
	if (board.sideToMove == WHITE) {
		if ((board.blackPieces() >> to) & 1)
			flags += CAPTURE;
		if ((*board.W_KING >> 4) & 1 && from == 4 && to == 6)
			flags += KS_CASTLE;
		if ((*board.W_KING >> 4) & 1 && from == 4 && to == 2)
			flags += QS_CASTLE;
		if ((*board.W_PAWN >> from) & 1 && inRange(from, 8, 15) && inRange(to, 24, 31))
			flags += DBL_PAWN;
		if ((board.enPassantSquare >> to) & 1 && (*board.W_PAWN >> from) & 1)
			flags += (EN_PASSANT + CAPTURE);
		if ((*board.W_PAWN >> from) & 1 && inRange(to, 56, 63))
			flags += PROMOTION;
		if ((*board.W_PAWN >> from) & 1)
			flags += PAWN_MOVE;
	} else {
		if ((board.whitePieces() >> to) & 1)
			flags += CAPTURE;
		if ((*board.B_KING >> 60) & 1 && from == 60 && to == 62)
			flags += KS_CASTLE;
		if ((*board.B_KING >> 60) & 1 && from == 60 && to == 58)
			flags += QS_CASTLE;
		if ((*board.B_PAWN >> from) & 1 && inRange(from, 48, 55) && inRange(to, 32, 39))
			flags += DBL_PAWN;
		if ((board.enPassantSquare >> to) & 1 && (*board.B_PAWN >> from) & 1)
			flags += (EN_PASSANT + CAPTURE);
		if ((*board.B_PAWN >> from) & 1 && inRange(to, 0, 7))
			flags += PROMOTION;
		if ((*board.B_PAWN >> from) & 1)
			flags += PAWN_MOVE;
	}
	return flags;
}

unsigned int Move::getFrom() {
	return from;
}

unsigned int Move::getTo() {
	return to;
}

unsigned int Move::getFlags() {
	return flags;
}

void Move::setPromoPiece(unsigned int piece) {
	promoPiece = piece;
}

std::string Move::notation() {
	if (flags & KS_CASTLE) return "0-0";
	if (flags & QS_CASTLE) return "0-0-0";
	std::unordered_map<int, char> indexToChar = {
		{0, 'K'},
		{1, 'Q'},
		{2, 'R'},
		{3, 'B'},
		{4, 'N'},
	};
	std::string notation = "";
	for (int i = 0; i < 12; i++) {
		int charIndex = -1;
		if ((board.pieces[i] >> from) & 1) {
			charIndex = i % 6;
			if (charIndex < 5) notation += indexToChar[charIndex];
			break;
		}
	}
	notation += from % 8 + 97;
	notation += from / 8 + 49;
	notation += flags & CAPTURE ? "x" : "-";
	notation += to % 8 + 97;
	notation += to / 8 + 49;
	if (flags & PROMOTION) {
		notation += "=";
		std::unordered_map<int, char> numToPromoPiece = {
			{1, 'Q'},
			{2, 'R'},
			{3, 'B'},
			{4, 'N'},
		};
		notation += numToPromoPiece[promoPiece];
	}
	return notation;
}

std::string Move::UCInotation() {
	std::string notation = "";
	notation += from % 8 + 97;
	notation += from / 8 + 49;
	notation += to % 8 + 97;
	notation += to / 8 + 49;
	if (flags & PROMOTION)
		notation += (std::unordered_map<int, char>){
			{1, 'q'},
			{2, 'r'},
			{3, 'b'},
			{4, 'n'},
		}[promoPiece];
	return notation;
}

Board Move::execute() {
	Board b = board;
	if (flags & KS_CASTLE) {
		if (b.sideToMove == WHITE) {
			*b.W_KING ^= (1UL << 6) + (1UL << 4);
			*b.W_ROOK ^= (1UL << 7) + (1UL << 5);
		} else {
			*b.B_KING ^= (1UL << 62) + (1UL << 60);
			*b.B_ROOK ^= (1UL << 63) + (1UL << 61);
		}
	} else if (flags & QS_CASTLE) {
		if (b.sideToMove == WHITE) {
			*b.W_KING ^= (1UL << 4) + (1UL << 2);
			*b.W_ROOK ^= (1UL << 3) + 1;
		} else {
			*b.B_KING ^= (1UL << 60) + (1UL << 58);
			*b.B_ROOK ^= (1UL << 59) + (1UL << 56);
		}
	} else {
		// removes piece from the destination square if theres one there
		if (flags & EN_PASSANT) {
			for (int i = 0; i < 12; i++)
				b.pieces[i] &= ~(1UL << (to + (b.sideToMove == WHITE ? -8 : 8)));
		} else {
			for (int i = 0; i < 12; i++)
				b.pieces[i] &= ~(1UL << to);
		}
		// moves the piece from its current square to the destination square
		for (int i = 0; i < 12; i++)
			if ((b.pieces[i] >> from) & 1) {
				bitboard moveMask = (1UL << from);
				if (!(flags & PROMOTION)) moveMask += (1UL << to);
				b.pieces[i] ^= moveMask;
			}
		if (flags & PROMOTION)
			b.pieces[promoPiece + (b.sideToMove == WHITE ? 0 : 6)] |= 1UL << to;
	}

	b = updateGameData(b);
	return b;
}

Board Move::updateGameData(Board b) {
	b.enPassantSquare = 0;
	b.hmClock++;
	if (b.sideToMove == BLACK)
		b.fmClock++;
	if (flags & PAWN_MOVE || flags & CAPTURE)
		b.hmClock = 0;

	// updates en passant square if dbl pawn push
	if (flags & DBL_PAWN)
		b.enPassantSquare = 1UL << (b.sideToMove == WHITE ? to - 8 : to + 8);

	b.castlingRights = updateCastlingRights(b);

	// switching side to move
	b.sideToMove = (Color)!b.sideToMove;
	return b;
}

unsigned short Move::updateCastlingRights(Board b) {
	// if either side castles remove all their castling rights
	if (b.sideToMove == WHITE) {
		if (flags & KS_CASTLE || flags & QS_CASTLE) b.castlingRights &= 3;
	} else if (flags & KS_CASTLE || flags & QS_CASTLE) b.castlingRights &= ~3;

	// if the kings move remove all their castling rights
	if ((*b.W_KING >> to) & 1)
		b.castlingRights &= 3;
	if ((*b.B_KING >> to) & 1)
		b.castlingRights &= ~3;

	// if the rooks move remove castling rights for that side
	if ((*b.W_ROOK >> to) & 1 && from == 7)
		b.castlingRights &= ~(1 << 3);
	if ((*b.W_ROOK >> to) & 1 && from == 0)
		b.castlingRights &= ~(1 << 2);
	if ((*b.B_ROOK >> to) & 1 && from == 63)
		b.castlingRights &= ~(1 << 1);
	if ((*b.B_ROOK >> to) & 1 && from == 56)
		b.castlingRights &= ~1;

	// if the rooks are captured remove castling rights for that side
	if (flags & CAPTURE) {
		if (to == 7)
			b.castlingRights &= ~(1 << 3);
		if (to == 0)
			b.castlingRights &= ~(1 << 2);
		if (to == 63)
			b.castlingRights &= ~(1 << 1);
		if (to == 56)
			b.castlingRights &= ~1;
	}
	return b.castlingRights;
}
