#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header/struct.h"
#include "header/print.h"
#include "header/generation.h"
#include "header/sum.h"
#include "header/moltiplication.h"

uint64_t generate_random_64bit() {
    uint64_t high = (uint64_t)rand() << 32;
    uint64_t low = (uint64_t)rand();
    return high | low;
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    /* Parsing of input. */
    int size;
    if (argc < 2) {
        size = SIZE;
    } else {
        size = atoi(argv[1]);
    }
    /* Generation of random numbers*/
    uint64_t num1 = generate_random_64bit();
    uint64_t num2 = generate_random_64bit();
    printf("First number : \t\t0x%lx\n", num1);
    printf("Second number : \t0x%lx\n", num2);
    printf("First number (high) : \t0x%lx\n", num1 >> 32);
    printf("First number (low): \t0x%lx\n", num1 & 0xFFFFFFFF);
    printf("Second number (high) : \t0x%lx\n", num2 >> 32);
    printf("Second number (low) : \t0x%lx\n", num2 & 0xFFFFFFFF);
    /* Creation of a, b, c vectors */
    uint384_t *a = malloc(size * sizeof(uint384_t));
    uint384_t *b = malloc(size * sizeof(uint384_t));
    uint384_t *c = malloc(size * sizeof(uint384_t));
    if (a == NULL || b == NULL || c == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    /* Initialization of the three vectors. */
    generate_large_number384(a, size, num1);
    generate_large_number384(b, size, num2);
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
    /* Free memory */
    free(a);
    free(b);
    free(c);
    /* Creation of all the memory I need */
    uint384_t_v2 upA[6] = {0};
    uint384_t_v2 lowA[6] = {0};
    uint384_t_v2 upB[6] = {0};
    uint384_t_v2 lowB[6] = {0};
    uint384_t_v2 upC[6] = {0};
    uint384_t_v2 lowC[6] = {0};
    /* Fill the memory */
    generate_number_384_v2(upB, size, num1 >> 32);
    generate_number_384_v2(lowB, size, num1 & 0xFFFFFFFF);
    generate_number_384_v2(upA, size, num2 >> 32);
    generate_number_384_v2(lowA, size, num2 & 0xFFFFFFFF);
    generate_number_384_v2(upC, size, 0x0);
    generate_number_384_v2(lowC, size, 0x0);
    /* Creation of the masks */
    uint256_t upMask = {0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000};
    uint256_t lowMask = {0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF,0x00000000FFFFFFFF};
    /* Run new simd sum */
    start = clock();
    simd_sum_32_ass_v2(size, upA, lowA, upB, lowB, upC, lowC, &upMask, &lowMask);
    end = clock();
    printFunction384_v2("simd_sum_32_ass_v2", (double)(end - start), upC, lowC);

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