#include <iostream>
#include "board.cpp"
#include "move.cpp"
#include "move_gen.cpp"

int main() {
	Board b = Board();
	b.setToFen("8/1k2K3/8/8/8/2B5/8/8 w - - 4 3");
	std::cout << b.stringBoard() << std::endl;
	std::cout << "\n///////////" << std::endl;
	MoveGen(&b).genBishopMoves();
	// for (Move i : MoveGen(&b).genKingMoves()) {
	// 	std::cout << i.getFrom() << " " << i.getTo() << " " << i.getFlags() << " " << std::endl;
	// 	i.execute();
	// 	std::cout << bin(*b.W_KING) << std::endl;
	// 	std::cout << b.stringBoard() << std::endl;
	// 	b.setToFen("8/1k6/8/4K3/8/2B5/8/8 w - - 0 1");
	// }
}
