#include "board.hpp"
#include "transposition_table.hpp"
#include "consts.hpp"
#include "eval.hpp"
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
			int count	   = 0;
			while (piece) {
				piece ^= LS1B(piece);
				count++;
			}
			material +=
				count * pieceToCentipawns[pieceType] *
				(color == WHITE ? 1 : -1);
		}
	}
	return material * (b.boardState.sideToMove == WHITE ? 1 : -1);
}

Centipawns Eval::evaluate(Board& b) {
	Moves moves = b.moveGenerator.genLegalMoves();
	if (!moves.size()) {
		// add hmClock to prioritize quicker checkmates
		return b.moveGenerator.inCheck() ? -INF_SCORE + (int)b.boardState.hmClock : 0;
	}

	return countMaterial(b);
}

std::tuple<Centipawns, Move, SearchState> Eval::quiescence_search(Board& b, Centipawns alpha, Centipawns beta) {
	TTEntry entry	  = TranspositionTable::getEntry(b.boardState.hash);
	bool TTEntryFound = entry.partial_hash == (b.boardState.hash & 0xFFFF);

	if (TTEntryFound) {
		const bool isExact		= entry.flag == EXACT;
		const bool isLowerBound = entry.flag == ALPHA && entry.score >= beta;
		const bool isUpperBound = entry.flag == BETA && entry.score <= alpha;

		if (isExact || isLowerBound || isUpperBound) {
			return {entry.score, Move(entry.bestMove.from, entry.bestMove.to, entry.bestMove.piece, entry.bestMove.promoPiece, entry.bestMove.flags), SearchState::SEARCH_COMPLETE};
		}
	}

	Centipawns current_best_score = evaluate(b);
	if (current_best_score >= beta) {
		return {beta, Move(), SearchState::SEARCH_COMPLETE};
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
			return {NONE_SCORE, Move(), SearchState::SEARCH_ABORTED};
		}
		b.execute(m);
		auto [score, _, __] = quiescence_search(b, -beta, -alpha);
		score				= -score;
		b.undoMove();

		if (score >= beta) {
			TranspositionTable::add(b.boardState.hash, beta, 0, BETA, m);
			return {beta, m, SearchState::SEARCH_COMPLETE};
		}
		if (score > current_best_score) {
			current_best_score = score;
			bestMove		   = m;
		}
		if (score > alpha) {
			alpha = score;
		}
	}
	TranspositionTable::add(b.boardState.hash, current_best_score, 0, current_best_score > alpha ? EXACT : ALPHA, bestMove);
	return {current_best_score, bestMove, SearchState::SEARCH_COMPLETE};
}

void Eval::resetKillerMoves() {
	killerMoves = {};
}

std::tuple<Centipawns, Move, SearchState> Eval::search(Moves& topLine, Board& b, int depthLeft, Centipawns alpha, Centipawns beta, int plyFromRoot, Move previousMove) {
	totalNodesSearched++;
	TTEntry entry = TranspositionTable::getEntry(b.boardState.hash);
	std::pair<Square, Square> bestMovePos((Square)entry.bestMove.from, (Square)entry.bestMove.to);
	bool TTEntryFound = entry.partial_hash == (b.boardState.hash & 0xFFFF) && entry.depth >= depthLeft;
	if (TTEntryFound) {
		const bool isExact		= entry.flag == EXACT;
		const bool isLowerBound = entry.flag == ALPHA && entry.score >= beta;
		const bool isUpperBound = entry.flag == BETA && entry.score <= alpha;

		if (isExact || isLowerBound || isUpperBound) {
			return {entry.score, TranspositionTable::getMove(entry.bestMove), SearchState::SEARCH_COMPLETE};
		}
	}

	if (depthLeft <= 0 || b.gameOver()) {
		return quiescence_search(b, alpha, beta);
	}

	MoveGen mg			   = b.moveGenerator;
	Moves moves			   = mg.genLegalMoves();
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

	for (Move m : moves) {
		if (std::chrono::high_resolution_clock::now() > m_iterative_deepening_cutoff_time) {
			return {bestScore, Move(), SearchState::SEARCH_ABORTED};
		}
		b.execute(m);
		Moves subline;

		movesSearched++;
		bool isQuietMove = !(m.getFlags() & (CAPTURE + PROMOTION)) && !mg.inCheck();

		Centipawns eval = alpha;
		bool didLMR		= false;
		if (movesSearched > 3 && depthLeft >= 6 && isQuietMove) {
			didLMR = true;
			// std::cout << "didLMR" << std::endl;
			int reductionFactor = calculateReductionFactor(movesSearched, depthLeft);
			int lmrDepthLeft	= std::max(1, depthLeft - reductionFactor);
			// std::cout << "LMR: " << lmrDepthLeft << std::endl;
			auto [e, _, __] = search(subline, b, lmrDepthLeft, -(alpha + 1), -alpha, plyFromRoot + 1);
			eval			= -e;
		}
		if (eval < beta || !didLMR || movesSearched == 1) {
			if (movesSearched == 1) {
				auto [e, _, __] = search(subline, b, depthLeft - 1, -beta, -alpha, plyFromRoot + 1);
				eval			= -e;
			} else {
				auto [e, _, __] = search(subline, b, depthLeft - 1, -alpha - 1, -alpha, plyFromRoot + 1);
				eval			= -e;
				if (eval > alpha && beta - alpha > 1) {
					auto [e, _, __] = search(subline, b, depthLeft - 1, -beta, -alpha, plyFromRoot + 1);
					eval			= -e;
				}
			}
		}
		b.undoMove();

		if (eval > bestScore) {
			bestScore = eval;
			bestMove  = m;
			alpha	  = std::max(alpha, bestScore);

			// add move to top line
			topLine.clear();
			topLine.push_back(bestMove);
			topLine.insert(topLine.end(), subline.begin(), subline.end());
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
		TranspositionTable::add(b.boardState.hash, bestScore, depthLeft, BETA, bestMove);
	} else if (bestScore > alpha) {
		TranspositionTable::add(b.boardState.hash, bestScore, depthLeft, EXACT, bestMove);
	} else {
		TranspositionTable::add(b.boardState.hash, bestScore, depthLeft, ALPHA, bestMove);
	}

	return {bestScore, bestMove, SearchState::SEARCH_COMPLETE};
}

Centipawns Eval::iterative_deepening_ply(Moves& topLine, Board& b, int maxDepth) {
	Move previousMove;
	Centipawns finalScore;
	for (int depth = 1; depth <= maxDepth; depth++) {
		auto [eval, bestMove, _] = search(topLine, b, depth, -INF_SCORE, INF_SCORE, 0, previousMove);
		previousMove			 = topLine.back();
		finalScore				 = eval;

		for (Move m : topLine) {
			std::cout << m.notation() << " ";
		}
		std::cout << std::endl;
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
		finalScore						   = eval;

		for (Move m : topLine) {
			std::cout << m.notation() << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "NPS: " << totalNodesSearched / (maxTimeMs / 1000.0) << std::endl;
	return finalScore;
}

int Eval::calculateReductionFactor(int movesSearched, int depthLeft) {
	int N	   = movesSearched * depthLeft;
	int log2_N = 31 - __builtin_clz(N);
	return log2_N / 1;
}
