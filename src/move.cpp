#include <unordered_map>

#include "move.hpp"
#include "util.cpp"

Move::Move(Board b, unsigned int f, unsigned int t, Piece p, unsigned int pP) {
	from	   = f;
	to		   = t;
	promoPiece = pP;
	board	   = b;
	pieceType  = p;

	flags = genFlags(board);
}

unsigned int Move::genFlags(Board board) {
	unsigned int flags = 0;
	if (board.sideToMove == WHITE) {
		if (pieceType == W_PAWN && inRange(to, 56, 63))
			flags += PROMOTION;
		if (pieceType == W_PAWN)
			flags += PAWN_MOVE;
		if ((board.blackPieces() >> to) & 1)
			flags += CAPTURE;
		else if (pieceType == W_KING && from == 4 && to == 6)
			flags += KS_CASTLE;
		else if (pieceType == W_KING && from == 4 && to == 2)
			flags += QS_CASTLE;
		else if (pieceType == W_PAWN && inRange(from, 8, 15) && inRange(to, 24, 31))
			flags += DBL_PAWN;
		else if ((board.enPassantSquare >> to) & 1 && pieceType == W_PAWN)
			flags += (EN_PASSANT + CAPTURE);
	} else {
		if (pieceType == B_PAWN && inRange(to, 0, 7))
			flags += PROMOTION;
		if (pieceType == B_PAWN)
			flags += PAWN_MOVE;
		if ((board.whitePieces() >> to) & 1)
			flags += CAPTURE;
		else if (pieceType == B_KING && from == 60 && to == 62)
			flags += KS_CASTLE;
		else if (pieceType == B_KING && from == 60 && to == 58)
			flags += QS_CASTLE;
		else if (pieceType == B_PAWN && inRange(from, 48, 55) && inRange(to, 32, 39))
			flags += DBL_PAWN;
		else if ((board.enPassantSquare >> to) & 1 && pieceType == B_PAWN)
			flags += (EN_PASSANT + CAPTURE);
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

void Move::setPromoPiece(unsigned int piece) {
	promoPiece = piece;
}

std::string Move::notation() {
	if (flags & KS_CASTLE) return "0-0";
	if (flags & QS_CASTLE) return "0-0-0";
	std::string notation = "";
	int charIndex		 = pieceType % 6;
	if (charIndex < 5)
		notation += (std::unordered_map<int, char>){
			{0, 'K'},
			{1, 'Q'},
			{2, 'R'},
			{3, 'B'},
			{4, 'N'},
		}[charIndex];
	notation += from % 8 + 97;
	notation += from / 8 + 49;
	notation += flags & CAPTURE ? "x" : "-";
	notation += to % 8 + 97;
	notation += to / 8 + 49;
	if (flags & PROMOTION) {
		notation += "=";
		notation += (std::unordered_map<int, char>){
			{1, 'Q'},
			{2, 'R'},
			{3, 'B'},
			{4, 'N'},
		}[promoPiece];
	}
	return notation;
}

std::string Move::UCInotation() {
	std::string notation = "";
	notation += from % 8 + 97;
	notation += from / 8 + 49;
	notation += to % 8 + 97;
	notation += to / 8 + 49;
	if (flags & PROMOTION)
		notation += (std::unordered_map<int, char>){
			{1, 'q'},
			{2, 'r'},
			{3, 'b'},
			{4, 'n'},
		}[promoPiece];
	return notation;
}

Board Move::execute() {
	Board b = board;
	if (flags & KS_CASTLE) {
		if (b.sideToMove == WHITE) {
			b.pieces[W_KING] ^= 0x50;
			b.pieces[W_ROOK] ^= 0xa0;
		} else {
			b.pieces[B_KING] ^= 0x5000000000000000;
			b.pieces[B_ROOK] ^= 0xa000000000000000;
		}
	} else if (flags & QS_CASTLE) {
		if (b.sideToMove == WHITE) {
			b.pieces[W_KING] ^= 0x14;
			b.pieces[W_ROOK] ^= 0x9;
		} else {
			b.pieces[B_KING] ^= 0x1400000000000000;
			b.pieces[B_ROOK] ^= 0x900000000000000;
		}
	} else {
		if (flags & CAPTURE) {
			// removes piece from the destination square
			if (flags & EN_PASSANT)
				if (b.sideToMove == WHITE) b.pieces[B_PAWN] &= ~(1UL << (to - 8));
				else b.pieces[W_PAWN] &= ~(1UL << (to + 8));
			else
				for (int i = 0; i < 12; i++)
					b.pieces[i] &= ~(1UL << to);
		}
		// moves the piece from its current square to the destination square
		bitboard moveMask = (1UL << from);
		if (!(flags & PROMOTION)) moveMask += (1UL << to);
		b.pieces[pieceType] ^= moveMask;
		if (flags & PROMOTION)
			b.pieces[promoPiece + (b.sideToMove == WHITE ? 0 : 6)] |= 1UL << to;
	}

	b = updateGameData(b);
	return b;
}

Board Move::updateGameData(Board b) {
	b.enPassantSquare = 0;
	b.hmClock++;
	if (b.sideToMove == BLACK)
		b.fmClock++;
	if (flags & PAWN_MOVE || flags & CAPTURE)
		b.hmClock = 0;

	// updates en passant square if dbl pawn push
	if (flags & DBL_PAWN)
		b.enPassantSquare = 1UL << (b.sideToMove == WHITE ? to - 8 : to + 8);

	b.castlingRights = updateCastlingRights(b);

	// switching side to move
	b.sideToMove = (Color)!b.sideToMove;
	return b;
}

unsigned short Move::updateCastlingRights(Board b) {
	// if either side castles remove all their castling rights
	if (b.sideToMove == WHITE) {
		if (flags & KS_CASTLE || flags & QS_CASTLE) b.castlingRights &= 3;
	} else if (flags & KS_CASTLE || flags & QS_CASTLE) b.castlingRights &= ~3;

	// if the kings move remove all their castling rights
	if ((b.pieces[W_KING] >> to) & 1)
		b.castlingRights &= 3;
	if ((b.pieces[B_KING] >> to) & 1)
		b.castlingRights &= ~3;

	// if the rooks move remove castling rights for that side
	if ((b.pieces[W_ROOK] >> to) & 1 && from == 7)
		b.castlingRights &= ~(1 << 3);
	if ((b.pieces[W_ROOK] >> to) & 1 && from == 0)
		b.castlingRights &= ~(1 << 2);
	if ((b.pieces[B_ROOK] >> to) & 1 && from == 63)
		b.castlingRights &= ~(1 << 1);
	if ((b.pieces[B_ROOK] >> to) & 1 && from == 56)
		b.castlingRights &= ~1;

	// if the rooks are captured remove castling rights for that side
	if (flags & CAPTURE) {
		if (to == 7)
			b.castlingRights &= ~(1 << 3);
		if (to == 0)
			b.castlingRights &= ~(1 << 2);
		if (to == 63)
			b.castlingRights &= ~(1 << 1);
		if (to == 56)
			b.castlingRights &= ~1;
	}
	return b.castlingRights;
}
