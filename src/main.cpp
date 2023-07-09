#include <iostream>
#include "board.cpp"
#include "move.cpp"
#include "move_gen.cpp"

int main() {
	Board b = Board();
	// Move m = Move(&b, 11, 27); std::cout << m.notation() << std::endl; m.execute(); m = Move(&b, 51, 35); std::cout << m.notation() << std::endl; m.execute(); m = Move(&b, 10, 26); std::cout << m.notation() << std::endl; m.execute(); m = Move(&b, 35, 26); std::cout << m.notation() << std::endl; m.execute(); m = Move(&b, 12, 28); std::cout << m.notation() << std::endl; m.execute(); m = Move(&b, 52, 36); std::cout << m.notation() << std::endl; m.execute(); m = Move(&b, 6, 21); std::cout << m.notation() << std::endl; m.execute(); m = Move(&b, 36, 27); std::cout << m.notation() << std::endl; m.execute(); m = Move(&b, 5, 26); std::cout << m.notation() << std::endl; m.execute(); m = Move(&b, 57, 42); std::cout << m.notation() << std::endl; m.execute(); m = Move(&b, 4, 6); std::cout << m.notation() << std::endl; m.execute(); std::cout << b.stringBoard() << std::endl;
	b.setToFen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
	std::cout << b.stringBoard() << std::endl;
	std::cout << "/////////////////////" << std::endl;
	for (Move i : MoveGen(&b).genPawnMoves()) {
		std::cout << i.getFrom() << " " << i.getTo() << " " << i.getFlags() << " " << std::endl;
		i.execute();
		std::cout << b.stringBoard() << std::endl;
		b.setToFen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
	}
}
