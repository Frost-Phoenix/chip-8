SRC_DIR := ./src
INCLUDE_DIR := ./include
BIN_DIR := ./bin
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SRC_FILES))

CSTD = c11
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CSTD = gnu11
endif

CC := gcc
CFLAGS := -std=$(CSTD) -Wall -Wextra -Werror
LIBS   = -lraylib
DEBUG_FLAGS := -fsanitize=address,undefined
RELEASE_FLAGS := -O2

TARGET := chip-8

.PHONY: all debug release run install uninstall clean

all: $(BIN_DIR)/$(TARGET) 

# Build rule
$(BIN_DIR)/$(TARGET): $(OBJ_FILES)
	$(CC) -I $(INCLUDE_DIR) $(CFLAGS) $^ -o $@ $(LIBS)

# Compile source files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) -I $(INCLUDE_DIR) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(BIN_DIR)/$(TARGET)

release: CFLAGS += $(RELEASE_FLAGS)
release: clean $(BIN_DIR)/$(TARGET)

run: $(BIN_DIR)/$(TARGET)
	$(BIN_DIR)/$(TARGET)

clean:
	rm -f $(BIN_DIR)/*.o
	rm -f $(BIN_DIR)/$(TARGET)
	rm -rf bin
