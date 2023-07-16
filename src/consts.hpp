#ifndef DEFS_H
#define DEFS_H

typedef unsigned long long bitboard;
#define bitscan std::__countr_zero

enum Color {
	WHITE,
	BLACK,
};

enum Piece {
	KING,
	QUEEN,
	ROOK,
	BISHOP,
	KNIGHT,
	PAWN,
};

#endif
