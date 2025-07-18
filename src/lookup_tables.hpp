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

	/**
	* @brief populates straightRayTable
	*/
	static void genStraightRayTable();

	/**
	* @brief populates diagonalRayTable
	*/
	static void genDiagonalRayTable();

public:
	/**
	* @brief index in little endian rank file. each square has a bitboard of all squares a knight could attack if it was on that square
	*/
	static Bitboard knightAttacks[64];

	/**
	* @brief index in little endian rank file. each square has a bitboard of all squares a king could attack if it was on that square
	*/
	static Bitboard kingAttacks[64];

	/**
	* @brief rays of all possible straight sliding moves in each direction from each square
	*/
	static Bitboard straightRayTable[64][4];

	/**
	* @brief rays of all possible diagonal sliding moves in each direction from each square
	*/
	static Bitboard diagonalRayTable[64][4];

	/**
	* @brief populates knightAttacks and kingAttacks
	*/
	static void init();
};

#endif
