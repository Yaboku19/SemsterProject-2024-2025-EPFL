#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "header/struct.h"
#include "header/print.h"
#include "header/generation.h"
#include "header/sum.h"

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