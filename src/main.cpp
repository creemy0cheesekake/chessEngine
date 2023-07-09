#include <iostream>
#include "board.cpp"
#include "move.cpp"
#include "move_gen.cpp"

int main() {
	// Board b = Board();
	// std::cout << b.stringBoard() << std::endl;
	// Move arr[] = {Move(&b, 12, 28), Move(&b, 52, 36), Move(&b, 5, 26), Move(&b, 57, 42), Move(&b, 3, 39), Move(&b, 62, 45), Move(&b, 39, 53)};
	// for (Move m : arr) {
	// 	m.execute();
	// 	std::cout << b.stringBoard() << std::endl;
	// }

	Board b = Board();
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
