#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include <string>
#include <vector>
#include "board.hpp"
#include "consts.hpp"
#include "move.hpp"

class MoveGen {
private:
	/**
	* @brief vector of legal moves
	*/
	std::vector<Move> moves;

	/**
	* @brief checks whether or not a king is in check when its not their turn to move
	* @param Board -- board for which the check will be performed
	*/
	bool inIllegalCheck(Board);

public:
	/**
	* @brief movegen constructor
	* @param Board -- board position or which moves will be generated
	*/
	MoveGen(Board);

	/**
	* @brief returns vector of legal moves
	*/
	std::vector<Move> getMoves();

	/**
	* @brief generates legal pawn moves
	*/
	std::vector<Move> genPawnMoves();

	/**
	* @brief generates legal knight moves
	*/
	std::vector<Move> genKnightMoves();

	/**
	* @brief generates legal bishop moves
	*/
	std::vector<Move> genBishopMoves();

	/**
	* @brief generates legal rook moves
	*/
	std::vector<Move> genRookMoves();

	/**
	* @brief generates legal queen moves
	*/
	std::vector<Move> genQueenMoves();
};
#endif
