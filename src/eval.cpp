#include "board.hpp"
#include "consts.hpp"
#include "eval.hpp"
#include "move_gen.hpp"
#include "util.hpp"

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

Centipawns Eval::search(Moves* topLine, Board& b, int depthLeft, Centipawns alpha, Centipawns beta) {
	if (depthLeft <= 0 || b.gameOver()) {
		return evaluate(b);
	}

	Moves moves			 = b.moveGenerator.genLegalMoves();
	Centipawns bestScore = -INF_SCORE;
	Move bestMove;

	for (Move m : moves) {
		b.execute(m);
		Moves subline;
		Centipawns eval = -search(&subline, b, depthLeft - 1, -beta, -alpha);
		b.undoMove();

		if (eval > bestScore) {
			bestScore = eval;
			bestMove  = m;
			alpha	  = std::max(alpha, bestScore);

			// add move to top line
			topLine->clear();
			topLine->push_back(bestMove);
			topLine->insert(topLine->end(), subline.begin(), subline.end());
		}
		if (eval >= beta) {
			break;
		}
	}
	return bestScore;
}
