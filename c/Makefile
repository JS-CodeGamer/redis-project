# Project Settings
debug ?= 0
NAME := redis_mini
BUILD_DIR := build
LINTER := clang-tidy
FORMATTER := clang-format


.DEFAULT_GOAL := $(NAME)

# Compiler and Linker flags Settings:
# 	-std=gnu17: Use the GNU17 standard
# 	-D _GNU_SOURCE: Use GNU extensions
# 	-D __STDC_WANT_LIB_EXT1__: Use C11 extensions
# 	-Wall: Enable all warnings
# 	-Wextra: Enable extra warnings
# 	-pedantic: Enable pedantic warnings
# 	-lm: Link to libm
CFLAGS := -std=gnu17 -D _GNU_SOURCE -D __STDC_WANT_LIB_EXT1__ -Wall -Wextra -pedantic
LDFLAGS := -lm

ifeq ($(debug), 1)
	CFLAGS := $(CFLAGS) -g -O0
else
	CFLAGS := $(CFLAGS) -Oz
endif

# Targets

# Build executable
$(NAME):
	@./build.sh

# Run linter on source directories
lint:
	@$(LINTER) --config-file=.clang-tidy $(SRC_DIR)/* $(INCLUDE_DIR)/* -- $(CFLAGS)

# Run formatter on source directories
format:
	@$(FORMATTER) -style=file -i $(SRC_DIR)/* $(INCLUDE_DIR)/*

# Run valgrind memory checker on executable
check: $(NAME)
	@sudo valgrind -s --leak-check=full --show-leak-kinds=all ./$(BUILD_DIR)/$< --help
	@sudo valgrind -s --leak-check=full --show-leak-kinds=all ./$(BUILD_DIR)/$< --version
	@sudo valgrind -s --leak-check=full --show-leak-kinds=all ./$(BUILD_DIR)/$< -v

# Clean build and bin directories
clean:
	@rm -rf $(BUILD_DIR)

.PHONY: lint format check clean
