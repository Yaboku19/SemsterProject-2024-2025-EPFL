#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define MULT_VALUE 10
#define SIZE 10

typedef struct {
    uint64_t low;
    uint64_t high;
} uint128_t;

typedef struct {
    uint64_t chunk[6];
} uint384_t;

typedef struct {
    uint64_t chunk[SIZE + SIZE % 4];
} uint384_t_v2;

typedef struct {
    uint64_t chunk[4];
} uint256_t;

typedef struct {
    uint64_t low[6];
    uint64_t high[6];
} uint384_t2;

void generate_large_number64(uint64_t *num, uint64_t *low, uint64_t *high, int length) {
    for (int i = 0; i < length; i++) {
        num[i] = 0xFFFFFFFFFFFFFFFF;
        low[i] = 0xFFFFFFFF;
        high[i] = 0xFFFFFFFF;
    }
}

void generate_large_number384(uint384_t *num, int length) {
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < 6; j++) {
            num[i].chunk[j] = 0x0F0000F0FFFFF000;
        }
    }
}

void generate_number_384_v2(uint384_t_v2 *num, int length, uint64_t value) {
    for(int i = 0; i < 6; i++) {
        for (int j = 0; j < length; j++) {
            num[i].chunk[j] = value;
        }
    }
}

void traditional_sum384(uint384_t *a, uint384_t *b, uint384_t *c, int length) {
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

uint128_t traditional_sum64(uint64_t *num, int length) {
    uint128_t result = {0,0};
    for (int i = 0; i < length; i++) {
        uint64_t new_value = result.low + num[i];
        if (new_value < result.low) {
            result.high++;
        }
        result.low = new_value;
    }
    return result;
}

void sequential_sum_ass(uint384_t *a, uint384_t *b, uint384_t *c, int length) {
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

uint128_t simd_sum_32_ass(int length, uint64_t *low, uint64_t *high) {
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

void simd_sum_32_ass_v2(int length, uint384_t_v2 *upA, uint384_t_v2 *lowA, uint384_t_v2 *upB, uint384_t_v2 *lowB, uint384_t_v2 *upC, uint384_t_v2 *lowC,
        uint256_t *upMask, uint256_t *lowMask) {
    uint64_t *PupA = upA[5].chunk;
    uint64_t *PlowA = lowA[5].chunk;
    uint64_t *PupB = upB[5].chunk;
    uint64_t *PlowB = lowB[5].chunk;
    uint64_t *PupC = upC[5].chunk;
    uint64_t *PlowC = lowC[5].chunk;
    size_t addLen = length * sizeof(uint64_t);
    size_t restore = (addLen * 6) + 32;
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "mov %[addLen], %%rax\n"
        "mov %[restore], %%rbx\n"
    "1:\n"
        "vpxor %%ymm6, %%ymm6, %%ymm6\n"    // ymm6 for rest
        "mov $6, %%r10\n"                   // loop of 6
    "2:\n"
        "vmovdqu (%[lowA]), %%ymm0\n"       // first operand in ymm0
        "vmovdqu (%[lowB]), %%ymm1\n"       // second operand in ymm1

        "vpaddq %%ymm1, %%ymm0, %%ymm0\n"   // sum in ymm0
        "vpaddq %%ymm6, %%ymm0, %%ymm0\n"   // sum the rest

        "vpand %%ymm0, %%ymm4, %%ymm1\n"    // and with lowerMap
        "vmovdqu %%ymm1, (%[lowC])\n"       // back in lowC
        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest

        "vmovdqu (%[upA]), %%ymm1\n"        // first operand in ymm1
        "vmovdqu (%[upB]), %%ymm2\n"        // second operand in ymm2

        "vpaddq %%ymm1, %%ymm2, %%ymm1\n"   // sum in ymm1
        "vpaddq %%ymm1, %%ymm6, %%ymm1\n"   // sum the rest

        "vpand %%ymm1, %%ymm4, %%ymm2\n"    // and with lowerMap
        "vmovdqu %%ymm2, (%[upC])\n"        // back in upC
        "vpand %%ymm1, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest

        "sub %%rax, %[lowA]\n"              // new pointers
        "sub %%rax, %[lowB]\n"
        "sub %%rax, %[upA]\n"
        "sub %%rax, %[upB]\n"
        "sub %%rax, %[lowC]\n"
        "sub %%rax, %[upC]\n"

        "dec %%r10\n"                       // Decrement counter
        "jnz 2b\n"                          //if not zero, loop again

        "add %%rbx, %[lowA]\n"              // new pointers
        "add %%rbx, %[lowB]\n"
        "add %%rbx, %[upA]\n"
        "add %%rbx, %[upB]\n"
        "add %%rbx, %[lowC]\n"
        "add %%rbx, %[upC]\n"

        "sub $4, %[len]\n"
        "jg 1b\n"
        : [len] "+r" (length), [addLen] "+r" (addLen), [restore] "+r" (restore), [upA]"+r" (PupA), [lowA]"+r" (PlowA), [upB]"+r" (PupB), [lowB]"+r" (PlowB), 
        [upC]"+r" (PupC), [lowC]"+r" (PlowC), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "r10", "rax", "rbx", "memory"
    );
}

void multiply_two_variables_optimized(const uint384_t *a, const uint384_t *b, uint384_t *c) {
    __uint128_t temp[6] = {0};

    for (int i = 0; i < 6; i++) {
        __uint128_t carry = 0;
        for (int j = 0; j < 6 - i; j++) {
            __uint128_t product = (__uint128_t)a->chunk[j] * b->chunk[i];
            product += carry + temp[i + j];
            temp[i + j] = (uint64_t)product;
            carry = product >> 64;
        }
    }

    for (int i = 0; i < 6; i++) {
        c->chunk[i] = temp[i];
    }
}

void fast_384_multiplication(uint384_t *a, uint384_t *b, uint384_t *c, int length) {
    for (int i = 0; i < length; i++) {
        multiply_two_variables_optimized(&a[i], &b[i], &c[i]);
    }
}

void multiply_two_variables_kar(const uint384_t *a, const uint384_t *b, uint384_t *c) {
    uint64_t res[12] = {0};

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6 - i; j++) {
            __uint128_t product = (__uint128_t)a->chunk[i] * b->chunk[j];
            res[i + j] += (uint64_t)product;
            res[i + j + 1] += (uint64_t)(product >> 64);

            if (res[i + j] < (uint64_t)product) {
                res[i + j + 1]++;
            }
        }
    }

    for (int i = 0; i < 6; i++) {
        c->chunk[i] = res[i];
    }
}

void multiplication384_kar(uint384_t *a, uint384_t *b, uint384_t *c, int length) {
    for (int i = 0; i < length; i++) {
        multiply_two_variables_optimized(&a[i], &b[i], &c[i]);
    }
}

void printFunction128(char *functionName, double time, uint128_t num) {
    int totalWidth = 15;
    int nameLength = strlen(functionName);
    int padding = (totalWidth - nameLength) / 2;
    printf("-------------------------------- %*s%*s --------------------------------\n", padding + nameLength, functionName, padding, "");
    printf("- Time (abs): \t%.1f\n", time);
    printf("- Time (sec): \t%.4f\n", time / CLOCKS_PER_SEC);
    printf("- Result: \t0x%016lx_%016lx", num.high, num.low);
    printf("\n");
}

void printFunction384(char *functionName, double time, uint384_t *result) {
    int totalWidth = 15;
    int nameLength = strlen(functionName);
    int padding = (totalWidth - nameLength) / 2;
    printf("-------------------------------- %*s%*s --------------------------------\n", padding + nameLength, functionName, padding, "");
    printf("- Time (abs): \t%.1f\n", time);
    printf("- Time (sec): \t%.4f\n", time / CLOCKS_PER_SEC);
    for(int i = 0; i < 4; i ++) {
        printf("- Result[%d]: \t0x%016lx", i, result[i].chunk[5]);
        for (int j = 4; j >= 0; j--) {
            printf("_%016lx", result[i].chunk[j]);
        }
        printf("\n");
    }

}

void printFunction384_v2(char *functionName, double time, uint384_t_v2 *upC, uint384_t_v2 *lowC) {
    int totalWidth = 15;
    int nameLength = strlen(functionName);
    int padding = (totalWidth - nameLength) / 2;
    printf("-------------------------------- %*s%*s --------------------------------\n", padding + nameLength, functionName, padding, "");
    printf("- Time (abs): \t%.1f\n", time);
    printf("- Time (sec): \t%.4f\n", time / CLOCKS_PER_SEC);
    for(int i = 0; i < 12; i ++) {
        printf("Result[%d] = \t0x%08lx%08lx", i, upC[0].chunk[i], lowC[0].chunk[i]);
        for(int j = 1; j < 6; j++) {
            printf("_%08lx%08lx", upC[j].chunk[i], lowC[j].chunk[i]);
        }
        printf("\n");
    }
    
}

int main(int argc, char* argv[]) {
    /* Parsing of input. */
    int size;
    if (argc < 2) {
        size = SIZE;
    } else {
        size = atoi(argv[1]);
    }
    /* Creation of a, b, c vectors */
    
    uint384_t *a = malloc(size * sizeof(uint384_t));
    uint384_t *b = malloc(size * sizeof(uint384_t));
    uint384_t *c = malloc(size * sizeof(uint384_t));
    if (a == NULL || b == NULL || c == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    /* Initialization of the three vectors. */
    generate_large_number384(a, size);
    generate_large_number384(b, size);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 6; j++) {
            c[i].chunk[j] = 0x0;
        }
    }
    /* Run of tradtional sum over 384 vector. */
    clock_t start = clock();
    traditional_sum384(a, b, c, size);
    clock_t end = clock();
    printFunction384("TraditionSum", (double)(end - start), c);
    /* Resetting c. */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 6; j++) {
            c[i].chunk[j] = 0x0;
        }
    }
    /* Run of sequential_sum_ass over 384 vector. */
    start = clock();
    sequential_sum_ass(a, b, c, size);
    end = clock();
    printFunction384("sequential_sum_ass", (double)(end - start), c);
    // /* Resetting c. */
    // for (int i = 0; i < size; i++) {
    //     for (int j = 0; j < 6; j++) {
    //         c[i].chunk[j] = 0x0;
    //     }
    // }
    // /* Run of tradtional moltiplication over 384 vector. */    
    // start = clock();
    // fast_384_multiplication(a, b, c, size);
    // end = clock();
    // printFunction384("fast_384_multiplication", (double)(end - start), c);
    // /* Resetting c. */
    // for (int i = 0; i < size; i++) {
    //     for (int j = 0; j < 6; j++) {
    //         c[i].chunk[j] = 0x0;
    //     }
    // }
    // start = clock();
    // multiplication384_kar(a, b, c, size);
    // end = clock();
    
    // printFunction384("multiplication384_kar", (double)(end - start), c);
    // /* Resetting c. */
    
    // for (int i = 0; i < size; i++) {
    //     for (int j = 0; j < 6; j++) {
    //         c[i].chunk[j] = 0x0;
    //     }
    // }

    free(a);
    free(b);
    free(c);

    uint384_t_v2 upA[6] = {0};
    uint384_t_v2 lowA[6] = {0};
    uint384_t_v2 upB[6] = {0};
    uint384_t_v2 lowB[6] = {0};
    uint384_t_v2 upC[6] = {0};
    uint384_t_v2 lowC[6] = {0};

    generate_number_384_v2(upA, size, 0x0);
    generate_number_384_v2(lowA, size, 0xFFFFF000);
    generate_number_384_v2(upB, size, 0x0);
    generate_number_384_v2(lowB, size, 0xFFFFF000);
    generate_number_384_v2(upC, size, 0x0);
    generate_number_384_v2(lowC, size, 0x0);

    uint256_t upMask = {0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000};
    uint256_t lowMask = {0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF,0x00000000FFFFFFFF};

    start = clock();
    simd_sum_32_ass_v2(size, upA, lowA, upB, lowB, upC, lowC, &upMask, &lowMask);
    end = clock();
    printFunction384_v2("simd_sum_32_ass_v2", (double)(end - start), upC, lowC);

    
    return 0;
}