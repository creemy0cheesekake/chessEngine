#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <unordered_map>

#include "consts.hpp"
class Board {
public:
	bitboard pieces[12] = {
		0x10,
		0x8,
		0x81,
		0x24,
		0x42,
		0xff00,
		0x1000000000000000,
		0x800000000000000,
		0x8100000000000000,
		0x2400000000000000,
		0x4200000000000000,
		0xff000000000000,
	};
	bitboard *W_KING   = &pieces[0];
	bitboard *W_QUEEN  = &pieces[1];
	bitboard *W_ROOK   = &pieces[2];
	bitboard *W_BISHOP = &pieces[3];
	bitboard *W_KNIGHT = &pieces[4];
	bitboard *W_PAWN   = &pieces[5];
	bitboard *B_KING   = &pieces[6];
	bitboard *B_QUEEN  = &pieces[7];
	bitboard *B_ROOK   = &pieces[8];
	bitboard *B_BISHOP = &pieces[9];
	bitboard *B_KNIGHT = &pieces[10];
	bitboard *B_PAWN   = &pieces[11];

	Color sideToMove = WHITE;

	/**
	* @brief each binary digit represents one of the castling options. KQkq
	* meaning 1st digit represents whites kingside castling, 2nd for white queenside castling, 3rd for black kingside, 4th for black queenside
	*/
	unsigned short castlingRights = 0b1111;

	/**
	* @brief square where en passant may be taken. for example, after 1. e4 the enPassantSquare
	* would be 20 (algebraic e3) because if there were a pawn on f4, fxe3 would be possible
	*/
	bitboard enPassantSquare = 0;

	/**
	* @brief half move clock. moves since last pawn move or capture
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
	* @brief return a visual representation of the board position using ascii art with a1 in the bottom left and h8 in the top right
	*/
	std::string stringBoard();

	/**
	* @brief return a visual representation of the board position using ascii art
	* @param flipped [bool] - if true, the board is flipped; shown from black's perspective
	*/
	std::string stringBoard(bool);

	/**
	* @brief set board to specified fen string position
	* @param fen [string] - fen string to set board to
	*/
	void setToFen(std::string fen);

	/**
	* @brief returns bitboard of all white pieces
	*/
	bitboard whitePieces();

	/**
	* @brief returns bitboard of all black pieces
	*/
	bitboard blackPieces();
};
#endif
