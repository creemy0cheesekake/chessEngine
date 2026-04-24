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
			int count	   = std::popcount(piece);
			material +=
				count * pieceToCentipawns[pieceType] *
				(color == WHITE ? 1 : -1);
		}
	}
	return material;
}

Centipawns Eval::evaluate(Board& b) {
	if (!b.moveGenerator.hasLegalMoves()) {
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
						Bitboard attacks = b.moveGenerator.genStraightRays(pieceSquare, allPieces) | b.moveGenerator.genDiagonalRays(pieceSquare, allPieces);
						attacks &= ~friendly;
						score += stmMultiplier * queenMobilityScore[std::popcount(attacks)];
						break;
					}
					case ROOK: {
						Bitboard attacks = b.moveGenerator.genStraightRays(pieceSquare, allPieces);
						attacks &= ~friendly;
						score += stmMultiplier * rookMobilityScore[std::popcount(attacks)];
						break;
					}
					case BISHOP: {
						Bitboard attacks = b.moveGenerator.genDiagonalRays(pieceSquare, allPieces);
						attacks &= ~friendly;
						score += stmMultiplier * bishopMobilityScore[std::popcount(attacks)];
						break;
					}
					case KNIGHT: {
						Bitboard attacks = LookupTables::s_knightAttacks[pieceSquare];
						attacks &= ~friendly;
						score += stmMultiplier * knightMobilityScore[std::popcount(attacks)];
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

SearchResult Eval::search(Moves& topLine, Board& b, int depthLeft, Moves& previousPV, Centipawns alpha, Centipawns beta, int plyFromRoot) {
	if (b.is50MoveRule() || b.isInsufficientMaterial()) {
		return {0, SEARCH_COMPLETE};
	}
	if (depthLeft <= 0) {
		return {quiescence_search(b, alpha, beta), SEARCH_COMPLETE};
	}

	Centipawns originalAlpha = alpha;

	MoveGen& mg = b.moveGenerator;
	Moves moves = mg.genLegalMoves();

	const Move killerMove1 = killerMoves[plyFromRoot][0];
	const Move killerMove2 = killerMoves[plyFromRoot][1];

	const TTEntry& entry = TranspositionTable::getEntry(b.boardState.hash);
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

	const uint32_t k1Key = killerMove1.getSignature();
	const uint32_t k2Key = killerMove2.getSignature();
	const uint32_t ttKey = ttMove.getSignature();
	const uint32_t pvKey = (plyFromRoot < previousPV.size()) ? previousPV[plyFromRoot].getSignature() : 0;

	for (Move& m : moves) {
		uint32_t mKey = m.getSignature(); 

		if (mKey == ttKey) m.setScore(MAX_MOVE_SCORE);
		else if (mKey == pvKey) m.setScore(MAX_MOVE_SCORE - 1);
		else if (mKey == k1Key) m.setScore(KILLER_MOVE_1_SCORE);
		else if (mKey == k2Key) m.setScore(KILLER_MOVE_2_SCORE);
	}

	/*
	 * we've commented out sorting entire list because its too expensive, and it leads to a very high number of cache misses.
	 * in the main loop, we just swap the best move to the front, and this is more performant because it avoids all the cache misses
	 */
	// std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
	// 	return a.getScore() > b.getScore();
	// });

	Moves subline;
	int movesSearched = 0;
	Move bestMove;
	for (size_t i = 0; i < moves.size(); i++) {
		int bestIdx = i;
		for (size_t j = i + 1; j < moves.size(); j++) {
			if (moves[j].getScore() > moves[bestIdx].getScore()) {
				bestIdx = j;
			}
		}
		std::swap(moves[i], moves[bestIdx]);

		const Move& m = moves[i];
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
			topLine.clear();
			topLine.push_back(m);
			topLine.insert(topLine.end(), subline.begin(), subline.end());
			TranspositionTable::add(b.boardState.hash, score, depthLeft, TTFlag::LOWER_BOUND, m);
			return {score, SEARCH_COMPLETE};
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
	for (size_t depth = 1; depth <= maxDepth; depth++) {
		auto [eval, searchState] = search(topLine, b, depth, previousPV, -INF_SCORE, INF_SCORE, 0);

		if (topLine.size() < depth) {
			// early alpha beta cutoff
			// fill the rest of the line with tt moves
			Board copy = b;
			for (Move m : topLine) copy.execute(m);
			int topLineSize = topLine.size();
			for (size_t i = 0; i < depth - topLineSize; i++) {
				const TTEntry& entry = TranspositionTable::getEntry(copy.boardState.hash);
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
		std::cout << " (depth " << depth << ")\n";

		finalScore = eval;
		previousPV = std::move(topLine);
		if (abs(finalScore) >= INF_SCORE - 2000) break;
	}
	topLine = std::move(previousPV);
	std::cout << "NPS: " << totalNodesSearched / std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - searchStartTime).count() << "\n";
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

		finalScore = eval;
		previousPV = std::move(topLine);

		if (topLine.size() < depth) {
			// early alpha beta cutoff
			// fill the rest of the line with tt moves
			Board copy = b;
			for (Move m : topLine) copy.execute(m);
			int topLineSize = topLine.size();
			for (size_t i = 0; i < depth - topLineSize; i++) {
				const TTEntry& entry = TranspositionTable::getEntry(copy.boardState.hash);
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
		std::cout << " (depth " << depth << ")\n";
		if (abs(finalScore) >= INF_SCORE - 2000) {
			std::cout << "MATE FOUND\n";
			break;
		}
	}
	topLine = std::move(previousPV);
	std::cout << "NPS: " << totalNodesSearched / std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - searchStartTime).count() << "\n";
	std::cout << "Score: " << finalScore << "\n";
	totalNodesSearched = 0;
	return finalScore;
}

int Eval::calculateReductionFactor(int movesSearched, int depthLeft) {
	unsigned int N = movesSearched * depthLeft;
	if (N == 0) return 0;
	int log2_N = std::bit_width(N) - 1;
	return log2_N / 2;
}

Centipawns Eval::positionalValue(Piece pieceType, Color color, Square square) {
	return m_materialWeights[pieceType][color][(int)square];
}
