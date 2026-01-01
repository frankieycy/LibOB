CXX      = g++
CXXFLAGS = -std=c++17 -ILib -Wall -Wextra -MMD -MP

# Build modes
DEBUG_FLAGS     = -g -O0
PROFILING_FLAGS = -g -O2
RELEASE_FLAGS   = -O2

# Default build mode
BUILD_MODE ?= DEBUG

ifeq ($(BUILD_MODE), DEBUG)
CXXFLAGS += $(DEBUG_FLAGS)
else ifeq ($(BUILD_MODE), PROFILING)
CXXFLAGS += $(PROFILING_FLAGS)
else
CXXFLAGS += $(RELEASE_FLAGS)
endif

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

# Track the last build mode
LAST_BUILD_MODE_FILE = .last_build_mode

# Check if the build mode has changed
FORCE_REBUILD = $(if $(filter $(BUILD_MODE),$(word 1,$(shell cat $(LAST_BUILD_MODE_FILE) 2>/dev/null))),0,1)

all: $(TARGET) update_last_build_mode

# Force rebuild if the build mode has changed
ifeq ($(FORCE_REBUILD),1)
all: clean rebuild

rebuild:
	$(MAKE) BUILD_MODE=$(BUILD_MODE)
endif

update_last_build_mode:
	echo "$(BUILD_MODE) FORCE_REBUILD=$(FORCE_REBUILD)" > $(LAST_BUILD_MODE_FILE)

# Link
$(TARGET): $(OBJ)
	@mkdir -p $(@D)
	$(CXX) $^ -o $@

# Compile (Obj/Utils/Utils.o from Lib/Utils/Utils.cpp, etc.)
Obj/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(if $(filter PROFILING,$(BUILD_MODE)),-lprofiler,)

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

debug:
	$(MAKE) BUILD_MODE=DEBUG

profiling:
	$(MAKE) BUILD_MODE=PROFILING

release:
	$(MAKE) BUILD_MODE=RELEASE

clean:
	rm -rf Obj Exe

# Pull in auto-generated header dependencies
-include $(DEP)

.SECONDARY: $(REG_OBJ)
