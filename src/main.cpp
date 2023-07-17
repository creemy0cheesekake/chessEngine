#include <iostream>
#include "board.cpp"
#include "move.cpp"
#include "move_gen.cpp"

int main() {
	Board b = Board();
	// std::string fen = "r1bqkbnr/ppp2ppp/2np4/1B2p3/4P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 1 4";
	std::string fen = "8/8/8/8/1Q2K3/8/k7/8 b - - 4 6";
	b.setToFen(fen);
	std::cout << b.stringBoard() << std::endl;
	std::cout << "\n///////////" << std::endl;
	int x = 0;
	for (Move i : MoveGen(b).genMoves()) {
		std::cout << i.getFrom() << " " << i.getTo() << " " << i.getFlags() << " " << std::endl;
		std::cout << i.notation() << std::endl;
		std::cout << i.execute().stringBoard() << std::endl;
		x++;
	}
	std::cout << x << std::endl;
}
