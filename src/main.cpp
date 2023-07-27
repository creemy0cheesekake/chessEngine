#include <iostream>

#include "board.cpp"
#include "move.cpp"
#include "move_gen.cpp"
#include "lookup_tables.cpp"
#include "util.cpp"

int moveGenTest(Board b, int depth) {
	if (depth <= 0) return 1;

	int n = 0;
	for (Move m : MoveGen(b).genMoves()) n += moveGenTest(m.execute(), depth - 1);
	return n;
}

bool test(Board b, int depth, int ans) {
	int num = moveGenTest(b, depth);
	return (num == ans);
}

int debug(Board b, int depth) {
	int total = 0;
	for (Move m : MoveGen(b).genMoves()) {
		int num = moveGenTest(m.execute(), depth - 1);
		total += num;
		std::cout << m.UCInotation() << ": " << num << std::endl;
	}
	return total;
}

int main() {
	LookupTables::init();
	Board b = Board();
	b.setToFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

	Timer::start();
	std::cout << moveGenTest(b, 3) << std::endl;
	Timer::end();
}
