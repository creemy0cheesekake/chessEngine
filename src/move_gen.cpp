#include "move_gen.hpp"
#include <iostream>
#include <type_traits>
#include "board.hpp"
#include "lookup_tables.hpp"
#include "util.hpp"

MoveGen::MoveGen(Board& b) : m_board(b) {
	m_attacks = genAttacks();
}

void MoveGen::genPawnMoves(Moves& pseudoLegalMoves, Moves& pseudoLegalCaptures) const {
	Bitboard pawns = m_board.boardState.pieces[m_board.boardState.sideToMove][PAWN];
	if (!pawns) {
		return;
	}
	Bitboard allPieces	 = m_board.whitePieces() | m_board.blackPieces();
	Bitboard theirPieces = m_board.boardState.sideToMove == WHITE ? m_board.blackPieces() : m_board.whitePieces();
	do {
		Bitboard pawn				= LS1B(pawns);
		Bitboard captureMoveTargets = (pawn & ~aFile ? pawn << 7 : 0) | (pawn & ~hFile ? pawn << 9 : 0);
		if (m_board.boardState.sideToMove == BLACK) {
			captureMoveTargets >>= 16;
		}
		captureMoveTargets &= theirPieces | m_board.boardState.enPassantSquare;
		Bitboard pushMoveTargets	= (m_board.boardState.sideToMove == WHITE ? pawn << 8 : pawn >> 8) & ~allPieces;
		Bitboard dblPushMoveTargets = 0;
		if (pawn & (secondRank | seventhRank)) {
			dblPushMoveTargets = (m_board.boardState.sideToMove == WHITE ? pushMoveTargets << 8 : pushMoveTargets >> 8) & ~(allPieces ^ pawn);
		}
		Bitboard pawnMoveTargets = captureMoveTargets | pushMoveTargets | dblPushMoveTargets;
		if (pawnMoveTargets != 0) {
			do {
				Bitboard moveTarget	  = LS1B(pawnMoveTargets);
				Moves& movesContainer = moveTarget & captureMoveTargets ? pseudoLegalCaptures : pseudoLegalMoves;
				if (moveTarget & (firstRank | eighthRank)) {
					for (Piece promoPiece : {QUEEN, ROOK, BISHOP, KNIGHT}) {
						movesContainer.emplace_back(m_board, (Square)bitscan(pawn), (Square)bitscan(moveTarget), PAWN, promoPiece);
					}
				} else {
					movesContainer.emplace_back(m_board, (Square)bitscan(pawn), (Square)bitscan(moveTarget), PAWN);
				}
			} while (removeLS1B(pawnMoveTargets));
		}
	} while (removeLS1B(pawns));
}

void MoveGen::genKnightMoves(Moves& pseudoLegalMoves, Moves& pseudoLegalCaptures) const {
	Bitboard knights = m_board.boardState.pieces[m_board.boardState.sideToMove][KNIGHT];
	if (!knights) {
		return;
	}
	Bitboard yourPieces	 = m_board.boardState.sideToMove == WHITE ? m_board.whitePieces() : m_board.blackPieces();
	Bitboard theirPieces = m_board.boardState.sideToMove == WHITE ? m_board.blackPieces() : m_board.whitePieces();
	do {
		Bitboard knight			   = LS1B(knights);
		Bitboard knightMoveTargets = LookupTables::s_knightAttacks[bitscan(knight)] & ~yourPieces;
		if (knightMoveTargets) {
			do {
				Bitboard moveTarget	  = LS1B(knightMoveTargets);
				Moves& movesContainer = moveTarget & theirPieces ? pseudoLegalCaptures : pseudoLegalMoves;
				movesContainer.emplace_back(m_board, (Square)bitscan(knight), (Square)bitscan(moveTarget), KNIGHT);
			} while (removeLS1B(knightMoveTargets));
		}
	} while (removeLS1B(knights));
}

bool MoveGen::inCheck() const {
	return m_attacks & m_board.boardState.pieces[m_board.boardState.sideToMove][KING];
}

void MoveGen::genKingMoves(Moves& pseudoLegalMoves, Moves& pseudoLegalCaptures) const {
	Bitboard yourPieces		 = m_board.boardState.sideToMove == WHITE ? m_board.whitePieces() : m_board.blackPieces();
	Bitboard theirPieces	 = m_board.boardState.sideToMove == WHITE ? m_board.blackPieces() : m_board.whitePieces();
	Bitboard king			 = m_board.boardState.pieces[m_board.boardState.sideToMove][KING];
	Bitboard kingMoveTargets = LookupTables::s_kingAttacks[bitscan(king)] & ~yourPieces;
	if (kingMoveTargets) {
		do {
			Bitboard moveTarget	  = LS1B(kingMoveTargets);
			Moves& movesContainer = moveTarget & theirPieces ? pseudoLegalCaptures : pseudoLegalMoves;
			movesContainer.emplace_back(m_board, (Square)bitscan(king), (Square)bitscan(moveTarget), KING);
		} while (removeLS1B(kingMoveTargets));
	}
}

void MoveGen::genCastlingMoves(Moves& pseudoLegalMoves) const {
	if (inCheck()) {
		return;
	}
	Bitboard king				  = m_board.boardState.pieces[m_board.boardState.sideToMove][KING];
	unsigned short castlingRights = 0;
	Bitboard allPieces			  = m_board.whitePieces() | m_board.blackPieces();
	if (m_board.boardState.sideToMove == WHITE) {
		castlingRights = m_board.boardState.castlingRights.getWhiteRights();
		// kingside castling
		if (castlingRights & 0b10 && !((m_attacks | allPieces) & (1UL << f1 | 1UL << g1))) {
			pseudoLegalMoves.emplace_back(m_board, e1, g1, KING);
		}
		// queenside castling
		if (castlingRights & 0b01 && !((m_attacks | allPieces) & (1UL << d1 | 1UL << c1)) && ~allPieces & 1UL << b1) {
			pseudoLegalMoves.emplace_back(m_board, e1, c1, KING);
		}
	} else {
		castlingRights = m_board.boardState.castlingRights.getBlackRights();
		// kingside castling
		if (castlingRights & 0b10 && !((m_attacks | allPieces) & (1UL << f8 | 1UL << g8))) {
			pseudoLegalMoves.emplace_back(m_board, e8, g8, KING);
		}
		// queenside castling
		if (castlingRights & 0b01 && !((m_attacks | allPieces) & (1UL << d8 | 1UL << c8)) && ~allPieces & 1UL << b8) {
			pseudoLegalMoves.emplace_back(m_board, e8, c8, KING);
		}
	}
}

void MoveGen::genSlidingPieces(Moves& pseudoLegalMoves, Moves& pseudoLegalCaptures, Piece p, Bitboard pieces, SlidingPieceDirectionFlags direction) const {
	if (!pieces) {
		return;
	}
	Bitboard yourPieces	 = m_board.boardState.sideToMove == WHITE ? m_board.whitePieces() : m_board.blackPieces();
	Bitboard theirPieces = m_board.boardState.sideToMove == WHITE ? m_board.blackPieces() : m_board.whitePieces();
	Bitboard allPieces	 = m_board.whitePieces() | m_board.blackPieces();

	do {
		Bitboard piece		  = LS1B(pieces);
		Bitboard straightRays = 0;
		Bitboard diagonalRays = 0;
		if (direction & STRAIGHT) {
			Bitboard Nrays = piece, Erays = piece, Wrays = piece, Srays = piece;
			while (!(Nrays & eighthRank)) {
				Nrays |= Nrays << 8;
				if (allPieces & MS1B(Nrays)) {
					break;
				}
			}
			while (!(Erays & hFile)) {
				Erays |= Erays << 1;
				if (allPieces & MS1B(Erays)) {
					break;
				}
			}
			while (!(Wrays & aFile)) {
				Wrays |= Wrays >> 1;
				if (allPieces & LS1B(Wrays)) {
					break;
				}
			}
			while (!(Srays & firstRank)) {
				Srays |= Srays >> 8;
				if (allPieces & LS1B(Srays)) {
					break;
				}
			}
			straightRays = Nrays | Erays | Wrays | Srays;
		}
		if (direction & DIAGONAL) {
			Bitboard NErays = piece, NWrays = piece, SWrays = piece, SErays = piece;
			while (!(NErays & (eighthRank | hFile))) {
				NErays |= NErays << 9;
				if (allPieces & MS1B(NErays)) {
					break;
				}
			}
			while (!(NWrays & (eighthRank | aFile))) {
				NWrays |= NWrays << 7;
				if (allPieces & MS1B(NWrays)) {
					break;
				}
			}
			while (!(SWrays & (firstRank | aFile))) {
				SWrays |= SWrays >> 9;
				if (allPieces & LS1B(SWrays)) {
					break;
				}
			}
			while (!(SErays & (firstRank | hFile))) {
				SErays |= SErays >> 7;
				if (allPieces & LS1B(SErays)) {
					break;
				}
			}
			diagonalRays = NErays | NWrays | SWrays | SErays;
		}

		Bitboard rays = ((straightRays | diagonalRays) ^ piece) & ~yourPieces;

		if (rays) {
			do {
				Bitboard moveTarget	  = LS1B(rays);
				Moves& movesContainer = moveTarget & theirPieces ? pseudoLegalCaptures : pseudoLegalMoves;
				movesContainer.emplace_back(m_board, (Square)bitscan(piece), (Square)bitscan(moveTarget), p);
			} while (removeLS1B(rays));
		}
	} while (removeLS1B(pieces));
}

void MoveGen::genBishopMoves(Moves& pseudoLegalMoves, Moves& pseudoLegalCaptures) const {
	Bitboard bishops = m_board.boardState.pieces[m_board.boardState.sideToMove][BISHOP];
	genSlidingPieces(pseudoLegalMoves, pseudoLegalCaptures, BISHOP, bishops, SlidingPieceDirectionFlags::DIAGONAL);
}

void MoveGen::genRookMoves(Moves& pseudoLegalMoves, Moves& pseudoLegalCaptures) const {
	Bitboard rooks = m_board.boardState.pieces[m_board.boardState.sideToMove][ROOK];
	genSlidingPieces(pseudoLegalMoves, pseudoLegalCaptures, ROOK, rooks, SlidingPieceDirectionFlags::STRAIGHT);
}

void MoveGen::genQueenMoves(Moves& pseudoLegalMoves, Moves& pseudoLegalCaptures) const {
	Bitboard queens = m_board.boardState.pieces[m_board.boardState.sideToMove][QUEEN];
	genSlidingPieces(pseudoLegalMoves, pseudoLegalCaptures, QUEEN, queens, SlidingPieceDirectionFlags(DIAGONAL | STRAIGHT));
}

Moves MoveGen::genLegalMoves() {
	m_attacks = genAttacks();
	Moves pseudoLegalMoves;
	Moves pseudoLegalCaptures;
	Moves moves;
	pseudoLegalMoves.reserve(218);	// known approximation for maximum number of legal moves possible in a position
	moves.reserve(218);

	genPawnMoves(pseudoLegalMoves, pseudoLegalCaptures);
	genKnightMoves(pseudoLegalMoves, pseudoLegalCaptures);
	genKingMoves(pseudoLegalMoves, pseudoLegalCaptures);
	genBishopMoves(pseudoLegalMoves, pseudoLegalCaptures);
	genRookMoves(pseudoLegalMoves, pseudoLegalCaptures);
	genQueenMoves(pseudoLegalMoves, pseudoLegalCaptures);
	genCastlingMoves(pseudoLegalMoves);

	Color victimColor	  = m_board.boardState.sideToMove == WHITE ? BLACK : WHITE;
	auto getMVV_LVA_score = [&](Move& m) {
		if (m.getFlags() & EN_PASSANT) {
			return MVV_LVA_table[PAWN][PAWN];
		}
		Bitboard toSquare = 1UL << m.getTo();
		Piece victimPiece = NONE_PIECE;
		for (int i = QUEEN; i < NONE_PIECE; i++) {
			Bitboard pieceBoard = m_board.boardState.pieces[victimColor][i];
			if (pieceBoard & toSquare) {
				victimPiece = (Piece)i;
				break;
			}
		}
		return MVV_LVA_table[m.getPieceType()][victimPiece];
	};

	std::sort(pseudoLegalCaptures.begin(), pseudoLegalCaptures.end(), [&](Move a, Move b) {
		return getMVV_LVA_score(a) > getMVV_LVA_score(b);
	});

	auto insertLegalMoves = [&](const Moves& movesContainer) {
		for (const Move& m : movesContainer) {
			m_board.execute(m);
			if (!m_board.inIllegalCheck()) {
				moves.push_back(m);
			}
			m_board.undoMove();
		}
	};

	insertLegalMoves(pseudoLegalCaptures);
	insertLegalMoves(pseudoLegalMoves);
	return moves;
}
