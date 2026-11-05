# O autocheck executa make e procura ./sync_car na raiz.
CC = gcc
CPPFLAGS += -D_DEFAULT_SOURCE
CFLAGS ?= -Wall -Wextra -std=c99 -O2
LDLIBS += -pthread

ifeq ($(OS),Windows_NT)
EXE = .exe
endif

TARGET = sync_car$(EXE)
SOURCES = unidade2/lgbm/sync_car.c unidade2/lgbm/sync_car-runner.c

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SOURCES) unidade2/lgbm/sync_car.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread $(SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

test: $(TARGET)
	./$(TARGET)

clean:
	$(RM) sync_car sync_car.exe
