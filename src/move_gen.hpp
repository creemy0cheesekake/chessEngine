#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include <string>
#include <vector>
#include "board.hpp"
#include "consts.hpp"
#include "move.hpp"
#include "util.cpp"

class MoveGen {
private:
	/**
	* @brief board to gen moves
	*/
	Board board;

	/**
	* @brief returns true if none of the squares between the kings position and the destination of a castle are under attack
	*/
	bool isntCastlingThroughCheck(bool longCastle);

	/**
	* @brief returns true if the king is in check
	*/
	bool inCheck();

public:
	/**
	* @brief movegen constructor
	* @param Board -- board position for which moves will be generated
	*/
	MoveGen(Board);

	/**
	* @brief returns vector of all legal moves
	*/
	std::vector<Move> genMoves();

	/**
	* @brief returns vector of all pseudo legal moves - including moves which cause the king to be put into check
	*/
	std::vector<Move> genPseudoLegalMoves();

	/**
	* @brief returns vector of all pseudo legal moves - including moves which cause the king to be put into check, but not including castling moves
	*/
	std::vector<Move> genPseudoLegalMovesWithoutCastling();

	/**
	* @brief generates legal pawn moves
	*/
	std::vector<Move> genPawnMoves();

	/**
	* @brief generates legal knight moves
	*/
	std::vector<Move> genKnightMoves();

	/**
	* @brief generates legal king moves
	*/
	std::vector<Move> genKingMoves();

	/**
	* @brief generates legal castling moves
	*/
	std::vector<Move> genCastlingMoves();

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
