#ifndef CONSTS_H
#define CONSTS_H

#include <array>
#include <bit>
#include <cstdint>
#include <unordered_map>
#include <vector>

class Move;
using Moves		 = std::vector<Move>;
using Bitboard	 = uint64_t;
using Centipawns = int16_t;

/**
* @brief counts the number of trailing zeros in binary
*/
constexpr inline int bitscan(Bitboard x) {
	return std::__countr_zero(x);
}

/**
* @brief bitscan from the other side
*/
constexpr inline int reverseBitscan(Bitboard x) {
	return x ? 63 - __builtin_clzll(x) : 0;
}

/**
* @brief centipawn constants for needed values.
* cant use float infinity because we still want to add and subtract to checkmate scores
*/
constexpr Centipawns INF_SCORE		 = 32000;
constexpr Centipawns CHECKMATE_SCORE = 30000;
constexpr Centipawns NONE_SCORE		 = -32001;
constexpr Centipawns DRAW_SCORE		 = 0;

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

/**
* @brief both chess piece colors
*/
enum Color {
	WHITE,
	BLACK,
};

/**
* @brief all the chess pieces and a none piece
*/
enum Piece {
	KING,
	QUEEN,
	ROOK,
	BISHOP,
	KNIGHT,
	PAWN,
	NONE_PIECE,
};

/**
* @brief chess squares in algebraic notation
*/
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

/**
* @brief move bitflags
*/
enum MoveFlag {
	NORMAL_MOVE = 0b0000000,  // 0
	CAPTURE		= 0b0000001,  // 1
	KS_CASTLE	= 0b0000010,  // 2
	QS_CASTLE	= 0b0000100,  // 4
	DBL_PAWN	= 0b0001000,  // 8
	EN_PASSANT	= 0b0010000,  // 16
	PROMOTION	= 0b0100000,  // 32
	PAWN_MOVE	= 0b1000000,  // 64
};

/**
* @brief directions for piece ray lookup tables
*/
enum DirectionStraight {
	NORTH,
	SOUTH,
	EAST,
	WEST,
};
enum DirectionDiagonal {
	NORTHEAST,
	NORTHWEST,
	SOUTHEAST,
	SOUTHWEST,
};

/**
* @brief bit flag enum for straight and diagonal directions for sliding move functions
*/
enum SlidingPieceDirectionFlags {
	STRAIGHT = 0b01,
	DIAGONAL = 0b10,
};

/**
* @brief centipawn values for each piece type
*/
inline std::unordered_map<Piece, Centipawns> pieceToCentipawns = {
	{QUEEN, 900},
	{ROOK, 500},
	{BISHOP, 310},
	{KNIGHT, 300},
	{PAWN, 100},
};

/**
* @brief convert piece to notation for algebraic notation
*/
inline std::unordered_map<Piece, char> pieceToNotationChar = {
	{KING, 'K'},
	{QUEEN, 'Q'},
	{ROOK, 'R'},
	{BISHOP, 'B'},
	{KNIGHT, 'N'},
};

/**
* @brief convert piece to notation for uci notation notation
*/
inline std::unordered_map<Piece, char> promotionPieceToUCINotationChar = {
	{QUEEN, 'q'},
	{ROOK, 'r'},
	{BISHOP, 'b'},
	{KNIGHT, 'n'},
};

/**
* @brief convert char from fen string to piece
*/
inline std::unordered_map<char, Piece> fenPieceChartoPieceType = {
	{'K', KING},
	{'Q', QUEEN},
	{'R', ROOK},
	{'B', BISHOP},
	{'N', KNIGHT},
	{'P', PAWN},
};

/**
* @brief index to ascii piece for ascii board representation
*/
constexpr std::array<const char*, 12> indexToPiece = {"♚ ", "♛ ", "♜ ", "♝ ", "♞ ", "♟︎ ", "♔ ", "♕ ", "♖ ", "♗ ", "♘ ", "♙ "};

#endif
