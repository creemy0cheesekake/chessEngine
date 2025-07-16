CXX = g++
CXXFLAGS = -std=c++20 -g
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
	src/util.cpp

# Automatically derive object files with build/ prefix
# (e.g., src/board.cpp -> build/board.o)
OBJS = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

all: $(TARGET) run

# Rule to link the executable from all object files
# Depends on the build directory existing
$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: all run clean

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

