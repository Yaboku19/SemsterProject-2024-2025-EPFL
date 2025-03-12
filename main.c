#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header/struct.h"
#include "header/print.h"
#include "header/generation.h"
#include "header/sum.h"
#include "header/moltiplication.h"

int main(int argc, char* argv[]) {
    /* Parsing of input. */
    int size;
    if (argc < 2) {
        size = SIZE;
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
    normalSumArray384(a, b, c, size);
    clock_t end = clock();
    printFunction384("NormalSumArray384", (double)(end - start), c);
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
    /* Free memory */
    free(c);
    /* Creation of all the memory I need */
    uint384_t_v2 upA[6] = {0};
    uint384_t_v2 lowA[6] = {0};
    uint384_t_v2 upB[6] = {0};
    uint384_t_v2 lowB[6] = {0};
    uint384_t_v2 upC[6] = {0};
    uint384_t_v2 lowC[6] = {0};
    /* Fill the memory */
    generate_number_384_v2(upB, size, b, 0);
    generate_number_384_v2(lowB, size, b, 1);
    generate_number_384_v2(upA, size, a, 0);
    generate_number_384_v2(lowA, size, a, 1);
    generate_number_384_v2(upC, size, NULL, 2);
    generate_number_384_v2(lowC, size, NULL, 2);
    /* Creation of the masks */
    uint256_t upMask = {0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000};
    uint256_t lowMask = {0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF,0x00000000FFFFFFFF};
    /* Run new simd sum */
    start = clock();
    simd_sum_32_ass_v2(size, upA, lowA, upB, lowB, upC, lowC, &upMask, &lowMask);
    end = clock();
    printFunction384_v2("simd_sum_32_ass_v2", (double)(end - start), upC, lowC);

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