#include "../include/doctest.h"
#include "custom_text_fixture.hpp"

#include "../src/move_gen.hpp"

CUSTOM_TEST_CASE("Test Attacks") {
	Board b;
	SUBCASE("Test Perft r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7") {
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		MoveGen mg = MoveGen(b);
		CHECK(mg.getAttacks() == 6836459791116673024UL);
	}
	SUBCASE("Test Perft r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1") {
		b.setToFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
		MoveGen mg = MoveGen(b);
		CHECK(mg.getAttacks() == 18427602327210643456UL);
	}
	SUBCASE("Test Perft 5b1k/6Q1/1r6/4K3/r7/8/3r1r2/8 b - - 0 1") {
		b.setToFen("5b1k/6Q1/1r6/4K3/r7/8/3r1r2/8 b - - 0 1");
		MoveGen mg = MoveGen(b);
		CHECK(mg.getAttacks() == 16194935981344833600UL);
	}
}

int perft(Board b, int depth) {
	if (depth <= 0) {
		return 1;
	}

	int n = 0;
	for (Move m : MoveGen(b).genLegalMoves()) {
		n += perft(m.execute(), depth - 1);
	}
	return n;
}

CUSTOM_TEST_CASE("Test Move Gen") {
	Board b;
	SUBCASE("Test Perft r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1") {
		b.setToFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
		CHECK(perft(b, 1) == 48);
		CHECK(perft(b, 2) == 2039);
		CHECK(perft(b, 3) == 97862);
	}
	SUBCASE("Test Perft rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8") {
		b.setToFen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
		CHECK(perft(b, 1) == 44);
		CHECK(perft(b, 2) == 1486);
		CHECK(perft(b, 3) == 62379);
	}
	SUBCASE("Test Perft r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ") {
		b.setToFen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ");
		CHECK(perft(b, 1) == 46);
		CHECK(perft(b, 2) == 2079);
		CHECK(perft(b, 3) == 89890);
	}
}
