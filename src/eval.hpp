#ifndef EVAL_H
#define EVAL_H

#include "board.hpp"
#include "consts.hpp"
#include "move_gen.hpp"

class Eval {
public:
	/**
	* @brief sums up material count, and its sign is whether or not the count is favorable to whoevers turn it is. all standard piece values except bishop is worth 3.1
	*/
	static Centipawns countMaterial(Board);

	/**
	* @brief spits out evaluation given a position. its sign is whether or not the count is favorable to whoevers turn it is. for now this just counts material.
	*/
	static Centipawns evaluate(Board);

	/**
	* @brief negamax search with alpha beta pruning. its sign is whether or not the count is favorable to whoevers turn it is. stores the top engine line in topLine.
	*/
	static Centipawns search(Moves *topLine, Board, int, Centipawns alpha = -INF_SCORE, Centipawns beta = INF_SCORE);
};
#endif
