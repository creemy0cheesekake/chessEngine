#include "move_gen.hpp"
#include "board.hpp"
#include "lookup_tables.hpp"
#include "util.hpp"

MoveGen::MoveGen(Board b) {
	board	= b;
	attacks = genAttacks();
}

void MoveGen::genPawnMoves() {
	bitboard pawns = board.pieces[board.sideToMove][PAWN];
	if (!pawns) {
		return;
	}
	bitboard allPieces	 = board.whitePieces() | board.blackPieces();
	bitboard theirPieces = board.sideToMove == WHITE ? board.blackPieces() : board.whitePieces();
	do {
		bitboard pawn				= LS1B(pawns);
		bitboard captureMoveTargets = (pawn & ~aFile ? pawn << 7 : 0) | (pawn & ~hFile ? pawn << 9 : 0);
		if (board.sideToMove == BLACK) {
			captureMoveTargets >>= 16;
		}
		captureMoveTargets &= theirPieces | board.enPassantSquare;
		bitboard pushMoveTargets	= (board.sideToMove == WHITE ? pawn << 8 : pawn >> 8) & ~allPieces;
		bitboard dblPushMoveTargets = 0;
		if (pawn & (secondRank | seventhRank)) {
			dblPushMoveTargets = (board.sideToMove == WHITE ? pushMoveTargets << 8 : pushMoveTargets >> 8) & ~(allPieces ^ pawn);
		}
		bitboard pawnMoveTargets = captureMoveTargets | pushMoveTargets | dblPushMoveTargets;
		if (pawnMoveTargets != 0) {
			do {
				bitboard moveTarget = LS1B(pawnMoveTargets);
				if (moveTarget & (firstRank | eighthRank)) {
					for (Piece promoPiece : {QUEEN, ROOK, BISHOP, KNIGHT}) {
						_pseudoLegalMoves.push_back(Move(board, (Square)bitscan(pawn), (Square)bitscan(moveTarget), PAWN, promoPiece));
					}
				} else {
					_pseudoLegalMoves.push_back(Move(board, (Square)bitscan(pawn), (Square)bitscan(moveTarget), PAWN));
				}
			} while (removeLS1B(pawnMoveTargets));
		}
	} while (removeLS1B(pawns));
}

void MoveGen::genKnightMoves() {
	bitboard knights = board.pieces[board.sideToMove][KNIGHT];
	if (!knights) {
		return;
	}
	bitboard yourPieces = board.sideToMove == WHITE ? board.whitePieces() : board.blackPieces();
	do {
		bitboard knight			   = LS1B(knights);
		bitboard knightMoveTargets = LookupTables::knightAttacks[bitscan(knight)] & ~yourPieces;
		if (knightMoveTargets) {
			do {
				bitboard moveTarget = LS1B(knightMoveTargets);
				_pseudoLegalMoves.push_back(Move(board, (Square)bitscan(knight), (Square)bitscan(moveTarget), KNIGHT));
			} while (removeLS1B(knightMoveTargets));
		}
	} while (removeLS1B(knights));
}

bool MoveGen::inCheck() {
	return attacks & board.pieces[board.sideToMove][KING];
}

void MoveGen::genKingMoves() {
	bitboard yourPieces		 = board.sideToMove == WHITE ? board.whitePieces() : board.blackPieces();
	bitboard king			 = board.pieces[board.sideToMove][KING];
	bitboard kingMoveTargets = LookupTables::kingAttacks[bitscan(king)] & ~yourPieces;
	if (kingMoveTargets) {
		do {
			bitboard moveTarget = LS1B(kingMoveTargets);
			_pseudoLegalMoves.push_back(Move(board, (Square)bitscan(king), (Square)bitscan(moveTarget), KING));
		} while (removeLS1B(kingMoveTargets));
	}
}

void MoveGen::genCastlingMoves() {
	if (inCheck()) {
		return;
	}
	bitboard king				  = board.pieces[board.sideToMove][KING];
	unsigned short castlingRights = 0;
	bitboard allPieces			  = board.whitePieces() | board.blackPieces();
	if (board.sideToMove == WHITE) {
		castlingRights = board.castlingRights.getWhiteRights();
		// kingside castling
		if (castlingRights & 0b10 && !((attacks | allPieces) & (1UL << f1 | 1UL << g1))) {
			_pseudoLegalMoves.push_back(Move(board, e1, g1, KING));
		}
		// queenside castling
		if (castlingRights & 0b01 && !((attacks | allPieces) & (1UL << d1 | 1UL << c1)) && ~allPieces & 1UL << b1) {
			_pseudoLegalMoves.push_back(Move(board, e1, c1, KING));
		}
	} else {
		castlingRights = board.castlingRights.getBlackRights();
		// kingside castling
		if (castlingRights & 0b10 && !((attacks | allPieces) & (1UL << f8 | 1UL << g8))) {
			_pseudoLegalMoves.push_back(Move(board, e8, g8, KING));
		}
		// queenside castling
		if (castlingRights & 0b01 && !((attacks | allPieces) & (1UL << d8 | 1UL << c8)) && ~allPieces & 1UL << b8) {
			_pseudoLegalMoves.push_back(Move(board, e8, c8, KING));
		}
	}
}

void MoveGen::genSlidingPieces(Piece p, bitboard pieces, SlidingPieceDirectionFlags direction) {
	if (!pieces) {
		return;
	}
	bitboard yourPieces = board.sideToMove == WHITE ? board.whitePieces() : board.blackPieces();
	bitboard allPieces	= board.whitePieces() | board.blackPieces();

	do {
		bitboard piece		  = LS1B(pieces);
		bitboard straightRays = 0;
		bitboard diagonalRays = 0;
		if (direction & STRAIGHT) {
			bitboard Nrays = piece, Erays = piece, Wrays = piece, Srays = piece;
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
			bitboard NErays = piece, NWrays = piece, SWrays = piece, SErays = piece;
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

		bitboard rays = ((straightRays | diagonalRays) ^ piece) & ~yourPieces;

		if (rays) {
			do {
				bitboard moveTarget = LS1B(rays);
				_pseudoLegalMoves.push_back(Move(board, (Square)bitscan(piece), (Square)bitscan(moveTarget), p));
			} while (removeLS1B(rays));
		}
	} while (removeLS1B(pieces));
}

void MoveGen::genBishopMoves() {
	bitboard bishops = board.pieces[board.sideToMove][BISHOP];
	genSlidingPieces(BISHOP, bishops, SlidingPieceDirectionFlags::DIAGONAL);
}

void MoveGen::genRookMoves() {
	bitboard rooks = board.pieces[board.sideToMove][ROOK];
	genSlidingPieces(ROOK, rooks, SlidingPieceDirectionFlags::STRAIGHT);
}

void MoveGen::genQueenMoves() {
	bitboard queens = board.pieces[board.sideToMove][QUEEN];
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
	for (Move m : _pseudoLegalMoves) {
		if (!m.execute().inIllegalCheck()) {
			moves.push_back(m);
		}
	}
	return moves;
}
