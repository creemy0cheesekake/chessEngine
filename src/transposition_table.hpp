#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include "consts.hpp"
#include "move.hpp"

constexpr int TT_SIZE_MB = 16 * 1024 * 1024;

enum TTFlag {
	EXACT,
	ALPHA,
	BETA,
};

struct MoveSkeleton {
	/**
    * @brief starting square
    */
	Square from : 6;

	/**
    * @brief destination square
    */
	Square to : 6;

	/**
    * @brief type of piece that moved
    */
	Piece piece : 3;

	/**
    * @brief type of piece to promote to
    */
	Piece promoPiece : 3;

	/**
    * @brief flags for special move types (capture, promotion, castling, etc.)
    */
	MoveFlag flags : 8;

	/**
    * @brief constructs a MoveSkeleton from a Move
    * @param Move -- move to extract skeleton from
    */
	MoveSkeleton(Move m) {
		from	   = m.getFrom();
		to		   = m.getTo();
		piece	   = m.getPieceType();
		promoPiece = m.getPromoPiece();
		flags	   = m.getFlags();
	}
};

struct alignas(16) TTEntry {
	/**
    * @brief partial hash for collision detection
    */
	u_int16_t partial_hash;

	/**
    * @brief evaluation score stored in the entry
    */
	Centipawns score;

	/**
    * @brief search depth at which this entry was created
    */
	uint8_t depth;

	/**
    * @brief type of node (exact, alpha, beta)
    */
	TTFlag flag;

	/**
    * @brief best move found at this position
    */
	MoveSkeleton bestMove;

	/**
    * @brief constructs a TTEntry
    */
	TTEntry() : partial_hash(0), score(0), depth(0), flag(EXACT), bestMove(Move()) {}

	/**
    * @brief constructs a TTEntry
    * @param ZobristHash -- full hash of the position
    * @param Centipawns -- evaluation score
    * @param int -- search depth
    * @param TTFlag -- node type flag
    * @param Move -- best move found
    */
	TTEntry(ZobristHash h, Centipawns s, int d, TTFlag f, Move m)
		: score(s), depth(d), flag(f), bestMove(m) {
		partial_hash = h & 0xFFFF;
	}
};

class TranspositionTable {
private:
	/**
	* @brief transposition table storage array
	*/
	static std::array<TTEntry, TT_SIZE_MB / sizeof(TTEntry)> m_table;

	/**
	* @brief Number of entries currently used in the transposition table.
	*/
	static int m_used;

	/**
	* @brief Total size of the transposition table in terms of number of entries.
	*/
	static int m_size;

public:
	/**
	* @brief adds an entry to the transposition table
	* @param ZobristHash -- hash of the position
	* @param Centipawns -- evaluation score
	* @param int -- search depth
	* @param TTFlag -- node type flag
	* @param Move -- best move found
	*/
	static void add(ZobristHash h, Centipawns s, int d, TTFlag f, Move m);

	/**
	* @brief retrieves the best move for a given hash from the transposition table
	* @param ZobristHash -- hash of the position to look up
	* @return Move -- best move found, or null move if not found
	*/
	static TTEntry getEntry(ZobristHash h);

	/**
	* @brief retrieves the best move for a given hash from the transposition table
	* @param ZobristHash -- hash of the position to look up
	* @return Move -- best move found, or null move if not found
	*/
	static Move get(ZobristHash h);

	/**
	* @brief retrieves the best move for a given move skeleton from the transposition table
	* @param MoveSkeleton -- move skeleton to look up
	* @return Move -- best move found, or null move if not found
	*/
	static Move getMove(MoveSkeleton m);

	/**
	* @brief Prints the current usage capacity of the transposition table.
	* This function calculates and displays the number of entries used in the transposition table
	* as a percentage of the total available entries.
	*/
	static void printCapacity();
};
#endif