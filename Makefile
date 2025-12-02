CXX = g++
CXXFLAGS = -g -MMD -MP --std=c++20 -O2
BUILD_DIR = build
TARGET = $(BUILD_DIR)/engine

SRCS = \
	src/board.cpp \
	src/eval.cpp \
	src/lookup_tables.cpp \
	src/main.cpp \
	src/move.cpp \
	src/move_gen_attacks.cpp \
	src/move_gen.cpp \
	src/util.cpp \
	src/transposition_table.cpp \
	src/zobrist.cpp \
	src/gui.cpp \
	src/game.cpp

TEST_TARGET = $(BUILD_DIR)/run_tests
TEST_BUILD_DIR = $(BUILD_DIR)/tests
TEST_SRCS = \
	tests/doctest_main.cpp \
	tests/test_board.cpp \
	tests/test_eval.cpp \
	tests/test_move_gen.cpp \
	tests/test_move.cpp

TEST_OBJS = $(patsubst tests/%.cpp,$(TEST_BUILD_DIR)/%.o,$(TEST_SRCS))
TEST_DEPS = $(patsubst tests/%.cpp,$(TEST_BUILD_DIR)/%.d,$(TEST_SRCS))

OBJS = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(patsubst src/%.cpp,$(BUILD_DIR)/%.d,$(SRCS))

IMGUI_DIR = imgui
IMGUI_SRCS = \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_glfw.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

IMGUI_OBJS = $(patsubst $(IMGUI_DIR)/%.cpp,$(BUILD_DIR)/imgui/%.o,$(IMGUI_SRCS))
IMGUI_DEPS = $(IMGUI_OBJS:.o=.d)
IMGUI_LIB = $(BUILD_DIR)/libimgui.a

IMGUI_INCLUDES = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
GLFW_FLAGS = $(shell pkgconf --cflags --libs glfw3)
GL_LIBS = -lGL -ldl -lpthread

all: $(TARGET) run

$(TARGET): $(OBJS) $(IMGUI_LIB) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(IMGUI_LIB) $(GLFW_FLAGS) $(GL_LIBS) -o $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(IMGUI_INCLUDES) -c $< -o $@

$(TEST_TARGET): $(TEST_OBJS) $(filter-out $(BUILD_DIR)/main.o, $(OBJS)) | $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_BUILD_DIR)/%.o: tests/%.cpp | $(TEST_BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui/%.o: $(IMGUI_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(IMGUI_INCLUDES) -c $< -o $@

$(IMGUI_LIB): $(IMGUI_OBJS)
	mkdir -p $(dir $@)
	ar rcs $@ $^

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

.PHONY: all run clean

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(DEPS)

-include $(DEPS) $(TEST_DEPS) $(IMGUI_DEPS)
