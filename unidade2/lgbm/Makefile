CC ?= gcc
CFLAGS = -Wall -Wextra -pthread -std=c99 -O2
LDFLAGS = -pthread
TARGET = sync_car
SOURCES = sync_car.c main.c
OBJECTS = sync_car.o main.o

ifeq ($(OS),Windows_NT)
    RM = rm -f
    LDFLAGS += -lpthread
    EXE = .exe
else
    RM = rm -f
    EXE =
endif

.PHONY: all clean test tar

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

sync_car.o: sync_car.c sync_car.h
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c sync_car.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET)$(EXE)

clean:
	-$(RM) $(TARGET) $(TARGET).exe $(OBJECTS) *.out

tar: clean
	cd .. && tar -cvf lgbm.tar lgbm/
