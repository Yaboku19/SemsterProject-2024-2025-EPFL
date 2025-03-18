#include "../header/moltiplication.h"
#include "../header/struct.h"
#include <stdio.h>
#include "../header/modulo.h"

void mul384_two_variables(const uint384_t *a, const uint384_t *b, uint384_t *c) {
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
    checkModulo384(c);
}

void mul384(uint384_t *a, uint384_t *b, uint384_t *c, int length) {
    for (int i = 0; i < length; i++) {
        mul384_two_variables(&a[i], &b[i], &c[i]);
    }
}

void mul384Fast_two_variables_kar(const uint384_t *a, const uint384_t *b, uint384_t *c) {
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
    checkModulo384(c);
}

void mul384Fast(uint384_t *a, uint384_t *b, uint384_t *c, int length) {
    for (int i = 0; i < length; i++) {
        mul384Fast_two_variables_kar(&a[i], &b[i], &c[i]);
    }
}

void mul384Simd_two_variables_ass (uint256_t *a, uint256_t *b, uint256_t *lowC, uint256_t *upC, uint256_t *upMask, uint256_t *lowMask) {
    uint256_t tempArray[24] = {0};
    uint256_t *temp = tempArray;
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "mov $12, %%rax\n"                  // loop of 12
    "1:\n"
        "mov $12, %%rbx\n"                  // loop of 12
        "vmovdqu (%[b]), %%ymm1\n"          // second operand in ymm1
        "vpxor %%ymm6, %%ymm6, %%ymm6\n"    // ymm6 for rest
    "2:\n"
        "vmovdqu (%[a]), %%ymm0\n"          // first operand in ymm1
        "vmovdqu (%[temp]), %%ymm2\n"       // temp in ymm2

        "vpmuldq %%ymm1, %%ymm0, %%ymm0\n"  // mul first and second operand
        "vpaddq %%ymm6, %%ymm0, %%ymm0\n"   // sum the rest
        "vpaddq %%ymm2, %%ymm0, %%ymm0\n"   // sum with temp

        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest
        "vpand %%ymm0, %%ymm4, %%ymm0\n"    // and with lowerMap
        "vmovdqu %%ymm0, (%[temp])\n"       // back in temp

        "add $32, %[a]\n"                   // new pointer for a
        "add $32, %[temp]\n"                // new pointer for b

        "dec %%rbx\n"                       // decrement counter
        "jge 2b\n"                          // if not zero, loop again

        "sub $384, %[a]\n"                  // resetting a
        "sub $352, %[temp]\n"               // resetting temp
        "add $32, %[b]\n"                   // new pointer b

        "dec %%rax\n"                       // decrement counter
        "jge 1b\n"                          // if not zero, loop again

        "sub $384, %[b]\n"                  // resetting b
        "sub $384, %[temp]\n"               // resetting temp
        : [a]"+r" (a), [b]"+r" (b), [upC]"+r" (upC), [lowC]"+r" (lowC), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        , [temp] "+r" (temp)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rax", "rbx", "memory"
    );
}

void mul384Simd_ass(four_uint384_t *lowA, four_uint384_t *upA, four_uint384_t *lowB, four_uint384_t *upB,
        four_uint384_t *lowC, four_uint384_t *upC, uint256_t *upMask, uint256_t *lowMask, int length) {
    for (int i = 0; i < length; i++) {
        uint256_t a[12] = {
            lowA[i].chunk[0], upA[i].chunk[0],
            lowA[i].chunk[1], upA[i].chunk[1],
            lowA[i].chunk[2], upA[i].chunk[2],
            lowA[i].chunk[3], upA[i].chunk[3],
            lowA[i].chunk[4], upA[i].chunk[4],
            lowA[i].chunk[5], upA[i].chunk[5],
        };
        uint256_t b[12] = {
            lowB[i].chunk[0], upB[i].chunk[0],
            lowB[i].chunk[1], upB[i].chunk[1],
            lowB[i].chunk[2], upB[i].chunk[2],
            lowB[i].chunk[3], upB[i].chunk[3],
            lowB[i].chunk[4], upB[i].chunk[4],
            lowB[i].chunk[5], upB[i].chunk[5],
        };
        mul384Simd_two_variables_ass(a, b, upC[i].chunk, lowC[i].chunk, upMask, lowMask);
    }
}
