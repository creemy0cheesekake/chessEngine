#include "eval.hpp"
#include <unordered_map>
#include "consts.hpp"
#include "move_gen.hpp"
#include "util.hpp"

Centipawns Eval::countMaterial(Board b) {
	Centipawns material = 0;
	for (Color color : {WHITE, BLACK}) {
		for (Piece pieceType : {QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
			Bitboard piece = b.pieces[color][pieceType];
			int count	   = 0;
			while (piece) {
				piece ^= LS1B(piece);
				count++;
			}
			material +=
				count * (std::unordered_map<int, Centipawns>){
							{QUEEN, 900},
							{ROOK, 500},
							{BISHOP, 310},
							{KNIGHT, 300},
							{PAWN, 100},
						}[pieceType] *
				(color == WHITE ? 1 : -1);
		}
	}
	return material * (b.sideToMove == WHITE ? 1 : -1);
}

Centipawns Eval::evaluate(Board b) {
	MoveGen mg	= MoveGen(b);
	Moves moves = mg.genLegalMoves();
	if (!moves.size()) {
		// add hmClock to prioritize quicker checkmates
		return mg.inCheck() ? -INF_SCORE + (int)b.hmClock : 0;
	}

	return countMaterial(b);
}

Centipawns Eval::search(Moves *topLine, Board b, int depthLeft, Centipawns alpha, Centipawns beta) {
	if (depthLeft <= 0 || b.gameOver()) {
		return evaluate(b);
	}

	MoveGen mg			 = MoveGen(b);
	Moves moves			 = mg.genLegalMoves();
	Centipawns bestScore = -INF_SCORE;
	Move bestMove;

	for (Move m : moves) {
		Board e = m.execute();
		Moves subline;
		Centipawns eval = -search(&subline, e, depthLeft - 1, -beta, -alpha);

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
