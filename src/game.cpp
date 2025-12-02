#include "eval.hpp"
#include "game.hpp"
#include "transposition_table.hpp"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

Game::Game(Board& b, Color pC) : m_board(b), m_playerColor(pC), m_gui(b, pC, [this](Square sq) { m_clicked = sq; }) {
	m_clicked	  = NONE_SQUARE;
	m_startOfMove = NONE_SQUARE;
}

void Game::gameLogic() {
	if (m_board.boardState.sideToMove != m_playerColor) {
		Move aiMove = getAIMove();
		m_gui.setSelected(aiMove.getTo());
		m_gui.gameHistory.push_back({aiMove.notationWithAnnotations(m_board), (Color)!m_playerColor, (int)m_board.boardState.fmClock});
		m_board.execute(aiMove);
		return;
	}

	if (m_clicked == NONE_SQUARE) return;
	Moves moves = m_board.moveGenerator.genLegalMoves();
	Move selectedMove;
	if (m_startOfMove != NONE_SQUARE) {
		for (const Move& m : moves) {
			if (m.getFrom() == m_startOfMove && m.getTo() == m_clicked) {
				selectedMove = m;
				break;
			}
		}
		m_startOfMove = NONE_SQUARE;
		m_gui.setSelected(NONE_SQUARE);

		if (selectedMove.getPieceType() != NONE_PIECE) {
			m_gui.gameHistory.push_back({selectedMove.notationWithAnnotations(m_board), m_playerColor, (int)m_board.boardState.fmClock});
			m_board.execute(selectedMove);
		}
	} else if ((m_board.whitePieces() | m_board.blackPieces()) & (1UL << m_clicked)) {
		m_startOfMove = m_clicked;
		m_gui.setSelected(m_startOfMove);
	}
	m_clicked = NONE_SQUARE;
}

Move Game::getAIMove() {
	Moves topLine;
	Eval::iterative_deepening_time(topLine, m_board, 5000);
	TranspositionTable::reset();
	return topLine[0];
}

void Game::run() {
	while (!glfwWindowShouldClose(m_gui.m_Window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		gameLogic();
		m_gui.drawUI();

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(m_gui.m_Window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_gui.m_Window);
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(m_gui.m_Window);
	glfwTerminate();
}
