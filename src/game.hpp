#ifndef GUI_H
#define GUI_H

#include <GLFW/glfw3.h>
#include "board.hpp"
#include "imgui.h"

class Game {
private:
	Board& m_board;
	Color playerColor;
	static GLFWwindow* g_Window;
	std::array<std::array<ImTextureID, 6>, 2> pieceTextures;

	void handleBoardClick(int squareIndex);

public:
	Game(Board&, Color);
	void init();
	void run();
	void shutdown();
	void drawUI();
};

#endif