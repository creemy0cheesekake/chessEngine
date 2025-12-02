#ifndef GUI_H
#define GUI_H

#include <GLFW/glfw3.h>
#include <functional>
#include "board.hpp"
#include "imgui.h"

#include <list>

struct MoveHistory {
	std::string notation;
	Color colorOfPlayer;
	int moveNum;
	MoveHistory(const std::string& n, const Color& c, const int mn) : notation(n), colorOfPlayer(c), moveNum(mn) {}
};

class Gui {
private:
	Board& m_board;
	Color m_playerColor;
	Square m_selected;
	bool m_resigned = false;
	std::array<std::array<ImTextureID, 6>, 2> pieceTextures;
	std::function<void(Square)> m_sendClick;
	std::string outputMoveHistory();

public:
	GLFWwindow* m_Window;
	std::list<MoveHistory> gameHistory;

	Gui(Board&, Color, std::function<void(Square)>);
	void init();
	void drawUI();
	void setSelected(Square);
};

#endif
