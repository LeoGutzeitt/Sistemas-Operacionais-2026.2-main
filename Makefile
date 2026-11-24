CC ?= gcc
CFLAGS ?= -O2
VM_CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -pthread $(CFLAGS)
BANKER_CFLAGS = -std=c11 -Wall -Wextra -Wpedantic $(CFLAGS)

ifeq ($(OS),Windows_NT)
EXE = .exe
else
EXE =
endif

VM = vm$(EXE)
VM_DIR = implementacao7/lgbm
BANKER = banker$(EXE)
SYNC_CAR = sync_car$(EXE)
SYNC_CAR_SOURCES = unidade2/implementacao4-metro/sync_car.c unidade2/implementacao4-metro/sync_car-runner.c

.PHONY: all clean vm banker sync_car

all: $(VM)

$(VM): $(VM_DIR)/vm.c $(VM_DIR)/vm.h
	$(CC) $(VM_CFLAGS) $(VM_DIR)/vm.c $(LDFLAGS) -pthread -o $@

banker: $(BANKER)

$(BANKER): banker.c
	$(CC) $(BANKER_CFLAGS) $(LDFLAGS) banker.c -o $@

sync_car: $(SYNC_CAR)

$(SYNC_CAR): $(SYNC_CAR_SOURCES) unidade2/implementacao4-metro/sync_car.h
	$(CC) -D_DEFAULT_SOURCE -Wall -Wextra -std=c99 -O2 -pthread $(SYNC_CAR_SOURCES) $(LDFLAGS) -pthread -o $@

clean:
	$(RM) vm vm.exe vm.o correct.txt
