#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <stdint.h>
#include <gmp.h>
#include <string.h>

typedef struct {
    uint64_t low;
    uint64_t high;
} uint128_t;

typedef struct {
    uint64_t chunk[6];
} uint384_t;

typedef struct {
    uint32_t low;
    uint32_t high;
} chunk;

typedef struct {
    chunk chunk[6];
} uint384_t2;

void generate_large_number(uint384_t* num, int length) {
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < 6; j++) {
            num[i].chunk[j] = 0xF000000000000000;
        }
    }
}

void generate_large_number2(uint384_t2* num, int length) {
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < 6; j++) {
            num[i].chunk[j].high = 0x900000;
            num[i].chunk[j].low = 0x245200;
        }
    }
}

void traditional_sum(uint384_t* a,uint384_t* b, uint384_t* c, int length) {
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < 6; j++) {
            uint64_t new_value = a[i].chunk[j] + a[i].chunk[j];
            if(new_value < a[i].chunk[j] && j != 5) {
                c[i].chunk[j+1] += 1;
            }
            c[i].chunk[j] += new_value;
        }
    }
}

void other_sum_v2(uint384_t* a, uint384_t* b, uint384_t* c, int length) {
    asm volatile(
        "1:\n"
        "   test %[len], %[len]\n"
        "   jle 2f\n"

        "   movq %[c], %%rdx\n" 

        "   movq (%[a]), %%rax\n"
        "   addq (%[b]), %%rax\n"
        "   movq %%rax, (%%rdx)\n"

        "   movq 8(%[a]), %%rax\n"
        "   adcq 8(%[b]), %%rax\n"
        "   movq %%rax, 8(%%rdx)\n"

        "   movq 16(%[a]), %%rax\n"
        "   adcq 16(%[b]), %%rax\n"
        "   movq %%rax, 16(%%rdx)\n"

        "   movq 24(%[a]), %%rax\n"
        "   adcq 24(%[b]), %%rax\n"
        "   movq %%rax, 24(%%rdx)\n"

        "   movq 32(%[a]), %%rax\n"
        "   adcq 32(%[b]), %%rax\n"
        "   movq %%rax, 32(%%rdx)\n"

        "   movq 40(%[a]), %%rax\n"
        "   adcq 40(%[b]), %%rax\n"
        "   movq %%rax, 40(%%rdx)\n"

        "   addq $48, %[a]\n"
        "   addq $48, %[b]\n"
        "   addq $48, %[c]\n"
        "   dec %[len]\n"
        "   jnz 1b\n"

        "2:\n"
        : [a]"+r"(a), [b]"+r"(b), [c]"+r"(c), [len]"+r"(length)
        :
        : "rax", "rdx", "memory"
    );
}

uint128_t simd_sum_32(int length, uint64_t* low, uint64_t* high) {
    uint128_t result = {0, 0};

    __m256i sum_low = _mm256_setzero_si256();
    __m256i sum_high = _mm256_setzero_si256();
    for (int i = 0; i < length; i += 4) {
        __m256i data = _mm256_load_si256((__m256i*)(low + i));
        __m256i data2 = _mm256_load_si256((__m256i*)(high + i));
        sum_low = _mm256_add_epi64(sum_low, data);
        sum_high = _mm256_add_epi64(sum_high, data2);
    }

    uint64_t highValue = 0;
    uint64_t lowValue = 0;
    for (int j = 0; j < 4; j++) {
        highValue += ((uint64_t*)&sum_high)[j];
        lowValue += ((uint64_t*)&sum_low)[j];
    }
    __uint128_t total = ((__uint128_t)(highValue + (lowValue >> 32)) << 32) | lowValue & 0xFFFFFFFF;
    result.low = (uint64_t)(total & 0xFFFFFFFFFFFFFFFF);
    result.high = (uint64_t)(total >> 64);
    return result;
}

uint128_t simd_sum_32_ass(int length, uint64_t* low, uint64_t* high) {
    uint128_t result = {0, 0};
    uint64_t lowValue, highvalue;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n"
        "1:\n"
        "   vpxor %%ymm2, %%ymm2, %%ymm2\n"
        "   vmovdqu (%[low]), %%ymm2\n"  
        "   vpaddq %%ymm2, %%ymm0, %%ymm0\n"
        "   vpxor %%ymm2, %%ymm2, %%ymm2\n"
        "   vmovdqu (%[high]), %%ymm2\n"  
        "   vpaddq %%ymm2, %%ymm1, %%ymm1\n"
        "   add $32, %[low]\n"
        "   add $32, %[high]\n"
        "   sub $4, %[len]\n"
        "   jg 1b\n"
        "xor %%rax, %%rax\n"
        "vextracti128 $1, %%ymm0, %%xmm0\n"
        "pextrq $1, %%xmm0, %%rcx\n"
        "addq %%rcx, %%rax\n"
        "pextrq $0, %%xmm0, %%rcx\n"
        "addq %%rcx, %%rax\n"
        "vextracti128 $0, %%ymm0, %%xmm0\n"
        "pextrq $1, %%xmm0, %%rcx\n"
        "addq %%rcx, %%rax\n"
        "pextrq $0, %%xmm0, %%rcx\n"
        "addq %%rcx, %%rax\n"

        "xor %%rbx, %%rbx\n"
        "vextracti128 $1, %%ymm1, %%xmm0\n"
        "pextrq $1, %%xmm0, %%rcx\n"
        "addq %%rcx, %%rbx\n"
        "pextrq $0, %%xmm0, %%rcx\n"
        "addq %%rcx, %%rbx\n"
        "vextracti128 $0, %%ymm1, %%xmm0\n"
        "pextrq $1, %%xmm0, %%rcx\n"
        "addq %%rcx, %%rbx\n"
        "pextrq $0, %%xmm0, %%rcx\n"
        "addq %%rcx, %%rbx\n"
        
        "movq %%rax, %[lowVal]\n"
        "movq %%rbx, %[highVal]\n"
        : [low]"+r" (low), [high]"+r" (high), [len] "+r" (length), [lowVal] "+m" (lowValue), [highVal] "+m" (highvalue) 
        :
        : "ymm0", "ymm1", "ymm2", "rax", "rbx", "rcx", "xmm0", "memory"
    );
    __uint128_t total = ((__uint128_t)(highvalue + (lowValue >> 32)) << 32) | lowValue & 0xFFFFFFFF;
    result.low = (uint64_t)(total & 0xFFFFFFFFFFFFFFFF);
    result.high = (uint64_t)(total >> 64);
    return result;
}

void printFunction(char *functionName, double time, uint384_t* result) {
    int totalWidth = 15;
    int nameLength = strlen(functionName);
    int padding = (totalWidth - nameLength) / 2;
    printf("-------------------------------- %*s%*s --------------------------------\n", padding + nameLength, functionName, padding, "");
    printf("- Time (abs): \t%.1f\n", time);
    printf("- Time (sec): \t%.4f\n", time / CLOCKS_PER_SEC);
    printf("- Result: \t0x%016lx", result[0].chunk[5]);
    for (int j = 4; j >= 0; j--) {
        printf("_%016lx", result[0].chunk[j]);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    int size;
    if (argc < 2) {
        size = 20000000;
    } else {
        size = atoi(argv[1]);
    }

    uint384_t *a = malloc(size * sizeof(uint384_t));
    uint384_t *b = malloc(size * sizeof(uint384_t));
    uint384_t *c = malloc(size * sizeof(uint384_t));
    if (a == NULL || b == NULL || c == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    generate_large_number(a, size);
    generate_large_number(b, size);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 6; j++) {
            c[i].chunk[j] = 0x0;
        }
    }
    clock_t start = clock();
    traditional_sum(a, b, c, size);
    clock_t end = clock();
    printFunction("TraditionSum", (double)(end - start), c);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 6; j++) {
            c[i].chunk[j] = 0x0;
        }
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 6; j++) {
            c[i].chunk[j] = 0x0;
        }
    }

    start = clock();
    other_sum_v2(a, b, c, size);
    end = clock();
    printFunction("other_sum_v2", (double)(end - start), c);

    free(a);
    free(b);
    free(c);

    uint384_t2 *num2 = malloc(size * sizeof(uint384_t2));
    if (num2 == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    generate_large_number2(num2, size);

    free(num2);
    // uint64_t *num = malloc(size * 4 * sizeof(uint64_t));
    // if (num == NULL) {
    //     printf("Memory allocation failed\n");
    //     return 1;
    // }
    // 

    // clock_t start = clock();
    // uint128_t result = traditional_sum(num, size*4);
    // clock_t end = clock();
    // double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    // printf("Time spent in traditional sum: %.3f seconds, Result: ", time_spent);
    // print_uint128(result);

    // start = clock();
    // result = simd_sum(num, size*4);
    // end = clock();
    // time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    // printf("Time spent in SIMD sum: %.3f seconds, Result: ", time_spent);
    // print_uint128(result);
    
    // uint64_t *low, *high;
    // // Usa posix_memalign per evitare problemi di allineamento
    // if (posix_memalign((void**)&low, 64, size*4 * sizeof(uint64_t)) != 0 ||
    //     posix_memalign((void**)&high, 64, size*4 * sizeof(uint64_t)) != 0) {
    //     printf("Memory allocation failed\n");
    //     exit(1);
    // }
    // for (int i = 0; i < size*4; i++) {
    //     low[i] = num[i] & 0xFFFFFFFF;       // Parte bassa
    //     high[i] = (num[i] >> 32) & 0xFFFFFFFF; // Parte alta
    // }

    // start = clock();
    // result = simd_sum_32(size*4, low, high);
    // end = clock();
    // time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    // printf("Time spent in SIMD_v4 sum: %.3f seconds, Result: ", time_spent);
    // print_uint128(result);

    // start = clock();
    // result = simd_sum_32_ass(size*4, low, high);
    // end = clock();
    // time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    // printf("Time spent in SIMD_v4 sum: %.3f seconds, Result: ", time_spent);
    // print_uint128(result);
    // free(low);
    // free(high);
    // free(num);

    return 0;
}
// void print_uint128(uint128_t num) {
//     mpz_t big_num;
//     mpz_init(big_num);
//     mpz_set_ui(big_num, num.high);  // Imposta la parte alta.
//     mpz_mul_2exp(big_num, big_num, 64);  // Sposta la parte alta di 64 bit a sinistra.
//     mpz_add_ui(big_num, big_num, num.low);  // Aggiungi la parte bassa.

//     char *text = mpz_get_str(NULL, 10, big_num);  // Converti il numero a stringa in base 10.
//     printf("%s\n", text);  // Stampa il numero.
//     printf("up: %lx down: %lx\n", num.high, num.low);
//     free(text);  // Libera la memoria allocata da GMP.

//     mpz_clear(big_num);  // Pulisci la memoria usata da GMP.
// }

