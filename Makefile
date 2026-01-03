# Makefile for the inLimbo project

PROJECT_NAME := inLimbo

SRC_DIRS := src include
BUILD_DIR := build
BUILD_DBG_DIR := build-dbg

# libbacktrace stuff
BACKTRACE_DIR := external/libbacktrace
BACKTRACE_BUILD_DIR := $(BACKTRACE_DIR)/build
BACKTRACE_PREFIX := $(BACKTRACE_BUILD_DIR)/install

# Find source files
CPP_FILES := $(shell find $(SRC_DIRS) -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.cxx" \))
HDR_FILES := $(shell find $(SRC_DIRS) -type f \( -name "*.h" -o -name "*.hpp" \))

# Tools
CLANG_FORMAT := clang-format
CLANG_TIDY   := clang-tidy
MAKE 	:= make
CMAKE := cmake

# Default frontend = command line
FRONTEND ?= cmdline
CMAKE_FRONTEND_FLAG := -DINLIMBO_FRONTEND_TYPE=$(FRONTEND)

# Path to compile_commands.json
COMPILE_COMMANDS := $(BUILD_DIR)/compile_commands.json

# Important files 

CI_BINARIES := ci/binaries.txt

# ============================================================
# ANSI colors
# ============================================================

COLOR_RESET := \033[0m
COLOR_BOLD  := \033[1m
COLOR_RED   := \033[31m
COLOR_GREEN := \033[32m
COLOR_YELLOW:= \033[33m
COLOR_BLUE  := \033[34m
COLOR_CYAN  := \033[36m

# ============================================================
# Phony targets
# ============================================================

.PHONY: \
	fmt fmt-check tidy clean\
	init submod-init submod-check init-dep-backtrace\
	app app-dbg \
	buildx build build-dbg rebuild rebuild-dbg\
	test test-dbg \
	verify-deps install-deps \
	stats all-checks help

# ============================================================
# Init stuff (explicit flags)
# ============================================================

init-dep-backtrace:
	@echo -e "$(COLOR_BLUE)▶ Initializing libbacktrace...$(COLOR_RESET)"
	@test -d $(BACKTRACE_DIR) || { \
		echo -e "$(COLOR_RED)✗ libbacktrace submodule not found$(COLOR_RESET)"; \
		echo -e "  Run: git submodule update --init --recursive"; \
		exit 1; \
	}
	@mkdir -p $(BACKTRACE_BUILD_DIR)
	@cd $(BACKTRACE_BUILD_DIR) && \
		../configure --prefix=$(abspath $(BACKTRACE_PREFIX))
	@$(MAKE) -C $(BACKTRACE_BUILD_DIR)
	@$(MAKE) -C $(BACKTRACE_BUILD_DIR) install
	@echo -e "$(COLOR_GREEN)✔ libbacktrace built and installed locally$(COLOR_RESET)"

# ============================================================
# Submodule initialization (explicit)
# ============================================================

submod-init:
	@echo -e "$(COLOR_BLUE)▶ Initializing git submodules...$(COLOR_RESET)"
	@git submodule update --init --recursive
	@echo -e "$(COLOR_GREEN)✔ Submodules initialized$(COLOR_RESET)"

submod-check:
	@echo -e "$(COLOR_BLUE)▶ Checking git submodules...$(COLOR_RESET)"
	@if git submodule status --recursive | grep -q '^-'; then \
		echo -e "$(COLOR_RED)✗ Some submodules are NOT initialized$(COLOR_RESET)"; \
		echo -e "  Run: $(COLOR_CYAN)git submodule update --init --recursive$(COLOR_RESET)"; \
		exit 1; \
	else \
		echo -e "$(COLOR_GREEN)✔ All submodules are initialized$(COLOR_RESET)"; \
	fi

init: submod-init submod-check

# ============================================================
# Initialize app type (Release)
# ============================================================

app:
	@if [ -z "$(name)" ]; then \
		echo -e "$(COLOR_YELLOW)⚠ No frontend specified, defaulting to 'cmdline'$(COLOR_RESET)"; \
		$(MAKE) init; \
		$(MAKE) buildx FRONTEND=cmdline; \
	else \
		echo -e "$(COLOR_BLUE)▶ Building frontend: $(name)$(COLOR_RESET)"; \
		$(MAKE) init; \
		$(MAKE) buildx FRONTEND=$(name); \
	fi

# ============================================================
# Initialize app type (Debug)
# ============================================================

app-dbg:
	@if [ -z "$(name)" ]; then \
		echo -e "$(COLOR_YELLOW)⚠ No frontend specified, defaulting to 'cmdline'$(COLOR_RESET)"; \
		$(MAKE) init; \
		$(MAKE) buildx-dbg FRONTEND=cmdline; \
	else \
		echo -e "$(COLOR_BLUE)▶ Building debug frontend: $(name)$(COLOR_RESET)"; \
		$(MAKE) init; \
		$(MAKE) init-dep-backtrace \
		$(MAKE) buildx-dbg FRONTEND=$(name); \
	fi

# ============================================================
# Build (Release)
# ============================================================

build:
	@echo -e "$(COLOR_BLUE)▶ Building...$(COLOR_RESET)"
	@$(CMAKE) --build $(BUILD_DIR)
	@echo -e "$(COLOR_GREEN)✔ Build complete!$(COLOR_RESET)"
	@echo -e ">> $(COLOR_CYAN)Frontend$(COLOR_RESET): $(FRONTEND)"

buildx:
	@echo -e "$(COLOR_BLUE)▶ Configuring build ($(BUILD_DIR))...$(COLOR_RESET)"
	@$(CMAKE) -S . -B $(BUILD_DIR) -D CMAKE_BUILD_TYPE=Release $(CMAKE_FRONTEND_FLAG)
	$(MAKE) build

rebuild: clean buildx

# ============================================================
# Build (Debug)
# ============================================================

build-dbg:
	@echo -e "$(COLOR_BLUE)▶ Building debug...$(COLOR_RESET)"
	@$(CMAKE) --build $(BUILD_DBG_DIR)
	@echo -e "$(COLOR_GREEN)✔ Debug build complete$(COLOR_RESET)"

buildx-dbg:
	@echo -e "$(COLOR_BLUE)▶ Configuring debug build ($(BUILD_DBG_DIR))...$(COLOR_RESET)"
	@$(CMAKE) -S . -B $(BUILD_DBG_DIR) -D CMAKE_BUILD_TYPE=Debug $(CMAKE_FRONTEND_FLAG)
	$(MAKE) build-dbg

rebuild-dbg: clean buildx-dbg

# ============================================================
# Tests (running them using ctest) - Release
# ============================================================

test:
	@echo -e "$(COLOR_BLUE)▶ Running tests...$(COLOR_RESET)"
	@ctest --test-dir $(BUILD_DIR) --output-on-failure

# ============================================================
# Tests (running them using ctest) - Debug
# ============================================================

test-dbg:
	@echo -e "$(COLOR_BLUE)▶ Running debug tests...$(COLOR_RESET)"
	@ctest --test-dir $(BUILD_DBG_DIR) --output-on-failure

# ============================================================
# Formatting
# ============================================================

fmt:
	@echo -e "$(COLOR_BLUE)▶ Running clang-format...$(COLOR_RESET)"
	@for file in $(CPP_FILES) $(HDR_FILES); do \
		echo -e "  $(COLOR_CYAN)fmt$(COLOR_RESET)  $$file"; \
		$(CLANG_FORMAT) -i "$$file"; \
	done
	@echo -e "$(COLOR_GREEN)✔ Formatting complete$(COLOR_RESET)"

# ============================================================
# Format check (no changes)
# ============================================================

fmt-check:
	@echo -e "$(COLOR_BLUE)▶ Checking formatting...$(COLOR_RESET)"
	@for file in $(CPP_FILES) $(HDR_FILES); do \
		$(CLANG_FORMAT) --dry-run --Werror "$$file" || exit 1; \
	done
	@echo -e "$(COLOR_GREEN)✔ Formatting OK$(COLOR_RESET)"

# ============================================================
# Static analysis
# ============================================================

tidy:
	@if [ ! -f "$(COMPILE_COMMANDS)" ]; then \
		echo -e "$(COLOR_RED)✗ compile_commands.json not found.$(COLOR_RESET)"; \
		echo -e "  $(COLOR_YELLOW)Hint: configure CMake with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON$(COLOR_RESET)"; \
		exit 1; \
	fi
	@echo -e "$(COLOR_BLUE)▶ Running clang-tidy...$(COLOR_RESET)"
	@$(CLANG_TIDY) $(CPP_FILES) -- -std=c++20
	@echo -e "$(COLOR_GREEN)✔ clang-tidy finished$(COLOR_RESET)"

# ============================================================
# Cleanup
# ============================================================

clean:
	@echo -e "$(COLOR_BLUE)▶ Cleaning build directories...$(COLOR_RESET)"
	@rm -rf $(BUILD_DIR) $(BUILD_DBG_DIR)
	@echo -e "$(COLOR_GREEN)✔ Clean complete$(COLOR_RESET)"

# ============================================================
# Dependency stuff
# ============================================================

verify-deps:
	@echo -e "$(COLOR_BLUE)▶ Verifying required tools...$(COLOR_RESET)"
	@test -f $(CI_BINARIES) || { \
		echo -e "$(COLOR_RED)✗ ci/binaries.txt not found$(COLOR_RESET)"; \
		exit 1; \
	}
	@missing=0; \
	while IFS= read -r tool; do \
		[ -z "$$tool" ] && continue; \
		if command -v $$tool >/dev/null 2>&1; then \
			echo -e "  $(COLOR_GREEN)✔$(COLOR_RESET) $$tool"; \
		else \
			echo -e "  $(COLOR_RED)✗$(COLOR_RESET) $$tool (missing)"; \
			missing=1; \
		fi; \
	done < ci/binaries.txt; \
	if [ $$missing -eq 1 ]; then \
		echo -e ""; \
		echo -e "$(COLOR_RED)✗ One or more required dependencies are missing$(COLOR_RESET)"; \
		echo -e "  $(COLOR_YELLOW)Hint: run 'make install-deps'$(COLOR_RESET)"; \
		exit 1; \
	else \
		echo -e ""; \
		echo -e "$(COLOR_GREEN)✔ All required dependencies are available$(COLOR_RESET)"; \
	fi

install-deps:
	@echo -e "$(COLOR_BLUE)▶ Installing project dependencies...$(COLOR_RESET)"
	@echo -e "$(COLOR_BOLD)$(COLOR_RED)"
	@echo -e "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo -e "⚠  WARNING: SYSTEM PACKAGE INSTALLATION"
	@echo -e "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo -e "$(COLOR_RESET)$(COLOR_YELLOW)"
	@echo -e "• This target executes your system package manager with sudo."
	@echo -e "• Packages will be installed directly onto your system."
	@echo -e "• Any failure MUST be resolved manually by the user."
	@echo -e "• The project takes NO responsibility for system changes."
	@echo -e "$(COLOR_RESET)"
	@echo -e "$(COLOR_DIM)Proceed only if you understand the implications.$(COLOR_RESET)"
	@echo -e ""
	@read -p "Continue with installation? [y/N] " ans; \
	[ "$$ans" = "y" ] || exit 1
	@if [ -f /etc/arch-release ]; then \
		echo -e "$(COLOR_CYAN)Detected Arch Linux$(COLOR_RESET)"; \
		echo -e "  $(COLOR_DIM)Command: sudo pacman -S --needed --noconfirm <packages>$(COLOR_RESET)"; \
		test -f ci/pacman/packages.txt || { \
			echo -e "$(COLOR_RED)✗ ci/pacman/packages.txt not found$(COLOR_RESET)"; exit 1; }; \
		sudo pacman -S --needed --noconfirm $$(cat ci/pacman/packages.txt); \
	elif [ -f /etc/debian_version ]; then \
		echo -e "$(COLOR_CYAN)Detected Debian / Ubuntu$(COLOR_RESET)"; \
		echo -e "  $(COLOR_DIM)Command: sudo apt update && sudo apt install -y <packages>$(COLOR_RESET)"; \
		test -f ci/apt/packages.txt || { \
			echo -e "$(COLOR_RED)✗ ci/apt/packages.txt not found$(COLOR_RESET)"; exit 1; }; \
		sudo apt update; \
		sudo apt install -y $$(cat ci/apt/packages.txt); \
	elif [ -f /etc/fedora-release ]; then \
		echo -e "$(COLOR_CYAN)Detected Fedora$(COLOR_RESET)"; \
		echo -e "  $(COLOR_DIM)Command: sudo dnf install -y <packages>$(COLOR_RESET)"; \
		test -f ci/rpm/packages.txt || { \
			echo -e "$(COLOR_RED)✗ ci/rpm/packages.txt not found$(COLOR_RESET)"; exit 1; }; \
		sudo dnf install -y $$(cat ci/rpm/packages.txt); \
	else \
		echo -e "$(COLOR_RED)✗ Unsupported distribution$(COLOR_RESET)"; \
		echo -e "  $(COLOR_YELLOW)Supported: Arch, Debian/Ubuntu, Fedora$(COLOR_RESET)"; \
		exit 1; \
	fi
	@echo -e ""
	@echo -e "$(COLOR_GREEN)✔ Dependency installation step completed$(COLOR_RESET)"

# ============================================================
# Project statistics
# ============================================================

stats:
	@echo -e "$(COLOR_BLUE)▶ Project statistics$(COLOR_RESET)"
	@echo -e "  Source files : $$(echo $(CPP_FILES) | wc -w)"
	@echo -e "  Header files : $$(echo $(HDR_FILES) | wc -w)"
	@echo -e "  Total files  : $$(echo $(CPP_FILES) $(HDR_FILES) | wc -w)"

all-checks: check verify-deps fmt-check tidy
	@echo -e "$(COLOR_GREEN)✔ All checks passed$(COLOR_RESET)"

# ============================================================
# Help
# ============================================================

help:
	@echo -e ""
	@echo -e "$(COLOR_BOLD)$(COLOR_GREEN)$(PROJECT_NAME) Makefile — Available Targets$(COLOR_RESET)"
	@echo -e ""
	@echo -e "  $(COLOR_CYAN)init$(COLOR_RESET)           Initialize project (git submodules, dependencies)"
	@echo -e "  $(COLOR_CYAN)submod-init$(COLOR_RESET)    Initialize git submodules"
	@echo -e "  $(COLOR_CYAN)submod-check$(COLOR_RESET)   Verify git submodules"
	@echo -e ""
	@echo -e "  $(COLOR_CYAN)app$(COLOR_RESET)            Build Release version of inLimbo with specified frontend"
	@echo -e "  $(COLOR_CYAN)app-dbg$(COLOR_RESET)        Build Debug version of inLimbo with specified frontend"
	@echo -e ""
	@echo -e "  $(COLOR_CYAN)build$(COLOR_RESET)          Build Release version of inLimbo with CMake (no init)"
	@echo -e "  $(COLOR_CYAN)buildx$(COLOR_RESET)         Initialize CMake and build Release version of inLimbo"
	@echo -e "  $(COLOR_CYAN)rebuild$(COLOR_RESET)        Rebuild Release version of inLimbo from scratch with CMake"
	@echo -e "  $(COLOR_CYAN)build-dbg$(COLOR_RESET)      Build Debug version of inLimbo with CMake (no init)"
	@echo -e "  $(COLOR_CYAN)buildx-dbg$(COLOR_RESET)     Initialize CMake and build Debug version of inLimbo"
	@echo -e "  $(COLOR_CYAN)rebuild-dbg$(COLOR_RESET)    Rebuild Debug version of inLimbo from scratch with CMake"
	@echo -e "  $(COLOR_CYAN)clean$(COLOR_RESET)          Remove build directories"
	@echo -e ""
	@echo -e "  $(COLOR_CYAN)test$(COLOR_RESET)           Run all compiles tests in Release build (uses GTest and ctest)"
	@echo -e "  $(COLOR_CYAN)test-dbg$(COLOR_RESET)       Run all compiles tests in Debug build(uses GTest and ctest)"
	@echo -e ""
	@echo -e "  $(COLOR_CYAN)fmt$(COLOR_RESET)            Format all C/C++ source and header files"
	@echo -e "  $(COLOR_CYAN)fmt-check$(COLOR_RESET)      Checks if all C/C++ source and header files are formatted (no edits)"
	@echo -e "  $(COLOR_CYAN)tidy$(COLOR_RESET)           Run clang-tidy static analysis"
	@echo -e ""
	@echo -e "  $(COLOR_CYAN)verify-deps$(COLOR_RESET)    Check required tools are installed"
	@echo -e "  $(COLOR_CYAN)install-deps$(COLOR_RESET)   Install system dependencies (uses sudo)"
	@echo -e "  $(COLOR_CYAN)stats$(COLOR_RESET)          Show project statistics"
	@echo -e "  $(COLOR_CYAN)all-checks$(COLOR_RESET)     Run all verification steps"
	@echo -e ""
	@echo -e "  $(COLOR_CYAN)help$(COLOR_RESET)           Show this help message"
	@echo -e ""
