#include "vm.h"
#include <ctype.h>
#include <errno.h>

/* Comparacao case-insensitive portavel em C11 puro (sem depender de strings.h) */
static int equals_ignore_case(const char *a, const char *b) {
    if (a == NULL || b == NULL) return 0;
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

/* Worker de cada thread da busca paralela na TLB */
static void *tlb_search_worker(void *arg) {
    TLBSearchWorkerArg *w = (TLBSearchWorkerArg *)arg;
    if (w->entry != NULL && w->entry->valid && w->entry->page_number == w->target_page) {
        w->hit = true;
        w->frame_number = w->entry->frame_number;
    } else {
        w->hit = false;
        w->frame_number = -1;
    }
    return NULL;
}

/* Inicializa o gerenciador de memoria virtual e abre o backing store */
int vm_init(VirtualMemoryManager *vm, int num_frames, ReplacementAlgo page_algo,
            ReplacementAlgo tlb_algo, bool use_tlb, bool use_threads,
            const char *backing_path) {
    if (vm == NULL) return -1;
    if (num_frames <= 0 || num_frames > MAX_FRAMES) return -1;

    memset(vm, 0, sizeof(VirtualMemoryManager));
    vm->num_frames = num_frames;
    vm->page_algo = page_algo;
    vm->tlb_algo = tlb_algo;
    vm->use_tlb = use_tlb;
    vm->use_threads = use_threads;

    for (int i = 0; i < TLB_SIZE; i++) {
        vm->tlb[i].page_number = -1;
        vm->tlb[i].frame_number = -1;
        vm->tlb[i].valid = false;
        vm->tlb[i].load_time = 0;
        vm->tlb[i].last_access = 0;
    }

    for (int i = 0; i < TOTAL_PAGES; i++) {
        vm->page_table[i].frame_number = -1;
        vm->page_table[i].valid = false;
    }

    for (int i = 0; i < MAX_FRAMES; i++) {
        vm->frame_meta[i].page_number = -1;
        vm->frame_meta[i].load_time = 0;
        vm->frame_meta[i].last_access = 0;
    }

    const char *path = (backing_path != NULL) ? backing_path : BACKING_STORE_FILE;
    vm->backing_store = fopen(path, "rb");
    if (vm->backing_store == NULL) {
        char alt_path[512];
        snprintf(alt_path, sizeof(alt_path), "../%s", path);
        vm->backing_store = fopen(alt_path, "rb");
        if (vm->backing_store == NULL) {
            fprintf(stderr, "Erro: nao foi possivel abrir o backing store '%s': %s\n",
                    path, strerror(errno));
            return -1;
        }
    }

    /* Verifica se o BACKING_STORE.bin possui pelo menos 65536 bytes */
    if (fseek(vm->backing_store, 0, SEEK_END) != 0) {
        fprintf(stderr, "Erro: falha ao inspecionar tamanho de '%s'.\n", path);
        fclose(vm->backing_store);
        vm->backing_store = NULL;
        return -1;
    }
    long file_size = ftell(vm->backing_store);
    if (file_size < (long)(TOTAL_PAGES * PAGE_SIZE)) {
        fprintf(stderr, "Erro: backing store '%s' incompleto (%ld bytes; minimo esperado: %d bytes).\n",
                path, file_size, TOTAL_PAGES * PAGE_SIZE);
        fclose(vm->backing_store);
        vm->backing_store = NULL;
        return -1;
    }
    rewind(vm->backing_store);

    return 0;
}

/* Libera recursos do gerenciador */
void vm_destroy(VirtualMemoryManager *vm) {
    if (vm == NULL) return;
    if (vm->backing_store != NULL) {
        fclose(vm->backing_store);
        vm->backing_store = NULL;
    }
}

/* Busca na TLB: 1 thread por entrada (16 threads) ou modo sequencial */
static int tlb_lookup(VirtualMemoryManager *vm, int page_number, int *frame_out, int *tlb_index_out) {
    if (!vm->use_tlb) {
        return 0;
    }

    int hit_index = -1;

    if (vm->use_threads) {
        pthread_t threads[TLB_SIZE];
        bool thread_created[TLB_SIZE];
        TLBSearchWorkerArg args[TLB_SIZE];

        for (int i = 0; i < TLB_SIZE; i++) {
            args[i].thread_id = i;
            args[i].target_page = page_number;
            args[i].entry = &vm->tlb[i];
            args[i].hit = false;
            args[i].frame_number = -1;

            if (pthread_create(&threads[i], NULL, tlb_search_worker, &args[i]) == 0) {
                thread_created[i] = true;
            } else {
                thread_created[i] = false;
                tlb_search_worker(&args[i]);
            }
        }

        for (int i = 0; i < TLB_SIZE; i++) {
            if (thread_created[i]) {
                pthread_join(threads[i], NULL);
            }
            if (args[i].hit && hit_index == -1) {
                hit_index = i;
                *frame_out = args[i].frame_number;
            }
        }
    } else {
        for (int i = 0; i < TLB_SIZE; i++) {
            if (vm->tlb[i].valid && vm->tlb[i].page_number == page_number) {
                hit_index = i;
                *frame_out = vm->tlb[i].frame_number;
                break;
            }
        }
    }

    if (hit_index != -1) {
        if (tlb_index_out != NULL) {
            *tlb_index_out = hit_index;
        }
        if (vm->tlb_algo == ALGO_LRU) {
            vm->tlb[hit_index].last_access = ++vm->global_clock;
        }
        if (vm->page_algo == ALGO_LRU) {
            vm->frame_meta[*frame_out].last_access = ++vm->global_clock;
        }
        return 1;
    }

    return 0;
}

/* Insere ou atualiza traducao na TLB aplicando FIFO ou LRU e retorna o indice */
static int tlb_insert(VirtualMemoryManager *vm, int page_number, int frame_number) {
    if (!vm->use_tlb) {
        return -1;
    }

    /* Se ja estiver presente, apenas atualiza */
    for (int i = 0; i < TLB_SIZE; i++) {
        if (vm->tlb[i].valid && vm->tlb[i].page_number == page_number) {
            vm->tlb[i].frame_number = frame_number;
            vm->tlb[i].last_access = ++vm->global_clock;
            return i;
        }
    }

    /* Procura slot livre */
    int target_idx = -1;
    for (int i = 0; i < TLB_SIZE; i++) {
        if (!vm->tlb[i].valid) {
            target_idx = i;
            break;
        }
    }

    /* TLB cheia: escolhe vitima por FIFO ou LRU */
    if (target_idx == -1) {
        uint64_t min_time = UINT64_MAX;
        for (int i = 0; i < TLB_SIZE; i++) {
            uint64_t metric = (vm->tlb_algo == ALGO_FIFO)
                                  ? vm->tlb[i].load_time
                                  : vm->tlb[i].last_access;
            if (metric < min_time) {
                min_time = metric;
                target_idx = i;
            }
        }
    }

    vm->tlb[target_idx].page_number = page_number;
    vm->tlb[target_idx].frame_number = frame_number;
    vm->tlb[target_idx].valid = true;
    vm->tlb[target_idx].load_time = ++vm->global_clock;
    vm->tlb[target_idx].last_access = vm->global_clock;
    return target_idx;
}

/* Invalida entrada na TLB quando uma pagina e removida da memoria fisica */
static void tlb_invalidate_page(VirtualMemoryManager *vm, int page_number) {
    if (!vm->use_tlb) {
        return;
    }
    for (int i = 0; i < TLB_SIZE; i++) {
        if (vm->tlb[i].valid && vm->tlb[i].page_number == page_number) {
            vm->tlb[i].valid = false;
            vm->tlb[i].page_number = -1;
            vm->tlb[i].frame_number = -1;
            break;
        }
    }
}

/* Seleciona frame vitima na memoria fisica quando cheia (FIFO ou LRU) */
static int select_victim_frame(VirtualMemoryManager *vm) {
    int victim_frame = 0;
    uint64_t min_metric = UINT64_MAX;

    for (int i = 0; i < vm->num_frames; i++) {
        uint64_t metric = (vm->page_algo == ALGO_FIFO)
                              ? vm->frame_meta[i].load_time
                              : vm->frame_meta[i].last_access;
        if (metric < min_metric) {
            min_metric = metric;
            victim_frame = i;
        }
    }

    return victim_frame;
}

/* Trata falta de pagina (Page Fault) */
static int handle_page_fault(VirtualMemoryManager *vm, int page_number) {
    vm->page_faults++;
    int frame = -1;

    /* Preenche frames sequencialmente de 0 a (num_frames - 1) */
    if (vm->allocated_frames_count < vm->num_frames) {
        frame = vm->allocated_frames_count++;
    } else {
        frame = select_victim_frame(vm);
        int victim_page = vm->frame_meta[frame].page_number;

        if (victim_page >= 0 && victim_page < TOTAL_PAGES) {
            vm->page_table[victim_page].valid = false;
            vm->page_table[victim_page].frame_number = -1;
            tlb_invalidate_page(vm, victim_page);
        }
    }

    long offset = (long)page_number * PAGE_SIZE;
    if (fseek(vm->backing_store, offset, SEEK_SET) != 0) {
        fprintf(stderr, "Erro: falha de fseek no backing store para pagina %d.\n", page_number);
        return -1;
    }

    size_t read_bytes = fread(vm->physical_memory[frame], sizeof(signed char),
                              PAGE_SIZE, vm->backing_store);
    if (read_bytes != PAGE_SIZE) {
        fprintf(stderr, "Erro: leitura incompleta do backing store para pagina %d (%zu bytes).\n",
                page_number, read_bytes);
        return -1;
    }

    vm->frame_meta[frame].page_number = page_number;
    vm->frame_meta[frame].load_time = ++vm->global_clock;
    vm->frame_meta[frame].last_access = vm->global_clock;

    vm->page_table[page_number].frame_number = frame;
    vm->page_table[page_number].valid = true;

    return frame;
}

/* Traduz endereco logico de 16 bits para endereco fisico, indice da TLB e valor */
int vm_translate(VirtualMemoryManager *vm, int logical_address,
                 int *tlb_index, int *physical_address, signed char *value) {
    if (vm == NULL || tlb_index == NULL || physical_address == NULL || value == NULL) return -1;
    if (logical_address < 0 || logical_address > 65535) return -1;

    int page_number = (logical_address >> 8) & 0xFF;
    int offset = logical_address & 0xFF;
    int frame = -1;
    int tlb_idx = -1;

    vm->total_addresses++;

    if (tlb_lookup(vm, page_number, &frame, &tlb_idx)) {
        vm->tlb_hits++;
    } else {
        if (vm->page_table[page_number].valid) {
            frame = vm->page_table[page_number].frame_number;
            if (vm->page_algo == ALGO_LRU) {
                vm->frame_meta[frame].last_access = ++vm->global_clock;
            }
        } else {
            frame = handle_page_fault(vm, page_number);
            if (frame < 0) {
                return -1;
            }
        }
        tlb_idx = tlb_insert(vm, page_number, frame);
    }

    *tlb_index = tlb_idx;
    *physical_address = (frame << 8) | offset;
    *value = vm->physical_memory[frame][offset];

    return 0;
}

/* Carrega e valida integralmente o arquivo de enderecos antes de gerar saida */
static int load_and_validate_addresses(const char *address_file_path,
                                       int **addresses_out, size_t *count_out) {
    FILE *addr_file = fopen(address_file_path, "r");
    if (addr_file == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo de enderecos '%s': %s\n",
                address_file_path, strerror(errno));
        return -1;
    }

    size_t capacity = 1024;
    size_t count = 0;
    int *addresses = (int *)malloc(capacity * sizeof(int));
    if (addresses == NULL) {
        fprintf(stderr, "Erro: falha de alocacao de memoria.\n");
        fclose(addr_file);
        return -1;
    }

    char line[256];
    size_t line_number = 0;

    while (fgets(line, sizeof(line), addr_file) != NULL) {
        line_number++;

        /* Detecta linha excessivamente longa sem quebra de linha */
        size_t len = strlen(line);
        if (len == sizeof(line) - 1 && line[len - 1] != '\n') {
            fprintf(stderr, "Erro de formatacao na linha %zu de '%s': linha muito longa.\n",
                    line_number, address_file_path);
            free(addresses);
            fclose(addr_file);
            return -1;
        }

        char *start = line;
        while (*start != '\0' && isspace((unsigned char)*start)) start++;
        if (*start == '\0') continue; /* Ignora linhas vazias */

        char *end = start + strlen(start) - 1;
        while (end > start && isspace((unsigned char)*end)) {
            *end = '\0';
            end--;
        }

        /* Garante que contem apenas digitos decimais (rejeita sinais +/- e letras) */
        for (const char *p = start; *p != '\0'; p++) {
            if (!isdigit((unsigned char)*p)) {
                fprintf(stderr, "Erro de formatacao na linha %zu de '%s': '%s' nao e um endereco inteiro valido.\n",
                        line_number, address_file_path, start);
                free(addresses);
                fclose(addr_file);
                return -1;
            }
        }

        char *endptr = NULL;
        errno = 0;
        long val = strtol(start, &endptr, 10);
        if (errno != 0 || *endptr != '\0' || val < 0 || val > 65535) {
            fprintf(stderr, "Erro de formatacao na linha %zu de '%s': '%s' fora do intervalo [0, 65535].\n",
                    line_number, address_file_path, start);
            free(addresses);
            fclose(addr_file);
            return -1;
        }

        if (count == capacity) {
            capacity *= 2;
            int *resized = (int *)realloc(addresses, capacity * sizeof(int));
            if (resized == NULL) {
                fprintf(stderr, "Erro: falha ao expandir buffer de enderecos.\n");
                free(addresses);
                fclose(addr_file);
                return -1;
            }
            addresses = resized;
        }

        addresses[count++] = (int)val;
    }

    fclose(addr_file);

    if (count == 0) {
        fprintf(stderr, "Erro: o arquivo de enderecos '%s' esta vazio ou nao contem enderecos validos.\n",
                address_file_path);
        free(addresses);
        return -1;
    }

    *addresses_out = addresses;
    *count_out = count;
    return 0;
}

/* Processa todos os enderecos e grava o resultado em correct.txt */
int vm_process_file(VirtualMemoryManager *vm, const char *address_file_path,
                    const char *output_file_path) {
    int *addresses = NULL;
    size_t count = 0;

    if (load_and_validate_addresses(address_file_path, &addresses, &count) != 0) {
        return -1;
    }

    FILE *out_file = fopen(output_file_path, "w");
    if (out_file == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo de saida '%s': %s\n",
                output_file_path, strerror(errno));
        free(addresses);
        return -1;
    }

    for (size_t i = 0; i < count; i++) {
        int tlb_index = -1;
        int physical_address = 0;
        signed char byte_val = 0;

        if (vm_translate(vm, addresses[i], &tlb_index, &physical_address, &byte_val) != 0) {
            fprintf(stderr, "Erro: falha na traducao do endereco %d (indice %zu).\n",
                    addresses[i], i + 1);
            fclose(out_file);
            free(addresses);
            return -1;
        }

        if (vm->use_tlb) {
            fprintf(out_file, "Virtual address: %d TLB: %d Physical address: %d Value: %d\n",
                    addresses[i], tlb_index, physical_address, (int)byte_val);
        } else {
            fprintf(out_file, "Virtual address: %d Physical address: %d Value: %d\n",
                    addresses[i], physical_address, (int)byte_val);
        }
    }

    double fault_rate = (double)vm->page_faults / (double)vm->total_addresses;
    double tlb_rate = (double)vm->tlb_hits / (double)vm->total_addresses;

    fprintf(out_file, "Number of Translated Addresses = %llu\n",
            (unsigned long long)vm->total_addresses);
    fprintf(out_file, "Page Faults = %llu\n",
            (unsigned long long)vm->page_faults);
    fprintf(out_file, "Page Fault Rate = %.3f\n", fault_rate);
    fprintf(out_file, "TLB Hits = %llu\n",
            (unsigned long long)vm->tlb_hits);
    fprintf(out_file, "TLB Hit Rate = %.3f\n", tlb_rate);

    fclose(out_file);
    free(addresses);
    return 0;
}

static void print_usage(const char *prog_name) {
    fprintf(stderr, "Uso: %s <arquivo_enderecos> <algoritmo_pagina> <algoritmo_tlb>\n", prog_name);
    fprintf(stderr, "  <arquivo_enderecos>: arquivo texto com enderecos logicos (0 a 65535)\n");
    fprintf(stderr, "  <algoritmo_pagina> : 'fifo' ou 'lru'\n");
    fprintf(stderr, "  <algoritmo_tlb>    : 'fifo' ou 'lru'\n");
    fprintf(stderr, "Exemplo: %s address.txt lru fifo\n", prog_name);
}

static int parse_algo(const char *str, ReplacementAlgo *out) {
    if (equals_ignore_case(str, "fifo")) {
        *out = ALGO_FIFO;
        return 0;
    }
    if (equals_ignore_case(str, "lru")) {
        *out = ALGO_LRU;
        return 0;
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Erro: numero invalido de argumentos (recebido: %d, esperado: 3).\n",
                argc - 1);
        print_usage(argv[0]);
        return 1;
    }

    const char *address_file = argv[1];
    ReplacementAlgo page_algo;
    ReplacementAlgo tlb_algo;

    if (parse_algo(argv[2], &page_algo) != 0) {
        fprintf(stderr, "Erro: algoritmo de substituicao de pagina invalido '%s'. Use 'fifo' ou 'lru'.\n",
                argv[2]);
        print_usage(argv[0]);
        return 1;
    }

    if (parse_algo(argv[3], &tlb_algo) != 0) {
        fprintf(stderr, "Erro: algoritmo de substituicao da TLB invalido '%s'. Use 'fifo' ou 'lru'.\n",
                argv[3]);
        print_usage(argv[0]);
        return 1;
    }

    VirtualMemoryManager vm;
    if (vm_init(&vm, VM_NUM_FRAMES, page_algo, tlb_algo,
                VM_USE_TLB != 0, VM_USE_THREADS != 0, BACKING_STORE_FILE) != 0) {
        return 1;
    }

    if (vm_process_file(&vm, address_file, OUTPUT_FILE) != 0) {
        vm_destroy(&vm);
        return 1;
    }

    printf("Traducao concluida. Saida salva em '%s'.\n", OUTPUT_FILE);
    printf("Number of Translated Addresses = %llu\n", (unsigned long long)vm.total_addresses);
    printf("Page Faults = %llu\n", (unsigned long long)vm.page_faults);
    printf("Page Fault Rate = %.3f\n", (double)vm.page_faults / (double)vm.total_addresses);
    printf("TLB Hits = %llu\n", (unsigned long long)vm.tlb_hits);
    printf("TLB Hit Rate = %.3f\n", (double)vm.tlb_hits / (double)vm.total_addresses);

    vm_destroy(&vm);
    return 0;
}
