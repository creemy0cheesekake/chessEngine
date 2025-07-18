#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include "consts.hpp"

#include <array>

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
	static std::array<Bitboard, 64> knightAttacks;

	/**
	* @brief index in little endian rank file. each square has a bitboard of all squares a king could attack if it was on that square
	*/
	static std::array<Bitboard, 64> kingAttacks;

	/**
	* @brief populates knightAttacks and kingAttacks
	*/
	static void init();
};

#endif
