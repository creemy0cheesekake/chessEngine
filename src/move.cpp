#include "move.hpp"
#include "util.hpp"

Move::Move()
	: m_from(NONE_SQUARE), m_to(NONE_SQUARE), m_promoPiece(NONE_PIECE), board(), m_pieceType(NONE_PIECE), m_flags(0) {}

Move::Move(Board board, Square from, Square to, Piece piece, Piece promoPiece)
	: m_from(from), m_to(to), m_promoPiece(promoPiece), board(board), m_pieceType(piece) {
	m_flags = genFlags(board);
}

std::ostream &operator<<(std::ostream &os, Move &m) {
	os << m.notation();
	return os;
}

unsigned int Move::genFlags(Board board) {
	unsigned int flags = 0;
	if (board.sideToMove == WHITE) {
		if (m_pieceType == PAWN && inRange(m_to, a8, h8)) {
			flags += PROMOTION;
		}
		if (m_pieceType == PAWN) {
			flags += PAWN_MOVE;
		}
		if ((board.blackPieces() >> m_to) & 1) {
			flags += CAPTURE;
		} else if (m_pieceType == KING && m_from == e1 && m_to == g1) {
			flags += KS_CASTLE;
		} else if (m_pieceType == KING && m_from == e1 && m_to == c1) {
			flags += QS_CASTLE;
		} else if (m_pieceType == PAWN && inRange(m_from, a2, h2) && inRange(m_to, a4, h4)) {
			flags += DBL_PAWN;
		} else if ((board.enPassantSquare >> m_to) & 1 && m_pieceType == PAWN) {
			flags += (EN_PASSANT + CAPTURE);
		}
	} else {
		if (m_pieceType == PAWN && inRange(m_to, a1, h1)) {
			flags += PROMOTION;
		}
		if (m_pieceType == PAWN) {
			flags += PAWN_MOVE;
		}
		if ((board.whitePieces() >> m_to) & 1) {
			flags += CAPTURE;
		} else if (m_pieceType == KING && m_from == e8 && m_to == g8) {
			flags += KS_CASTLE;
		} else if (m_pieceType == KING && m_from == e8 && m_to == c8) {
			flags += QS_CASTLE;
		} else if (m_pieceType == PAWN && inRange(m_from, a7, h7) && inRange(m_to, a5, h5)) {
			flags += DBL_PAWN;
		} else if ((board.enPassantSquare >> m_to) & 1 && m_pieceType == PAWN) {
			flags += (EN_PASSANT + CAPTURE);
		}
	}
	return flags;
}

unsigned int Move::getFrom() {
	return m_from;
}

unsigned int Move::getTo() {
	return m_to;
}

unsigned int Move::getFlags() {
	return m_flags;
}

void Move::setPromoPiece(Piece piece) {
	m_promoPiece = piece;
}

std::string Move::notation() {
	if (m_pieceType == NONE_PIECE) {
		return "null";
	}
	if (m_flags & KS_CASTLE) {
		return "0-0";
	}
	if (m_flags & QS_CASTLE) {
		return "0-0-0";
	}
	std::string notation = "";
	if (m_pieceType != PAWN) {
		notation += pieceToNotationChar[m_pieceType];
	}
	notation += m_from % 8 + 'a';
	notation += m_from / 8 + '1';
	notation += m_flags & CAPTURE ? "x" : "-";
	notation += m_to % 8 + 'a';
	notation += m_to / 8 + '1';
	if (m_flags & PROMOTION) {
		notation += "=";
		notation += pieceToNotationChar[m_promoPiece];
	}
	return notation;
}

std::string Move::UCInotation() {
	std::string notation = "";
	notation += m_from % 8 + 'a';
	notation += m_from / 8 + '1';
	notation += m_to % 8 + 'a';
	notation += m_to / 8 + '1';
	if (m_flags & PROMOTION) {
		notation += promotionPieceToUCINotationChar[m_promoPiece];
	}
	return notation;
}

Board Move::execute() {
	Board b			 = board;
	Color sideToMove = b.sideToMove;
	if (m_flags & KS_CASTLE) {
		if (sideToMove == WHITE) {
			b.pieces[WHITE][KING] ^= 0x50;
			b.pieces[WHITE][ROOK] ^= 0xa0;
		} else {
			b.pieces[BLACK][KING] ^= 0x5000000000000000;
			b.pieces[BLACK][ROOK] ^= 0xa000000000000000;
		}
	} else if (m_flags & QS_CASTLE) {
		if (sideToMove == WHITE) {
			b.pieces[WHITE][KING] ^= 0x14;
			b.pieces[WHITE][ROOK] ^= 0x9;
		} else {
			b.pieces[BLACK][KING] ^= 0x1400000000000000;
			b.pieces[BLACK][ROOK] ^= 0x900000000000000;
		}
	} else {
		if (m_flags & CAPTURE) {
			// removes piece from the destination square
			if (m_flags & EN_PASSANT) {
				b.pieces[!sideToMove][PAWN] &= ~(1UL << (m_to + (sideToMove == WHITE ? -8 : 8)));
			} else {
				for (int i = 0; i < 6; i++) {
					b.pieces[!sideToMove][i] &= ~(1UL << m_to);
				}
			}
		}
		// moves the piece from its current square to the destination square
		Bitboard moveMask = (1UL << m_from);
		if (!(m_flags & PROMOTION)) {
			moveMask += (1UL << m_to);
		}
		b.pieces[sideToMove][m_pieceType] ^= moveMask;
		if (m_flags & PROMOTION) {
			b.pieces[sideToMove][m_promoPiece] |= 1UL << m_to;
		}
	}

	updateGameData(b);
	return b;
}

void Move::updateGameData(Board &b) {
	b.enPassantSquare = 0;
	b.hmClock++;
	if (b.sideToMove == BLACK) {
		b.fmClock++;
	}
	if (m_flags & PAWN_MOVE || m_flags & CAPTURE) {
		b.hmClock = 0;
	}

	// updates en passant square if dbl pawn push
	if (m_flags & DBL_PAWN) {
		b.enPassantSquare = 1UL << (b.sideToMove == WHITE ? m_to - 8 : m_to + 8);
	}

	updateCastlingRights(b);

	// switching side to move
	b.sideToMove = (Color)!b.sideToMove;
}

void Move::updateCastlingRights(Board &b) {
	// if either side castles remove all their castling rights
	if (m_flags & KS_CASTLE || m_flags & QS_CASTLE) {
		if (b.sideToMove == WHITE) {
			b.castlingRights.setWhiteKS(false);
			b.castlingRights.setWhiteQS(false);
		} else {
			b.castlingRights.setBlackKS(false);
			b.castlingRights.setBlackQS(false);
		}
	}

	// if the kings move remove all their castling rights
	if ((b.pieces[WHITE][KING] >> m_to) & 1) {
		b.castlingRights.setWhiteKS(false);
		b.castlingRights.setWhiteQS(false);
	}
	if ((b.pieces[BLACK][KING] >> m_to) & 1) {
		b.castlingRights.setBlackKS(false);
		b.castlingRights.setBlackQS(false);
	}

	// if the rooks move remove castling rights for that side
	if ((b.pieces[WHITE][ROOK] >> m_to) & 1 && m_from == h1) {
		b.castlingRights.setWhiteKS(false);
	}
	if ((b.pieces[WHITE][ROOK] >> m_to) & 1 && m_from == a1) {
		b.castlingRights.setWhiteQS(false);
	}
	if ((b.pieces[BLACK][ROOK] >> m_to) & 1 && m_from == h8) {
		b.castlingRights.setBlackKS(false);
	}
	if ((b.pieces[BLACK][ROOK] >> m_to) & 1 && m_from == a1) {
		b.castlingRights.setBlackQS(false);
	}

	// if the rooks are captured remove castling rights for that side. dont need to check what the piece
	// being captured is, bc if its not a rook that means it has moved so remove castling rights anyway
	if (m_flags & CAPTURE) {
		if (m_to == h1) {
			b.castlingRights.setWhiteKS(false);
		}
		if (m_to == a1) {
			b.castlingRights.setWhiteQS(false);
		}
		if (m_to == h8) {
			b.castlingRights.setBlackKS(false);
		}
		if (m_to == a1) {
			b.castlingRights.setBlackQS(false);
		}
	}
}
