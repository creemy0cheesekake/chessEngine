#include "move_gen.hpp"
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
		while (pawnMoveTargets) {
			Bitboard moveTarget	  = LS1B(pawnMoveTargets);
			Moves& movesContainer = moveTarget & captureMoveTargets ? pseudoLegalCaptures : pseudoLegalMoves;
			if (moveTarget & (firstRank | eighthRank)) {
				for (Piece promoPiece : {QUEEN, ROOK, BISHOP, KNIGHT}) {
					movesContainer.emplace_back(m_board, (Square)bitscan(pawn), (Square)bitscan(moveTarget), PAWN, promoPiece);
				}
			} else {
				movesContainer.emplace_back(m_board, (Square)bitscan(pawn), (Square)bitscan(moveTarget), PAWN);
			}
			removeLS1B(pawnMoveTargets);
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
		while (knightMoveTargets) {
			Bitboard moveTarget	  = LS1B(knightMoveTargets);
			Moves& movesContainer = moveTarget & theirPieces ? pseudoLegalCaptures : pseudoLegalMoves;
			movesContainer.emplace_back(m_board, (Square)bitscan(knight), (Square)bitscan(moveTarget), KNIGHT);
			removeLS1B(knightMoveTargets);
		}
	} while (removeLS1B(knights));
}

bool MoveGen::inCheck() const {
	return genAttacks() & m_board.boardState.pieces[m_board.boardState.sideToMove][KING];
}

void MoveGen::genKingMoves(Moves& pseudoLegalMoves, Moves& pseudoLegalCaptures) const {
	Bitboard yourPieces		 = m_board.boardState.sideToMove == WHITE ? m_board.whitePieces() : m_board.blackPieces();
	Bitboard theirPieces	 = m_board.boardState.sideToMove == WHITE ? m_board.blackPieces() : m_board.whitePieces();
	Bitboard king			 = m_board.boardState.pieces[m_board.boardState.sideToMove][KING];
	Bitboard kingMoveTargets = LookupTables::s_kingAttacks[bitscan(king)] & ~yourPieces;
	while (kingMoveTargets) {
		Bitboard moveTarget	  = LS1B(kingMoveTargets);
		Moves& movesContainer = moveTarget & theirPieces ? pseudoLegalCaptures : pseudoLegalMoves;
		movesContainer.emplace_back(m_board, (Square)bitscan(king), (Square)bitscan(moveTarget), KING);
		removeLS1B(kingMoveTargets);
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
		int sq				  = bitscan(piece);
		Bitboard straightRays = 0;
		Bitboard diagonalRays = 0;
		if (direction & STRAIGHT) {
			Bitboard Nrays	   = LookupTables::s_straightRayTable[sq][NORTH],
					 Erays	   = LookupTables::s_straightRayTable[sq][EAST],
					 Wrays	   = LookupTables::s_straightRayTable[sq][WEST],
					 Srays	   = LookupTables::s_straightRayTable[sq][SOUTH];
			Bitboard Nblockers = Nrays & allPieces;
			if (Nblockers != 0) Nrays ^= (LookupTables::s_straightRayTable[bitscan(Nblockers)][NORTH]);
			Bitboard Eblockers = Erays & allPieces;
			if (Eblockers != 0) Erays ^= (LookupTables::s_straightRayTable[bitscan(Eblockers)][EAST]);
			Bitboard Sblockers = Srays & allPieces;
			if (Sblockers != 0) Srays ^= (LookupTables::s_straightRayTable[reverseBitscan(Sblockers)][SOUTH]);
			Bitboard Wblockers = Wrays & allPieces;
			if (Wblockers != 0) Wrays ^= (LookupTables::s_straightRayTable[reverseBitscan(Wblockers)][WEST]);
			straightRays = Nrays | Erays | Wrays | Srays;
		}
		if (direction & DIAGONAL) {
			Bitboard NErays		= LookupTables::s_diagonalRayTable[sq][NORTHEAST],
					 NWrays		= LookupTables::s_diagonalRayTable[sq][NORTHWEST],
					 SWrays		= LookupTables::s_diagonalRayTable[sq][SOUTHWEST],
					 SErays		= LookupTables::s_diagonalRayTable[sq][SOUTHEAST];
			Bitboard NEblockers = NErays & allPieces;
			if (NEblockers != 0) NErays ^= (LookupTables::s_diagonalRayTable[bitscan(NEblockers)][NORTHEAST]);
			Bitboard NWblockers = NWrays & allPieces;
			if (NWblockers != 0) NWrays ^= (LookupTables::s_diagonalRayTable[bitscan(NWblockers)][NORTHWEST]);
			Bitboard SWblockers = SWrays & allPieces;
			if (SWblockers != 0) SWrays ^= (LookupTables::s_diagonalRayTable[reverseBitscan(SWblockers)][SOUTHWEST]);
			Bitboard SEblockers = SErays & allPieces;
			if (SEblockers != 0) SErays ^= (LookupTables::s_diagonalRayTable[reverseBitscan(SEblockers)][SOUTHEAST]);
			diagonalRays = NErays | NWrays | SWrays | SErays;
		}

		Bitboard rays = ((straightRays | diagonalRays) ^ piece) & ~yourPieces;

		while (rays) {
			Bitboard moveTarget	  = LS1B(rays);
			Moves& movesContainer = moveTarget & theirPieces ? pseudoLegalCaptures : pseudoLegalMoves;
			movesContainer.emplace_back(m_board, (Square)sq, (Square)bitscan(moveTarget), p);
			removeLS1B(rays);
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
	pseudoLegalCaptures.reserve(218);
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
		for (size_t i = QUEEN; i < NONE_PIECE; i++) {
			Bitboard pieceBoard = m_board.boardState.pieces[victimColor][i];
			if (pieceBoard & toSquare) {
				victimPiece = (Piece)i;
				break;
			}
		}
		return MVV_LVA_table[m.getPieceType()][victimPiece];
	};

	for (Move& m : pseudoLegalCaptures) {
		m.setScore(getMVV_LVA_score(m));
	}

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

void MoveGen::genPawnCaptures(Moves& pseudoLegalCaptures) const {
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
		while (captureMoveTargets) {
			Bitboard moveTarget = LS1B(captureMoveTargets);
			if (moveTarget & (firstRank | eighthRank)) {
				for (Piece promoPiece : {QUEEN, ROOK, BISHOP, KNIGHT}) {
					pseudoLegalCaptures.emplace_back(m_board, (Square)bitscan(pawn), (Square)bitscan(moveTarget), PAWN, promoPiece);
				}
			} else {
				pseudoLegalCaptures.emplace_back(m_board, (Square)bitscan(pawn), (Square)bitscan(moveTarget), PAWN);
			}
			removeLS1B(captureMoveTargets);
		}
	} while (removeLS1B(pawns));
}

void MoveGen::genKnightCaptures(Moves& pseudoLegalCaptures) const {
	Bitboard knights = m_board.boardState.pieces[m_board.boardState.sideToMove][KNIGHT];
	if (!knights) {
		return;
	}
	Bitboard theirPieces = m_board.boardState.sideToMove == WHITE ? m_board.blackPieces() : m_board.whitePieces();
	do {
		Bitboard knight				= LS1B(knights);
		Bitboard captureMoveTargets = LookupTables::s_knightAttacks[bitscan(knight)] & theirPieces;
		while (captureMoveTargets) {
			Bitboard moveTarget = LS1B(captureMoveTargets);
			pseudoLegalCaptures.emplace_back(m_board, (Square)bitscan(knight), (Square)bitscan(moveTarget), KNIGHT);
			removeLS1B(captureMoveTargets);
		}
	} while (removeLS1B(knights));
}

void MoveGen::genKingCaptures(Moves& pseudoLegalCaptures) const {
	Bitboard theirPieces		= m_board.boardState.sideToMove == WHITE ? m_board.blackPieces() : m_board.whitePieces();
	Bitboard king				= m_board.boardState.pieces[m_board.boardState.sideToMove][KING];
	Bitboard captureMoveTargets = LookupTables::s_kingAttacks[bitscan(king)] & theirPieces;
	while (captureMoveTargets) {
		Bitboard moveTarget = LS1B(captureMoveTargets);
		pseudoLegalCaptures.emplace_back(m_board, (Square)bitscan(king), (Square)bitscan(moveTarget), KING);
		removeLS1B(captureMoveTargets);
	}
}

void MoveGen::genSlidingPiecesCaptures(Moves& pseudoLegalCaptures, Piece p, Bitboard pieces, SlidingPieceDirectionFlags direction) const {
	if (!pieces) {
		return;
	}
	Bitboard theirPieces = m_board.boardState.sideToMove == WHITE ? m_board.blackPieces() : m_board.whitePieces();
	Bitboard allPieces	 = m_board.whitePieces() | m_board.blackPieces();

	do {
		Bitboard piece		  = LS1B(pieces);
		int sq				  = bitscan(piece);
		Bitboard straightRays = 0;
		Bitboard diagonalRays = 0;
		if (direction & STRAIGHT) {
			Bitboard Nrays	   = LookupTables::s_straightRayTable[sq][NORTH],
					 Erays	   = LookupTables::s_straightRayTable[sq][EAST],
					 Wrays	   = LookupTables::s_straightRayTable[sq][WEST],
					 Srays	   = LookupTables::s_straightRayTable[sq][SOUTH];
			Bitboard Nblockers = Nrays & allPieces;
			if (Nblockers != 0) Nrays ^= (LookupTables::s_straightRayTable[bitscan(Nblockers)][NORTH]);
			Bitboard Eblockers = Erays & allPieces;
			if (Eblockers != 0) Erays ^= (LookupTables::s_straightRayTable[bitscan(Eblockers)][EAST]);
			Bitboard Sblockers = Srays & allPieces;
			if (Sblockers != 0) Srays ^= (LookupTables::s_straightRayTable[reverseBitscan(Sblockers)][SOUTH]);
			Bitboard Wblockers = Wrays & allPieces;
			if (Wblockers != 0) Wrays ^= (LookupTables::s_straightRayTable[reverseBitscan(Wblockers)][WEST]);
			straightRays = Nrays | Erays | Wrays | Srays;
		}
		if (direction & DIAGONAL) {
			Bitboard NErays		= LookupTables::s_diagonalRayTable[sq][NORTHEAST],
					 NWrays		= LookupTables::s_diagonalRayTable[sq][NORTHWEST],
					 SWrays		= LookupTables::s_diagonalRayTable[sq][SOUTHWEST],
					 SErays		= LookupTables::s_diagonalRayTable[sq][SOUTHEAST];
			Bitboard NEblockers = NErays & allPieces;
			if (NEblockers != 0) NErays ^= (LookupTables::s_diagonalRayTable[bitscan(NEblockers)][NORTHEAST]);
			Bitboard NWblockers = NWrays & allPieces;
			if (NWblockers != 0) NWrays ^= (LookupTables::s_diagonalRayTable[bitscan(NWblockers)][NORTHWEST]);
			Bitboard SWblockers = SWrays & allPieces;
			if (SWblockers != 0) SWrays ^= (LookupTables::s_diagonalRayTable[reverseBitscan(SWblockers)][SOUTHWEST]);
			Bitboard SEblockers = SErays & allPieces;
			if (SEblockers != 0) SErays ^= (LookupTables::s_diagonalRayTable[reverseBitscan(SEblockers)][SOUTHEAST]);
			diagonalRays = NErays | NWrays | SWrays | SErays;
		}

		Bitboard rays = ((straightRays | diagonalRays) ^ piece) & theirPieces;

		while (rays) {
			Bitboard moveTarget = LS1B(rays);
			pseudoLegalCaptures.emplace_back(m_board, (Square)sq, (Square)bitscan(moveTarget), p);
			removeLS1B(rays);
		}
	} while (removeLS1B(pieces));
}

void MoveGen::genBishopCaptures(Moves& pseudoLegalCaptures) const {
	Bitboard bishops = m_board.boardState.pieces[m_board.boardState.sideToMove][BISHOP];
	genSlidingPiecesCaptures(pseudoLegalCaptures, BISHOP, bishops, SlidingPieceDirectionFlags::DIAGONAL);
}

void MoveGen::genRookCaptures(Moves& pseudoLegalCaptures) const {
	Bitboard rooks = m_board.boardState.pieces[m_board.boardState.sideToMove][ROOK];
	genSlidingPiecesCaptures(pseudoLegalCaptures, ROOK, rooks, SlidingPieceDirectionFlags::STRAIGHT);
}

void MoveGen::genQueenCaptures(Moves& pseudoLegalCaptures) const {
	Bitboard queens = m_board.boardState.pieces[m_board.boardState.sideToMove][QUEEN];
	genSlidingPiecesCaptures(pseudoLegalCaptures, QUEEN, queens, SlidingPieceDirectionFlags(DIAGONAL | STRAIGHT));
}

Moves MoveGen::genLegalCaptures() {
	m_attacks = genAttacks();
	Moves pseudoLegalCaptures;
	Moves moves;
	pseudoLegalCaptures.reserve(218);  // known approximation for maximum number of legal moves possible in a position
	moves.reserve(218);

	genPawnCaptures(pseudoLegalCaptures);
	genKnightCaptures(pseudoLegalCaptures);
	genKingCaptures(pseudoLegalCaptures);
	genBishopCaptures(pseudoLegalCaptures);
	genRookCaptures(pseudoLegalCaptures);
	genQueenCaptures(pseudoLegalCaptures);

	Color victimColor	  = m_board.boardState.sideToMove == WHITE ? BLACK : WHITE;
	auto getMVV_LVA_score = [&](Move& m) {
		if (m.getFlags() & EN_PASSANT) {
			return MVV_LVA_table[PAWN][PAWN];
		}
		Bitboard toSquare = 1UL << m.getTo();
		Piece victimPiece = NONE_PIECE;
		for (size_t i = QUEEN; i < NONE_PIECE; i++) {
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

	for (const Move& m : pseudoLegalCaptures) {
		m_board.execute(m);
		if (!m_board.inIllegalCheck()) {
			moves.push_back(m);
		}
		m_board.undoMove();
	}

	return moves;
}

bool MoveGen::hasLegalMoves() {
	m_attacks	 = genAttacks();
	auto isLegal = [&](Square from, Square to, Piece p, MoveFlag flags = NORMAL_MOVE) {
		Move m(from, to, p, NONE_PIECE, flags);
		m_board.execute(m);
		bool illegal = m_board.inIllegalCheck();
		m_board.undoMove();
		return !illegal;
	};
	Bitboard allPieces	 = m_board.whitePieces() | m_board.blackPieces();
	Bitboard yourPieces	 = m_board.boardState.sideToMove == WHITE ? m_board.whitePieces() : m_board.blackPieces();
	Bitboard theirPieces = m_board.boardState.sideToMove == WHITE ? m_board.blackPieces() : m_board.whitePieces();

	Bitboard pawns = m_board.boardState.pieces[m_board.boardState.sideToMove][PAWN];
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
		while (captureMoveTargets) {
			if (isLegal((Square)bitscan(pawn), (Square)bitscan(LS1B(captureMoveTargets)), PAWN, CAPTURE)) return true;
			removeLS1B(captureMoveTargets);
		}
		Bitboard targets = pushMoveTargets | dblPushMoveTargets;
		while (targets) {
			if (isLegal((Square)bitscan(pawn), (Square)bitscan(LS1B(targets)), PAWN)) return true;
			removeLS1B(targets);
		}
	} while (removeLS1B(pawns));

	Bitboard knights = m_board.boardState.pieces[m_board.boardState.sideToMove][KNIGHT];
	do {
		Bitboard knight	 = LS1B(knights);
		Bitboard targets = LookupTables::s_knightAttacks[bitscan(knight)] & ~yourPieces;
		while (targets) {
			if (isLegal((Square)bitscan(knight), (Square)bitscan(LS1B(targets)), KNIGHT, LS1B(targets) & theirPieces ? CAPTURE : NORMAL_MOVE)) return true;
			removeLS1B(targets);
		}
	} while (removeLS1B(knights));

	// sliding moves
	for (Piece p : {BISHOP, ROOK, QUEEN}) {
		Bitboard pieces = m_board.boardState.pieces[m_board.boardState.sideToMove][p];
		while (pieces) {
			Square from		 = (Square)bitscan(LS1B(pieces));
			Bitboard targets = 0;
			if (p != ROOK) targets |= genDiagonalRays(from, allPieces);
			if (p != BISHOP) targets |= genStraightRays(from, allPieces);
			targets &= ~yourPieces;

			while (targets) {
				if (isLegal(from, (Square)bitscan(LS1B(targets)), p, LS1B(targets) & theirPieces ? CAPTURE : NORMAL_MOVE)) return true;
				removeLS1B(targets);
			}
			removeLS1B(pieces);
		}
	}

	Bitboard king	 = m_board.boardState.pieces[m_board.boardState.sideToMove][KING];
	Bitboard targets = LookupTables::s_kingAttacks[bitscan(king)] & ~yourPieces;
	while (targets) {
		if (isLegal((Square)bitscan(king), (Square)bitscan(LS1B(targets)), KING, LS1B(targets) & theirPieces ? CAPTURE : NORMAL_MOVE)) return true;
		removeLS1B(targets);
	}

	// then checkmate or stalemate
	return false;
}
