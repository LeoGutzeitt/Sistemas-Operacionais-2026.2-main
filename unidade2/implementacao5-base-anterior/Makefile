CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
ifeq ($(OS),Windows_NT)
TARGET = banker.exe
else
TARGET = banker
endif
OBJECTS = banker.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

banker.o: banker.c
	$(CC) $(CFLAGS) -c banker.c -o $@

clean:
	$(RM) $(TARGET) $(OBJECTS)
