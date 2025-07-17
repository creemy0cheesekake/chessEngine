#include <iostream>

#include "board.hpp"
#include "move.hpp"
#include "move_gen.hpp"
#include "lookup_tables.hpp"
#include "eval.hpp"
#include "util.hpp"

int main() {
	LookupTables::init();
	Board b = Board();
	b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
	// b.setToFen("6k1/8/R5K1/8/8/8/8/8 w - - 4 3");

	std::cout << b << std::endl;
	Timer::start();

	Moves topLine;
	float evaluation = Eval::search(&topLine, b, 4);
	std::cout << "score: " << evaluation << std::endl;

	std::cout << "pv: ";
	for (Move m : topLine) {
		b = m.execute();
		std::cout << m.notation() << " ";
	}
	std::cout << std::endl;

	Timer::end();
	std::cout << b << std::endl;
}
