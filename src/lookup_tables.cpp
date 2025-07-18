#include "lookup_tables.hpp"

std::array<Bitboard, 64> LookupTables::knightAttacks = {0};
std::array<Bitboard, 64> LookupTables::kingAttacks	 = {0};
void LookupTables::init() {
	genKingLookupTable();
	genKnightLookupTable();
};

void LookupTables::genKnightLookupTable() {
	for (int i = Square::a1; i <= Square::h8; i++) {
		Bitboard knight = 1UL << i;
		knightAttacks[i] =
			((knight << 15 & ~(hFile | firstRank | secondRank)) |
			 (knight << 17 & ~(aFile | firstRank | secondRank)) |
			 (knight >> 17 & ~(hFile | seventhRank | eighthRank)) |
			 (knight >> 15 & ~(aFile | seventhRank | eighthRank)) |
			 (knight << 6 & ~(firstRank | gFile | hFile)) |
			 (knight << 10 & ~(firstRank | aFile | bFile)) |
			 (knight >> 10 & ~(eighthRank | gFile | hFile)) |
			 (knight >> 6 & ~(eighthRank | aFile | bFile)));
	}
}

void LookupTables::genKingLookupTable() {
	for (int i = Square::a1; i <= Square::h8; i++) {
		Bitboard king = 1UL << i;
		kingAttacks[i] =
			((king << 8) |
			 (king >> 8) |
			 (king << 1 & ~aFile) |
			 (king >> 1 & ~hFile) |
			 (king << 7 & ~hFile) |
			 (king << 9 & ~aFile) |
			 (king >> 7 & ~aFile) |
			 (king >> 9 & ~hFile));
	}
}
