#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header/struct.h"
#include "header/print.h"
#include "header/generation.h"
#include "header/sum.h"
#include "header/multiplication.h"

int main(int argc, char* argv[]) {
    srand(time(NULL));
    /* Creation of all the memory needed */
    uint384_t *a = malloc(SIZE * sizeof(uint384_t));
    uint384_t *b = malloc(SIZE * sizeof(uint384_t));
    uint384_t *c = malloc(SIZE * sizeof(uint384_t));
    four_uint384_t *upA = calloc(SIZE_SIMD, sizeof(four_uint384_t));
    four_uint384_t *lowA = calloc(SIZE_SIMD, sizeof(four_uint384_t));
    four_uint384_t *upB = calloc(SIZE_SIMD, sizeof(four_uint384_t));
    four_uint384_t *lowB = calloc(SIZE_SIMD, sizeof(four_uint384_t));
    four_uint384_t *upC = calloc(SIZE_SIMD, sizeof(four_uint384_t));
    four_uint384_t *lowC = calloc(SIZE_SIMD, sizeof(four_uint384_t));
    if (a == NULL || b == NULL || c == NULL 
            || upA == NULL || lowA == NULL || upB == NULL || lowB == NULL || upC == NULL || lowC == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    /* Initialization of all the memory. */
    generate_large_number384(a, SIZE, (unsigned int)rand());
    generate_large_number384(b, SIZE, (unsigned int)rand());

    a[0].chunk[0] = 0xd34b7babb7a5545c;
    a[0].chunk[1] = 0x96db91722868700d;
    a[0].chunk[2] = 0xf7b1464746447580;
    a[0].chunk[3] = 0x70875ac2bec64327;
    a[0].chunk[4] = 0x9817c17ea10c4154;
    a[0].chunk[5] = 0x1784552029e56a41;

    a[1].chunk[0] = 0x9f800e327413c2c0;
    a[1].chunk[1] = 0xfebc0b207f15d1be;
    a[1].chunk[2] = 0x64aa769ac305ddcd;
    a[1].chunk[3] = 0xa44b17f06c1c459f;
    a[1].chunk[4] = 0x013abd6d41881691;
    a[1].chunk[5] = 0x0cb53e8a9ea119fc;

    b[0].chunk[0] = 0x9f800e327413c2c0;
    b[0].chunk[1] = 0xfebc0b207f15d1be;
    b[0].chunk[2] = 0x64aa769ac305ddcd;
    b[0].chunk[3] = 0xa44b17f06c1c459f;
    b[0].chunk[4] = 0x013abd6d41881691;
    b[0].chunk[5] = 0x0cb53e8a9ea119fc;

    b[1].chunk[0] = 0xf77c51155b9310eb;
    b[1].chunk[1] = 0x344bea1fa6a8f0f7;
    b[1].chunk[2] = 0x4f2b37769e531e7f;
    b[1].chunk[3] = 0x11726bfd55027eff;
    b[1].chunk[4] = 0x1eeea475ef7996f7;
    b[1].chunk[5] = 0x071806aac9c82e66;

    generate_number_384_v2(upB, SIZE_SIMD, b, 0);
    generate_number_384_v2(lowB, SIZE_SIMD, b, 1);
    generate_number_384_v2(upA, SIZE_SIMD, a, 0);
    generate_number_384_v2(lowA, SIZE_SIMD, a, 1);
    generate_number_384_v2(upC, SIZE_SIMD, NULL, 2);
    generate_number_384_v2(lowC, SIZE_SIMD, NULL, 2);
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < 6; j++) {
            c[i].chunk[j] = 0x0;
        }
    }
    /* Creation of the masks */
    uint256_t upMask = {0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000};
    uint256_t lowMask = {0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF,0x00000000FFFFFFFF};
    // Sums
    //printf("------------------------------------------------------- [ SUM ] -------------------------------------------------------\n\n");
    // /* Run of normalSumArray384 over 384 vector. */
    struct timespec start, end;
    // clock_gettime(CLOCK_MONOTONIC, &start);
    // normalSumArray384(a, b, c, SIZE);
    // clock_gettime(CLOCK_MONOTONIC, &end);
    // printFunction384("NormalSumArray384", (double)(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec), c);
    // /* Resetting c. */
    // for (int i = 0; i < SIZE; i++) {
    //     for (int j = 0; j < 6; j++) {
    //         c[i].chunk[j] = 0x0;
    //     }
    // }
    // /* Run of sequentialSumArray384 over 384 vector. */
    // clock_gettime(CLOCK_MONOTONIC, &start);
    // sequentialSumArray384_ass(a, b, c, SIZE);
    // clock_gettime(CLOCK_MONOTONIC, &end);
    // printFunction384("SequentialSumArray384_ass", (double)(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec), c);
    // /* Run new simd sum */
    // clock_gettime(CLOCK_MONOTONIC, &start);
    // simdSumArray384_ass(SIZE_SIMD, upA, lowA, upB, lowB, upC, lowC, &upMask, &lowMask);
    // clock_gettime(CLOCK_MONOTONIC, &end);
    // printFunction384_v2("SimdSumArray384_ass", (double)(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec), upC, lowC);
    // /* Resetting c */
    generate_number_384_v2(upC, SIZE_SIMD, NULL, 2);
    generate_number_384_v2(lowC, SIZE_SIMD, NULL, 2);
    /* sub */
    clock_gettime(CLOCK_MONOTONIC, &start);
    simdSubArray384_ass(SIZE_SIMD, upA, lowA, upB, lowB, upC, lowC, &upMask, &lowMask);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printFunction384_v2("SimdSubArray384_ass", (double)(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec), upC, lowC);
    // // Moltiplication
    // printf("-------------------------------------------------- [ Moltiplication ] -------------------------------------------------\n\n");
    // /* Resetting c. */
    // for (int i = 0; i < SIZE; i++) {
    //     for (int j = 0; j < 6; j++) {
    //         c[i].chunk[j] = 0x0;
    //     }
    // }
    // /* Run of tradtional moltiplication over 384 vector. */    
    // clock_gettime(CLOCK_MONOTONIC, &start);
    // mul384(a, b, c, SIZE);
    // clock_gettime(CLOCK_MONOTONIC, &end);
    // printFunction384("mul384", (double)(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec), c);
    // /* Resetting c. */
    // for (int i = 0; i < SIZE; i++) {
    //     for (int j = 0; j < 6; j++) {
    //         c[i].chunk[j] = 0x0;
    //     }
    // }
    // clock_gettime(CLOCK_MONOTONIC, &start);
    // mul384Fast(a, b, c, SIZE);
    // clock_gettime(CLOCK_MONOTONIC, &end);
    // printFunction384("mul384Fast", (double)(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec), c);
    // /* Resetting c*/
    // generate_number_384_v2(upC, SIZE_SIMD, NULL, 2);
    // generate_number_384_v2(lowC, SIZE_SIMD, NULL, 2);
    // /**/
    // clock_gettime(CLOCK_MONOTONIC, &start);
    // mul384Simd_ass(lowA, upA, lowB, upB, lowC, upC, &upMask, &lowMask, SIZE_SIMD);
    // clock_gettime(CLOCK_MONOTONIC, &end);
    // printFunction384_v2("mul384Simd_ass", (double)(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec), upC, lowC);
    // /* Free memory*/
    free(a);
    free(b);
    free(c);
    
    return 0;
}
