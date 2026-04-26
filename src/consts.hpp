#ifndef CONSTS_H
#define CONSTS_H

#include <array>
#include <bit>
#include <cstdint>
#include <unordered_map>
#include <vector>

class Move;
using Moves		  = std::vector<Move>;
using Bitboard	  = uint64_t;
using Centipawns  = int16_t;
using MoveScore	  = int16_t;
using ZobristHash = uint64_t;

/**
* @brief counts the number of trailing zeros in binary
*/
constexpr inline int bitscan(Bitboard x) {
	return std::countr_zero(x);
}

/**
* @brief bitscan from the other side
*/
constexpr inline int reverseBitscan(Bitboard x) {
	return x ? std::bit_width(x) - 1 : 0;
}

/**
* @brief centipawn constants for needed values.
* cant use float infinity because we still want to add and subtract to checkmate scores
*/
constexpr Centipawns INF_SCORE		 = 32000;
constexpr Centipawns CHECKMATE_SCORE = 30000;
constexpr Centipawns NONE_SCORE		 = -32001;
constexpr Centipawns DRAW_SCORE		 = 0;

constexpr MoveScore KILLER_MOVE_1_SCORE = 99;
constexpr MoveScore KILLER_MOVE_2_SCORE = 98;
constexpr MoveScore MAX_MOVE_SCORE		= 999;

constexpr Bitboard firstRank   = 0xff;
constexpr Bitboard secondRank  = 0xff00;
constexpr Bitboard thirdRank   = 0xff0000;
constexpr Bitboard fourthRank  = 0xff000000;
constexpr Bitboard fifthRank   = 0xff00000000;
constexpr Bitboard sixthRank   = 0xff0000000000;
constexpr Bitboard seventhRank = 0xff000000000000;
constexpr Bitboard eighthRank  = 0xff00000000000000;
constexpr Bitboard aFile	   = 0x0101010101010101;
constexpr Bitboard bFile	   = 0x0202020202020202;
constexpr Bitboard cFile	   = 0x0404040404040404;
constexpr Bitboard dFile	   = 0x0808080808080808;
constexpr Bitboard eFile	   = 0x1010101010101010;
constexpr Bitboard fFile	   = 0x2020202020202020;
constexpr Bitboard gFile	   = 0x4040404040404040;
constexpr Bitboard hFile	   = 0x8080808080808080;

/**
* @brief both chess piece colors
*/
enum Color : uint8_t {
	WHITE = 0,
	BLACK = 1,
};

/**
* @brief all the chess pieces and a none piece
*/
enum Piece : uint8_t {
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
enum Square : uint8_t {
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
enum MoveFlag : uint8_t {
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
* @brief states for search process
*/
enum SearchState {
	SEARCH_ABORTED,
	SEARCH_COMPLETE,
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
inline constexpr std::array<Centipawns, NONE_PIECE> pieceToCentipawns = []() {
	std::array<Centipawns, NONE_PIECE> values{};
	values[PAWN]   = 100;
	values[KNIGHT] = 300;
	values[BISHOP] = 310;
	values[ROOK]   = 500;
	values[QUEEN]  = 900;
	return values;
}();

/**
* @brief convert piece to notation for algebraic notation
*/
inline constexpr std::array<char, NONE_PIECE> pieceToNotationChar = []() {
	std::array<char, NONE_PIECE> values{};
	values[KING]   = 'K';
	values[QUEEN]  = 'Q';
	values[ROOK]   = 'R';
	values[BISHOP] = 'B';
	values[KNIGHT] = 'N';
	return values;
}();

/**
* @brief convert piece to notation for uci notation notation
*/
inline constexpr std::array<char, NONE_PIECE> promotionPieceToUCINotationChar = []() {
	std::array<char, NONE_PIECE> values{};
	values[QUEEN]  = 'q';
	values[ROOK]   = 'r';
	values[BISHOP] = 'b';
	values[KNIGHT] = 'n';
	return values;
}();

/**
* @brief convert char from fen string to piece
*/
inline constexpr Piece fenPieceChartoPieceType(char c) {
	switch (c) {
		case 'P':
		case 'p': return PAWN;
		case 'N':
		case 'n': return KNIGHT;
		case 'B':
		case 'b': return BISHOP;
		case 'R':
		case 'r': return ROOK;
		case 'Q':
		case 'q': return QUEEN;
		case 'K':
		case 'k': return KING;
		default: return NONE_PIECE;
	}
}

/**
* @brief index to ascii piece for ascii board representation
*/
inline constexpr std::array<const char*, 12> indexToPiece = {"♚ ", "♛ ", "♜ ", "♝ ", "♞ ", "♟︎ ", "♔ ", "♕ ", "♖ ", "♗ ", "♘ ", "♙ "};

inline constexpr std::array<std::array<MoveScore, 6>, 6> MVV_LVA_table = {{
	/* victims
	 K	  Q	   R	B	 N	  P		 // aggressors */
	{999, 500, 400, 300, 200, 100},	 // KING
	{999, 501, 401, 301, 201, 101},	 // QUEEN
	{999, 502, 402, 302, 202, 102},	 // ROOK
	{999, 503, 403, 303, 203, 103},	 // BISHOP
	{999, 504, 404, 304, 204, 104},	 // KNIGHT
	{999, 505, 405, 305, 205, 105},	 // PAWN
}};

// clang-format off
inline constexpr std::array<uint8_t, 64> castlingMask = []() {
	std::array<uint8_t, 64> values{};
	values[a8] = 0b1110; values[b8] = 0b1111; values[c8] = 0b1111; values[d8] = 0b1111; values[e8] = 0b1100; values[f8] = 0b1111; values[g8] = 0b1111; values[h8] = 0b1101;
	values[a7] = 0b1111; values[b7] = 0b1111; values[c7] = 0b1111; values[d7] = 0b1111; values[e7] = 0b1111; values[f7] = 0b1111; values[g7] = 0b1111; values[h7] = 0b1111;
	values[a6] = 0b1111; values[b6] = 0b1111; values[c6] = 0b1111; values[d6] = 0b1111; values[e6] = 0b1111; values[f6] = 0b1111; values[g6] = 0b1111; values[h6] = 0b1111;
	values[a5] = 0b1111; values[b5] = 0b1111; values[c5] = 0b1111; values[d5] = 0b1111; values[e5] = 0b1111; values[f5] = 0b1111; values[g5] = 0b1111; values[h5] = 0b1111;
	values[a4] = 0b1111; values[b4] = 0b1111; values[c4] = 0b1111; values[d4] = 0b1111; values[e4] = 0b1111; values[f4] = 0b1111; values[g4] = 0b1111; values[h4] = 0b1111;
	values[a3] = 0b1111; values[b3] = 0b1111; values[c3] = 0b1111; values[d3] = 0b1111; values[e3] = 0b1111; values[f3] = 0b1111; values[g3] = 0b1111; values[h3] = 0b1111;
	values[a2] = 0b1111; values[b2] = 0b1111; values[c2] = 0b1111; values[d2] = 0b1111; values[e2] = 0b1111; values[f2] = 0b1111; values[g2] = 0b1111; values[h2] = 0b1111;
	values[a1] = 0b1011; values[b1] = 0b1111; values[c1] = 0b1111; values[d1] = 0b1111; values[e1] = 0b0011; values[f1] = 0b1111; values[g1] = 0b1111; values[h1] = 0b0111;
	return values;
}();
// clang-format on
#endif
