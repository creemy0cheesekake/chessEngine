#include <unordered_map>

#include "move.hpp"
#include "util.hpp"

#include <iostream>

Move::Move()
	: from(NONE_SQUARE), to(NONE_SQUARE), promoPiece(NONE_PIECE), board(), pieceType(NONE_PIECE), flags(0) {}

Move::Move(Board board, Square from, Square to, Piece piece, Piece promoPiece)
	: from(from), to(to), promoPiece(promoPiece), board(board), pieceType(piece) {
	flags = genFlags(board);
}

std::ostream &operator<<(std::ostream &os, Move &m) {
	os << m.notation();
	return os;
}

unsigned int Move::genFlags(Board board) {
	unsigned int flags = 0;
	if (board.sideToMove == WHITE) {
		if (pieceType == PAWN && inRange(to, a8, h8)) {
			flags += PROMOTION;
		}
		if (pieceType == PAWN) {
			flags += PAWN_MOVE;
		}
		if ((board.blackPieces() >> to) & 1) {
			flags += CAPTURE;
		} else if (pieceType == KING && from == e1 && to == g1) {
			flags += KS_CASTLE;
		} else if (pieceType == KING && from == e1 && to == c1) {
			flags += QS_CASTLE;
		} else if (pieceType == PAWN && inRange(from, a2, h2) && inRange(to, a4, h4)) {
			flags += DBL_PAWN;
		} else if ((board.enPassantSquare >> to) & 1 && pieceType == PAWN) {
			flags += (EN_PASSANT + CAPTURE);
		}
	} else {
		if (pieceType == PAWN && inRange(to, a1, h1)) {
			flags += PROMOTION;
		}
		if (pieceType == PAWN) {
			flags += PAWN_MOVE;
		}
		if ((board.whitePieces() >> to) & 1) {
			flags += CAPTURE;
		} else if (pieceType == KING && from == e8 && to == g8) {
			flags += KS_CASTLE;
		} else if (pieceType == KING && from == e8 && to == c8) {
			flags += QS_CASTLE;
		} else if (pieceType == PAWN && inRange(from, a7, h7) && inRange(to, a5, h5)) {
			flags += DBL_PAWN;
		} else if ((board.enPassantSquare >> to) & 1 && pieceType == PAWN) {
			flags += (EN_PASSANT + CAPTURE);
		}
	}
	return flags;
}

unsigned int Move::getFrom() {
	return from;
}

unsigned int Move::getTo() {
	return to;
}

unsigned int Move::getFlags() {
	return flags;
}

void Move::setPromoPiece(Piece piece) {
	promoPiece = piece;
}

std::string Move::notation() {
	if (pieceType == NONE_PIECE) {
		return "null";
	}
	if (flags & KS_CASTLE) {
		return "0-0";
	}
	if (flags & QS_CASTLE) {
		return "0-0-0";
	}
	std::string notation = "";
	if (pieceType != PAWN) {
		notation += (std::unordered_map<Piece, char>){
			{KING, 'K'},
			{QUEEN, 'Q'},
			{ROOK, 'R'},
			{BISHOP, 'B'},
			{KNIGHT, 'N'},
		}[pieceType];
	}
	notation += from % 8 + 'a';
	notation += from / 8 + '1';
	notation += flags & CAPTURE ? "x" : "-";
	notation += to % 8 + 'a';
	notation += to / 8 + '1';
	if (flags & PROMOTION) {
		notation += "=";
		notation += (std::unordered_map<Piece, char>){
			{QUEEN, 'Q'},
			{ROOK, 'R'},
			{BISHOP, 'B'},
			{KNIGHT, 'N'},
		}[promoPiece];
	}
	return notation;
}

std::string Move::UCInotation() {
	std::string notation = "";
	notation += from % 8 + 'a';
	notation += from / 8 + '1';
	notation += to % 8 + 'a';
	notation += to / 8 + '1';
	if (flags & PROMOTION) {
		notation += (std::unordered_map<Piece, char>){
			{QUEEN, 'q'},
			{ROOK, 'r'},
			{BISHOP, 'b'},
			{KNIGHT, 'n'},
		}[promoPiece];
	}
	return notation;
}

Board Move::execute() {
	Board b			 = board;
	Color sideToMove = b.sideToMove;
	if (flags & KS_CASTLE) {
		if (sideToMove == WHITE) {
			b.pieces[WHITE][KING] ^= 0x50;
			b.pieces[WHITE][ROOK] ^= 0xa0;
		} else {
			b.pieces[BLACK][KING] ^= 0x5000000000000000;
			b.pieces[BLACK][ROOK] ^= 0xa000000000000000;
		}
	} else if (flags & QS_CASTLE) {
		if (sideToMove == WHITE) {
			b.pieces[WHITE][KING] ^= 0x14;
			b.pieces[WHITE][ROOK] ^= 0x9;
		} else {
			b.pieces[BLACK][KING] ^= 0x1400000000000000;
			b.pieces[BLACK][ROOK] ^= 0x900000000000000;
		}
	} else {
		if (flags & CAPTURE) {
			// removes piece from the destination square
			if (flags & EN_PASSANT) {
				b.pieces[!sideToMove][PAWN] &= ~(1UL << (to + (sideToMove == WHITE ? -8 : 8)));
			} else {
				for (int i = 0; i < 6; i++) {
					b.pieces[!sideToMove][i] &= ~(1UL << to);
				}
			}
		}
		// moves the piece from its current square to the destination square
		bitboard moveMask = (1UL << from);
		if (!(flags & PROMOTION)) {
			moveMask += (1UL << to);
		}
		b.pieces[sideToMove][pieceType] ^= moveMask;
		if (flags & PROMOTION) {
			b.pieces[sideToMove][promoPiece] |= 1UL << to;
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
	if (flags & PAWN_MOVE || flags & CAPTURE) {
		b.hmClock = 0;
	}

	// updates en passant square if dbl pawn push
	if (flags & DBL_PAWN) {
		b.enPassantSquare = 1UL << (b.sideToMove == WHITE ? to - 8 : to + 8);
	}

	updateCastlingRights(b);

	// switching side to move
	b.sideToMove = (Color)!b.sideToMove;
}

void Move::updateCastlingRights(Board &b) {
	// if either side castles remove all their castling rights
	if (flags & KS_CASTLE || flags & QS_CASTLE) {
		if (b.sideToMove == WHITE) {
			b.castlingRights.setWhiteKS(false);
			b.castlingRights.setWhiteQS(false);
		} else {
			b.castlingRights.setBlackKS(false);
			b.castlingRights.setBlackQS(false);
		}
	}

	// if the kings move remove all their castling rights
	if ((b.pieces[WHITE][KING] >> to) & 1) {
		b.castlingRights.setWhiteKS(false);
		b.castlingRights.setWhiteQS(false);
	}
	if ((b.pieces[BLACK][KING] >> to) & 1) {
		b.castlingRights.setBlackKS(false);
		b.castlingRights.setBlackQS(false);
	}

	// if the rooks move remove castling rights for that side
	if ((b.pieces[WHITE][ROOK] >> to) & 1 && from == h1) {
		b.castlingRights.setWhiteKS(false);
	}
	if ((b.pieces[WHITE][ROOK] >> to) & 1 && from == a1) {
		b.castlingRights.setWhiteQS(false);
	}
	if ((b.pieces[BLACK][ROOK] >> to) & 1 && from == h8) {
		b.castlingRights.setBlackKS(false);
	}
	if ((b.pieces[BLACK][ROOK] >> to) & 1 && from == a1) {
		b.castlingRights.setBlackQS(false);
	}

	// if the rooks are captured remove castling rights for that side. dont need to check what the piece
	// being captured is, bc if its not a rook that means it has moved so remove castling rights anyway
	if (flags & CAPTURE) {
		if (to == h1) {
			b.castlingRights.setWhiteKS(false);
		}
		if (to == a1) {
			b.castlingRights.setWhiteQS(false);
		}
		if (to == h8) {
			b.castlingRights.setBlackKS(false);
		}
		if (to == a1) {
			b.castlingRights.setBlackQS(false);
		}
	}
}
