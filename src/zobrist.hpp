#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <array>
#include <random>

#include "board.hpp"
#include "consts.hpp"

class Zobrist {
public:
	/**
	* @brief initialize all the values with random values
	*/
	static void init();

	/**
	* @brief hashes a board state
	*/
	static ZobristHash hash(Board::BoardState&);

	/**
	* @brief returns the hash for the default board state
	*/
	static ZobristHash initialHash();

	/**
	* @brief fixed key prng
	*/
	static std::mt19937_64 rng;

	/**
	* @brief need to generate a number for each piece at each square
	*/
	static std::array<std::array<ZobristHash, 64>, 12> pieceKeys;

	/**
	* @brief need a number to indicate its black to move
	*/
	static ZobristHash blackSideKey;

	/**
	* @brief need a number for each possible castling rights permutation
	*/
	static std::array<ZobristHash, 16> castlingKeys;

	/**
	* @brief need a number to indicate the existence of en passant on each file
	*/
	static std::array<ZobristHash, 8> epFileKeys;
};
#endif
