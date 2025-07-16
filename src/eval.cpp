#include "eval.hpp"
#include <unordered_map>
#include "consts.hpp"
#include "move_gen.hpp"
#include "util.hpp"

float Eval::countMaterial(Board b) {
	float material = 0;
	for (int color = WHITE; color <= BLACK; color++) {
		for (int i = QUEEN; i < NONE; i++) {
			bitboard piece = b.pieces[color][i];
			int count	   = 0;
			while (piece) {
				piece ^= LS1B(piece);
				count++;
			}
			material +=
				count * (std::unordered_map<int, float>){
							{QUEEN, 9.0f},
							{ROOK, 5.0f},
							{BISHOP, 3.1f},
							{KNIGHT, 3.0f},
							{PAWN, 1.0f},
						}[i] *
				(color == WHITE ? 1 : -1);
		}
	}
	return material * (b.sideToMove == WHITE ? 1 : -1);
}

float Eval::evaluate(Board b) {
	float evaluation = countMaterial(b);
	return round(evaluation, 5);
}

float Eval::search(Moves *topLine, Board b, int depthLeft, float alpha, float beta) {
	if (depthLeft <= 0) return evaluate(b);

	MoveGen mg	= MoveGen(b);
	Moves moves = mg.genMoves();
	if (!moves.size() && !mg.inCheck()) return 0;  // stalemate

	float bestScore = -INF;
	Move bestMove;

	for (Move m : moves) {
		Board e = m.execute();
		Moves subline;
		float eval = -search(&subline, e, depthLeft - 1, -beta, -alpha);

		if (eval > bestScore) {
			bestScore = eval;
			bestMove  = m;
			alpha	  = std::max(alpha, bestScore);

			// add move to top line
			topLine->clear();
			topLine->push_back(bestMove);
			topLine->insert(topLine->end(), subline.begin(), subline.end());
		}
		if (eval >= beta) break;
	}
	return bestScore;
}
