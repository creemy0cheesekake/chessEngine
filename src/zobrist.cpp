#include "board.hpp"
#include "util.hpp"
#include "zobrist.hpp"

std::mt19937_64 Zobrist::rng = std::mt19937_64(0xa3db3b);
std::array<std::array<ZobristHash, 64>, 12> Zobrist::pieceKeys{};
ZobristHash Zobrist::blackSideKey{};
std::array<ZobristHash, 16> Zobrist::castlingKeys{};
std::array<ZobristHash, 8> Zobrist::epFileKeys{};

void Zobrist::init() {
	for (int i = 0; i < pieceKeys.size(); i++) {
		for (int j = 0; j < pieceKeys[0].size(); j++) {
			pieceKeys[i][j] = rng();
		}
	}

	blackSideKey = rng();

	for (int i = 0; i < castlingKeys.size(); i++) {
		castlingKeys[i] = rng();
	}

	for (int i = 0; i < epFileKeys.size(); i++) {
		epFileKeys[i] = rng();
	}
}

ZobristHash Zobrist::initialHash() {
	Board::BoardState b;
	return hash(b);
}

ZobristHash Zobrist::hash(Board::BoardState& b) {
	ZobristHash res = 0;
	if (b.sideToMove == BLACK) {
		res ^= blackSideKey;
	}
	for (Color color : {WHITE, BLACK}) {
		for (Piece pieceType : {KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
			int pieceIndex = (color == BLACK ? 6 : 0) + pieceType;
			Bitboard piece = b.pieces[color][pieceType];
			do {
				res ^= pieceKeys[pieceIndex][bitscan(piece)];
			} while (removeLS1B(piece));
		}
	}
	res ^= castlingKeys[b.castlingRights.rights];
	if (b.enPassantSquare) {
		res ^= epFileKeys[bitscan(b.enPassantSquare) % 8];
	}
	return res;
}
