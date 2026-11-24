CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -pthread -O2
LDFLAGS = -pthread

TARGET = vm
SRCS = main.c tlb.c paginacao.c
OBJS = $(SRCS:.c=.o)
HEADER = gerenciador_vm.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

# Alvos auxiliares para demonstracao isolada das etapas da rubrica
vm-256-notlb: $(SRCS) $(HEADER)
	$(CC) $(CFLAGS) -DVM_NUM_FRAMES=256 -DVM_USE_TLB=0 $(SRCS) -o $(TARGET) $(LDFLAGS)

vm-128-notlb: $(SRCS) $(HEADER)
	$(CC) $(CFLAGS) -DVM_NUM_FRAMES=128 -DVM_USE_TLB=0 $(SRCS) -o $(TARGET) $(LDFLAGS)

vm-128-nothreads: $(SRCS) $(HEADER)
	$(CC) $(CFLAGS) -DVM_NUM_FRAMES=128 -DVM_USE_TLB=1 -DVM_USE_THREADS=0 $(SRCS) -o $(TARGET) $(LDFLAGS)

test: $(TARGET)
	@echo "Executando teste padrao (lru fifo)..."
	@if [ -f address.txt ] && [ -f BACKING_STORE.bin ]; then \
		./$(TARGET) address.txt lru fifo; \
		if [ -f correct_ref.txt ]; then \
			diff -u correct.txt correct_ref.txt && echo "SUCESSO: correct.txt 100% identico ao gabarito!"; \
		fi; \
	elif [ -f addresses.txt ] && [ -f BACKING_STORE.bin ]; then \
		./$(TARGET) addresses.txt lru fifo; \
	else \
		echo "Aviso: address.txt/addresses.txt ou BACKING_STORE.bin nao encontrados."; \
	fi

clean:
	rm -f $(OBJS) $(TARGET) correct.txt *.o *.exe

.PHONY: all clean test vm-256-notlb vm-128-notlb vm-128-nothreads
