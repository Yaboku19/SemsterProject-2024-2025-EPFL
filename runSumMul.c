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
    // a_dash = "0f1fbe0ef913ef77_f35cc0cc24d16fa6_5c6390814f4e2c71_b12bdd1490201e07_05c5f2214542a91e_ffd93a7995b82354".replace("_", "")
    // b_dash = "02ec6d906e960b93_45a5fa5428a901c9_a4cf9964ee7c8c24_c56f01f1dc303c59_3e729f97c6a99335_4a729d6536d71097".replace("_", "")
    a[0].chunk[0] = 0xffd93a7995b82354;
    a[0].chunk[1] = 0x05c5f2214542a91e;
    a[0].chunk[2] = 0xb12bdd1490201e07;
    a[0].chunk[3] = 0x5c6390814f4e2c71;
    a[0].chunk[4] = 0xf35cc0cc24d16fa6;
    a[0].chunk[5] = 0x0f1fbe0ef913ef77;

    b[0].chunk[0] = 0x4a729d6536d71097;
    b[0].chunk[1] = 0x3e729f97c6a99335;
    b[0].chunk[2] = 0xc56f01f1dc303c59;
    b[0].chunk[3] = 0xa4cf9964ee7c8c24;
    b[0].chunk[4] = 0x45a5fa5428a901c9;
    b[0].chunk[5] = 0x02ec6d906e960b93;

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
    /* Sums */
    // printf("------------------------------------------------------- [ SUM ] -------------------------------------------------------\n\n");
    /* Run of normalSumArray384 over 384 vector. */
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
    // generate_number_384_v2(upC, SIZE_SIMD, NULL, 2);
    // generate_number_384_v2(lowC, SIZE_SIMD, NULL, 2);
    // /* sub */
    // clock_gettime(CLOCK_MONOTONIC, &start);
    // simdSubArray384_ass(SIZE_SIMD, upA, lowA, upB, lowB, upC, lowC, &upMask, &lowMask);
    // clock_gettime(CLOCK_MONOTONIC, &end);
    // printFunction384_v2("SimdSubArray384_ass", (double)(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec), upC, lowC);
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
    clock_gettime(CLOCK_MONOTONIC, &start);
    mul384SimdMon_ass(lowA, upA, lowB, upB, lowC, upC, &upMask, &lowMask, SIZE_SIMD);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printFunction384_v2("mul384SimdMon_ass", (double)(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec), upC, lowC);
    printf("- ris_a: 0x169bafc1fc4b974a_e0fdd29032746711_ee3c6956ebc1bfd7_c38b614185430837_b401ffcc72ed439b_3a57108ff2b5497a\n");
    free(a);
    free(b);
    free(c);
    
    return 0;
}
