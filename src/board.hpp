#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <array>

#include "consts.hpp"

class Board {
public:
	std::array<std::array<Bitboard, 6>, 2> pieces = {
		{{
			 0x10,
			 0x8,
			 0x81,
			 0x24,
			 0x42,
			 0xff00,
		 },
		 {
			 0x1000000000000000,
			 0x800000000000000,
			 0x8100000000000000,
			 0x2400000000000000,
			 0x4200000000000000,
			 0xff000000000000,
		 }}
	};

	struct CastlingRights {
		/**
		* @brief each binary digit represents one of the castling options. KQkq
		* meaning 1st digit represents whites kingside castling, 2nd for white queenside castling, 3rd for black kingside, 4th for black queenside
		*/
		unsigned short rights = 0b1111;

		/**
		* @brief return first two digits
		*/
		inline unsigned short getWhiteRights() {
			return rights >> 2;
		};

		/**
		* @brief return second two digits
		*/
		inline unsigned short getBlackRights() {
			return rights & 0b0011;
		};

		/**
		* @brief set first bit of rights to 1 if true else 0
		*/
		inline void setWhiteKS(bool whiteKSCastlingRights) {
			rights = whiteKSCastlingRights ? rights | 0b1000 : rights & 0b0111;
		};

		/**
		* @brief set second bit of rights to 1 if true else 0
		*/
		inline void setWhiteQS(bool whiteQSCastlingRights) {
			rights = whiteQSCastlingRights ? rights | 0b0100 : rights & 0b1011;
		};

		/**
		* @brief set third bit of rights to 1 if true else 0
		*/
		inline void setBlackKS(bool blackKSCastlingRights) {
			rights = blackKSCastlingRights ? rights | 0b0010 : rights & 0b1101;
		};

		/**
		* @brief set fourth bit of rights to 1 if true else 0
		*/
		inline void setBlackQS(bool blackQSCastlingRights) {
			rights = blackQSCastlingRights ? rights | 0b0001 : rights & 0b1110;
		};
	};

	Color sideToMove = WHITE;

	CastlingRights castlingRights;

	/**
	* @brief square where en passant may be taken. for example, after 1. e4 the enPassantSquare
	* would be (1 << 20) (algebraic e3) because if there were a pawn on f4, fxe3 would be possible
	*/
	Bitboard enPassantSquare = 0;

	/**
	* @brief half move clock. number of half moves since last pawn move or capture
	*/
	unsigned int hmClock = 0;

	/**
	* @brief full move clock. number of full moves, starts at 1, gets incremented every time black moves
	*/
	unsigned int fmClock = 1;

	/**
	* @brief initalizes board to default starter position
	*/
	Board();

	/**
	* @brief print a visual representation of the board position using ascii art with a1 in the bottom left and h8 in the top right
	*/
	std::string printBoard() const;

	/**
	* @brief overload the << operator to call printBoard
	*/
	friend std::ostream &operator<<(std::ostream &os, const Board &b);

	/**
	* @brief set board to specified fen string position
	* @param fen [string] - fen string to set board to
	*/
	void setToFen(std::string fen);

	/**
	* @brief returns bitboard of all white pieces
	*/
	Bitboard whitePieces();

	/**
	* @brief returns bitboard of all black pieces
	*/
	Bitboard blackPieces();

	/**
	* @brief resets board to initial position
	*/
	void reset();

	/**
	* @brief checks whether or not a king is in check when its not their turn to move
	*/
	bool inIllegalCheck();

	/**
	* @brief checks whether or not the game is over
	*/
	bool gameOver();
};
#endif
