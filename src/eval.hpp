#ifndef EVAL_H
#define EVAL_H

#include <array>
#include <chrono>

#include "board.hpp"
#include "consts.hpp"
#include "move.hpp"

class Eval {
public:
	/**
	* @brief Tracks the total number of nodes searched during the current search session.
	* This counter is used for performance monitoring and calculating nodes per second (NPS).
	*/
	static uint64_t totalNodesSearched;

	/**
	* @brief sums up material count from whites POV.
	*/
	static Centipawns countMaterial(const Board&);

	/**
	* @brief spits out evaluation given a position. its sign is whether or not the count is favorable to whoevers turn it is.
	*/
	static Centipawns evaluate(Board&);

	/**
	* @brief negamax search with alpha beta pruning. its sign is whether or not the count is favorable to whoevers turn it is. stores the top engine line in topLine.
	*/
	static std::tuple<Centipawns, SearchState> search(Moves& topLine, Board&, int, Moves& previousPV, Centipawns alpha = -INF_SCORE, Centipawns beta = INF_SCORE, int plyFromRoot = 0);

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

	/**
	* @brief Calculates the weight given to each piece based on its location for evaluation.
	* This function is used to adjust the material value of pieces depending on their position on the board.
	* The weight can vary for different pieces and colors, reflecting strategic considerations such as control of the center,
	* piece activity, and other positional factors.
	* 
	* @param pieceType The type of the piece (e.g., QUEEN, ROOK, BISHOP, KNIGHT, PAWN).
	* @param color The color of the piece (WHITE or BLACK).
	* @param position The position of the piece on the board.
	* @return A multiplier that adjusts the material value of the piece based on its position.
	*/
	static Centipawns positionalValue(Piece pieceType, Color color, Square position);

	constexpr const static std::array<std::array<std::array<Centipawns, 64>, 2>, 6> m_materialWeights = {{
		// clang-format off
		// KING
		{{
			// WHITE
			{   8,   13,   -1,   -5,   -1,   -5,   13,   13,
			    0,   -3,   -8,   -8,   -8,   -8,   -3,    0,
			  -10,  -13,  -15,  -15,  -15,  -15,  -13,  -10,
			  -30,  -30,  -30,  -30,  -30,  -30,  -30,  -30,
			  -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,
			  -50,  -50,  -50,  -50,  -50,  -50,  -50,  -50,
			  -60,  -60,  -60,  -60,  -60,  -60,  -60,  -60,
			  -70,  -70,  -70,  -70,  -70,  -70,  -70,  -70},
			// BLACK
			{ -70,  -70,  -70,  -70,  -70,  -70,  -70,  -70,
			  -60,  -60,  -60,  -60,  -60,  -60,  -60,  -60,
			  -50,  -50,  -50,  -50,  -50,  -50,  -50,  -50,
			  -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,
			  -30,  -30,  -30,  -30,  -30,  -30,  -30,  -30,
			  -10,  -13,  -15,  -15,  -15,  -15,  -13,  -10,
			    0,   -3,   -8,   -8,   -8,   -8,   -3,    0,
			    8,   13,   -1,   -5,   -1,   -1,   13,   13},
		}},
		// QUEEN
		{{
			// WHITE
			{0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0},
			// BLACK
			{0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0}
		}},
		// ROOK
		{{
			// WHITE
			{0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0},
			// BLACK
			{0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0}
		}},
		// BISHOP
		{{
			// WHITE
			{-10, -10, -10, -10, -10, -10, -10, -10,
			  -5,  10,   0,   5,   5,   0,  10,  -5,
			 -10,   0,   0,  10,  10,   0,   0, -10,
			 -10,   0,  10,   7,   7,  10,   0, -10,
			 -10,   7,   0,   7,   7,   0,   7, -10,
			 -10,   0,   0,   0,   0,   0,   0, -10,
			 -10,   0,   0,   0,   0,   0,   0, -10,
			 -10, -10, -10, -10, -10, -10, -10, -10},
			// BLACK
			{-10, -10, -10, -10, -10, -10, -10, -10,
			 -10,   0,   0,   0,   0,   0,   0, -10,
			 -10,   0,   0,   0,   0,   0,   0, -10,
			 -10,   7,   0,   7,   7,   0,   7, -10,
			 -10,   0,  10,   7,   7,  10,   0, -10,
			 -10,   0,   0,  10,  10,   0,   0, -10,
			  -5,  10,   0,   5,   5,   0,  10,  -5,
			 -10, -10, -10, -10, -10, -10, -10, -10},
		}},
		// KNIGHT
		{{
			// WHITE
			{-7, -2, -5, -5, -5, -5, -2, -7,
			 -7,  0,  0,  0,  0,  0,  0, -7,
			 -5,  0,  5,  5,  5,  5,  0, -5,
			 -5,  5,  6,  6,  6,  6,  5, -5,
			 -5,  5,  6,  6,  6,  6,  5, -5,
			 -5,  0,  5,  5,  5,  5,  0, -5,
			 -5,  0,  0,  0,  0,  0,  0, -5,
			 -7, -5, -5, -5, -5, -5, -5, -7},
			// BLACK
			{-7, -5, -5, -5, -5, -5, -5, -7,
			 -5,  0,  0,  0,  0,  0,  0, -5,
			 -5,  0,  5,  5,  5,  5,  0, -5,
			 -5,  5,  6,  6,  6,  6,  5, -5,
			 -5,  5,  6,  6,  6,  6,  5, -5,
			 -5,  0,  5,  5,  5,  5,  0, -5,
			 -7,  0,  0,  0,  0,  0,  0, -7,
			 -7, -2, -5, -5, -5, -5, -2, -7},
		}},
		// PAWN
		{{
			// WHITE
			{ 0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0, -1,  2,  2, -1,  0,  0,
			  0,  0,  0,  7,  7,  0,  0,  0,
			 10, 10, 10,  7,  7, 10, 10, 10,
			 20, 20, 20, 20, 20, 20, 20, 20,
			 60, 60, 60, 60, 60, 60, 60, 60,
			  0,  0,  0,  0,  0,  0,  0,  0},
			// BLACK
			{ 0,  0,  0,  0,  0,  0,  0,  0,
			 60, 60, 60, 60, 60, 60, 60, 60,
			 20, 20, 20, 20, 20, 20, 20, 20,
			 10, 10, 10,  7,  7, 10, 10, 10,
			  0,  0,  0,  7,  7,  0,  0,  0,
			  0,  0, -1,  2,  2, -1,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0}
		}}
		// clang-format on
	}};

	constexpr static std::array<int, 28> queenMobilityScore	 = {-23, -20, -15, -12, -8, -5, 0, 2, 2, 3, 5, 5, 6, 6, 8, 9, 9, 11, 12, 12, 14, 14, 15, 17, 17, 20, 21, 23};
	constexpr static std::array<int, 15> rookMobilityScore	 = {-10, -8, -5, -3, 0, 2, 3, 4, 5, 6, 6, 7, 8, 8, 9};
	constexpr static std::array<int, 14> bishopMobilityScore = {-10, -8, -5, -3, 0, 2, 3, 4, 5, 6, 6, 7, 8, 8};
	constexpr static std::array<int, 9> knightMobilityScore	 = {-8, -5, -3, 0, 2, 4, 5, 6, 8};
};
#endif
