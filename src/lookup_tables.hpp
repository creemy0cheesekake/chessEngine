#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include <iostream>
#include "consts.hpp"

class LookupTables {
private:
	static void genKnightLookupTable();
	static void genKingLookupTable();

public:
	static bitboard knightAttacks[64];
	static bitboard kingAttacks[64];
	static void init();
};

#endif
