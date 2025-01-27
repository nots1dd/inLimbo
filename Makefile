# Makefile for the inLimbo Project

# Variables
BUILD_DIR := build
BUILD_DIR_DBG_ASAN := build-dbg-asan
BUILD_DIR_DBG_TSAN := build-dbg-tsan
BUILD_DBG_ASAN_DIR := build-dbg-asan
BUILD_DBG_TSAN_DIR := build-dbg-tsan
BUILD_GLOBAL := GLOBAL_BUILD
EXECUTABLE := inLimbo
CMAKE := cmake
CMAKE_BUILD_TYPE := Release
SCRIPT := ./init.sh
TEST_SUITE_SCRIPT := ./run_tests.sh
VERBOSE_FLAG := VERBOSE=1
TESTING_FLAG := INLIMBO_TESTING
INSTALL_MANIFEST := $(BUILD_DIR)/install_manifest.txt

# Targets
.PHONY: all build clean rebuild build-all init asan tsan build-global build-test build-test-all build-global-uninstall

all: build-all

build-test-all:
	@echo "==> Building inLimbo with script and tests using GTest..."
	$(SCRIPT)
	$(MAKE) build-test

build-test:
	$(CMAKE) -S . -B $(BUILD_DIR) -D $(TESTING_FLAG)=ON -D $(BUILD_GLOBAL)=OFF
	$(CMAKE) --build $(BUILD_DIR)

build-all:
		@echo "==> Running initialization script..."
		$(SCRIPT)
		$(MAKE) build 

build:
	@echo "==> Fresh Building inLimbo with $(CMAKE_BUILD_TYPE)..."
	$(CMAKE) -S . -B build $(BUILD_DIR) -D $(TESTING_FLAG)=OFF -D $(BUILD_GLOBAL)=OFF
	$(CMAKE) --build $(BUILD_DIR)

rebuild:
	@echo "==> Rebuilding inLimbo with $(CMAKE_BUILD_TYPE)..."
	$(CMAKE) --build $(BUILD_DIR)

asan:
	@echo "==> Building in AddressSanitizer mode..."
	$(MAKE) init
	mkdir -p $(BUILD_DIR_DBG_ASAN)
	cmake -S . -B $(BUILD_DIR_DBG_ASAN) -DCMAKE_BUILD_TYPE=Debug-ASan
	cmake --build $(BUILD_DIR_DBG_ASAN)

asan_run: asan
	@echo "==> Running AddressSanitizer build..."
	$(BUILD_DIR)/inLimbo-DBG-Asan

tsan:
	@echo "==> Building in ThreadSanitizer mode..."
	$(MAKE) init
	mkdir -p $(BUILD_DIR_DBG_TSAN)
	cmake -S . -B $(BUILD_DIR_DBG_TSAN) -DCMAKE_BUILD_TYPE=Debug-TSan
	cmake --build $(BUILD_DIR_DBG_TSAN)

tsan_run: tsan
	@echo "==> Running ThreadSanitizer build..."
	$(BUILD_DIR)/inLimbo-DBG-TSan

clean:
	@echo "==> Cleaning build directory..."
	rm -rf $(BUILD_DIR)

init:
	@echo "==> Running initialization script..."
	$(SCRIPT)

build-global:
	@echo "==> Building inLimbo GLOBALLY and installing..."
	$(CMAKE) -S . -B build $(BUILD_DIR) -D $(TESTING_FLAG)=OFF -D $(BUILD_GLOBAL)=ON
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
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) .. && $(NINJA) $(VERBOSE_FLAG)

run-tests:
	$(TEST_SUITE_SCRIPT)
