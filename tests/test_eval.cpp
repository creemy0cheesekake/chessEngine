#include "../include/doctest.h"
#include "custom_text_fixture.hpp"

#include "../src/board.hpp"
#include "../src/eval.hpp"

CUSTOM_TEST_CASE("Test Material") {
	SUBCASE("Test captures and promotion for white") {
		Board b;
		b.setToFen("2k5/7r/8/6P1/8/1p6/1K6/3R2b1 w - - 0 1");
		CHECK(b.boardState.material == -310);
		b.execute(Move(b, d1, g1, ROOK));
		CHECK(b.boardState.material == 0);
		b.execute(Move(b, h7, h5, ROOK));
		CHECK(b.boardState.material == 0);
		b.execute(Move(b, g5, g6, PAWN));
		CHECK(b.boardState.material == 0);
		b.execute(Move(b, h5, h6, ROOK));
		b.execute(Move(b, g6, g7, PAWN));
		b.execute(Move(b, h6, h7, ROOK));
		b.execute(Move(b, g7, g8, PAWN, QUEEN));
		CHECK(b.boardState.material == 800);
		b.undoMove();
		b.execute(Move(b, g7, g8, PAWN, ROOK));
		CHECK(b.boardState.material == 400);
		b.undoMove();
		b.execute(Move(b, g7, g8, PAWN, KNIGHT));
		CHECK(b.boardState.material == 200);
	}
	SUBCASE("Test captures and promotion for black") {
		Board b;
		b.setToFen("1B1r4/6k1/6P1/8/8/1p6/R7/5K2 b - - 0 1");
		CHECK(b.boardState.material == 310);
		b.execute(Move(b, d8, b8, ROOK));
		CHECK(b.boardState.material == 0);
		b.execute(Move(b, a2, a3, ROOK));
		CHECK(b.boardState.material == 0);
		b.execute(Move(b, b3, b2, PAWN));
		CHECK(b.boardState.material == 0);
		b.execute(Move(b, a3, a2, ROOK));
		b.execute(Move(b, b2, b1, PAWN, QUEEN));
		CHECK(b.boardState.material == -800);
		b.undoMove();
		b.execute(Move(b, b2, b1, PAWN, ROOK));
		CHECK(b.boardState.material == -400);
		b.undoMove();
		b.execute(Move(b, b2, b1, PAWN, KNIGHT));
		CHECK(b.boardState.material == -200);
	}
}

CUSTOM_TEST_CASE("Test evaluate") {
	SUBCASE("Equal material position") {
		Board b;
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		Centipawns score = Eval::evaluate(b);
		CHECK(abs(score) < 200);
	}
	SUBCASE("White material advantage") {
		Board b;
		b.setToFen("r2k1bnr/p1p2ppp/1pbp4/4p3/4P2N/P1N4P/1PPP1PP1/R1BQK2R w KQ - 0 9");
		Centipawns score = Eval::evaluate(b);
		CHECK(score > 800);
	}
	SUBCASE("Black material advantage") {
		Board b;
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2N/P1N4P/1PPP1PP1/R1BQK2R b KQ - 0 7");
		Centipawns score = Eval::evaluate(b);
		CHECK(score < -800);
	}
}
