# To compile the program, type "make" in the terminal.
# To clean up the directory, type "make clean" in the terminal.

# Description: Makefile for the assignment.
# To use the debug mode, type "make DEBUG=1" in the terminal.
# The debug mode will enable the -g flag and disable the -Werror flag.
# To run valgrind, type "make valgrind" in the terminal.
# To run helgrind, type "make helgrind" in the terminal.
# To run the program, type "./bin/assignment" in the terminal.
# The layout of the program is as follows:
# 1. The main function is in src/main.c.
# 2. The functions for the threads are in src/threads.c.

CC = gcc
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin
CFLAGS = -pthread -std=c89 -Wall -Wextra -Werror -pedantic-errors -g -I./src

EXEC = $(BIN_DIR)/assignment

# Debug mode
ifeq ($(DEBUG), 1)
  $(info Debug mode enabled)
  CFLAGS += -Wno-unused-parameter -g -O0 -DDEBUG
endif

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(addprefix $(OBJ_DIR)/,$(notdir $(SRC:.c=.o)))

.PHONY: all

all: $(OBJECTS) $(EXEC)

clean: $(EXEC)
	@echo "Cleaning up..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "\033[32m✓\033[0m Done!"

$(EXEC): $(OBJECTS)
	@echo "\nLinking $@..."
	@rm -f debug.log
	@mkdir -p $(BIN_DIR)
	@$(CC) $(LDFLAGS) $^ -o $@
	@echo "\n\033[32m✓\033[0m Done!\n"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "\nCompiling $<..."
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(notdir $@)
	@echo "\033[32m✓\033[0m Done!"


valgrind: $(EXEC)
	@echo "\nRunning valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes --track-origins=yes --quiet ./$(EXEC) 100 1 1 1 1
	@echo "\033[32m✓\033[0m Done!"

helgrind: $(EXEC)
	@echo "\nRunning helgrind..."
	@rm -f helgrind.log
	valgrind --tool=helgrind -s --quiet --track-lockorders=yes  ./$(EXEC) 100 1 1 1 1
	@echo "\033[32m✓\033[0m Done!"