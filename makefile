# --------------------------------------------------------------------
# Toolchain
CXX ?= clang++
CXXFLAGS = -std=c++17 -ILib -Wall -Wextra -MMD -MP
LDFLAGS  =
LDLIBS   =
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Build modes
DEBUG_FLAGS     = -g -O0
PROFILING_FLAGS = -g -O2
RELEASE_FLAGS   = -O2

BUILD_MODE ?= DEBUG

ifeq ($(BUILD_MODE),DEBUG)
  CXXFLAGS += $(DEBUG_FLAGS)
else ifeq ($(BUILD_MODE),PROFILING)
  CXXFLAGS += $(PROFILING_FLAGS)
  LDFLAGS  += -L/opt/homebrew/lib
  LDLIBS   += -lprofiler
else ifeq ($(BUILD_MODE),RELEASE)
  CXXFLAGS += $(RELEASE_FLAGS)
else
  $(error Unknown BUILD_MODE=$(BUILD_MODE))
endif
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Output directories (mode-scoped)
OBJ_DIR := Obj/$(BUILD_MODE)
BIN_DIR := Exe/$(BUILD_MODE)
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Sources
SRC := $(shell find Run Lib -name '*.cpp')
OBJ := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

TARGET := $(BIN_DIR)/main
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Regression tests
REG_SRC := $(shell find RegressionTests/Inputs -name '*.cpp')
REG_OBJ := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(REG_SRC))
REG_EXE := $(patsubst RegressionTests/Inputs/%.cpp,$(BIN_DIR)/RegressionTests/%,$(REG_SRC))
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Phony targets
.PHONY: all clean debug profiling release regression regression_build regression_run help_profiling
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Default target
all: $(TARGET)
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Link main executable
$(TARGET): $(OBJ)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Compile objects
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Regression tests
regression:
	@echo "=== Running regression tests in RELEASE mode ==="
	@$(MAKE) BUILD_MODE=RELEASE regression_impl

regression_debug:
	@$(MAKE) BUILD_MODE=DEBUG regression_impl

regression_impl: regression_build regression_run

regression_build: $(REG_EXE)

regression_run:
	@echo "Running regression tests..."
	@for exe in $(REG_EXE); do \
		name=$$(basename $$exe); \
		if [ -z "$(FILTER)" ] || echo $$name | grep -q "$(FILTER)"; then \
			echo "Running $$exe..."; \
			./$$exe || echo "Test failed: $$exe"; \
		fi \
	done

$(BIN_DIR)/RegressionTests/%: $(OBJ_DIR)/RegressionTests/Inputs/%.o \
                             $(filter-out $(OBJ_DIR)/Run/%.o,$(OBJ))
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Convenience build-mode targets
debug:
	@$(MAKE) BUILD_MODE=DEBUG all

profiling: help_profiling
	@$(MAKE) BUILD_MODE=PROFILING all

release:
	@$(MAKE) BUILD_MODE=RELEASE all
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Utilities
clean:
	rm -rf Obj Exe

help_profiling:
	@echo "HELP: run \`CPUPROFILE=profile.out ./Exe/PROFILING/main\` to collect profiling data"
	@echo "HELP: run \`pprof --http=:8080 ./Exe/PROFILING/main profile.out\` to view report"
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Dependency files
-include $(DEP)

.SECONDARY: $(REG_OBJ)
# --------------------------------------------------------------------
