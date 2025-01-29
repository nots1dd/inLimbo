# Makefile for the inLimbo project

BUILD_DIR := build
BUILD_DIR_DBG_ASAN := build-dbg-asan
BUILD_DIR_DBG_TSAN := build-dbg-tsan
EXECUTABLE := inLimbo
CMAKE := cmake
CMAKE_BUILD_TYPE := Release
SCRIPT := ./init.sh
TEST_SUITE_SCRIPT := ./run_tests.sh
VERBOSE_FLAG := VERBOSE=1
TESTING_FLAG := -DINLIMBO_TESTING
GLOBAL_FLAG := -DBUILD_GLOBAL
CLANG_FLAG := -DUSE_CLANG
INSTALL_MANIFEST := $(BUILD_DIR)/install_manifest.txt
CMAKE_EXTRA_FLAGS ?= # Default empty, set dynamically

# Targets
.PHONY: all build clean rebuild build-all init asan tsan build-global build-test build-test-all build-global-uninstall

all: build-all

build-test-all:
	@echo "==> Building inLimbo with script and tests using GTest..."
	$(SCRIPT)
	$(MAKE) build-test CMAKE_EXTRA_FLAGS="$(CMAKE_EXTRA_FLAGS)"

build-test:
	$(CMAKE) -S . -B $(BUILD_DIR) $(TESTING_FLAG)=ON $(GLOBAL_FLAG)=OFF $(CMAKE_EXTRA_FLAGS)
	$(CMAKE) --build $(BUILD_DIR)

build-all:
	@echo "==> Running initialization script..."
	$(SCRIPT)
	$(MAKE) build CMAKE_EXTRA_FLAGS="$(CMAKE_EXTRA_FLAGS)"

build:
	@echo "==> Fresh Building inLimbo with $(CMAKE_BUILD_TYPE)..."
	rm -rf $(BUILD_DIR)
	$(CMAKE) -S . -B $(BUILD_DIR) $(TESTING_FLAG)=OFF $(GLOBAL_FLAG)=OFF $(CMAKE_EXTRA_FLAGS)
	$(CMAKE) --build $(BUILD_DIR)

rebuild:
	@echo "==> Rebuilding inLimbo with $(CMAKE_BUILD_TYPE)..."
	$(CMAKE) --build $(BUILD_DIR)

asan:
	@echo "==> Building in AddressSanitizer mode..."
	$(MAKE) init
	mkdir -p $(BUILD_DIR_DBG_ASAN)
	cmake -S . -B $(BUILD_DIR_DBG_ASAN) -DCMAKE_BUILD_TYPE=Debug-ASan $(CMAKE_EXTRA_FLAGS)
	cmake --build $(BUILD_DIR_DBG_ASAN)

asan_run: asan
	@echo "==> Running AddressSanitizer build..."
	$(BUILD_DIR_DBG_ASAN)/$(EXECUTABLE)

tsan:
	@echo "==> Building in ThreadSanitizer mode..."
	$(MAKE) init
	mkdir -p $(BUILD_DIR_DBG_TSAN)
	cmake -S . -B $(BUILD_DIR_DBG_TSAN) -DCMAKE_BUILD_TYPE=Debug-TSan $(CMAKE_EXTRA_FLAGS)
	cmake --build $(BUILD_DIR_DBG_TSAN)

tsan_run: tsan
	@echo "==> Running ThreadSanitizer build..."
	$(BUILD_DIR_DBG_TSAN)/$(EXECUTABLE)

clean:
	@echo "==> Cleaning build directory..."
	rm -rf $(BUILD_DIR) $(BUILD_DIR_DBG_ASAN) $(BUILD_DIR_DBG_TSAN)

init:
	@echo "==> Running initialization script..."
	$(SCRIPT)

build-global:
	@echo "==> Building inLimbo GLOBALLY and installing..."
	$(CMAKE) -S . -B $(BUILD_DIR) $(TESTING_FLAG)=OFF $(GLOBAL_FLAG)=ON $(CMAKE_EXTRA_FLAGS)
	$(CMAKE) --build $(BUILD_DIR)
	cd $(BUILD_DIR) && sudo $(MAKE) install

build-global-uninstall:
	@echo "==> Uninstalling inLimbo from the GLOBAL build..."
	@if [ -f "$(INSTALL_MANIFEST)" ]; then \
		xargs -a $(INSTALL_MANIFEST) sudo rm -v; \
		echo "--> Uninstallation complete."; \
	else \
		echo "**Error: No install_manifest.txt found. Please ensure the project was installed.**"; \
		exit 1; \
	fi

verbose:
	@echo "==> Building with verbose output..."
	cmake -S . -B $(BUILD_DIR) $(CMAKE_EXTRA_FLAGS) --trace
	cmake --build $(BUILD_DIR) -v

run-tests:
	$(TEST_SUITE_SCRIPT)
