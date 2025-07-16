#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include "consts.hpp"

class LookupTables {
private:
	/**
	* @brief populates knightAttacks
	*/
	static void genKnightLookupTable();

	/**
	* @brief populates kingAttacks
	*/
	static void genKingLookupTable();

public:
	/**
	* @brief index in little endian rank file. each square has a bitboard of all squares a knight could attack if it was on that square
	*/
	static bitboard knightAttacks[64];

	/**
	* @brief index in little endian rank file. each square has a bitboard of all squares a king could attack if it was on that square
	*/
	static bitboard kingAttacks[64];

	/**
	* @brief populates knightAttacks and kingAttacks
	*/
	static void init();
};

#endif
