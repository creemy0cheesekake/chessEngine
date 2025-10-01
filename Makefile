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
	src/zobrist.cpp

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

all: $(TARGET) run

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_TARGET): $(TEST_OBJS) $(filter-out $(BUILD_DIR)/main.o, $(OBJS)) | $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_BUILD_DIR)/%.o: tests/%.cpp | $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

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

-include $(DEPS) $(TEST_DEPS)
