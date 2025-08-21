#ifndef EVAL_H
#define EVAL_H

#include <array>

#include "board.hpp"
#include "consts.hpp"
#include "move.hpp"

class Board;
class MoveGen;

class Eval {
public:
	/**
	* @brief sums up material count, and its sign is whether or not the count is favorable to whoevers turn it is. all standard piece values except bishop is worth 3.1
	*/
	static Centipawns countMaterial(const Board&);

	/**
	* @brief spits out evaluation given a position. its sign is whether or not the count is favorable to whoevers turn it is. for now this just counts material.
	*/
	static Centipawns evaluate(Board&);

	/**
	* @brief negamax search with alpha beta pruning. its sign is whether or not the count is favorable to whoevers turn it is. stores the top engine line in topLine.
	*/
	static Centipawns search(Moves& topLine, Board&, int, Centipawns alpha = -INF_SCORE, Centipawns beta = INF_SCORE, int plyFromRoot = 0);

	/**
	* @brief quiescensce search to avoid horizon effect.
	*/
	static Centipawns quiescence_search(Board& b, Centipawns alpha, Centipawns beta);

	static constexpr int MAX_SEARCH_DEPTH = 64;
	static constexpr int NUM_KILLER_MOVES = 2;

	/**
	* @brief store killer moves.the killer move is a quiet move which caused a beta cutoff in a sibling cut branch. the idea is that these moves are likely to be good in this branch as well
	*/
	static std::array<std::array<Move, NUM_KILLER_MOVES>, MAX_SEARCH_DEPTH> killerMoves;

	/**
	* @brief clears killer moves array
	*/
	static void resetKillerMoves();
};
#endif
