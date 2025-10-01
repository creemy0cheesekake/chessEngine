#ifndef EVAL_H
#define EVAL_H

#include <array>
#include <chrono>

#include "board.hpp"
#include "consts.hpp"
#include "move.hpp"

class Eval {
public:
	static uint64_t totalNodesSearched;
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
	static std::tuple<Centipawns, Move, SearchState> search(Moves& topLine, Board&, int, Centipawns alpha = -INF_SCORE, Centipawns beta = INF_SCORE, int plyFromRoot = 0, Move previousMove = Move());

	/**
	* @brief Performs iterative deepening search to improve move ordering and find the best move.
	* This method repeatedly calls the search function with increasing depth limits until the maximum depth is reached or time runs out.
	* It helps in finding the best move by progressively deepening the search and using results from shallower searches to improve efficiency.
	* @param topLine A reference to a Moves object where the best line of moves will be stored.
	* @param b The current board state.
	* @param maxDepth The maximum depth to search to.
	* @return The evaluation score of the best move found.
	*/
	static Centipawns iterative_deepening_ply(Moves& topLine, Board& b, int maxDepth);

	/**
	* @brief Performs iterative deepening search with a time constraint to improve move ordering and find the best move.
	* This method repeatedly calls the search function with increasing depth limits until the maximum depth is reached or the time limit is exceeded.
	* It helps in finding the best move by progressively deepening the search and using results from shallower searches to improve efficiency.
	* @param topLine A reference to a Moves object where the best line of moves will be stored.
	* @param b The current board state.
	* @param maxTimeMs The maximum time allowed for the search in milliseconds.
	* @return The evaluation score of the best move found.
	*/
	static Centipawns iterative_deepening_time(Moves& topLine, Board& b, int maxTimeMs);

	/**
	* @brief quiescensce search to avoid horizon effect.
	*/
	static std::tuple<Centipawns, Move, SearchState> quiescence_search(Board& b, Centipawns alpha, Centipawns beta);

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

private:
	/**
    * @brief The end time for the search to be cut off.
    * This variable is used to determine when the search should stop based on the time constraint.
    */
	static std::chrono::time_point<std::chrono::high_resolution_clock> m_iterative_deepening_cutoff_time;

	/**
	* @brief Calculates the reduction factor for late move reduction (LMR)
	* @param movesSearched Number of moves searched so far at this node
	* @param depthLeft Remaining search depth
	* @return Reduction factor to apply to search depth
	*/
	static int calculateReductionFactor(int movesSearched, int depthLeft);
};
#endif
