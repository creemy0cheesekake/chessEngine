#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <string>

#include "consts.hpp"
#include "move_gen.hpp"

class Move;

class Board {
public:
	/**
	* @brief represents the current castling rights for the board position
	*/
	struct CastlingRights {
		/**
		* @brief each binary digit represents one of the castling options. KQkq
		* meaning 1st digit represents whites kingside castling, 2nd for white queenside castling, 3rd for black kingside, 4th for black queenside
		*/
		unsigned short rights = 0b1111;

		/**
		* @brief return first two digits
		*/
		inline unsigned short getWhiteRights() const {
			return rights >> 2;
		};

		/**
		* @brief return second two digits
		*/
		inline unsigned short getBlackRights() const {
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

	/**
	* @brief contains all the board data at any given time
	*/
	struct BoardState {
		/**
		* @brief bitboards of all current pieces
		*/
		Bitboard pieces[2][6] = {
			{
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
			}
		};
		/**
		* @brief holds the board states castling rights
		*/
		CastlingRights castlingRights;
		/**
		* @brief stores which side it is to move
		*/
		Color sideToMove = WHITE;
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
	};

	/**
	* @brief initalizes board to default starter position
	*/
	Board();

	/**
	* @brief current board state
	*/
	BoardState boardState;

	/**
	* @brief MoveGen object to generate moves on this board
	*/
	MoveGen moveGenerator = MoveGen(this);

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
	* @requires position before setting is the default position
	* @param fen [string] - fen string to set board to
	*/
	void setToFen(const char *fen);

	/**
	* @brief execute move by adding current boardState to previousBoardStates and updating the current boardState
	*/
	void execute(const Move &);

	/**
	* @brief goes back a move to the previous board state
	*/
	void undoMove();

	/**
	* @brief returns bitboard of all white pieces
	*/
	Bitboard whitePieces() const;

	/**
	* @brief returns bitboard of all black pieces
	*/
	Bitboard blackPieces() const;

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

private:
	/**
	* @brief stack of historical board states
	*/
	std::vector<BoardState> m_previousBoardStates;

	/**
	* @brief updates hmclock, fmclock, ep square, etc.
	*/
	void updateBoardStateGameData(const Move &);

	/**
	* @brief revokes castling rights if king or rook moves
	*/
	void updateCastlingRights(const Move &);
};
#endif
