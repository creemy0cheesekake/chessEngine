#include <iostream>
#include "board.cpp"
#include "move.cpp"
#include "move_gen.cpp"

int main() {
	Board b = Board();
	b.setToFen("8/1k6/8/8/8/8/8/7K w - - 0 1");
	std::cout << b.stringBoard() << std::endl;
	std::cout << "\n///////////" << std::endl;
	for (Move i : MoveGen(&b).genKingMoves()) {
		std::cout << i.getFrom() << " " << i.getTo() << " " << i.getFlags() << " " << std::endl;
		i.execute();
		std::cout << b.stringBoard() << std::endl;
		b.setToFen("8/1k6/8/8/8/8/8/7K w - - 0 1");
	}
}
