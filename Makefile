CC := gcc
TARGET := lgbm_exec
SRC_DIR := lgbm

SRCS := \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/intertivo.c \
	$(SRC_DIR)/tarefas.c \
	$(SRC_DIR)/tarefa.c \
	$(SRC_DIR)/util.c \
	$(SRC_DIR)/utils.c \
	$(SRC_DIR)/redirection.c

CFLAGS := -Wall -Wextra -g -O0 -std=c11 -D_POSIX_C_SOURCE=200809L -I$(SRC_DIR)
LDFLAGS := -pthread

.PHONY: all clean

.PHONY: run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)