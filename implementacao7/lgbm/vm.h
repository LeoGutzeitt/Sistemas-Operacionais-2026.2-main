#ifndef VM_H
#define VM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#define PAGE_SIZE 256
#define TOTAL_PAGES 256
#define MAX_FRAMES 256
#define TLB_SIZE 16

#ifndef VM_NUM_FRAMES
#define VM_NUM_FRAMES 128
#endif

#ifndef VM_USE_TLB
#define VM_USE_TLB 1
#endif

#ifndef VM_USE_THREADS
#define VM_USE_THREADS 1
#endif

#define BACKING_STORE_FILE "BACKING_STORE.bin"
#define OUTPUT_FILE "correct.txt"

typedef enum {
    ALGO_FIFO = 0,
    ALGO_LRU = 1
} ReplacementAlgo;

/* Entrada da TLB */
typedef struct {
    int page_number;
    int frame_number;
    bool valid;
    uint64_t load_time;     /* Ordem de insercao (FIFO) */
    uint64_t last_access;   /* Ultimo acesso logico (LRU) */
} TLBEntry;

/* Argumento passado para cada thread de busca paralela na TLB */
typedef struct {
    int thread_id;
    int target_page;
    const TLBEntry *entry;
    bool hit;
    int frame_number;
} TLBSearchWorkerArg;

/* Entrada da Tabela de Paginas */
typedef struct {
    int frame_number;
    bool valid;
} PageTableEntry;

/* Metadados de um frame na memoria fisica */
typedef struct {
    int page_number;        /* Pagina alocada no frame (-1 se livre) */
    uint64_t load_time;     /* Timestamp de carregamento (FIFO) */
    uint64_t last_access;   /* Timestamp de ultimo acesso (LRU) */
} FrameMeta;

/* Estado completo do Gerenciador de Memoria Virtual */
typedef struct {
    int num_frames;
    ReplacementAlgo page_algo;
    ReplacementAlgo tlb_algo;
    bool use_tlb;
    bool use_threads;

    /* Estatisticas */
    uint64_t total_addresses;
    uint64_t page_faults;
    uint64_t tlb_hits;

    /* Relogio logico monotonico para FIFO e LRU */
    uint64_t global_clock;

    /* Estruturas de Memoria */
    TLBEntry tlb[TLB_SIZE];
    PageTableEntry page_table[TOTAL_PAGES];
    FrameMeta frame_meta[MAX_FRAMES];
    signed char physical_memory[MAX_FRAMES][PAGE_SIZE];

    int allocated_frames_count;
    FILE *backing_store;
} VirtualMemoryManager;

/* Prototipos */
int vm_init(VirtualMemoryManager *vm, int num_frames, ReplacementAlgo page_algo,
            ReplacementAlgo tlb_algo, bool use_tlb, bool use_threads,
            const char *backing_path);
void vm_destroy(VirtualMemoryManager *vm);
int vm_translate(VirtualMemoryManager *vm, int logical_address,
                 int *tlb_index, int *physical_address, signed char *value);
int vm_process_file(VirtualMemoryManager *vm, const char *address_file_path,
                    const char *output_file_path);

#endif /* VM_H */
