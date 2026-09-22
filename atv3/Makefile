CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
SRCDIR = src
INCDIR = include
BINDIR = .

SOURCES = $(SRCDIR)/main.c $(SRCDIR)/parser.c $(SRCDIR)/scheduler.c $(SRCDIR)/task.c
HEADERS = $(INCDIR)/parser.h $(INCDIR)/scheduler.h $(INCDIR)/task.h
OBJECTS = $(SOURCES:.c=.o)
TARGET = $(BINDIR)/scheduler

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f *_*.out

.PHONY: all clean
