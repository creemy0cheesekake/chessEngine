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
				Bitboard allPieces = b.whitePieces() | b.blackPieces();
				Bitboard friendly  = color == WHITE ? b.whitePieces() : b.blackPieces();
				Square pieceSquare = (Square)bitscan(piece);
				switch (pieceType) {
					case QUEEN: {
						Bitboard attacks = b.moveGenerator.genStraightRays(b, pieceSquare, allPieces) | b.moveGenerator.genDiagonalRays(b, pieceSquare, allPieces);
						attacks &= ~friendly;
						score += stmMultiplier * queenMobilityScore[__builtin_popcountll(attacks)];
						break;
					}
					case ROOK: {
						Bitboard attacks = b.moveGenerator.genStraightRays(b, pieceSquare, allPieces);
						attacks &= ~friendly;
						score += stmMultiplier * rookMobilityScore[__builtin_popcountll(attacks)];
						break;
					}
					case BISHOP: {
						Bitboard attacks = b.moveGenerator.genDiagonalRays(b, pieceSquare, allPieces);
						attacks &= ~friendly;
						score += stmMultiplier * bishopMobilityScore[__builtin_popcountll(attacks)];
						break;
					}
					case KNIGHT: {
						Bitboard attacks = LookupTables::s_knightAttacks[pieceSquare];
						attacks &= ~friendly;
						score += stmMultiplier * knightMobilityScore[__builtin_popcountll(attacks)];
						break;
					}
					default: {
						break;
					}
				}

				piece ^= LS1B(piece);
			};
		}
	}
	return score * (b.boardState.sideToMove == WHITE ? 1 : -1);
}

Centipawns Eval::quiescence_search(Board& b, Centipawns alpha, Centipawns beta) {
	Centipawns static_eval = evaluate(b);

	Centipawns bestScore = static_eval;
	if (bestScore >= beta) {
		return bestScore;
	}
	if (bestScore > alpha) {
		alpha = bestScore;
	}

	// we only want to generate legal captures for quiescence_search, but if the king is in check, all legal moves must be searched
	Moves moves = b.moveGenerator.inCheck() ? b.moveGenerator.genLegalMoves() : b.moveGenerator.genLegalCaptures();
	for (const Move& m : moves) {
		b.execute(m);
		Centipawns score = -quiescence_search(b, -beta, -alpha);
		b.undoMove();

		if (score >= beta) {
			return score;
		}
		if (score > bestScore) {
			bestScore = score;
		}
		if (score > alpha) {
			alpha = score;
		}
	}
	return bestScore;
}

std::tuple<Centipawns, SearchState> Eval::search(Moves& topLine, Board& b, int depthLeft, Moves& previousPV, Centipawns alpha, Centipawns beta, int plyFromRoot) {
	if (b.is50MoveRule() || b.isInsufficientMaterial()) {
		return {0, SEARCH_COMPLETE};
	}
	if (depthLeft <= 0) {
		return {quiescence_search(b, alpha, beta), SEARCH_COMPLETE};
	}

	Centipawns originalAlpha = alpha;

	MoveGen mg	= b.moveGenerator;
	Moves moves = mg.genLegalMoves();

	const Move killerMove1 = killerMoves[plyFromRoot][0];
	const Move killerMove2 = killerMoves[plyFromRoot][1];

	TTEntry entry = TranspositionTable::getEntry(b.boardState.hash);
	Move ttMove;
	if (entry.partial_hash == (b.boardState.hash & 0xFFFF) && entry.bestMove.piece != NONE_PIECE) {
		ttMove = TranspositionTable::getMove(entry.bestMove);
		if (entry.depth >= depthLeft) {
			if (entry.flag == EXACT) {
				topLine.clear();
				topLine.push_back(TranspositionTable::getMove(entry.bestMove));
				return {(Centipawns)entry.score, SEARCH_COMPLETE};
			} else if (entry.flag == LOWER_BOUND) {
				alpha = std::max(alpha, entry.score);
			} else if (entry.flag == UPPER_BOUND) {
				beta = std::min(beta, entry.score);
			}
		}
		if (alpha >= beta) {
			topLine.clear();
			topLine.push_back(TranspositionTable::getMove(entry.bestMove));
			return {(Centipawns)entry.score, SEARCH_COMPLETE};
		}
	}

	for (Move& m : moves) {
		if (m == killerMove1) {
			m.setScore(KILLER_MOVE_1_SCORE);
		} else if (m == killerMove2) {
			m.setScore(KILLER_MOVE_2_SCORE);
		} else if (m == ttMove) {
			m.setScore(MAX_MOVE_SCORE);
		} else if (plyFromRoot < previousPV.size() && m == previousPV[plyFromRoot]) {
			m.setScore(MAX_MOVE_SCORE - 1);
		}
	}

	std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
		return a.getScore() > b.getScore();
	});

	Moves subline;
	int movesSearched = 0;
	Move bestMove;
	for (const Move& m : moves) {
		if (std::chrono::high_resolution_clock::now() > m_iterative_deepening_cutoff_time) {
			return {NONE_SCORE, SEARCH_ABORTED};
		}
		movesSearched++;
		totalNodesSearched++;
		b.execute(m);

		Centipawns score = -INF_SCORE;

		int depthReduction = 0;
		bool isQuietMove   = !(m.getFlags() & (CAPTURE | PROMOTION));

		// lmr
		if (isQuietMove && movesSearched > 3 && depthLeft >= 3) {
			depthReduction = calculateReductionFactor(movesSearched, depthLeft);
		}

		if (movesSearched == 1) {
			// full window search
			auto [childScore, searchState] = search(subline, b, depthLeft - 1, previousPV, -beta, -alpha, plyFromRoot + 1);
			if (searchState == SEARCH_ABORTED) {
				b.undoMove();
				return {NONE_SCORE, SEARCH_ABORTED};
			}
			score = -childScore;
		} else {
			// zero window search
			int newDepth				   = depthLeft - 1 - depthReduction;
			auto [childScore, searchState] = search(subline, b, newDepth, previousPV, -alpha - 1, -alpha, plyFromRoot + 1);
			if (searchState == SEARCH_ABORTED) {
				b.undoMove();
				return {NONE_SCORE, SEARCH_ABORTED};
			}
			score = -childScore;

			// lmr failed high
			if (depthReduction > 0 && score > alpha) {
				// re-search full depth
				auto [childScore, searchState] = search(subline, b, depthLeft - 1, previousPV, -beta, -alpha, plyFromRoot + 1);
				if (searchState == SEARCH_ABORTED) {
					b.undoMove();
					return {NONE_SCORE, SEARCH_ABORTED};
				}
				score = -childScore;
			}
			if (score > alpha && beta - alpha > 1) {
				// re-search full window
				auto [childScore, searchState] = search(subline, b, depthLeft - 1, previousPV, -beta, -alpha, plyFromRoot + 1);
				if (searchState == SEARCH_ABORTED) {
					b.undoMove();
					return {NONE_SCORE, SEARCH_ABORTED};
				}
				score = -childScore;
			}
		}
		b.undoMove();
		if (score >= beta) {
			if (m != killerMoves[plyFromRoot][0]) {
				killerMoves[plyFromRoot][1] = killerMoves[plyFromRoot][0];
				killerMoves[plyFromRoot][0] = m;
			}
			TranspositionTable::add(b.boardState.hash, score, depthLeft, TTFlag::LOWER_BOUND, m);
			return {beta, SEARCH_COMPLETE};
		}
		if (score > alpha) {
			alpha	 = score;
			bestMove = m;
			topLine.clear();
			topLine.push_back(m);
			topLine.insert(topLine.end(), subline.begin(), subline.end());
		}
	}
	if (alpha <= originalAlpha) {
		TranspositionTable::add(b.boardState.hash, alpha, depthLeft, TTFlag::UPPER_BOUND, bestMove);
	} else if (alpha >= beta) {
		TranspositionTable::add(b.boardState.hash, alpha, depthLeft, TTFlag::LOWER_BOUND, bestMove);
	} else {
		TranspositionTable::add(b.boardState.hash, alpha, depthLeft, TTFlag::EXACT, bestMove);
	}
	return {alpha, SEARCH_COMPLETE};
}

Centipawns Eval::iterative_deepening_ply(Moves& topLine, Board& b, int maxDepth) {
	Centipawns finalScore;
	Moves previousPV;

	auto searchStartTime = std::chrono::high_resolution_clock::now();
	for (int depth = 1; depth <= maxDepth; depth++) {
		auto [eval, searchState] = search(topLine, b, depth, previousPV, -INF_SCORE, INF_SCORE, 0);

		if (topLine.size() < depth) {
			// early alpha beta cutoff
			// fill the rest of the line with tt moves
			Board copy = b;
			for (Move m : topLine) copy.execute(m);
			int topLineSize = topLine.size();
			for (int i = 0; i < depth - topLineSize; i++) {
				TTEntry entry = TranspositionTable::getEntry(copy.boardState.hash);
				if (entry.partial_hash == (copy.boardState.hash & 0xFFFF) && entry.bestMove.piece != NONE_PIECE) {
					Move m = TranspositionTable::getMove(entry.bestMove);
					topLine.push_back(m);
					copy.execute(m);
				} else break;
			}
		}

		for (Move m : topLine) {
			std::cout << m.notation() << " ";
		}
		std::cout << std::endl;

		finalScore = eval;
		previousPV = std::move(topLine);
		if (abs(finalScore) >= INF_SCORE - 2000) break;
	}
	topLine = std::move(previousPV);
	std::cout << "NPS: " << totalNodesSearched / std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - searchStartTime).count() << std::endl;
	return finalScore;
}

Centipawns Eval::iterative_deepening_time(Moves& topLine, Board& b, int maxTimeMs) {
	Centipawns finalScore;
	Moves previousPV;

	m_iterative_deepening_cutoff_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(maxTimeMs);
	int depth						  = 0;
	auto searchStartTime			  = std::chrono::high_resolution_clock::now();
	while (1) {
		if (std::chrono::high_resolution_clock::now() > m_iterative_deepening_cutoff_time) break;
		auto [eval, searchState] = search(topLine, b, ++depth, previousPV, -INF_SCORE, INF_SCORE, 0);
		if (searchState == SearchState::SEARCH_ABORTED) break;

		if (topLine.size() < depth) {
			// early alpha beta cutoff
			// fill the rest of the line with tt moves
			Board copy = b;
			for (Move m : topLine) copy.execute(m);
			int topLineSize = topLine.size();
			for (int i = 0; i < depth - topLineSize; i++) {
				TTEntry entry = TranspositionTable::getEntry(copy.boardState.hash);
				if (entry.partial_hash == (copy.boardState.hash & 0xFFFF) && entry.bestMove.piece != NONE_PIECE) {
					Move m = TranspositionTable::getMove(entry.bestMove);
					topLine.push_back(m);
					copy.execute(m);
				} else break;
			}
		}

		for (Move m : topLine) {
			std::cout << m.notation() << " ";
		}
		std::cout << std::endl;

		finalScore = eval;
		previousPV = std::move(topLine);
		if (finalScore >= INF_SCORE - 2000) break;
	}
	topLine = std::move(previousPV);
	std::cout << "NPS: " << totalNodesSearched / std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - searchStartTime).count() << std::endl;
	return finalScore;
}

int Eval::calculateReductionFactor(int movesSearched, int depthLeft) {
	int N	   = movesSearched * depthLeft;
	int log2_N = 31 - __builtin_clz(N);
	return log2_N / 2;
}

Centipawns Eval::positionalValue(Piece pieceType, Color color, Square square) {
	return m_materialWeights[pieceType][color][(int)square];
}
