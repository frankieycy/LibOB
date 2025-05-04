CXX = g++
CXXFLAGS = -Ilib -std=c++11
SRC = run/main.cpp
OBJ_DIR = obj
OBJ = $(OBJ_DIR)/$(notdir $(SRC:.cpp=.o)) # Place .o files in obj/
TARGET = exe/main

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(dir $@) # Ensure exe/ directory exists
	$(CXX) -o $@ $^

$(OBJ_DIR)/%.o: run/%.cpp
	mkdir -p $(dir $@) # Ensure obj/ directory exists
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)