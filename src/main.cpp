#include <iostream>
#include "board.cpp"
#include "move.cpp"
#include "move_gen.cpp"

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

bool debug(Board b, int depth, int ans) {
	int total = 0;
	for (Move m : MoveGen(b).genMoves()) {
		int num = moveGenTest(m.execute(), depth - 1);
		total += num;
		std::cout << m.UCInotation() << ": " << num << std::endl;
	}
	std::cout << total << std::endl;
	return (total == ans);
}

int main() {
	Board b = Board();
	b.setToFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
	std::cout << test(b, 3, 97862) << std::endl;
}
