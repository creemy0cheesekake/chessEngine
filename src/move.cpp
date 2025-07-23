#include "board.hpp"
#include "move.hpp"
#include "util.hpp"

Move::Move()
	: m_from(NONE_SQUARE), m_to(NONE_SQUARE), m_promoPiece(NONE_PIECE), m_pieceType(NONE_PIECE), m_flags(NORMAL_MOVE) {}

Move::Move(Board board, Square from, Square to, Piece piece, Piece promoPiece)
	: m_from(from), m_to(to), m_promoPiece(promoPiece), m_pieceType(piece) {
	m_flags = genFlags(board);
}

std::ostream &operator<<(std::ostream &os, Move &m) {
	os << m.notation();
	return os;
}

MoveFlag Move::genFlags(const Board &board) {
	unsigned int flags = 0;
	if (board.boardState.sideToMove == WHITE) {
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
		} else if ((board.boardState.enPassantSquare >> m_to) & 1 && m_pieceType == PAWN) {
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
		} else if ((board.boardState.enPassantSquare >> m_to) & 1 && m_pieceType == PAWN) {
			flags += (EN_PASSANT + CAPTURE);
		}
	}
	return (MoveFlag)flags;
}

Square Move::getFrom() const {
	return m_from;
}

Square Move::getTo() const {
	return m_to;
}

MoveFlag Move::getFlags() const {
	return m_flags;
}

Piece Move::getPromoPiece() const {
	return m_promoPiece;
}

void Move::setPromoPiece(Piece piece) {
	m_promoPiece = piece;
}

Piece Move::getPieceType() const {
	return m_pieceType;
}

std::string Move::notation() const {
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

std::string Move::UCInotation() const {
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
