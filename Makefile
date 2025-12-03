CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lm

# directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
EXAMPLES_DIR = examples

# source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# library name
LIB_NAME = libcwiz.a
LIB = $(BUILD_DIR)/$(LIB_NAME)

# examples
EXAMPLE_SOURCES = $(wildcard $(EXAMPLES_DIR)/*.c)
EXAMPLE_BINS = $(patsubst $(EXAMPLES_DIR)/%.c,$(BUILD_DIR)/%,$(EXAMPLE_SOURCES))

.PHONY: all clean lib examples install build-clean

all: lib examples
	@rm -f $(BUILD_DIR)/*.o

# build everything and clean intermediate files
build-clean: all

# create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# build library
lib: $(BUILD_DIR) $(LIB)

$(LIB): $(OBJECTS)
	@ar rcs $@ $^

# compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# build examples
examples: lib $(EXAMPLE_BINS)

$(BUILD_DIR)/%: $(EXAMPLES_DIR)/%.c $(LIB)
	@$(CC) $(CFLAGS) $< -L$(BUILD_DIR) -lcwiz $(LDFLAGS) -o $@

# install library (optional)
install: lib
	@sudo cp $(LIB) /usr/local/lib/
	@sudo cp $(INC_DIR)/*.h /usr/local/include/

# clean build artifacts
clean:
	@rm -rf $(BUILD_DIR)

# help target
help:
	@echo "cwiz Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build library and examples (default)"
	@echo "  lib       - Build the cwiz library"
	@echo "  examples  - Build example programs"
	@echo "  install   - Install library system-wide (requires sudo)"
	@echo "  clean     - Remove build artifacts"
	@echo "  help      - Show this help message"
