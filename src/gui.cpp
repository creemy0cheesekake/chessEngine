#include "board.hpp"
#include "consts.hpp"
#include "eval.hpp"
#include "gui.hpp"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

static ImTextureID LoadTextureFromFile(const char* filename, int* out_width = nullptr, int* out_height = nullptr) {
	int image_width = 0, image_height = 0, channels = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, &channels, 4);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image_data);

	if (out_width) *out_width = image_width;
	if (out_height) *out_height = image_height;

	return (ImTextureID)(intptr_t)tex;
}

Gui::Gui(Board& b, Color pC, std::function<void(Square)> func) : m_board(b), m_playerColor(pC), m_Window(nullptr), m_sendClick(func) {
	m_selected = NONE_SQUARE;
	init();

	pieceTextures[WHITE][KING]	 = LoadTextureFromFile("assets/pieces/wk.png");
	pieceTextures[WHITE][QUEEN]	 = LoadTextureFromFile("assets/pieces/wq.png");
	pieceTextures[WHITE][ROOK]	 = LoadTextureFromFile("assets/pieces/wr.png");
	pieceTextures[WHITE][BISHOP] = LoadTextureFromFile("assets/pieces/wb.png");
	pieceTextures[WHITE][KNIGHT] = LoadTextureFromFile("assets/pieces/wn.png");
	pieceTextures[WHITE][PAWN]	 = LoadTextureFromFile("assets/pieces/wp.png");
	pieceTextures[BLACK][KING]	 = LoadTextureFromFile("assets/pieces/bk.png");
	pieceTextures[BLACK][QUEEN]	 = LoadTextureFromFile("assets/pieces/bq.png");
	pieceTextures[BLACK][ROOK]	 = LoadTextureFromFile("assets/pieces/br.png");
	pieceTextures[BLACK][BISHOP] = LoadTextureFromFile("assets/pieces/bb.png");
	pieceTextures[BLACK][KNIGHT] = LoadTextureFromFile("assets/pieces/bn.png");
	pieceTextures[BLACK][PAWN]	 = LoadTextureFromFile("assets/pieces/bp.png");
}

void Gui::init() {
	glfwInit();
	m_Window = glfwCreateWindow(1280, 720, "Chess", nullptr, nullptr);
	glfwMakeContextCurrent(m_Window);
	glfwSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

void Gui::setSelected(Square sq) {
	m_selected = sq;
}

void Gui::drawUI() {
	ImGui::BeginChild("Move History");
	ImGui::TextWrapped("%s", outputMoveHistory().c_str());
	if (m_resigned || m_board.isGameOver()) {
		Centipawns score = Eval::evaluate(m_board);
		ImGui::Text("Game over. %s", score > 0 || m_resigned ? "Player loses." : score < 0 ? "Player wins." : "Draw.");
		setSelected(NONE_SQUARE);
	}
	ImGui::EndChild();
	ImGui::Begin("Undo Move");
	ImGui::Text("Press to undo last move. Undos move regardless of who played it.");
	if (ImGui::Button("UNDO MOVE", ImVec2(200, 100))) {
		m_board.undoMove();
		m_board.undoMove();
		gameHistory.pop_back();
		gameHistory.pop_back();
	}
	ImGui::End();
	ImGui::Begin("Resign");
	ImGui::Text("Press to resign. Cannot be undone.");
	ImGui::PushStyleColor(ImGuiCol_Button, 0xFF050547);
	if (ImGui::Button("RESIGN", ImVec2(200, 100))) {
		m_resigned = true;
	}
	ImGui::PopStyleColor();
	ImGui::End();

	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImVec2 center	  = ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + vp->WorkSize.y * 0.5f);

	int viewportWidth		  = vp->WorkSize.x;
	int viewportHeight		  = vp->WorkSize.y;
	const int squareSize	  = std::min(viewportHeight, viewportWidth) / 10;
	const int boardSize		  = squareSize * 8;
	const ImU32 lightColor	  = 0xFFB5D9F0;
	const ImU32 darkColor	  = 0xFF6388B5;
	const ImU32 selectedColor = 0xFFB58863;

	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(boardSize, boardSize), ImGuiCond_Always);

	ImGui::Begin("##board_centered", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);

	ImVec2 p	   = ImGui::GetCursorScreenPos();
	ImDrawList* dl = ImGui::GetWindowDrawList();

	for (int r = 0; r < 8; r++) {
		for (int c = 0; c < 8; c++) {
			int squareIndex = m_playerColor == WHITE ? (7 - r) * 8 + c : r * 8 + (7 - c);
			ImVec2 min		= ImVec2(p.x + c * squareSize, p.y + r * squareSize);
			ImVec2 max		= ImVec2(min.x + squareSize, min.y + squareSize);
			bool isDark		= ((c + r) & 1);
			ImU32 color		= isDark ? darkColor : lightColor;
			if (squareIndex == m_selected) color = selectedColor;
			dl->AddRectFilled(min, max, color);

			float pieceSize = squareSize * 0.8f;
			float offset	= (squareSize - pieceSize) * 0.5f;
			ImVec2 pieceMin = ImVec2(min.x + offset, min.y + offset);
			ImVec2 pieceMax = ImVec2(pieceMin.x + pieceSize, pieceMin.y + pieceSize);

			for (Color color : {WHITE, BLACK}) {
				for (Piece pieceType : {KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
					if (!(m_board.boardState.pieces[color][pieceType] & (1UL << squareIndex))) continue;
					dl->AddImage(pieceTextures[color][pieceType], pieceMin, pieceMax);
				}
			}
		}
	}
	ImGui::SetCursorScreenPos(p);
	if (!(m_resigned || m_board.isGameOver())) ImGui::InvisibleButton("board_invisible_btn_centered", ImVec2(boardSize, boardSize));

	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
		ImVec2 mouse	= io.MousePos;
		int localX		= mouse.x - p.x;
		int localY		= mouse.y - p.y;
		int squareIndex = m_playerColor == WHITE ? 8 * (7 - localY / squareSize) + localX / squareSize : 8 * (localY / squareSize) + (7 - localX / squareSize);

		m_sendClick((Square)squareIndex);
	}

	ImGui::End();
}

std::string Gui::outputMoveHistory() {
	std::ostringstream res;
	for (const MoveHistory& mh : gameHistory) {
		if (mh.colorOfPlayer == WHITE) res << std::endl << mh.moveNum << ". ";
		res << mh.notation << " ";
	}
	return res.str();
}
