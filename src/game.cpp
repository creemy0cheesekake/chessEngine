#include "board.hpp"
#include "consts.hpp"
#include "eval.hpp"
#include "game.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

GLFWwindow* Game::g_Window = nullptr;

bool moveQueued = false;

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

Game::Game(Board& b, Color pC) : m_board(b), playerColor(pC) {
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

void Game::init() {
	glfwInit();
	g_Window = glfwCreateWindow(1280, 720, "window", nullptr, nullptr);
	glfwMakeContextCurrent(g_Window);
	glfwSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(g_Window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

static Square selectedSquare = NONE_SQUARE;

void Game::handleBoardClick(int squareIndex) {
	if (m_board.boardState.sideToMove != playerColor) return;
	if (selectedSquare == NONE_SQUARE) {
		selectedSquare = (Square)squareIndex;
		return;
	}

	Moves moves = m_board.moveGenerator.genLegalMoves();
	Move selectedMove;
	for (const Move& m : moves) {
		if (m.getFrom() == selectedSquare && m.getTo() == (Square)squareIndex) {
			selectedMove = m;
		}
	}

	if (selectedMove.getPieceType() == NONE_PIECE) {
		selectedSquare = NONE_SQUARE;
		return;
	}

	m_board.execute(selectedMove);
	selectedSquare = NONE_SQUARE;
	moveQueued	   = true;
}

void Game::drawUI() {
	// Compute center of main viewport (works well with docking & multi-monitor)
	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImVec2 center	  = ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + vp->WorkSize.y * 0.5f);

	int viewportWidth		  = vp->WorkSize.x;
	int viewportHeight		  = vp->WorkSize.y;
	const int squareSize	  = std::min(viewportHeight, viewportWidth) / 10;
	const int boardSize		  = squareSize * 8;
	const ImU32 lightColor	  = 0xFFB5D9F0;
	const ImU32 darkColor	  = 0xFF6388B5;
	const ImU32 selectedColor = 0xFFB58863;

	// Place the window so its CENTER is at `center`
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(boardSize, boardSize), ImGuiCond_Always);

	ImGui::Begin("##board_centered", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);

	// Top-left of drawing area (content region)
	ImVec2 p	   = ImGui::GetCursorScreenPos();
	ImDrawList* dl = ImGui::GetWindowDrawList();

	for (int r = 0; r < 8; ++r) {
		for (int c = 0; c < 8; ++c) {
			int squareIndex = (7 - r) * 8 + c;
			ImVec2 min		= ImVec2(p.x + c * squareSize, p.y + r * squareSize);
			ImVec2 max		= ImVec2(min.x + squareSize, min.y + squareSize);
			bool isDark		= ((c + r) & 1);
			ImU32 color		= isDark ? darkColor : lightColor;
			if (selectedSquare != NONE_SQUARE && selectedSquare == squareIndex) color = selectedColor;
			dl->AddRectFilled(min, max, color);

			float pieceWidth  = squareSize * 0.8f;
			float pieceHeight = squareSize * 0.8f;
			float offsetX	  = (squareSize - pieceWidth) * 0.5f;
			float offsetY	  = (squareSize - pieceHeight) * 0.5f;
			ImVec2 pieceMin	  = ImVec2(min.x + offsetX, min.y + offsetY);
			ImVec2 pieceMax	  = ImVec2(pieceMin.x + pieceWidth, pieceMin.y + pieceHeight);

			if (1UL << squareIndex & (m_board.whitePieces() | m_board.blackPieces())) {
				for (Color color : {WHITE, BLACK}) {
					for (Piece pieceType : {KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN}) {
						if (!(m_board.boardState.pieces[color][pieceType] & (1UL << squareIndex))) continue;
						ImTextureID tex = pieceTextures[color][pieceType];
						dl->AddImage(tex, pieceMin, pieceMax);
					}
				}
			}
			moveQueued = false;
		}
	}
	// Capture clicks with invisible button
	ImGui::SetCursorScreenPos(p);
	ImGui::InvisibleButton("board_invisible_btn_centered", ImVec2(boardSize, boardSize));

	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
		ImVec2 mouse	= io.MousePos;
		int localX		= mouse.x - p.x;
		int localY		= mouse.y - p.y;
		int squareIndex = 8 * (7 - localY / squareSize) + localX / squareSize;

		handleBoardClick(squareIndex);
	}

	ImGui::End();
}

void Game::run() {
	while (!glfwWindowShouldClose(g_Window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (!moveQueued && m_board.boardState.sideToMove != playerColor) {
			Moves topLine;
			int timeAlloted = 5000;
			while (topLine.empty()) {
				Eval::iterative_deepening_time(topLine, m_board, timeAlloted);
				timeAlloted *= 2;
			}
			m_board.execute(topLine[0]);
		}
		drawUI();

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(g_Window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(g_Window);
	}
}

void Game::shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(g_Window);
	glfwTerminate();
}