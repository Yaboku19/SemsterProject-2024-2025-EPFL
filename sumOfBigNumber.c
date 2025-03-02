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
#define MAX_LEN 400
typedef struct {
    uint64_t low;
    uint64_t high;
} uint128_t;

typedef struct {
    uint128_t low;
    uint128_t high;
} uint256_t;

typedef struct {
    uint64_t chunk[6];
} uint384_t;

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
            num[i].chunk[j] = 0xFFFFFFFFFFFFFFFF;
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
    printf("- Result: \t0x%016lx", result[0].chunk[5]);
    for (int j = 4; j >= 0; j--) {
        printf("_%016lx", result[0].chunk[j]);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    /* Parsing of input. */
    int size;
    if (argc < 2) {
        //     49689.0
        size = 1000000;
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
    /* Resetting c. */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 6; j++) {
            c[i].chunk[j] = 0x0;
        }
    }
    /* Run of tradtional moltiplication over 384 vector. */    
    start = clock();
    fast_384_multiplication(a, b, c, size);
    end = clock();
    printFunction384("fast_384_multiplication", (double)(end - start), c);
    /* Resetting c. */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 6; j++) {
            c[i].chunk[j] = 0x0;
        }
    }
    start = clock();
    multiplication384_kar(a, b, c, size);
    end = clock();
    
    printFunction384("multiplication384_kar", (double)(end - start), c);
    /* Resetting c. */
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 6; j++) {
            c[i].chunk[j] = 0x0;
        }
    }
    // printf("okwhat-------------------\n");
    // start = clock();
    // multiplication384_simd(a, b, c, size);
    // end = clock();
    // printFunction384("multiplication384_simd", (double)(end - start), c);
    /* Free unused memory*/
    free(a);
    free(b);
    free(c);
    /* Creation of num, high, low vectors */
    uint64_t *num = malloc(size * MULT_VALUE * sizeof(uint384_t));
    uint64_t *high = malloc(size * MULT_VALUE *  sizeof(uint384_t));
    uint64_t *low = malloc(size * MULT_VALUE * sizeof(uint384_t));
    if (num == NULL || high == NULL || low == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    /* Initialization of the three vectors. */
    generate_large_number64(num, high, low, size * MULT_VALUE);
    /* Run of tradtional sum over 64 vector. */
    start = clock();
    uint128_t result = traditional_sum64(num, size * MULT_VALUE);
    end = clock();
    printFunction128("traditional_sum64", (double)(end - start), result);
    /* Run of simd sum with 32 bit vector. */
    start = clock();
    result = simd_sum_32_ass(size * MULT_VALUE, low, high);
    end = clock();
    printFunction128("simd_sum_32_ass", (double)(end - start), result);
    /* Free unused memory*/
    free(num);
    free(high);
    free(low);

    return 0;
}

// void karatsubaTry(uint64_t a, uint64_t b, uint128_t *c) {
//     __uint128_t product = (__uint128_t)a * b;
//     c->low = (uint64_t)product;
//     c->high = (uint64_t)(product >> 64);
// }

// void karatsubaTry128(uint128_t a, uint128_t b, uint256_t *c) {
//     uint64_t a_low = a.low;
//     uint64_t a_high = a.high;
//     uint64_t b_low = b.low;
//     uint64_t b_high = b.high;

//     uint128_t Y, X, mid;

//     karatsubaTry(a_low, b_low, &Y);
//     karatsubaTry(a_high, b_high, &X);

//     uint128_t a_sum = {a_low + a_high, (a_low + a_high) < a_low};
//     uint128_t b_sum = {b_low + b_high, (b_low + b_high) < b_low};
//     if (a_sum.high > 0 || b_sum.high > 0) {
//         uint256_t temp = {0};
//         karatsubaTry128(a_sum, b_sum, &temp);
//         mid = temp.low;
        
//     } else {
//         karatsubaTry(a_sum.low, b_sum.low, &mid);
//     }
//     uint64_t old = mid.low;
//     mid.low -= X.low;
//     int carry = (old < mid.low ? 1 : 0);
//     old = mid.low;
//     mid.low -= Y.low;
//     carry += (old < mid.low ? 1 : 0);
//     mid.high -= X.high + Y.high + carry;

//     c->low.low = Y.low;
//     c->low.high = Y.high + mid.low;
//     carry = (c->low.high < Y.high ? 1 : 0);
//     c->high.low = mid.high + X.low + carry;
//     c->high.high = X.high + (((carry == 1 && c->low.high <= Y.high) || c->low.high < Y.high) ? 1 : 0);
// }

// void karatsubaTry384(uint384_t a, uint384_t b, uint384_t *c) {
//     uint128_t a0 = {a.chunk[0], a.chunk[1]};
//     uint128_t a1 = {a.chunk[2], a.chunk[3]};
//     uint128_t a2 = {a.chunk[4], a.chunk[5]};

//     uint128_t b0 = {b.chunk[0], b.chunk[1]};
//     uint128_t b1 = {b.chunk[2], b.chunk[3]};
//     uint128_t b2 = {b.chunk[4], b.chunk[5]};

//     uint256_t P0, P1;
//     karatsubaTry128(a0, b0, &P0);
//     karatsubaTry128(a1, b1, &P1);

//     uint384_t sumA = {0}, sumB = {0};
//     sumA.chunk[0] = a0.low + a1.low;
//     sumA.chunk[1] += (sumA.chunk[0] < a0.low ? 1 : 0);
//     sumA.chunk[0] += a2.low;
//     sumA.chunk[1] += (sumA.chunk[0] < a2.low ? 1 : 0);
//     sumA.chunk[1] += a0.low + a1.low;
//     sumA.chunk[2] += (sumA.chunk[1] < a0.low ? 1 : 0);
//     sumA.chunk[1] += a2.low;
//     sumA.chunk[2] += (sumA.chunk[0] < a2.low ? 1 : 0);

//     sumB.chunk[0] = b0.low + b1.low;
//     sumB.chunk[1] += (sumB.chunk[0] < b0.low ? 1 : 0);
//     sumB.chunk[0] += b2.low;
//     sumB.chunk[1] += (sumB.chunk[0] < b2.low ? 1 : 0);
//     sumB.chunk[1] += b0.low + b1.low;
//     sumB.chunk[2] += (sumB.chunk[1] < b0.low ? 1 : 0);
//     sumB.chunk[1] += b2.low;
//     sumB.chunk[2] += (sumB.chunk[0] < b2.low ? 1 : 0);

//     uint256_t Psum = {0};
//     if (sumA.chunk[2] > 0 || sumB.chunk[2] > 0) {
//         uint384_t temp = {0};
//         karatsubaTry384(sumA, sumB, &temp);
//         Psum.low.low = temp.chunk[0];
//         Psum.low.high = temp.chunk[1];
//         Psum.high.low = temp.chunk[2];
//         Psum.high.high = temp.chunk[3];
//         printf("Psum: \t\t%016lx_%016lx_%016lx_%016lx\n", Psum.high.high, Psum.high.low, Psum.low.high, Psum.low.low);
//     } else {
//         uint128_t sumA128 = {sumA.chunk[0], sumA.chunk[1]}, sumB128 = {sumB.chunk[0], sumB.chunk[1]};
//         karatsubaTry128(sumA128, sumB128, &Psum);
//         printf("Psum: \t\t%016lx_%016lx_%016lx_%016lx\n", Psum.high.high, Psum.high.low, Psum.low.high, Psum.low.low);
//     }
//     uint64_t old = Psum.low.low;
//     int carry = 0;
//     Psum.low.low -= P0.low.low;
//     carry = (old < Psum.low.low ? 1 : 0);
//     old = Psum.low.low;
//     Psum.low.low -= P1.low.low;
//     carry += (old < Psum.low.low ? 1 : 0);
//     old = Psum.low.low;

//     old = Psum.low.high;
//     Psum.low.high -= carry;
//     carry = (old < Psum.low.high ? 1 : 0);
//     old = Psum.low.high;
//     Psum.low.high -= P0.low.high;
//     carry += (old < Psum.low.high ? 1 : 0);
//     old = Psum.low.high;
//     Psum.low.high  -= P1.low.high;
//     carry += (old < Psum.low.high ? 1 : 0);

//     old = Psum.high.low;
//     Psum.high.low -= carry;
//     carry = (old < Psum.high.low ? 1 : 0);
//     old = Psum.high.low;
//     Psum.high.low -= P0.high.low;
//     carry += (old < Psum.high.low ? 1 : 0);
//     old = Psum.high.low;
//     Psum.high.low -= P1.high.low;
//     carry += (old < Psum.high.low ? 1 : 0);

//     Psum.high.high -= (P0.high.high + P1.high.high + carry);
//     printf("P0: \t\t%016lx_%016lx_%016lx_%016lx\n", P0.high.high, P0.high.low, P0.low.high, P0.low.low);
//     printf("P1: \t\t%016lx_%016lx_%016lx_%016lx\n", P1.high.high, P1.high.low, P1.low.high, P1.low.low);
//     printf("Psum: \t\t%016lx_%016lx_%016lx_%016lx\n", Psum.high.high, Psum.high.low, Psum.low.high, Psum.low.low);
//     // // Assemblaggio del risultato finale
//     // 0x8_FFFFFFFFFFFFFFFF_FFFFFFFFFFFFFFEE_0000000000000000_0000000000000009
//     c->chunk[0] = P0.low.low;
//     c->chunk[1] = P0.low.high;
//     c->chunk[2] = P0.high.low + Psum.low.low;
//     c->chunk[3] = (c->chunk[2] < P0.high.low ? 1 : 0);
//     c->chunk[3] += P0.high.high + Psum.low.high;
//     c->chunk[4] = (c->chunk[3] < P0.high.high ? 1 : 0);
//     c->chunk[4] += Psum.high.low + P1.low.low;
//     c->chunk[5] = (c->chunk[4] < Psum.high.low ? 1 : 0);
//     c->chunk[5] += Psum.high.high + P1.low.high;
// }