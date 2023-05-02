CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c89 -pedantic-errors
LDFLAGS = 
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin
EXEC = $(BIN_DIR)/assignment-v$(shell git rev-list --count HEAD)-$(shell git rev-parse --short HEAD)

ifeq ($(DEBUG), 1)
  CFLAGS += -g -O0 -DDEBUG
endif

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJ)
	@echo "\nLinking $@..."
	@mkdir -p $(BIN_DIR)
	@$(CC) $(LDFLAGS) $^ -o $@
	@echo "\n\033[32m✓\033[0m Done!\n"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "\nCompiling $<..."
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "\033[32m✓\033[0m Done!"

clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "\033[32m✓\033[0m Done!"

