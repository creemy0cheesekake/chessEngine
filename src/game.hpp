#ifndef GAME_H
#define GAME_H

#include <GLFW/glfw3.h>
#include "board.hpp"
#include "gui.hpp"


class Game {
private:
	Square m_clicked;
	Square m_startOfMove;
	Color m_playerColor;
	Gui m_gui;
	Board& m_board;

	Move getAIMove();

public:
	Game(Board&, Color);
	void setClicked(Square);
	void gameLogic();
	void run();
};

#endif
