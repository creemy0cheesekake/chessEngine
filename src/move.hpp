#ifndef MOVE_H
#define MOVE_H

#include "consts.hpp"
#include "board.hpp"

class Move {
private:
	/**
	* @brief starting square in little endian rank file 
	*/
	unsigned int from;

	/**
	* @brief destination square in little endian rank file 
	*/
	unsigned int to;

	/**
	* @brief move flags: see enum
	*/
	unsigned int flags;

	/**
	* @brief promotion piece: 1-Q, 2-R, 3-B, 4-N
	*/
	unsigned int promoPiece;

	/**
	* @brief type of piece that moved
	*/

	/**
	* @brief infers flags from move and board position
	* @param Board -- board of the move
	*/
	unsigned int genFlags(Board);

	/**
	* @brief updates hmclock, fmclock, ep square, etc.
	*/
	Board updateGameData(Board);

	/**
	* @brief revokes castling rights if king or rook moves
	*/
	unsigned short updateCastlingRights(Board);

public:
	Piece pieceType;
	/**
	* @brief board on which move will be executed
	*/
	Board board;
	/**
	* @brief initalizes null move
	*/
	Move();
	/**
	* @brief move constructor. infers the flags from the move
	* @param Board -- board on which to execute the move
	* @param UInt -- starting square in little endian rank file 
	* @param UInt -- ending square in little endian rank file 
	* @param UInt -- promotion piece: 1-Q, 2-R, 3-B, 4-N
	* @param Piece -- type of piece that moved
	*/
	Move(Board, unsigned int, unsigned int, Piece, unsigned int = 0);

	/**
	* @brief move flags
	*/
	enum Flag {
		NORMAL_MOVE = 0b0000000,  // 0
		CAPTURE		= 0b0000001,  // 1
		KS_CASTLE	= 0b0000010,  // 2
		QS_CASTLE	= 0b0000100,  // 4
		DBL_PAWN	= 0b0001000,  // 8
		EN_PASSANT	= 0b0010000,  // 16
		PROMOTION	= 0b0100000,  // 32
		PAWN_MOVE	= 0b1000000,  // 64
	};

	/**
	* @brief returns "from" square as an int
	*/
	unsigned int getFrom();

	/**
	* @brief returns "to" square as an int
	*/
	unsigned int getTo();

	/**
	* @brief returns flags of move
	*/
	unsigned int getFlags();

	/**
	* @brief sets promotion piece
	*/
	void setPromoPiece(unsigned int);

	/**
	* @brief return move notation in long algebraic form
	*/
	std::string notation();

	/**
	* @brief return move notation in uci algebraic form
	*/
	std::string UCInotation();

	/**
	* @brief execute move
	*/
	Board execute();
};
#endif
