#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include <array>

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
	static std::array<Bitboard, 64> s_knightAttacks;

	/**
	* @brief index in little endian rank file. each square has a bitboard of all squares a king could attack if it was on that square
	*/
	static std::array<Bitboard, 64> s_kingAttacks;

	/**
	* @brief rays of all possible straight sliding moves in each direction from each square
	*/
	static std::array<std::array<Bitboard, 4>, 64> s_straightRayTable;

	/**
	* @brief rays of all possible diagonal sliding moves in each direction from each square
	*/
	static std::array<std::array<Bitboard, 4>, 64> s_diagonalRayTable;

	/**
	* @brief populates all the lookup tables
	*/
	static void init();
};

#endif
