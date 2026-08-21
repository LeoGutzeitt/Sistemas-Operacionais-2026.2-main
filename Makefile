CC := gcc
TARGET := processflow
SRC_DIR := lgbm

SRCS := \
    $(SRC_DIR)/main.c \
    $(SRC_DIR)/intertivo.c \
    $(SRC_DIR)/task.c \
    $(SRC_DIR)/catalogo.c \
    $(SRC_DIR)/executor.c \
    $(SRC_DIR)/comando.c

CFLAGS := -Wall -Wextra -g -O0 -std=c11 \
    -D_POSIX_C_SOURCE=200809L \
    -I$(SRC_DIR)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
    $(CC) $(CFLAGS) $(SRCS) -o $@

run: $(TARGET)
	./$(TARGET) $(WORKFLOW)

clean:
	rm -f $(TARGET)