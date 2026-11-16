CC ?= gcc
CFLAGS ?= -O2
BANKER_CFLAGS = -std=c11 -Wall -Wextra -Wpedantic $(CFLAGS)

ifeq ($(OS),Windows_NT)
EXE = .exe
else
EXE =
endif

BANKER = banker$(EXE)
SYNC_CAR = sync_car$(EXE)
SYNC_CAR_SOURCES = unidade2/lgbm/sync_car.c unidade2/lgbm/sync_car-runner.c

.PHONY: all clean sync_car

all: $(BANKER)

$(BANKER): banker.c
	$(CC) $(BANKER_CFLAGS) $(LDFLAGS) banker.c -o $@

sync_car: $(SYNC_CAR)

$(SYNC_CAR): $(SYNC_CAR_SOURCES) unidade2/lgbm/sync_car.h
	$(CC) -D_DEFAULT_SOURCE -Wall -Wextra -std=c99 -O2 -pthread $(SYNC_CAR_SOURCES) $(LDFLAGS) -pthread -o $@

clean:
	$(RM) banker banker.exe banker.o
