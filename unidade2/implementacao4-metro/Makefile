CC ?= gcc
ifeq ($(origin CC),default)
    CC = gcc
endif
CFLAGS = -Wall -Wextra -pthread -std=c99 -D_DEFAULT_SOURCE -O2
LDFLAGS = -pthread
TARGET = sync_car
SOURCES = sync_car.c sync_car-runner.c
OBJECTS = sync_car.o sync_car-runner.o

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

sync_car-runner.o: sync_car-runner.c sync_car.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET)$(EXE)

clean:
	-$(RM) $(TARGET) $(TARGET).exe $(OBJECTS) *.out

tar: clean
	cd .. && tar -cvf lgbm.tar lgbm/
