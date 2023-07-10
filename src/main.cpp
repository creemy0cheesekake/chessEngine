#include <iostream>
#include "board.cpp"
#include "move.cpp"
#include "move_gen.cpp"

int main() {
	Board b = Board();
	b.setToFen("7K/8/4N3/8/8/3N4/8/k7 w - - 0 1");
	std::cout << b.stringBoard() << std::endl;
	std::cout << "\n///////////" << std::endl;
	for (Move i : MoveGen(&b).genKnightMoves()) {
		std::cout << i.getFrom() << " " << i.getTo() << " " << i.getFlags() << " " << std::endl;
		i.execute();
		std::cout << b.stringBoard() << std::endl;
		b.setToFen("7K/8/4N3/8/8/3N4/8/k7 w - - 0 1");
	}
}
