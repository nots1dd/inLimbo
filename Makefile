# Makefile for the inLimbo Project

# Variables
BUILD_DIR := build
BUILD_DBG_ASAN_DIR := build-dbg-asan
BUILD_DBG_TSAN_DIR :- build-dbg-tsan
EXECUTABLE := inLimbo
CMAKE := cmake
CMAKE_BUILD_TYPE := Release
SCRIPT := ./init.sh
VERBOSE_FLAG := VERBOSE=1

# Targets
.PHONY: all build clean rebuild build-all init asan tsan global_build

all: build-all

build-all:
		@echo "==> Running initialization script..."
		$(SCRIPT)
		$(MAKE) build 

build:
	@echo "==> Fresh Building inLimbo with $(CMAKE_BUILD_TYPE)..."
	$(CMAKE) -S . -B build $(BUILD_DIR)
	$(CMAKE) --build $(BUILD_DIR)

rebuild:
	@echo "==> Rebuilding inLimbo with $(CMAKE_BUILD_TYPE)..."
	$(CMAKE) --build $(BUILD_DIR)

asan:
	@echo "==> Building in AddressSanitizer mode..."
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=Debug-ASan .. && $(NINJA)

asan_run: asan
	@echo "==> Running AddressSanitizer build..."
	$(BUILD_DIR)/inLimbo-DBG-Asan

tsan:
	@echo "==> Building in ThreadSanitizer mode..."
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=Debug-TSan .. && $(NINJA)

tsan_run: tsan
	@echo "==> Running ThreadSanitizer build..."
	$(BUILD_DIR)/inLimbo-DBG-TSan

clean:
	@echo "==> Cleaning build directory..."
	rm -rf $(BUILD_DIR)

init:
	@echo "==> Running initialization script..."
	$(SCRIPT)

global_build:
	@echo "==> Building globally and installing..."
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DBUILD_GLOBAL=ON .. && $(NINJA)
	cd $(BUILD_DIR) && sudo ninja install

verbose:
	@echo "==> Building with verbose output..."
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) .. && $(NINJA) $(VERBOSE_FLAG)
