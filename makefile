CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g -Ilib -MMD -MP

# --------------------------------------------------------------------
# Source discovery (recursive)
SRC := $(shell find run lib -name '*.cpp')
OBJ := $(patsubst %.cpp, obj/%.o, $(SRC))
DEP := $(OBJ:.o=.d)

TARGET = exe/main
# --------------------------------------------------------------------

all: $(TARGET)

# Link
$(TARGET): $(OBJ)
	@mkdir -p $(@D)
	$(CXX) $^ -o $@

# Compile (obj/Utils/Utils.o from lib/Utils/Utils.cpp, etc.)
obj/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj exe

# Pull in auto-generated header dependencies
-include $(DEP)
