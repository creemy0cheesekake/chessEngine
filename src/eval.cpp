#include "board.hpp"
#include "transposition_table.hpp"
#include "consts.hpp"
#include "eval.hpp"
#include "lookup_tables.hpp"
#include "move_gen.hpp"
#include "util.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>

uint64_t Eval::totalNodesSearched = 0;
std::array<std::array<Move, Eval::NUM_KILLER_MOVES>, Eval::MAX_SEARCH_DEPTH> Eval::killerMoves{};
std::chrono::time_point<std::chrono::high_resolution_clock> Eval::m_iterative_deepening_cutoff_time = std::chrono::time_point<std::chrono::high_resolution_clock>::max();

Centipawns Eval::countMaterial(const Board& b) {
	Centipawns material = 0;
	for (Color color : {WHITE, BLACK}) {
		for (Piece pieceType : {QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
			Bitboard piece = b.boardState.pieces[color][pieceType];
			int count	   = __builtin_popcountll(piece);
			material +=
				count * pieceToCentipawns[pieceType] *
				(color == WHITE ? 1 : -1);
		}
	}
	return material;
}

Centipawns Eval::evaluate(Board& b) {
	Moves moves = b.moveGenerator.genLegalMoves();
	if (!moves.size()) {
		// add hmClock to prioritize quicker checkmates
		return b.moveGenerator.inCheck() ? -INF_SCORE + (int)b.boardState.hmClock : 0;
	}

	Centipawns score = countMaterial(b);
	for (Color color : {WHITE, BLACK}) {
		for (Piece pieceType : {KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
			Bitboard piece = b.boardState.pieces[color][pieceType];
			while (piece) {
				int stmMultiplier = color == WHITE ? 1 : -1;

				score += stmMultiplier * positionalValue(pieceType, color, (Square)bitscan(piece));
				// Bitboard allPieces = b.whitePieces() | b.blackPieces();
				// switch (pieceType) {
				// 	case QUEEN: {
				// 		Bitboard attacks = b.moveGenerator.genStraightRays(b, (Square)bitscan(piece), allPieces) | b.moveGenerator.genDiagonalRays(b, (Square)bitscan(piece), allPieces);
				// 		score += stmMultiplier * queenMobilityScore[__builtin_popcountll(attacks)];
				// 		break;
				// 	}
				// 	case ROOK: {
				// 		Bitboard attacks = b.moveGenerator.genStraightRays(b, (Square)bitscan(piece), allPieces);
				// 		score += stmMultiplier * rookMobilityScore[__builtin_popcountll(attacks)];
				// 		break;
				// 	}
				// 	case BISHOP: {
				// 		Bitboard attacks = b.moveGenerator.genDiagonalRays(b, (Square)bitscan(piece), allPieces);
				// 		score += stmMultiplier * bishopMobilityScore[__builtin_popcountll(attacks)];
				// 		break;
				// 	}
				// 	case KNIGHT: {
				// 		Bitboard attacks = LookupTables::s_knightAttacks[bitscan(piece)];
				// 		score += stmMultiplier * knightMobilityScore[__builtin_popcountll(attacks)];
				// 		break;
				// 	}
				// 	default: {
				// 		break;
				// 	}
				// }

				piece ^= LS1B(piece);
			};
		}
	}
	return score * (b.boardState.sideToMove == WHITE ? 1 : -1);
}

Centipawns Eval::quiescence_search(Board& b, Centipawns alpha, Centipawns beta) {
	TTEntry entry	  = TranspositionTable::getEntry(b.boardState.hash);
	bool TTEntryFound = entry.partial_hash == (b.boardState.hash & 0xFFFF);

	if (TTEntryFound) {
		const bool isExact		= entry.flag == EXACT;
		const bool isLowerBound = entry.flag == LOWER_BOUND && entry.score >= beta;
		const bool isUpperBound = entry.flag == UPPER_BOUND && entry.score <= alpha;

		if (isExact || isLowerBound || isUpperBound) {
			return entry.score;
		}
	}

	Centipawns current_best_score = evaluate(b);
	if (current_best_score >= beta) {
		return beta;
	}
	if (current_best_score > alpha) {
		alpha = current_best_score;
	}

	MoveGen mg = b.moveGenerator;

	// we only want to generate legal captures for quiescence_search, but if the king is in check, all legal moves must be searched
	Moves moves = mg.inCheck() ? mg.genLegalMoves() : mg.genLegalCaptures();
	Move bestMove;
	for (Move m : moves) {
		if (std::chrono::high_resolution_clock::now() > m_iterative_deepening_cutoff_time) {
			return NONE_SCORE;
		}
		b.execute(m);
		Centipawns score = quiescence_search(b, -beta, -alpha);
		score			 = -score;
		b.undoMove();

		if (score >= beta) {
			TranspositionTable::add(b.boardState.hash, beta, 0, UPPER_BOUND, m);
			return beta;
		}
		if (score > current_best_score) {
			current_best_score = score;
			bestMove		   = m;
		}
		if (score > alpha) {
			alpha = score;
		}
	}
	TranspositionTable::add(b.boardState.hash, current_best_score, 0, current_best_score > alpha ? EXACT : LOWER_BOUND, bestMove);
	return current_best_score;
}

static inline void addToLine(Moves& topLine, const Moves& subline, const Move& m) {
	topLine.clear();
	topLine.push_back(m);
	topLine.insert(topLine.end(), subline.begin(), subline.end());
}

std::tuple<Centipawns, Move, SearchState> Eval::search(Moves& topLine, Board& b, int depthLeft, Centipawns alpha, Centipawns beta, int plyFromRoot, Move previousMove) {
	if (depthLeft <= 0 || b.gameOver()) {
		return {quiescence_search(b, alpha, beta), Move(), SearchState::SEARCH_COMPLETE};
	}
	totalNodesSearched++;
	TTEntry entry = TranspositionTable::getEntry(b.boardState.hash);
	std::pair<Square, Square> bestMovePos((Square)entry.bestMove.from, (Square)entry.bestMove.to);
	bool TTEntryFound = entry.partial_hash == (b.boardState.hash & 0xFFFF) && entry.depth >= depthLeft;
	if (TTEntryFound) {
		const bool isExact		= entry.flag == EXACT;
		const bool isLowerBound = entry.flag == LOWER_BOUND && entry.score >= beta;
		const bool isUpperBound = entry.flag == UPPER_BOUND && entry.score <= alpha;

		if (isExact || isLowerBound || isUpperBound) {
			Move m = TranspositionTable::getMove(entry.bestMove);
			topLine.clear();
			topLine.push_back(m);
			return {(Centipawns)entry.score, m, SearchState::SEARCH_COMPLETE};
		}
	}

	Moves moves			   = b.moveGenerator.genLegalMoves();
	const Move killerMove1 = killerMoves[plyFromRoot][0];
	const Move killerMove2 = killerMoves[plyFromRoot][1];

	for (Move& m : moves) {
		if (m == killerMove1) {
			m.setScore(KILLER_MOVE_1_SCORE);
		} else if (m == killerMove2) {
			m.setScore(KILLER_MOVE_2_SCORE);
		}
	}

	std::sort(moves.begin(), moves.end(), [TTEntryFound, &bestMovePos, &previousMove](const Move& a, const Move& b) {
		if (TTEntryFound) {
			if (a.getFrom() == bestMovePos.first && a.getTo() == bestMovePos.second) {
				return true;
			}
			if (b.getFrom() == bestMovePos.first && b.getTo() == bestMovePos.second) {
				return false;
			}
		}
		if (previousMove == a) {
			return true;
		}
		if (previousMove == b) {
			return false;
		}
		return a.getScore() > b.getScore();
	});

	Centipawns bestScore = -INF_SCORE;
	Move bestMove;
	int movesSearched = 0;

	Centipawns originalAlpha = alpha;
	for (Move m : moves) {
		if (std::chrono::high_resolution_clock::now() > m_iterative_deepening_cutoff_time) {
			return {NONE_SCORE, Move(), SearchState::SEARCH_ABORTED};
		}
		movesSearched++;
		bool isQuietMove = !(m.getFlags() & (CAPTURE | PROMOTION));

		b.execute(m);
		Moves subline;

		Centipawns eval = alpha;
		bool didLMR		= false;
		if (movesSearched > 3 && depthLeft >= 6 && isQuietMove) {
			didLMR					 = true;
			int reductionFactor		 = calculateReductionFactor(movesSearched, depthLeft);
			int lmrDepthLeft		 = std::max(1, depthLeft - reductionFactor);
			auto [e, _, searchState] = search(subline, b, lmrDepthLeft, -(alpha + 1), -alpha, plyFromRoot + 1, m);
			if (searchState == SearchState::SEARCH_ABORTED) {
				b.undoMove();
				return {NONE_SCORE, Move(), SearchState::SEARCH_ABORTED};
			}
			eval = -e;
		}
		if ((eval > alpha && eval < beta) || !didLMR || movesSearched == 1) {
			if (movesSearched == 1 || isQuietMove) {
				auto [e, _, searchState] = search(subline, b, depthLeft - 1, -beta, -alpha, plyFromRoot + 1, m);
				if (searchState == SearchState::SEARCH_ABORTED) {
					b.undoMove();
					return {NONE_SCORE, Move(), SearchState::SEARCH_ABORTED};
				}
				eval = -e;
			} else {
				auto [e, _, searchState] = search(subline, b, depthLeft - 1, -alpha - 1, -alpha, plyFromRoot + 1, m);
				if (searchState == SearchState::SEARCH_ABORTED) {
					b.undoMove();
					return {NONE_SCORE, Move(), SearchState::SEARCH_ABORTED};
				}
				eval = -e;
				if (eval > alpha && beta - alpha > 1) {
					auto [e, _, searchState] = search(subline, b, depthLeft - 1, -beta, -alpha, plyFromRoot + 1, m);
					if (searchState == SearchState::SEARCH_ABORTED) {
						b.undoMove();
						return {NONE_SCORE, Move(), SearchState::SEARCH_ABORTED};
					}
					eval = -e;
				}
			}
		}
		b.undoMove();

		if (eval > bestScore) {
			bestScore = eval;
			bestMove  = m;
			alpha	  = std::max(alpha, bestScore);

			addToLine(topLine, subline, bestMove);
		}
		if (eval >= beta) {
			if (m != killerMoves[plyFromRoot][0]) {
				killerMoves[plyFromRoot][1] = killerMoves[plyFromRoot][0];
				killerMoves[plyFromRoot][0] = m;
			}
			return {beta, m, SearchState::SEARCH_COMPLETE};
		}
	}
	if (bestScore >= beta) {
		TranspositionTable::add(b.boardState.hash, bestScore, depthLeft, LOWER_BOUND, bestMove);
	} else if (bestScore > originalAlpha) {
		TranspositionTable::add(b.boardState.hash, bestScore, depthLeft, EXACT, bestMove);
	} else {
		TranspositionTable::add(b.boardState.hash, bestScore, depthLeft, UPPER_BOUND, bestMove);
	}

	return {bestScore, bestMove, SearchState::SEARCH_COMPLETE};
}

Centipawns Eval::iterative_deepening_ply(Moves& topLine, Board& b, int maxDepth) {
	Move previousMove;
	Centipawns finalScore;
	for (int depth = 1; depth <= maxDepth; depth++) {
		auto [eval, bestMove, searchState] = search(topLine, b, depth, -INF_SCORE, INF_SCORE, 0, previousMove);
		previousMove					   = topLine.back();
		for (Move m : topLine) {
			std::cout << m.notation() << " ";
		}
		std::cout << std::endl;
		if (searchState != SearchState::SEARCH_ABORTED) {
			finalScore = eval;
			if (abs(finalScore) >= INF_SCORE - 2000) break;
		} else {
			break;
		}
	}
	return finalScore;
}

Centipawns Eval::iterative_deepening_time(Moves& topLine, Board& b, int maxTimeMs) {
	Move previousMove;
	Centipawns finalScore;

	m_iterative_deepening_cutoff_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(maxTimeMs);
	int depth						  = 1;
	while (1) {
		if (std::chrono::high_resolution_clock::now() > m_iterative_deepening_cutoff_time) {
			break;
		}
		auto [eval, bestMove, searchState] = search(topLine, b, depth++, -INF_SCORE, INF_SCORE, 0, previousMove);
		previousMove					   = topLine.back();
		for (Move m : topLine) {
			std::cout << m.notation() << " ";
		}
		std::cout << std::endl;
		if (searchState != SearchState::SEARCH_ABORTED) {
			finalScore = eval;
			if (finalScore >= INF_SCORE - 2000) break;
		} else {
			break;
		}
	}
	std::cout << "NPS: " << totalNodesSearched / (maxTimeMs / 1000.0) << std::endl;
	return finalScore;
}

int Eval::calculateReductionFactor(int movesSearched, int depthLeft) {
	int N	   = movesSearched * depthLeft;
	int log2_N = 31 - __builtin_clz(N);
	return log2_N / 1;
}

Centipawns Eval::positionalValue(Piece pieceType, Color color, Square square) {
	return m_materialWeights[pieceType][color][(int)square];
}
