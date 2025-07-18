#include "board.hpp"
#include "move_gen.hpp"
#include "util.hpp"
#include <unordered_map>

Board::Board() {}

std::string Board::printBoard() const {
	std::string board = "";
	for (int rank = 7; rank >= 0; rank--) {
		for (int file = 0; file < 8; file++) {
			int squareIndex			= 8 * rank + file;
			std::string pieceString = ". ";
			for (Color color : {WHITE, BLACK}) {
				for (Piece piece : {KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
					if ((pieces[color][piece] >> squareIndex) & 1) {
						pieceString = indexToPiece[6 * color + piece];
					}
				}
			}
			board += pieceString;
		}
		board += '\n';
	}
	return board;
}

std::ostream &operator<<(std::ostream &os, const Board &b) {
	os << b.printBoard();
	return os;
}

void Board::setToFen(std::string fen) {
	/* 
	 * heres a fen string for reference:
	 * r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7
	 */
	// clear board
	for (Color color : {WHITE, BLACK}) {
		for (Piece piece : {KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
			pieces[color][piece] = 0;
		}
	}

	int squareToWriteTo = Square::a8;
	char *fenChar		= &fen[0];
	do {
		if (*fenChar == ' ') {
			break;
		}

		if (*fenChar == '/') {
			squareToWriteTo -= 16;
		} else if (inRange(*fenChar, '1', '9')) {
			squareToWriteTo += *fenChar - '0';
		} else {
			Color colorOfPiece = islower(*fenChar) ? BLACK : WHITE;
			Piece typeOfPiece  = fenPieceChartoPieceType[toupper(*fenChar)];

			pieces[colorOfPiece][typeOfPiece] |= (1UL << squareToWriteTo);

			squareToWriteTo++;
		}
	} while (*fenChar++);

	sideToMove = *(++fenChar) == 'w' ? WHITE : BLACK;
	fenChar += 2;
	castlingRights.rights = 0;
	while (*fenChar != ' ') {
		switch (*fenChar) {
			case 'K':
				castlingRights.setWhiteKS(true);
				break;
			case 'Q':
				castlingRights.setWhiteQS(true);
				break;
			case 'k':
				castlingRights.setBlackKS(true);
				break;
			case 'q':
				castlingRights.setBlackQS(true);
				break;
		}
		fenChar++;
	}
	fenChar++;
	if (*fenChar != '-') {
		unsigned int enPassantSquareIndex = *fenChar++ - 'a';
		enPassantSquareIndex += 8 * (*fenChar - '1');
		enPassantSquare = 1UL << enPassantSquareIndex;
	}
	fenChar += 2;
	char *endOfClock;
	hmClock = std::strtol(fenChar, &endOfClock, 10);
	fenChar = endOfClock + 1;
	fmClock = std::strtol(fenChar, &endOfClock, 10);
}

bool Board::inIllegalCheck() const {
	// make a copy, flip sides and see if the king is attacked
	Board copiedBoard	   = *this;
	copiedBoard.sideToMove = (Color)!copiedBoard.sideToMove;
	Bitboard king		   = copiedBoard.pieces[copiedBoard.sideToMove][KING];
	return king & MoveGen(copiedBoard).getAttacks();
}

bool Board::gameOver() const {
	MoveGen mg = MoveGen(*this);
	if (!mg.genLegalMoves().size()) {
		return true;
	}
	if (hmClock == 100) {
		return true;
	}
	bool sufficientMaterial = false;
	for (Color color : {WHITE, BLACK}) {
		for (Piece piece : {QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
			if (pieces[color][piece] != 0) {
				sufficientMaterial = true;
				break;
			}
		}
	}
	if (!sufficientMaterial) {
		return true;
	}

	// TODO: implement threefold repetition
	return false;
}

Bitboard Board::whitePieces() const {
	return pieces[WHITE][PAWN] | pieces[WHITE][KNIGHT] | pieces[WHITE][BISHOP] | pieces[WHITE][ROOK] | pieces[WHITE][QUEEN] | pieces[WHITE][KING];
}

Bitboard Board::blackPieces() const {
	return pieces[BLACK][PAWN] | pieces[BLACK][KNIGHT] | pieces[BLACK][BISHOP] | pieces[BLACK][ROOK] | pieces[BLACK][QUEEN] | pieces[BLACK][KING];
}

void Board::reset() {
	// all the magic numbers just correspond to the bitboards for each of those
	// pieces as they would be at the starting position
	pieces[WHITE][KING]	  = 0x10;
	pieces[WHITE][QUEEN]  = 0x8;
	pieces[WHITE][ROOK]	  = 0x81;
	pieces[WHITE][BISHOP] = 0x24;
	pieces[WHITE][KNIGHT] = 0x42;
	pieces[WHITE][PAWN]	  = 0xff00;
	pieces[BLACK][KING]	  = 0x1000000000000000;
	pieces[BLACK][QUEEN]  = 0x800000000000000;
	pieces[BLACK][ROOK]	  = 0x8100000000000000;
	pieces[BLACK][BISHOP] = 0x2400000000000000;
	pieces[BLACK][KNIGHT] = 0x4200000000000000;
	pieces[BLACK][PAWN]	  = 0xff000000000000;
}
