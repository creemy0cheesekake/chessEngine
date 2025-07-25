#ifndef MOVE_H
#define MOVE_H

#include <string>

#include "consts.hpp"

class Board;

class Move {
private:
	/**
	* @brief starting square
	*/
	Square m_from;

	/**
	* @brief destination square
	*/
	Square m_to;

	/**
	* @brief move bitflags
	*/
	MoveFlag m_flags;

	/**
	* @brief type of piece to promote to
	*/
	Piece m_promoPiece;

	/**
	* @brief type of piece that moved
	*/
	Piece m_pieceType;

	/**
	* @brief infers flags from move and board position
	* @param Board -- board of the move
	*/
	MoveFlag genFlags(const Board &);

	/**
	* @brief updates hmclock, fmclock, ep square, side to move
	*/
	void updateGameData(Board &) const;

	/**
	* @brief revokes castling rights if king or rook moves
	*/
	void updateCastlingRights(Board &) const;

public:
	/**
	* @brief initalizes null move
	*/
	Move();
	/**
	* @brief move constructor. infers the flags from the move
	* @param Board -- provides board data for context
	* @param Square -- starting square in little endian rank file 
	* @param Square -- ending square in little endian rank file 
	* @param Piece -- type of piece to promote to
	* @param Piece -- type of piece that moved
	*/
	Move(Board, Square, Square, Piece, Piece = NONE_PIECE);

	/**
	* @brief returns "from" square as an int
	*/
	Square getFrom() const;

	/**
	* @brief returns "to" square as an int
	*/
	Square getTo() const;

	/**
	* @brief returns piece being moved
	*/
	Piece getPieceType() const;

	/**
	* @brief returns flags of move
	*/
	MoveFlag getFlags() const;

	/**
	* @brief sets promotion piece
	*/
	void setPromoPiece(Piece);

	/**
	* @brief gets promotion piece
	*/
	Piece getPromoPiece() const;

	/**
	* @brief return move notation in long algebraic form
	*/
	std::string notation() const;

	/**
	* @brief return move notation in uci algebraic form
	*/
	std::string UCInotation() const;
};
#endif
