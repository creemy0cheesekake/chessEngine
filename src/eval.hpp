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
			{   15,   25,   25,  -10,   -1,   -1,   25,   25,
			    0,   -5,  -15,  -15,  -15,  -15,   -5,    0,
			  -20,  -25,  -30,  -30,  -30,  -30,  -25,  -20,
			  -60,  -60,  -60,  -60,  -60,  -60,  -60,  -60,
			  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,
			 -100, -100, -100, -100, -100, -100, -100, -100,
			 -120, -120, -120, -120, -120, -120, -120, -120,
			 -140, -140, -140, -140, -140, -140, -140, -140 },
			// BLACK
			{ -140, -140, -140, -140, -140, -140, -140, -140,
			 -120, -120, -120, -120, -120, -120, -120, -120,
			 -100, -100, -100, -100, -100, -100, -100, -100,
			  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,
			  -60,  -60,  -60,  -60,  -60,  -60,  -60,  -60,
			  -20,  -25,  -30,  -30,  -30,  -30,  -25,  -20,
			    0,   -5,  -15,  -15,  -15,  -15,   -5,    0,
			   15,   25,   25,  -10,   -1,   -1,   25,   25 },
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
		// KNIGHT
		{{
			// WHITE
			{-7, -2, -5, -5, -5, -5, -2, -7,
			 -7,  0,  0,  0,  0,  0,  0, -7,
			 -5,  0,  5,  5,  5,  5,  0, -5,
			 -5,  5,  5,  6,  6,  5,  5, -5,
			 -5,  5,  5,  6,  6,  5,  5, -5,
			 -5,  0,  5,  5,  5,  5,  0, -5,
			 -5,  0,  0,  0,  0,  0,  0, -5,
			 -7, -5, -5, -5, -5, -5, -5, -7},
			// BLACK
			{-7, -5, -5, -5, -5, -5, -5, -7,
			 -5,  0,  0,  0,  0,  0,  0, -5,
			 -5,  0,  5,  5,  5,  5,  0, -5,
			 -5,  5,  5,  6,  6,  5,  5, -5,
			 -5,  5,  5,  6,  6,  5,  5, -5,
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
			  0,  0,  0,  7,  7,  0,  0,  0,
			 10, 10, 10, 10, 10, 10, 10, 10,
			 15, 15, 15, 15, 15, 15, 15, 15,
			  0,  0,  0,  0,  0,  0,  0,  0},
			// BLACK
			{ 0,  0,  0,  0,  0,  0,  0,  0,
			 15, 15, 15, 15, 15, 15, 15, 15,
			 10, 10, 10, 10, 10, 10, 10, 10,
			  0,  0,  0,  7,  7,  0,  0,  0,
			  0,  0,  0,  7,  7,  0,  0,  0,
			  0,  0, -1,  2,  2, -1,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0}
		}}
		// clang-format on
	}};

	constexpr static std::array<int, 28> queenMobilityScore	 = {-60, -50, -40, -30, -20, -10, 0, 3, 5, 7, 10, 12, 15, 17, 20, 22, 25, 27, 30, 32, 35, 37, 40, 42, 45, 50, 55, 60};
	constexpr static std::array<int, 15> rookMobilityScore	 = {-40, -30, -20, -10, 0, 8, 12, 15, 20, 23, 25, 27, 30, 32, 35};
	constexpr static std::array<int, 14> bishopMobilityScore = {-40, -30, -20, -10, 0, 8, 12, 15, 20, 23, 25, 27, 30, 32};
	constexpr static std::array<int, 9> knightMobilityScore	 = {-30, -20, -10, 0, 8, 15, 20, 25, 30};
};
#endif
