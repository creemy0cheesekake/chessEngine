#include "lookup_tables.hpp"

Bitboard LookupTables::knightAttacks[64];
Bitboard LookupTables::kingAttacks[64];
Bitboard LookupTables::straightRayTable[64][4];
Bitboard LookupTables::diagonalRayTable[64][4];

void LookupTables::init() {
	genKingLookupTable();
	genKnightLookupTable();
	genStraightRayTable();
	genDiagonalRayTable();
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

void LookupTables::genStraightRayTable() {
	for (int square = Square::a1; square <= Square::h8; square++) {
		Bitboard *squareRays = straightRayTable[square];

		Bitboard *Nrays = &squareRays[NORTH];
		*Nrays |= 1UL << square;
		while (!(*Nrays & eighthRank)) {
			*Nrays |= *Nrays << 8;
		}
		Bitboard *Erays = &squareRays[EAST];
		*Erays |= 1UL << square;
		while (!(*Erays & hFile)) {
			*Erays |= *Erays << 1;
		}
		Bitboard *Wrays = &squareRays[WEST];
		*Wrays |= 1UL << square;
		while (!(*Wrays & aFile)) {
			*Wrays |= *Wrays >> 1;
		}
		Bitboard *Srays = &squareRays[SOUTH];
		*Srays |= 1UL << square;
		while (!(*Srays & firstRank)) {
			*Srays |= *Srays >> 8;
		}
		*Nrays ^= 1UL << square;
		*Erays ^= 1UL << square;
		*Wrays ^= 1UL << square;
		*Srays ^= 1UL << square;
	}
}

void LookupTables::genDiagonalRayTable() {
	for (int square = Square::a1; square <= Square::h8; square++) {
		Bitboard *squareRays = diagonalRayTable[square];

		Bitboard *NErays = &squareRays[NORTHEAST];
		*NErays |= 1UL << square;
		while (!(*NErays & (eighthRank | hFile))) {
			*NErays |= *NErays << 9;
		}
		Bitboard *NWrays = &squareRays[NORTHWEST];
		*NWrays |= 1UL << square;
		while (!(*NWrays & (eighthRank | aFile))) {
			*NWrays |= *NWrays << 7;
		}
		Bitboard *SWrays = &squareRays[SOUTHWEST];
		*SWrays |= 1UL << square;
		while (!(*SWrays & (firstRank | aFile))) {
			*SWrays |= *SWrays >> 9;
		}
		Bitboard *SErays = &squareRays[SOUTHEAST];
		*SErays |= 1UL << square;
		while (!(*SErays & (firstRank | hFile))) {
			*SErays |= *SErays >> 7;
		}
		*NErays ^= 1UL << square;
		*NWrays ^= 1UL << square;
		*SErays ^= 1UL << square;
		*SWrays ^= 1UL << square;
	}
}
