#include <iostream>

#include "consts.hpp"
#include "move.hpp"
#include "lookup_tables.hpp"
#include "eval.hpp"
#include "util.hpp"
#include "zobrist.hpp"
#include "transposition_table.hpp"

int main() {
	LookupTables::init();
	Zobrist::init();
	Board b = Board();
	b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
	// b.setToFen("6k1/8/R5K1/8/8/8/8/8 w - - 4 3");
	// b.setToFen("3k4/8/3K4/3P4/8/8/8/8 w - - 0 1");

	// b.execute(Move(b, f3, h4, KNIGHT));
	// b.execute(Move(b, c6, d4, KNIGHT));
	// std::cout << b << std::endl;
	// std::cout << Eval::evaluate(b) << std::endl;
	// b.undoMove();
	// b.execute(Move(b, c8, b7, BISHOP));
	// std::cout << b << std::endl;
	// std::cout << Eval::evaluate(b) << std::endl;
	// abort();

	Timer::start();

	Moves topLine;
	Centipawns evaluation = Eval::iterative_deepening_ply(topLine, b, 7);
	// Centipawns evaluation = Eval::iterative_deepening_time(topLine, b, 5000);
	std::cout << "score: " << evaluation << std::endl;

	std::cout << "pv: ";
	for (Move m : topLine) {
		b.execute(m);
		std::cout << m.notation() << " ";
	}
	std::cout << std::endl;
	std::cout << b.boardState.hash << std::endl;

	Timer::end();
	std::cout << b << std::endl;
	TranspositionTable::printCapacity();
}
