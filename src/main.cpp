#include <iostream>
#include <vector>

#include "board.hpp"
#include "move.hpp"
#include "move_gen.hpp"
#include "lookup_tables.hpp"
#include "eval.hpp"
#include "util.hpp"

int perft(Board b, int depth) {
	if (depth <= 0) return 1;

	int n = 0;
	for (Move m : MoveGen(b).genMoves()) n += perft(m.execute(), depth - 1);
	return n;
}

bool test(Board b, int depth, int ans) {
	int num = perft(b, depth);
	return (num == ans);
}

int divide(Board b, int depth) {
	int total = 0;
	for (Move m : MoveGen(b).genMoves()) {
		int num = perft(m.execute(), depth - 1);
		total += num;
		std::cout << m.UCInotation() << ": " << num << std::endl;
	}
	return total;
}

int main() {
	LookupTables::init();
	Board b = Board();
	b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");

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
