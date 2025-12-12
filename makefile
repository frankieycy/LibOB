CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g -ILib -MMD -MP

# --------------------------------------------------------------------
# Source discovery (recursive)
SRC := $(shell find Run Lib -name '*.cpp')
OBJ := $(patsubst %.cpp, Obj/%.o, $(SRC))
DEP := $(OBJ:.o=.d)

TARGET = Exe/main
# --------------------------------------------------------------------
# Regression test setup
REG_SRC := $(shell find RegressionTests/Inputs -name '*.cpp')
REG_OBJ := $(patsubst %.cpp, Obj/%.o, $(REG_SRC))
REG_EXE := $(patsubst RegressionTests/Inputs/%.cpp, Exe/RegressionTests/%, $(REG_SRC))
# --------------------------------------------------------------------

all: $(TARGET)

# Link
$(TARGET): $(OBJ)
	@mkdir -p $(@D)
	$(CXX) $^ -o $@

# Compile (Obj/Utils/Utils.o from Lib/Utils/Utils.cpp, etc.)
Obj/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regression: build and run all regression tests
regression: regression_build regression_run

regression_build: $(REG_EXE)

regression_run: regression_build
	@echo "Running regression tests..."
	@for exe in $(REG_EXE); do \
		name=$$(basename $$exe); \
		if [ -z "$(FILTER)" ] || echo $$name | grep -q "$(FILTER)"; then \
			echo "Running $$exe..."; \
			./$$exe || echo "Test failed: $$exe"; \
		fi \
	done

# Rule to build each regression test executable from its object file
Exe/RegressionTests/%: Obj/RegressionTests/Inputs/%.o $(filter-out Obj/Run/%.o, $(OBJ))
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -rf Obj Exe

# Pull in auto-generated header dependencies
-include $(DEP)

.SECONDARY: $(REG_OBJ)
