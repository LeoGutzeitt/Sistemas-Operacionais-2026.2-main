CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -pthread -O2
LDFLAGS = -pthread

TARGET = vm
SRCS = vm.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c vm.h
	$(CC) $(CFLAGS) -c $< -o $@

# Alvos auxiliares para demonstracao isolada das etapas da rubrica
vm-256-notlb: vm.c vm.h
	$(CC) $(CFLAGS) -DVM_NUM_FRAMES=256 -DVM_USE_TLB=0 vm.c -o $(TARGET) $(LDFLAGS)

vm-128-notlb: vm.c vm.h
	$(CC) $(CFLAGS) -DVM_NUM_FRAMES=128 -DVM_USE_TLB=0 vm.c -o $(TARGET) $(LDFLAGS)

vm-128-nothreads: vm.c vm.h
	$(CC) $(CFLAGS) -DVM_NUM_FRAMES=128 -DVM_USE_TLB=1 -DVM_USE_THREADS=0 vm.c -o $(TARGET) $(LDFLAGS)

test: $(TARGET)
	@echo "Executando teste padrao (lru fifo)..."
	@if [ -f address.txt ] && [ -f BACKING_STORE.bin ]; then \
		./$(TARGET) address.txt lru fifo; \
	elif [ -f addresses.txt ] && [ -f BACKING_STORE.bin ]; then \
		./$(TARGET) addresses.txt lru fifo; \
	else \
		echo "Aviso: address.txt/addresses.txt ou BACKING_STORE.bin nao encontrados."; \
	fi

clean:
	rm -f $(OBJS) $(TARGET) correct.txt *.o *.exe

.PHONY: all clean test vm-256-notlb vm-128-notlb vm-128-nothreads
