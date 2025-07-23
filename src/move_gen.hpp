#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include "consts.hpp"
#include "move.hpp"

class MoveGen {
private:
	/**
	* @brief board to gen moves
	*/
	Board *m_board;

	/**
	* @brief bitboard showing every square attacked by oppponent
	*/
	Bitboard m_attacks = 0;

	/**
	* @brief returns true if none of the squares between the kings position and the destination of a castle are under attack
	*/
	bool isntCastlingThroughCheck(bool longCastle) const;

	/**
	* @brief returns a bitboard of all squares attacked by pawns
	*/
	Bitboard genPawnAttacks() const;

	/**
	* @brief returns a bitboard of all squares attacked by knights
	*/
	Bitboard genKnightAttacks() const;

	/**
	* @brief returns a bitboard of all squares attacked by the king
	*/
	Bitboard genKingAttacks() const;

	/**
	* @brief general function for sliding piece attacks
	* @param pieces -- bitboard of pieces which reperesent the pieces that the moves will be generated for
	* @param direction -- specifies the direction(s) that the sliding pieces go in
	*/
	Bitboard genSlidingPiecesAttacks(Bitboard pieces, SlidingPieceDirectionFlags direction) const;
	/**
	* @brief returns a bitboard of all squares attacked by bishops
	*/
	Bitboard genBishopAttacks() const;

	/**
	* @brief returns a bitboard of all squares attacked by rooks
	*/
	Bitboard genRookAttacks() const;

	/**
	* @brief bitboard of all squares attacked by queens
	*/
	Bitboard genQueenAttacks() const;

public:
	/**
	* @brief returns true if the king is in check
	*/
	bool inCheck() const;
	/**
	* @brief movegen constructor
	* @param Board -- board position for which moves will be generated
	*/
	MoveGen(Board *);

	/**
	* @brief returns vector of all legal moves
	*/
	Moves genLegalMoves() const;

	/**
	* @brief generates legal pawn moves
	*/
	void genPawnMoves(Moves &) const;

	/**
	* @brief generates legal knight moves
	*/
	void genKnightMoves(Moves &) const;

	/**
	* @brief generates legal king moves
	*/
	void genKingMoves(Moves &) const;

	/**
	* @brief generates legal castling moves
	*/
	void genCastlingMoves(Moves &) const;

	/**
	* @brief general function to generate all sliding moves
	* @param pseudoLegalMoves -- reference for vector to add moves to
	* @param p -- Piece type of piece
	* @param pieces -- bitboard of pieces which reperesent the pieces that the moves will be generated for
	* @param direction -- specifies the direction(s) that the sliding pieces go in
	*/
	void genSlidingPieces(Moves &, Piece p, Bitboard pieces, SlidingPieceDirectionFlags direction) const;
	/**
	* @brief generates legal bishop moves
	*/
	void genBishopMoves(Moves &) const;

	/**
	* @brief generates legal rook moves
	*/
	void genRookMoves(Moves &) const;

	/**
	* @brief generates legal queen moves
	*/
	void genQueenMoves(Moves &) const;

	/**
	* @brief returns a bitboard with every square that is being attacked by an enemy piece
	*/
	Bitboard genAttacks() const;

	/**
	* @brief returns attacks bitboard
	*/
	Bitboard getAttacks() const;
};
#endif
