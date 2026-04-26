#include "board.hpp"
#include "move_gen.hpp"
#include "util.hpp"
#include "zobrist.hpp"

Board::Board() {
	m_previousBoardStates.reserve(999);
	boardState.hash					 = Zobrist::initialHash();
	boardState.allColorPieces[WHITE] = firstRank | secondRank;
	boardState.allColorPieces[BLACK] = seventhRank | eighthRank;
}

Board::Board(const Board& other) : boardState(other.boardState), moveGenerator(*this) {}

Board& Board::operator=(const Board& other) {
	if (this != &other) {
		boardState = other.boardState;
		moveGenerator.~MoveGen();
		new (&moveGenerator) MoveGen(*this);
	}
	return *this;
}

std::string Board::printBoard() const {
	std::string board = "";
	for (ptrdiff_t rank = 7; rank >= 0; rank--) {
		for (size_t file = 0; file < 8; file++) {
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

std::ostream& operator<<(std::ostream& os, const Board& b) {
	os << b.printBoard();
	return os;
}

void Board::setToFen(const char* fen) {
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
			Piece typeOfPiece  = fenPieceChartoPieceType(*fen);

			boardState.pieces[colorOfPiece][typeOfPiece] |= (1UL << squareToWriteTo);

			squareToWriteTo++;
			boardState.material += (colorOfPiece == WHITE ? 1 : -1) * pieceToCentipawns[typeOfPiece];
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
	char* endOfClock;
	boardState.hmClock				 = std::strtol(fen, &endOfClock, 10);
	fen								 = endOfClock + 1;
	boardState.fmClock				 = std::strtol(fen, &endOfClock, 10);
	boardState.hash					 = Zobrist::hash(boardState);
	boardState.allColorPieces[WHITE] = boardState.pieces[WHITE][PAWN] | boardState.pieces[WHITE][KNIGHT] | boardState.pieces[WHITE][BISHOP] | boardState.pieces[WHITE][ROOK] | boardState.pieces[WHITE][QUEEN] | boardState.pieces[WHITE][KING];
	boardState.allColorPieces[BLACK] = boardState.pieces[BLACK][PAWN] | boardState.pieces[BLACK][KNIGHT] | boardState.pieces[BLACK][BISHOP] | boardState.pieces[BLACK][ROOK] | boardState.pieces[BLACK][QUEEN] | boardState.pieces[BLACK][KING];
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

bool Board::is50MoveRule() const {
	return boardState.hmClock == 100;
}

bool Board::isInsufficientMaterial() const {
	// FIDE rules, not USCF

	// if there are any pawns, rooks, or queens, material is sufficient.
	if (boardState.pieces[WHITE][PAWN] | boardState.pieces[WHITE][ROOK] | boardState.pieces[WHITE][QUEEN] | boardState.pieces[BLACK][PAWN] | boardState.pieces[BLACK][ROOK] | boardState.pieces[BLACK][QUEEN]) {
		return false;
	}

	// count minor pieces
	int whiteKnights = std::popcount(boardState.pieces[WHITE][KNIGHT]);
	int blackKnights = std::popcount(boardState.pieces[BLACK][KNIGHT]);
	int whiteBishops = std::popcount(boardState.pieces[WHITE][BISHOP]);
	int blackBishops = std::popcount(boardState.pieces[BLACK][BISHOP]);

	int whiteMinors = whiteKnights + whiteBishops;
	int blackMinors = blackKnights + blackBishops;
	int totalMinors = whiteMinors + blackMinors;

	// king vs king or king + minor vs king
	if (totalMinors <= 1) return true;

	// if there are any knights there is sufficient material
	if (whiteKnights != 0 || blackKnights != 0) return false;

	// at this point there are only bishops on the board
	constexpr Bitboard LIGHT_SQUARES = 0x55AA55AA55AA55AAULL;
	constexpr Bitboard DARK_SQUARES	 = 0xAA55AA55AA55AA55ULL;

	Bitboard bishops		   = boardState.pieces[WHITE][BISHOP] | boardState.pieces[BLACK][BISHOP];
	bool bishopsOnLightSquares = bishops & LIGHT_SQUARES;
	bool bishopsOnDarkSquares  = bishops & DARK_SQUARES;

	return bishopsOnLightSquares != bishopsOnDarkSquares;
}

bool Board::isGameOver() {
	return is50MoveRule() || isInsufficientMaterial() || !moveGenerator.hasLegalMoves();
}

void Board::execute(const Move& m) {
	m_previousBoardStates.push_back(boardState);
	MoveFlag flags = m.getFlags();

	if (boardState.enPassantSquare) {
		boardState.hash ^= Zobrist::epFileKeys[bitscan(boardState.enPassantSquare) % 8];
	}
	boardState.hash ^= Zobrist::castlingKeys[boardState.castlingRights.rights];
	boardState.hash ^= Zobrist::pieceKeys[(boardState.sideToMove == BLACK ? 6 : 0) + m.getPieceType()][m.getFrom()];

	if (flags & KS_CASTLE) {
		if (boardState.sideToMove == WHITE) {
			boardState.pieces[WHITE][KING] ^= 0x50;
			boardState.pieces[WHITE][ROOK] ^= 0xa0;
			boardState.allColorPieces[WHITE] ^= 0x50;
			boardState.allColorPieces[WHITE] ^= 0xa0;
		} else {
			boardState.pieces[BLACK][KING] ^= 0x5000000000000000;
			boardState.pieces[BLACK][ROOK] ^= 0xa000000000000000;
			boardState.allColorPieces[BLACK] ^= 0x5000000000000000;
			boardState.allColorPieces[BLACK] ^= 0xa000000000000000;
		}
	} else if (flags & QS_CASTLE) {
		if (boardState.sideToMove == WHITE) {
			boardState.pieces[WHITE][KING] ^= 0x14;
			boardState.pieces[WHITE][ROOK] ^= 0x9;
			boardState.allColorPieces[WHITE] ^= 0x14;
			boardState.allColorPieces[WHITE] ^= 0x9;
		} else {
			boardState.pieces[BLACK][KING] ^= 0x1400000000000000;
			boardState.pieces[BLACK][ROOK] ^= 0x900000000000000;
			boardState.allColorPieces[BLACK] ^= 0x1400000000000000;
			boardState.allColorPieces[BLACK] ^= 0x900000000000000;
		}
	} else {
		if (flags & CAPTURE) {
			// removes piece from the destination square
			if (flags & EN_PASSANT) {
				int capturedPawnSquare = m.getTo() + (boardState.sideToMove == WHITE ? -8 : 8);
				boardState.hash ^= Zobrist::pieceKeys[(boardState.sideToMove == WHITE ? 6 : 0) + PAWN][capturedPawnSquare];
				boardState.pieces[!boardState.sideToMove][PAWN] &= ~(1UL << (capturedPawnSquare));
				boardState.allColorPieces[!boardState.sideToMove] &= ~(1UL << (capturedPawnSquare));
				boardState.material += boardState.sideToMove == WHITE ? 100 : -100;
			} else {
				for (Piece p : {KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
					auto& theirPieces = boardState.pieces[!boardState.sideToMove];
					if (theirPieces[p] & 1UL << m.getTo()) {
						theirPieces[p] &= ~(1UL << m.getTo());
						boardState.allColorPieces[!boardState.sideToMove] &= ~(1UL << m.getTo());
						boardState.hash ^= Zobrist::pieceKeys[(boardState.sideToMove == WHITE ? 6 : 0) + p][m.getTo()];
						boardState.material += (boardState.sideToMove == WHITE ? 1 : -1) * pieceToCentipawns[p];
						break;
					}
				}
			}
		}
		// moves the piece from its current square to the destination square
		Bitboard moveMask = (1UL << m.getFrom());
		if (!(flags & PROMOTION)) {
			moveMask += (1UL << m.getTo());
		}
		boardState.pieces[boardState.sideToMove][m.getPieceType()] ^= moveMask;
		boardState.allColorPieces[boardState.sideToMove] ^= moveMask;
		if (flags & PROMOTION) {
			static constexpr int PAWN_VAL = pieceToCentipawns[PAWN];
			boardState.material += (boardState.sideToMove == WHITE ? 1 : -1) * (pieceToCentipawns[m.getPromoPiece()] - PAWN_VAL);
			boardState.pieces[boardState.sideToMove][m.getPromoPiece()] |= 1UL << m.getTo();
			boardState.allColorPieces[boardState.sideToMove] |= 1UL << m.getTo();
			boardState.hash ^= Zobrist::pieceKeys[(boardState.sideToMove == BLACK ? 6 : 0) + PAWN][m.getTo()];
			boardState.hash ^= Zobrist::pieceKeys[(boardState.sideToMove == BLACK ? 6 : 0) + m.getPromoPiece()][m.getTo()];
		}
	}

	updateBoardStateGameData(m);
	boardState.hash ^= Zobrist::pieceKeys[(boardState.sideToMove == BLACK ? 6 : 0) + m.getPieceType()][m.getTo()];
}

void Board::undoMove() {
	BoardState bs = m_previousBoardStates.back();
	m_previousBoardStates.pop_back();
	boardState = bs;
}

void Board::updateBoardStateGameData(const Move& m) {
	MoveFlag flags = m.getFlags();
	[[unlikely]]
	if (boardState.enPassantSquare) {
		boardState.hash ^= Zobrist::epFileKeys[bitscan(boardState.enPassantSquare) % 8];
	}
	// branchlessly increments fmClock if sideToMove == BLACK
	boardState.fmClock += 1 & -boardState.sideToMove;
	// branchlessly resets hmClock if flags & PAWN_MOVE | CAPTURE else increments
	boardState.hmClock = (boardState.hmClock + 1) & -!(flags & (PAWN_MOVE | CAPTURE));

	// updates en passant square if dbl pawn push
	boardState.enPassantSquare = 0;
	if (flags & DBL_PAWN) {
		int enPassantSquareIndex   = m.getTo() + (boardState.sideToMove == WHITE ? -8 : 8);
		boardState.enPassantSquare = 1UL << enPassantSquareIndex;
		boardState.hash ^= Zobrist::epFileKeys[enPassantSquareIndex % 8];
	}

	updateCastlingRights(m);

	// switching side to move
	boardState.sideToMove = (Color)!boardState.sideToMove;
	boardState.hash ^= Zobrist::blackSideKey;
}

void Board::updateCastlingRights(const Move& m) {
	boardState.castlingRights.rights &= castlingMask[m.getFrom()];
	boardState.castlingRights.rights &= castlingMask[m.getTo()];
	boardState.hash ^= Zobrist::castlingKeys[boardState.castlingRights.rights];
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
