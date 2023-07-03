#include "consts.hpp"
#include "util.cpp"
#include "move.hpp"
#include <algorithm>
#include <bitset>
#include <iostream>
#include <string>
#include <unordered_map>

Move::Move(Board &b, unsigned int f, unsigned int t)
	: board(b) {
	from = f;
	to	 = t;

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
		if ((board.enPassantSquare >> to) & 1)
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
		if ((board.enPassantSquare >> to) & 1)
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

void Move::execute() {
	// removes piece from the destination square if theres one there
	for (int i = 0; i < 12; i++)
		board.pieces[i] &= ~(0 | 1UL << to);
	// moves the piece from its current square to the destination square
	for (int i = 0; i < 12; i++)
		if ((board.pieces[i] >> from) & 1)
			board.pieces[i] ^= (1UL << to) + (1UL << from);

	updateGameData();
}

void Move::updateGameData() {
	board.hmClock++;
	if (board.sideToMove == BLACK)
		board.fmClock++;
	if (flags & PAWN_MOVE || flags & CAPTURE)
		board.hmClock = 0;

	// updates en passant square if dbl pawn push
	if (flags & DBL_PAWN)
		board.enPassantSquare = 1UL << (board.sideToMove == WHITE ? to - 8 : to + 8);

	updateCastlingRights();

	// switching side to move
	board.sideToMove = board.sideToMove ? WHITE : BLACK;
}

void Move::updateCastlingRights() {
	// if either side castles remove all their castling rights
	if (board.sideToMove == WHITE) {
		if (flags & KS_CASTLE || flags & QS_CASTLE) board.castlingRights &= 3;
	} else if (flags & KS_CASTLE || flags & QS_CASTLE) board.castlingRights &= ~3;

	// if the kings move remove all their castling rights
	if ((*board.W_KING >> to) & 1 && ((board.castlingRights >> 3) & 1 || (board.castlingRights >> 2) & 1))
		board.castlingRights &= 3;
	if ((*board.B_KING >> to) & 1 && ((board.castlingRights >> 1) & 1 || (board.castlingRights) & 1))
		board.castlingRights &= ~3;

	// if the rooks move remove castling rights for that side
	if ((*board.W_ROOK >> to) & 1 && from == 7)
		board.castlingRights &= ~(1 << 3);
	if ((*board.W_ROOK >> to) & 1 && from == 0)
		board.castlingRights &= ~(1 << 2);
	if ((*board.B_ROOK >> to) & 1 && from == 63)
		board.castlingRights &= ~(1 << 1);
	if ((*board.W_ROOK >> to) & 1 && from == 56)
		board.castlingRights &= ~1;
}
