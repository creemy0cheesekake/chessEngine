#ifndef CONSTS_H
#define CONSTS_H

#include <bit>
#include <limits>

using bitboard = unsigned long long;
#define bitscan std::__countr_zero
#define INF		std::numeric_limits<float>::infinity()

#define firstRank	0xff
#define secondRank	0xff00
#define thirdRank	0xff0000
#define fourthRank	0xff000000
#define fifthRank	0xff00000000
#define sixthRank	0xff0000000000
#define seventhRank 0xff000000000000
#define eighthRank	0xff00000000000000
#define aFile		0x0101010101010101
#define bFile		0x0202020202020202
#define cFile		0x0404040404040404
#define dFile		0x0808080808080808
#define eFile		0x1010101010101010
#define fFile		0x2020202020202020
#define gFile		0x4040404040404040
#define hFile		0x8080808080808080

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
	NONE_PIECE,
};

// clang-format off
enum Square {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8,
	NONE_SQUARE
};
// clang-format on

#endif
