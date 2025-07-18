#include "move_gen.hpp"
#include "board.hpp"
#include "lookup_tables.hpp"
#include "util.hpp"

MoveGen::MoveGen(Board b) {
	m_board	  = b;
	m_attacks = genAttacks();
	m_pseudoLegalMoves.reserve(218);  // known approximation for maximum number of legal moves possible in a position
}

void MoveGen::genPawnMoves() {
	Bitboard pawns = m_board.pieces[m_board.sideToMove][PAWN];
	if (!pawns) {
		return;
	}
	Bitboard allPieces	 = m_board.whitePieces() | m_board.blackPieces();
	Bitboard theirPieces = m_board.sideToMove == WHITE ? m_board.blackPieces() : m_board.whitePieces();
	do {
		Bitboard pawn				= LS1B(pawns);
		Bitboard captureMoveTargets = (pawn & ~aFile ? pawn << 7 : 0) | (pawn & ~hFile ? pawn << 9 : 0);
		if (m_board.sideToMove == BLACK) {
			captureMoveTargets >>= 16;
		}
		captureMoveTargets &= theirPieces | m_board.enPassantSquare;
		Bitboard pushMoveTargets	= (m_board.sideToMove == WHITE ? pawn << 8 : pawn >> 8) & ~allPieces;
		Bitboard dblPushMoveTargets = 0;
		if (pawn & (secondRank | seventhRank)) {
			dblPushMoveTargets = (m_board.sideToMove == WHITE ? pushMoveTargets << 8 : pushMoveTargets >> 8) & ~(allPieces ^ pawn);
		}
		Bitboard pawnMoveTargets = captureMoveTargets | pushMoveTargets | dblPushMoveTargets;
		if (pawnMoveTargets != 0) {
			do {
				Bitboard moveTarget = LS1B(pawnMoveTargets);
				if (moveTarget & (firstRank | eighthRank)) {
					for (Piece promoPiece : {QUEEN, ROOK, BISHOP, KNIGHT}) {
						m_pseudoLegalMoves.emplace_back(m_board, (Square)bitscan(pawn), (Square)bitscan(moveTarget), PAWN, promoPiece);
					}
				} else {
					m_pseudoLegalMoves.emplace_back(m_board, (Square)bitscan(pawn), (Square)bitscan(moveTarget), PAWN);
				}
			} while (removeLS1B(pawnMoveTargets));
		}
	} while (removeLS1B(pawns));
}

void MoveGen::genKnightMoves() {
	Bitboard knights = m_board.pieces[m_board.sideToMove][KNIGHT];
	if (!knights) {
		return;
	}
	Bitboard yourPieces = m_board.sideToMove == WHITE ? m_board.whitePieces() : m_board.blackPieces();
	do {
		Bitboard knight			   = LS1B(knights);
		Bitboard knightMoveTargets = LookupTables::s_knightAttacks[bitscan(knight)] & ~yourPieces;
		if (knightMoveTargets) {
			do {
				Bitboard moveTarget = LS1B(knightMoveTargets);
				m_pseudoLegalMoves.emplace_back(m_board, (Square)bitscan(knight), (Square)bitscan(moveTarget), KNIGHT);
			} while (removeLS1B(knightMoveTargets));
		}
	} while (removeLS1B(knights));
}

bool MoveGen::inCheck() {
	return m_attacks & m_board.pieces[m_board.sideToMove][KING];
}

void MoveGen::genKingMoves() {
	Bitboard yourPieces		 = m_board.sideToMove == WHITE ? m_board.whitePieces() : m_board.blackPieces();
	Bitboard king			 = m_board.pieces[m_board.sideToMove][KING];
	Bitboard kingMoveTargets = LookupTables::s_kingAttacks[bitscan(king)] & ~yourPieces;
	if (kingMoveTargets) {
		do {
			Bitboard moveTarget = LS1B(kingMoveTargets);
			m_pseudoLegalMoves.emplace_back(m_board, (Square)bitscan(king), (Square)bitscan(moveTarget), KING);
		} while (removeLS1B(kingMoveTargets));
	}
}

void MoveGen::genCastlingMoves() {
	if (inCheck()) {
		return;
	}
	Bitboard king				  = m_board.pieces[m_board.sideToMove][KING];
	unsigned short castlingRights = 0;
	Bitboard allPieces			  = m_board.whitePieces() | m_board.blackPieces();
	if (m_board.sideToMove == WHITE) {
		castlingRights = m_board.castlingRights.getWhiteRights();
		// kingside castling
		if (castlingRights & 0b10 && !((m_attacks | allPieces) & (1UL << f1 | 1UL << g1))) {
			m_pseudoLegalMoves.emplace_back(m_board, e1, g1, KING);
		}
		// queenside castling
		if (castlingRights & 0b01 && !((m_attacks | allPieces) & (1UL << d1 | 1UL << c1)) && ~allPieces & 1UL << b1) {
			m_pseudoLegalMoves.emplace_back(m_board, e1, c1, KING);
		}
	} else {
		castlingRights = m_board.castlingRights.getBlackRights();
		// kingside castling
		if (castlingRights & 0b10 && !((m_attacks | allPieces) & (1UL << f8 | 1UL << g8))) {
			m_pseudoLegalMoves.emplace_back(m_board, e8, g8, KING);
		}
		// queenside castling
		if (castlingRights & 0b01 && !((m_attacks | allPieces) & (1UL << d8 | 1UL << c8)) && ~allPieces & 1UL << b8) {
			m_pseudoLegalMoves.emplace_back(m_board, e8, c8, KING);
		}
	}
}

void MoveGen::genSlidingPieces(Piece p, Bitboard pieces, SlidingPieceDirectionFlags direction) {
	if (!pieces) {
		return;
	}
	Bitboard yourPieces = m_board.sideToMove == WHITE ? m_board.whitePieces() : m_board.blackPieces();
	Bitboard allPieces	= m_board.whitePieces() | m_board.blackPieces();

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
				Bitboard moveTarget = LS1B(rays);
				m_pseudoLegalMoves.emplace_back(m_board, (Square)bitscan(piece), (Square)bitscan(moveTarget), p);
			} while (removeLS1B(rays));
		}
	} while (removeLS1B(pieces));
}

void MoveGen::genBishopMoves() {
	Bitboard bishops = m_board.pieces[m_board.sideToMove][BISHOP];
	genSlidingPieces(BISHOP, bishops, SlidingPieceDirectionFlags::DIAGONAL);
}

void MoveGen::genRookMoves() {
	Bitboard rooks = m_board.pieces[m_board.sideToMove][ROOK];
	genSlidingPieces(ROOK, rooks, SlidingPieceDirectionFlags::STRAIGHT);
}

void MoveGen::genQueenMoves() {
	Bitboard queens = m_board.pieces[m_board.sideToMove][QUEEN];
	genSlidingPieces(QUEEN, queens, SlidingPieceDirectionFlags(DIAGONAL | STRAIGHT));
}

void MoveGen::genPseudoLegalMoves() {
	genPawnMoves();
	genKnightMoves();
	genKingMoves();
	genBishopMoves();
	genRookMoves();
	genQueenMoves();
	genCastlingMoves();
}

Moves MoveGen::genLegalMoves() {
	Moves moves;
	moves.reserve(218);	 // known approximation for maximum number of legal moves possible in a position
	genPseudoLegalMoves();
	for (Move m : m_pseudoLegalMoves) {
		if (!m.execute().inIllegalCheck()) {
			moves.push_back(m);
		}
	}
	return moves;
}
