#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include <vector>

#include "consts.hpp"
#include "move.hpp"

using Moves = std::vector<Move>;

class MoveGen {
private:
	/**
	* @brief board to gen moves
	*/
	Board m_board;

	/**
	* @brief bitboard showing every square attacked by oppponent
	*/
	Bitboard m_attacks = 0;

	/**
	* @brief Moves list of all pseudo legal moves generated so far
	*/
	Moves m_pseudoLegalMoves;

	/**
	* @brief returns true if none of the squares between the kings position and the destination of a castle are under attack
	*/
	bool isntCastlingThroughCheck(bool longCastle);

	/**
	* @brief returns a bitboard of all squares attacked by pawns
	*/
	Bitboard genPawnAttacks();

	/**
	* @brief returns a bitboard of all squares attacked by knights
	*/
	Bitboard genKnightAttacks();

	/**
	* @brief returns a bitboard of all squares attacked by the king
	*/
	Bitboard genKingAttacks();

	/**
	* @brief general function for sliding piece attacks
	* @param pieces -- bitboard of pieces which reperesent the pieces that the moves will be generated for
	* @param direction -- specifies the direction(s) that the sliding pieces go in
	*/
	Bitboard
	genSlidingPiecesAttacks(Bitboard pieces, SlidingPieceDirectionFlags direction);
	/**
	* @brief returns a bitboard of all squares attacked by bishops
	*/
	Bitboard genBishopAttacks();

	/**
	* @brief returns a bitboard of all squares attacked by rooks
	*/
	Bitboard genRookAttacks();

	/**
	* @brief bitboard of all squares attacked by queens
	*/
	Bitboard genQueenAttacks();

public:
	/**
	* @brief returns true if the king is in check
	*/
	bool inCheck();
	/**
	* @brief movegen constructor
	* @param Board -- board position for which moves will be generated
	*/
	MoveGen(Board);

	/**
	* @brief returns vector of all legal moves
	*/
	Moves genLegalMoves();

	/**
	* @brief returns vector of all pseudo legal moves - including moves which cause the king to be put into check
	*/
	void genPseudoLegalMoves();

	/**
	* @brief generates legal pawn moves
	*/
	void genPawnMoves();

	/**
	* @brief generates legal knight moves
	*/
	void genKnightMoves();

	/**
	* @brief generates legal king moves
	*/
	void genKingMoves();

	/**
	* @brief generates legal castling moves
	*/
	void genCastlingMoves();

	/**
	* @brief general function to generate all sliding moves
	* @param p -- Piece type of piece
	* @param pieces -- bitboard of pieces which reperesent the pieces that the moves will be generated for
	* @param direction -- specifies the direction(s) that the sliding pieces go in
	*/
	void genSlidingPieces(Piece p, Bitboard pieces, SlidingPieceDirectionFlags direction);
	/**
	* @brief generates legal bishop moves
	*/
	void genBishopMoves();

	/**
	* @brief generates legal rook moves
	*/
	void genRookMoves();

	/**
	* @brief generates legal queen moves
	*/
	void genQueenMoves();

	/**
	* @brief returns a bitboard with every square that is being attacked by an enemy piece
	*/
	Bitboard genAttacks();

	/**
	* @brief returns attacks bitboard
	*/
	Bitboard getAttacks();
};
#endif
