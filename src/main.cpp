#include <iostream>
#include "board.cpp"
#include "move.cpp"

int main() {
	Board b = Board();
	std::cout << b.stringBoard() << std::endl;
	// Move arr[] = {Move(b, 12, 28), Move(b, 52, 36), Move(b, 5, 26), Move(b, 57, 42), Move(b, 3, 39), Move(b, 62, 45), Move(b, 39, 53)};
	// for (Move m : arr) {
	// 	m.execute();
	// 	std::cout << b.stringBoard() << std::endl;
	// }
	//
	// Move(b, 12, 28).execute();
	// Move(b, 55, 47).execute();
	// Move(b, 28, 36).execute();
	// Move(b, 51, 35).execute();
	// Move m = Move(b, 36, 43);
	// std::cout << m.getFlags() << std::endl;
	// m.execute();
	std::cout << b.stringBoard() << std::endl;
}
