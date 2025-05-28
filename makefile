CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g -Ilib -MMD -MP

# --------------------------------------------------------------------
# Source discovery (recursive)
SRC := $(shell find run lib -name '*.cpp')
OBJ := $(patsubst %.cpp, obj/%.o, $(SRC))
DEP := $(OBJ:.o=.d)

TARGET = exe/main
# --------------------------------------------------------------------
# Regression test setup
REG_SRC := $(shell find RegressionTests/Inputs -name '*.cpp')
REG_OBJ := $(patsubst %.cpp, obj/%.o, $(REG_SRC))
REG_EXE := $(patsubst RegressionTests/Inputs/%.cpp, exe/RegressionTests/%, $(REG_SRC))
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

# Regression: build and run all regression tests
regression: $(REG_EXE)
	@echo "Running regression tests..."
	@for exe in $(REG_EXE); do \
		echo "Running $$exe..."; \
		./$$exe || echo "Test failed: $$exe"; \
	done

# Rule to build each regression test executable from its object file
exe/RegressionTests/%: obj/RegressionTests/Inputs/%.o $(filter-out obj/run/%.o, $(OBJ))
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -rf obj exe

# Pull in auto-generated header dependencies
-include $(DEP)
