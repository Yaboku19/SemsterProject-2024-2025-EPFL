#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header/struct.h"
#include "header/print.h"
#include "header/generation.h"
#include "header/sum.h"
#include "header/moltiplication.h"


int main(int argc, char* argv[]) {
    uint384_t *a = malloc(SIZE * sizeof(uint384_t));
    uint384_t *b = malloc(SIZE * sizeof(uint384_t));
    uint384_t *c = malloc(SIZE * sizeof(uint384_t));
    if (a == NULL || b == NULL || c == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    /* Initialization of the three vectors. */
    generate_large_number384(a, SIZE);
    generate_large_number384(b, SIZE);
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < 6; j++) {
            c[i].chunk[j] = 0x0;
        }
    }
    /* Run of normalSumArray384 over 384 vector. */
    clock_t start = clock();
    normalSumArray384(a, b, c, SIZE);
    clock_t end = clock();
    printFunction384("NormalSumArray384", (double)(end - start), c);
    /* Resetting c. */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < 6; j++) {
            c[i].chunk[j] = 0x0;
        }
    }
    /* Run of sequentialSumArray384 over 384 vector. */
    start = clock();
    sequentialSumArray384_ass(a, b, c, SIZE);
    end = clock();
    printFunction384("SequentialSumArray384_ass", (double)(end - start), c);
    /* Free memory */
    free(c);
    /* Creation of all the memory I need */
    
    four_uint384_t *upA = calloc(SIZE_SIMD, sizeof(four_uint384_t));
    four_uint384_t *lowA = calloc(SIZE_SIMD, sizeof(four_uint384_t));
    four_uint384_t *upB = calloc(SIZE_SIMD, sizeof(four_uint384_t));
    four_uint384_t *lowB = calloc(SIZE_SIMD, sizeof(four_uint384_t));
    four_uint384_t *upC = calloc(SIZE_SIMD, sizeof(four_uint384_t));
    four_uint384_t *lowC = calloc(SIZE_SIMD, sizeof(four_uint384_t));
    if (upA == NULL || lowA == NULL || upB == NULL || lowB == NULL || upC == NULL || lowC == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    /* Fill the memory */
    generate_number_384_v2(upB, SIZE_SIMD, b, 0);
    generate_number_384_v2(lowB, SIZE_SIMD, b, 1);
    generate_number_384_v2(upA, SIZE_SIMD, a, 0);
    generate_number_384_v2(lowA, SIZE_SIMD, a, 1);
    generate_number_384_v2(upC, SIZE_SIMD, NULL, 2);
    generate_number_384_v2(lowC, SIZE_SIMD, NULL, 2);
    // /* Creation of the masks */
    uint256_t upMask = {0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000};
    uint256_t lowMask = {0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF,0x00000000FFFFFFFF};
    // /* Run new simd sum */
    start = clock();
    simdSumArray384_ass(SIZE_SIMD, upA, lowA, upB, lowB, upC, lowC, &upMask, &lowMask);
    end = clock();
    printFunction384_v2("SimdSumArray384_ass", (double) end - start, upC, lowC);

    free(a);
    free(b);
    return 0;
}

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