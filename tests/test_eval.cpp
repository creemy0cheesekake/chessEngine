#include "../include/doctest.h"
#include "custom_text_fixture.hpp"

#include "../src/eval.hpp"

CUSTOM_TEST_CASE("Test Material") {
	Board b;
	b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
	CHECK(Eval::countMaterial(b) == 0);
	b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2N/P1N4P/1PPP1PP1/R1BQK2R b KQ - 0 7");
	CHECK(Eval::countMaterial(b) == -900);
	b.setToFen("r2k1bnr/p1p2ppp/1pbp4/4p3/4P2N/P1N4P/1PPP1PP1/R1BQK2R w KQ - 0 9");
	CHECK(Eval::countMaterial(b) == 890);
}
