CXX = g++
CXXFLAGS = -Ilib -std=c++17 -Wall -Wextra -O2 -g

# Source files
SRC_MAIN = run/main.cpp
SRC_LIB = $(wildcard lib/*.cpp)
SRC = $(SRC_MAIN) $(SRC_LIB)

# Object files
OBJ_DIR = obj
OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC))

# Executable
TARGET = exe/main

# Default target
all: $(TARGET)

# Link all object files into the executable
$(TARGET): $(OBJ)
	mkdir -p $(dir $@)
	$(CXX) -o $@ $^

# Compile source files to object files
$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
