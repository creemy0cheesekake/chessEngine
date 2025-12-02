#include <iostream>
#include "consts.hpp"
#include "move.hpp"
#include "lookup_tables.hpp"
#include "eval.hpp"
#include "game.hpp"
#include "transposition_table.hpp"
#include "util.hpp"
#include "zobrist.hpp"

int main() {
	LookupTables::init();
	Zobrist::init();
	Board b = Board();
	// b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
	// b.setToFen("6k1/8/R5K1/8/8/8/8/8 w - - 4 3");
	// b.setToFen("3k4/8/3K4/3P4/8/8/8/8 w - - 0 1");
	// b.setToFen("8/8/8/4K3/3Q4/1k6/8/8 w - - 0 1");

	Game g = Game(b, WHITE);
	g.run();
	TranspositionTable::printCapacity();
}
