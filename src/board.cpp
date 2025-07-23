#include "board.hpp"
#include <iostream>
#include "move_gen.hpp"
#include "util.hpp"

Board::Board() {
	m_previousBoardStates.reserve(999);
}

std::string Board::printBoard() const {
	std::string board = "";
	for (int rank = 7; rank >= 0; rank--) {
		for (int file = 0; file < 8; file++) {
			int squareIndex			= 8 * rank + file;
			std::string pieceString = ". ";
			for (Color color : {WHITE, BLACK}) {
				for (Piece piece : {KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
					if ((boardState.pieces[color][piece] >> squareIndex) & 1) {
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

void Board::setToFen(const char *fen) {
	/* 
	 * heres a fen string for reference:
	 * r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7
	 */
	// clear board
	for (Color color : {WHITE, BLACK}) {
		for (Piece piece : {KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
			boardState.pieces[color][piece] = 0;
		}
	}

	int squareToWriteTo = Square::a8;
	do {
		if (*fen == ' ') {
			break;
		}

		if (*fen == '/') {
			squareToWriteTo -= 16;
		} else if (inRange(*fen, '1', '9')) {
			squareToWriteTo += *fen - '0';
		} else {
			Color colorOfPiece = islower(*fen) ? BLACK : WHITE;
			Piece typeOfPiece  = fenPieceChartoPieceType[toupper(*fen)];

			boardState.pieces[colorOfPiece][typeOfPiece] |= (1UL << squareToWriteTo);

			squareToWriteTo++;
		}
	} while (*fen++);

	boardState.sideToMove = *(++fen) == 'w' ? WHITE : BLACK;
	fen += 2;
	boardState.castlingRights.rights = 0;
	while (*fen != ' ') {
		switch (*fen) {
			case 'K':
				boardState.castlingRights.setWhiteKS(true);
				break;
			case 'Q':
				boardState.castlingRights.setWhiteQS(true);
				break;
			case 'k':
				boardState.castlingRights.setBlackKS(true);
				break;
			case 'q':
				boardState.castlingRights.setBlackQS(true);
				break;
		}
		fen++;
	}
	fen++;
	if (*fen != '-') {
		unsigned int enPassantSquareIndex = *fen++ - 'a';
		enPassantSquareIndex += 8 * (*fen - '1');
		boardState.enPassantSquare = 1UL << enPassantSquareIndex;
	}
	fen += 2;
	char *endOfClock;
	boardState.hmClock = std::strtol(fen, &endOfClock, 10);
	fen				   = endOfClock + 1;
	boardState.fmClock = std::strtol(fen, &endOfClock, 10);
}

bool Board::inIllegalCheck() {
	// make a copy, flip sides and see if the king is attacked, then restore previous state
	BoardState copiedBoardState = boardState;
	boardState.sideToMove		= (Color)!boardState.sideToMove;
	Bitboard king				= boardState.pieces[boardState.sideToMove][KING];
	bool inIllegalCheck			= king & moveGenerator.genAttacks();
	boardState					= copiedBoardState;
	return inIllegalCheck;
}

bool Board::gameOver() {
	if (!moveGenerator.genLegalMoves().size()) {
		return true;
	}
	if (boardState.hmClock == 100) {
		return true;
	}
	bool sufficientMaterial = false;
	for (Color color : {WHITE, BLACK}) {
		for (Piece piece : {QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
			if (boardState.pieces[color][piece] != 0) {
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

void Board::execute(const Move &m) {
	m_previousBoardStates.push_back(boardState);
	MoveFlag flags = m.getFlags();
	if (flags & KS_CASTLE) {
		if (boardState.sideToMove == WHITE) {
			boardState.pieces[WHITE][KING] ^= 0x50;
			boardState.pieces[WHITE][ROOK] ^= 0xa0;
		} else {
			boardState.pieces[BLACK][KING] ^= 0x5000000000000000;
			boardState.pieces[BLACK][ROOK] ^= 0xa000000000000000;
		}
	} else if (flags & QS_CASTLE) {
		if (boardState.sideToMove == WHITE) {
			boardState.pieces[WHITE][KING] ^= 0x14;
			boardState.pieces[WHITE][ROOK] ^= 0x9;
		} else {
			boardState.pieces[BLACK][KING] ^= 0x1400000000000000;
			boardState.pieces[BLACK][ROOK] ^= 0x900000000000000;
		}
	} else {
		if (flags & CAPTURE) {
			// removes piece from the destination square
			if (flags & EN_PASSANT) {
				boardState.pieces[!boardState.sideToMove][PAWN] &= ~(1UL << (m.getTo() + (boardState.sideToMove == WHITE ? -8 : 8)));
			} else {
				for (int i = 0; i < 6; i++) {
					boardState.pieces[!boardState.sideToMove][i] &= ~(1UL << m.getTo());
				}
			}
		}
		// moves the piece from its current square to the destination square
		Bitboard moveMask = (1UL << m.getFrom());
		if (!(flags & PROMOTION)) {
			moveMask += (1UL << m.getTo());
		}
		boardState.pieces[boardState.sideToMove][m.getPieceType()] ^= moveMask;
		if (flags & PROMOTION) {
			boardState.pieces[boardState.sideToMove][m.getPromoPiece()] |= 1UL << m.getTo();
		}
	}

	updateBoardStateGameData(m);
}

void Board::undoMove() {
	BoardState bs = m_previousBoardStates.back();
	m_previousBoardStates.pop_back();

	boardState = bs;
}

void Board::updateBoardStateGameData(const Move &m) {
	MoveFlag flags			   = m.getFlags();
	boardState.enPassantSquare = 0;
	boardState.hmClock++;
	if (boardState.sideToMove == BLACK) {
		boardState.fmClock++;
	}
	if (flags & PAWN_MOVE || flags & CAPTURE) {
		boardState.hmClock = 0;
	}

	// updates en passant square if dbl pawn push
	if (flags & DBL_PAWN) {
		boardState.enPassantSquare = 1UL << (boardState.sideToMove == WHITE ? m.getTo() - 8 : m.getTo() + 8);
	}

	updateCastlingRights(m);

	// switching side to move
	boardState.sideToMove = (Color)!boardState.sideToMove;
}

void Board::updateCastlingRights(const Move &m) {
	Square from = m.getFrom(), to = m.getTo();
	// if either side castles remove all their castling rights
	if (m.getFlags() & KS_CASTLE || m.getFlags() & QS_CASTLE) {
		if (boardState.sideToMove == WHITE) {
			boardState.castlingRights.setWhiteKS(false);
			boardState.castlingRights.setWhiteQS(false);
		} else {
			boardState.castlingRights.setBlackKS(false);
			boardState.castlingRights.setBlackQS(false);
		}
	}

	// if the kings move remove all their castling rights
	if ((boardState.pieces[WHITE][KING] >> to) & 1) {
		boardState.castlingRights.setWhiteKS(false);
		boardState.castlingRights.setWhiteQS(false);
	}
	if ((boardState.pieces[BLACK][KING] >> to) & 1) {
		boardState.castlingRights.setBlackKS(false);
		boardState.castlingRights.setBlackQS(false);
	}

	// if the rooks move remove castling rights for that side
	if ((boardState.pieces[WHITE][ROOK] >> to) & 1 && from == h1) {
		boardState.castlingRights.setWhiteKS(false);
	}
	if ((boardState.pieces[WHITE][ROOK] >> to) & 1 && from == a1) {
		boardState.castlingRights.setWhiteQS(false);
	}
	if ((boardState.pieces[BLACK][ROOK] >> to) & 1 && from == h8) {
		boardState.castlingRights.setBlackKS(false);
	}
	if ((boardState.pieces[BLACK][ROOK] >> to) & 1 && from == a1) {
		boardState.castlingRights.setBlackQS(false);
	}

	// if the rooks are captured remove castling rights for that side. dont need to check what the piece
	// being captured is, bc if its not a rook that means it has moved so remove castling rights anyway
	if (m.getFlags() & CAPTURE) {
		switch (to) {
			case h1:
				boardState.castlingRights.setWhiteKS(false);
				break;
			case a1:
				boardState.castlingRights.setWhiteQS(false);
				break;
			case h8:
				boardState.castlingRights.setBlackKS(false);
				break;
			case a8:
				boardState.castlingRights.setBlackQS(false);
				break;
			default: break;
		}
	}
}

Bitboard Board::whitePieces() const {
	return boardState.pieces[WHITE][PAWN] | boardState.pieces[WHITE][KNIGHT] | boardState.pieces[WHITE][BISHOP] | boardState.pieces[WHITE][ROOK] | boardState.pieces[WHITE][QUEEN] | boardState.pieces[WHITE][KING];
}

Bitboard Board::blackPieces() const {
	return boardState.pieces[BLACK][PAWN] | boardState.pieces[BLACK][KNIGHT] | boardState.pieces[BLACK][BISHOP] | boardState.pieces[BLACK][ROOK] | boardState.pieces[BLACK][QUEEN] | boardState.pieces[BLACK][KING];
}

void Board::reset() {
	// all the magic numbers just correspond to the bitboards for each of those
	// pieces as they would be at the starting position
	boardState.pieces[WHITE][KING]	 = 0x10;
	boardState.pieces[WHITE][QUEEN]	 = 0x8;
	boardState.pieces[WHITE][ROOK]	 = 0x81;
	boardState.pieces[WHITE][BISHOP] = 0x24;
	boardState.pieces[WHITE][KNIGHT] = 0x42;
	boardState.pieces[WHITE][PAWN]	 = 0xff00;
	boardState.pieces[BLACK][KING]	 = 0x1000000000000000;
	boardState.pieces[BLACK][QUEEN]	 = 0x800000000000000;
	boardState.pieces[BLACK][ROOK]	 = 0x8100000000000000;
	boardState.pieces[BLACK][BISHOP] = 0x2400000000000000;
	boardState.pieces[BLACK][KNIGHT] = 0x4200000000000000;
	boardState.pieces[BLACK][PAWN]	 = 0xff000000000000;
}
