CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -fopenmp
CPPFLAGS = -I.
LDLIBS = -lm
TARGET = mandelbrot
OBJECTS = threads.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDLIBS) -o $@

threads.o: threads.c threads.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c threads.c -o threads.o

clean:
	rm -f $(TARGET) $(OBJECTS) *.pgm times.txt evidencias.log
