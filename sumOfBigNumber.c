#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <stdint.h>
#include <gmp.h>

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

uint128_t simd_sum(uint64_t* num, int length) {
    __m256i sum = _mm256_setzero_si256();
    uint128_t result = {0, 0};
    uint64_t overflow_count = 0;
    uint64_t oldSum[4];

    for (int i = 0; i < length; i += 4) {
        __m256i data = _mm256_loadu_si256((__m256i*)(num + i));
        for (int j = 0; j < 4; j++) {
            oldSum[j] = ((uint64_t*)&sum)[j];
        }
        sum = _mm256_add_epi64(sum, data);

        for (int j = 0; j < 4; j++) {
            uint64_t old_val = oldSum[j];
            uint64_t new_val = ((uint64_t*)&sum)[j];
            if (new_val >= old_val) {
                continue;
            }
            overflow_count++;
            if (j < 3) {
                ((uint64_t*)&sum)[j + 1] -= 1;
            }

        }
    }
    printf("over: %lu\n", overflow_count);
    uint64_t carry = 0;
    for (int j = 0; j < 4; j++) {
        uint64_t temp = result.low + ((uint64_t*)&sum)[j];
        if (temp < result.low) {
            carry++;
        }
        result.low = temp;
    }
    result.high = overflow_count + carry;
    return result;
}

uint128_t simd_sum_v2(uint64_t* num, int length) {
    uint64_t result_array[4] = {0, 0, 0, 0};
    uint128_t result;
    uint64_t overflow_count = 0;
    uint64_t *oldSum = malloc(4 * sizeof(uint64_t));
    uint64_t *currentSum = malloc(4 * sizeof(uint64_t));
    const char *fmt = "valore: %lu\n";
    if(oldSum == NULL || currentSum == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    asm volatile(
        "vpxor %%ymm0, %%ymm0, %%ymm0\n"                // set ymm0 to zero
        "1:\n"
        "   vpxor %%ymm1, %%ymm1, %%ymm1\n"  
        "   vmovdqu (%[nums]), %%ymm1\n"                 // load 256 bits from num into ymm1
        "   vmovdqu %%ymm0, (%3)\n"                     // store current ymm0 to oldSum before summing
        "   vpaddq %%ymm1, %%ymm0, %%ymm0\n"            // add ymm1 to ymm0, correct to use 64-bit add
        "   vmovdqu %%ymm0, (%4)\n"                     // store current ymm0 to currentSum after summing
        "   add $32, %0\n"                              // move pointer to the next 256 bits
        "   sub $4, %[len]\n"                               // decrement length by 4
        "   mov $4, %%ecx\n"                            // Set loop counter for element comparison (4 elements, index 3 to 0)
        "   mov %3, %%rsi\n"                           // Save oldSum in rsi
        "   mov %4, %%rdi\n"                           // Save currentSum in rdi
        "2:\n"
        "   movq (%%rsi), %%rax\n"                          // Load from oldSum for comparison
        "   movq (%%rdi), %%rdx\n"                          // Load from currentSum for comparison
        "   cmp %%rax, %%rdx\n"                             // Compare currentSum[i] with oldSum[i]
        "   jge 3f\n"                                       // Jump if currentSum[i] >= oldSum[i], skip to next
        "   incq %5\n"                                      // Increment overflow_count
        "3:\n"
        "   lea 8(%%rdi), %%rdi\n"                      // Move currentSum pointer
        "   lea 8(%%rsi), %%rsi\n"                      // Move oldSum pointer
        "   loop 2b\n"                                  // Decrement ecx and loop back to 2:
        "   jg 1b\n"                                    // loop until length is not greater than 0
        "vmovdqu %%ymm0, %2\n"                          // store result from ymm0 to result_array
        : [nums]"+r" (num), [len] "+r" (length), "+m" (result_array), "+r" (oldSum), "+r" (currentSum), "+m" (overflow_count)
        :
        : "ymm0", "ymm1", "rcx", "rax", "rdx", "rsi", "rdi", "cc", "memory"
    );
    //result = traditional_sum(result_array, 4);
    result.high += overflow_count;
    printf("over: %lu\n", overflow_count);
    free(oldSum);
    free(currentSum);
    return result;
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

int main(int argc, char* argv[]) {
    int size;
    if (argc < 2) {
        size = 1000;
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
    printf("time: %.1f\n", (double)(end - start));
    printf("result: 0x%016lx", c[0].chunk[5]);
    for (int j = 4; j >= 0; j--) {
        printf("_%016lx", c[0].chunk[j]);
    }
    printf("\n");

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

