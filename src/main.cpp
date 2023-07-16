#include <iostream>
#include "board.cpp"
#include "move.cpp"
#include "move_gen.cpp"

int main() {
	Board b			= Board();
	std::string fen = "r1bqrnk1/pp2bpp1/2p2n1p/3p4/3P3B/2NBP3/PPQ1NPPP/R4RK1 w - - 4 12";
	b.setToFen(fen);
	std::cout << b.stringBoard() << std::endl;
	std::cout << "\n///////////" << std::endl;
	int x = 0;
	for (Move i : MoveGen(&b).genQueenMoves()) {
		std::cout << i.getFrom() << " " << i.getTo() << " " << i.getFlags() << " " << std::endl;
		i.execute();
		std::cout << b.stringBoard() << std::endl;
		b.setToFen(fen);
		x++;
	}
	std::cout << x << std::endl;
}
