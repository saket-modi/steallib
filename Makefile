CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LIB_DIR = ./lib
SRC_DIR = ./src
OBJ_DIR = ./obj
LIB_NAME = $(LIB_DIR)/libsteal.a

OBJS = $(OBJ_DIR)/protocols.o $(OBJ_DIR)/util.o

all: $(OBJ_DIR) $(LIB_DIR) $(LIB_NAME)

# create the library from object files
$(LIB_NAME): $(OBJS)
	ar rcs $@ $^

# Pattern rule: how to turn any .c file in src/ into a .o file in obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories if they don't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# Cleanup target: run 'make clean' to delete built files
clean:
	rm -rf $(OBJ_DIR) $(LIB_NAME)