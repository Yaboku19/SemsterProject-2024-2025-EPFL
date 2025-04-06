#include "../header/multiplication.h"
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

void mul384Simd_two_variables_ass (uint256_t *a, uint256_t *b, uint256_t *upC, uint256_t *lowC, uint256_t *upMask, uint256_t *lowMask) {
    uint256_t tempArray[12] = {0};
    uint256_t *temp = &tempArray[0];
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "mov $12, %%rax\n"                  // loop of 12
    "1:\n"
        "mov %%rax, %%rbx\n"                // loop of 12
        "vmovdqu (%[b]), %%ymm1\n"          // second operand in ymm1
        "vpxor %%ymm6, %%ymm6, %%ymm6\n"    // ymm6 for rest
    "2:\n"
        "vmovdqu (%[a]), %%ymm0\n"          // first operand in ymm1
        "vmovdqu (%[temp]), %%ymm2\n"       // temp in ymm2

        "vpmuludq %%ymm1, %%ymm0, %%ymm0\n" // mul first and second operand
        "vpaddq %%ymm6, %%ymm0, %%ymm0\n"   // sum the rest
        "vpaddq %%ymm2, %%ymm0, %%ymm0\n"   // sum with temp

        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest
        "vpand %%ymm0, %%ymm4, %%ymm0\n"    // and with lowerMap
        "vmovdqu %%ymm0, (%[temp])\n"       // back in temp

        "add $32, %[a]\n"                   // new pointer for a
        "add $32, %[temp]\n"                // new pointer for b

        "dec %%rbx\n"                       // decrement counter
        "jg 2b\n"                          // if not zero, loop again

        "mov %%rax, %%rcx\n"
        "imul $32, %%rcx\n"

        "sub %%rcx, %[a]\n"                 // resetting a
        "sub %%rcx, %[temp]\n"              // resetting temp
        "add $32, %[temp]\n"
        "add $32, %[b]\n"                   // new pointer b

        "dec %%rax\n"                       // decrement counter
        "jg 1b\n"                          // if not zero, loop again
        : [a]"+r" (a), [b]"+r" (b), [upC]"+r" (upC), [lowC]"+r" (lowC), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        , [temp] "+r" (temp)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rax", "rbx", "rcx", "memory"
    );
    for (int i = 0; i < 12; i ++) {
        if (i % 2 == 0) {
            lowC[i / 2] = tempArray[i];
        } else {
            upC[i / 2] = tempArray[i];
        }
    }
    checkFourModulo384(upC, lowC);
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

void mul384SimdMon_two_variables_ass (uint256_t *a, uint256_t *b, uint256_t *upC, uint256_t *lowC, uint256_t *upMask, uint256_t *lowMask) {
    uint256_t t[24] = {0}, m[12] = {0}, finalMul[24] = {0}, final[24] = {0};
    uint256_t n0[12] = {
        {0xfffcfffd, 0xfffcfffd, 0xfffcfffd, 0xfffcfffd},
        {0x89f3fffc, 0x89f3fffc, 0x89f3fffc, 0x89f3fffc},
        {0xd9d113e8, 0xd9d113e8, 0xd9d113e8, 0xd9d113e8},
        {0x286adb92, 0x286adb92, 0x286adb92, 0x286adb92},
        {0xc8e30b48, 0xc8e30b48, 0xc8e30b48, 0xc8e30b48},
        {0x16ef2ef0, 0x16ef2ef0, 0x16ef2ef0, 0x16ef2ef0},
        {0x8eb2db4c, 0x8eb2db4c, 0x8eb2db4c, 0x8eb2db4c},
        {0x19ecca0e, 0x19ecca0e, 0x19ecca0e, 0x19ecca0e},
        {0xe268cf58, 0xe268cf58, 0xe268cf58, 0xe268cf58},
        {0x68b316fe, 0x68b316fe, 0x68b316fe, 0x68b316fe},
        {0xfeaafc94, 0xfeaafc94, 0xfeaafc94, 0xfeaafc94},
        {0xceb06106, 0xceb06106, 0xceb06106, 0xceb06106}
    };
    uint256_t prime[12] = {
        {0xffffaaab, 0xffffaaab, 0xffffaaab, 0xffffaaab},
        {0xb9feffff, 0xb9feffff, 0xb9feffff, 0xb9feffff},
        {0xb153ffff, 0xb153ffff, 0xb153ffff, 0xb153ffff},
        {0x1eabfffe, 0x1eabfffe, 0x1eabfffe, 0x1eabfffe},
        {0xf6b0f624, 0xf6b0f624, 0xf6b0f624, 0xf6b0f624},
        {0x6730d2a0, 0x6730d2a0, 0x6730d2a0, 0x6730d2a0},
        {0xf38512bf, 0xf38512bf, 0xf38512bf, 0xf38512bf},
        {0x64774b84, 0x64774b84, 0x64774b84, 0x64774b84},
        {0x434bacd7, 0x434bacd7, 0x434bacd7, 0x434bacd7},
        {0x4b1ba7b6, 0x4b1ba7b6, 0x4b1ba7b6, 0x4b1ba7b6},
        {0x397fe69a, 0x397fe69a, 0x397fe69a, 0x397fe69a},
        {0x1a0111ea, 0x1a0111ea, 0x1a0111ea, 0x1a0111ea}
    };
    uint256_t *pt = &t[0], *pm = &m[0], *pn0 = &n0[0], *pfinalMul = &finalMul[0], *pprime = &prime[0], *pfinal = &final[0];
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

        "vpmuludq %%ymm1, %%ymm0, %%ymm0\n" // mul first and second operand
        "vpaddq %%ymm6, %%ymm0, %%ymm0\n"   // sum the rest
        "vpaddq %%ymm2, %%ymm0, %%ymm0\n"   // sum with temp

        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest
        "vpand %%ymm0, %%ymm4, %%ymm0\n"    // and with lowerMap
        "vmovdqu %%ymm0, (%[temp])\n"       // back in temp

        "add $32, %[a]\n"                   // new pointer for a
        "add $32, %[temp]\n"                // new pointer for b

        "dec %%rbx\n"                       // decrement counter
        "jg 2b\n"                          // if not zero, loop again

        "vmovdqu %%ymm6, (%[temp])\n"       // back in temp

        "mov $12, %%rcx\n"
        "imul $32, %%rcx\n"

        "sub %%rcx, %[a]\n"                 // resetting a
        "sub %%rcx, %[temp]\n"              // resetting temp
        "add $32, %[temp]\n"
        "add $32, %[b]\n"                   // new pointer b

        "dec %%rax\n"                       // decrement counter
        "jg 1b\n"                          // if not zero, loop again
        : [a]"+r" (a), [b]"+r" (b), [upC]"+r" (upC), [lowC]"+r" (lowC), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        , [temp] "+r" (pt)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rax", "rbx", "rcx", "memory"
    );
    pt = &t[0];
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "mov $12, %%rax\n"                  // loop of 12
    "1:\n"
        "mov %%rax, %%rbx\n"                // loop of 12
        "vmovdqu (%[b]), %%ymm1\n"          // second operand in ymm1
        "vpxor %%ymm6, %%ymm6, %%ymm6\n"    // ymm6 for rest
    "2:\n"
        "vmovdqu (%[a]), %%ymm0\n"          // first operand in ymm1
        "vmovdqu (%[temp]), %%ymm2\n"       // temp in ymm2

        "vpmuludq %%ymm1, %%ymm0, %%ymm0\n" // mul first and second operand
        "vpaddq %%ymm6, %%ymm0, %%ymm0\n"   // sum the rest
        "vpaddq %%ymm2, %%ymm0, %%ymm0\n"   // sum with temp

        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest
        "vpand %%ymm0, %%ymm4, %%ymm0\n"    // and with lowerMap
        "vmovdqu %%ymm0, (%[temp])\n"       // back in temp

        "add $32, %[a]\n"                   // new pointer for a
        "add $32, %[temp]\n"                // new pointer for b

        "dec %%rbx\n"                       // decrement counter
        "jg 2b\n"                          // if not zero, loop again

        "mov %%rax, %%rcx\n"
        "imul $32, %%rcx\n"

        "sub %%rcx, %[a]\n"                 // resetting a
        "sub %%rcx, %[temp]\n"              // resetting temp
        "add $32, %[temp]\n"
        "add $32, %[b]\n"                   // new pointer b

        "dec %%rax\n"                       // decrement counter
        "jg 1b\n"                          // if not zero, loop again
        : [a]"+r" (pn0), [b]"+r" (pt), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        , [temp] "+r" (pm)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rax", "rbx", "rcx", "memory"
    );
    pm = &m[0];
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

        "vpmuludq %%ymm1, %%ymm0, %%ymm0\n" // mul first and second operand
        "vpaddq %%ymm6, %%ymm0, %%ymm0\n"   // sum the rest
        "vpaddq %%ymm2, %%ymm0, %%ymm0\n"   // sum with temp

        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest
        "vpand %%ymm0, %%ymm4, %%ymm0\n"    // and with lowerMap
        "vmovdqu %%ymm0, (%[temp])\n"       // back in temp

        "add $32, %[a]\n"                   // new pointer for a
        "add $32, %[temp]\n"                // new pointer for b

        "dec %%rbx\n"                       // decrement counter
        "jg 2b\n"                          // if not zero, loop again

        "vmovdqu %%ymm6, (%[temp])\n"       // back in temp

        "mov $12, %%rcx\n"
        "imul $32, %%rcx\n"

        "sub %%rcx, %[a]\n"                 // resetting a
        "sub %%rcx, %[temp]\n"              // resetting temp
        "add $32, %[temp]\n"
        "add $32, %[b]\n"                   // new pointer b

        "dec %%rax\n"                       // decrement counter
        "jg 1b\n"                          // if not zero, loop again
        : [a]"+r" (pm), [b]"+r" (pprime), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        , [temp] "+r" (pfinalMul)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rax", "rbx", "rcx", "memory"
    );
    pfinalMul = &finalMul[0];
    pt = &t[0];
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "vpxor %%ymm6, %%ymm6, %%ymm6\n"    // ymm6 for rest
        "mov $23, %%rcx\n"                  // loop of 6
    "1:\n"
        "vmovdqu (%[a]), %%ymm0\n"          // first operand in ymm0
        "vmovdqu (%[b]), %%ymm1\n"          // second operand in ymm1

        "vpaddq %%ymm1, %%ymm0, %%ymm0\n"   // sum in ymm0
        "vpaddq %%ymm6, %%ymm0, %%ymm0\n"   // sum the rest

        "vpand %%ymm0, %%ymm4, %%ymm1\n"    // and with lowerMap
        "vmovdqu %%ymm1, (%[c])\n"          // back in lowC
        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest

        "add $32, %[a]\n"                   // new pointers
        "add $32, %[b]\n"
        "add $32, %[c]\n"

        "dec %%rcx\n"                       // decrement counter
        "jge 1b\n"                          // if not zero, loop again
        : [a]"+r" (pfinalMul), [b]"+r" (pt), [c]"+r" (pfinal), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rcx", "memory"
    );
    for (int i = 0; i < 12; i ++) {
        if (i % 2 == 0) {
            lowC[i / 2] = final[i+12];
        } else {
            upC[i / 2] = final[i+12];
        }
    }
}

void mul384SimdMon_ass(four_uint384_t *lowA, four_uint384_t *upA, four_uint384_t *lowB, four_uint384_t *upB,
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
        mul384SimdMon_two_variables_ass(a, b, upC[i].chunk, lowC[i].chunk, upMask, lowMask);
        //checkFourModulo384(upC[i].chunk, lowC[i].chunk);
    }
}
