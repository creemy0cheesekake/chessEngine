#ifndef MOVE_H
#define MOVE_H

#include <string>
#include "board.hpp"
#include "consts.hpp"

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
	* @brief board on which move will be executed
	*/
	Board &board;

	/**
	* @brief infers flags from move and board position
	* @param Board -- board of the move
	*/
	unsigned int genFlags(Board);

	/**
	* @brief updates hmclock, fmclock, ep square, etc.
	*/
	void updateGameData();

	/**
	* @brief revokes castling rights if king or rook moves
	*/
	void updateCastlingRights();

public:
	// default constructor shouldnt be called
	Move() = delete;
	/**
	* @brief move constructor. infers the flags from the move
	* @param Board -- board on which to execute the move
	* @param UInt -- starting square in little endian rank file 
	* @param UInt -- ending square in little endian rank file 
	*/
	Move(Board &, unsigned int, unsigned int);

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
	* @brief execute move
	*/
	void execute();
};
#endif
