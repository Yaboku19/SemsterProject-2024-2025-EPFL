/*
 * Copyright Supranational LLC
 * Licensed under the Apache License, Version 2.0, see LICENSE for details.
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BLS12_381_ASM_FIELDS_H__
#define __BLS12_381_ASM_FIELDS_H__

#include "vect.h"
#include "consts.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

vec256 n0[2] = {
    {0xfffcfffd, 0xfffcfffd, 0xfffcfffd, 0xfffcfffd},
    {0x89f3fffc, 0x89f3fffc, 0x89f3fffc, 0x89f3fffc}
};

vec256 prime[12] = {
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
vec256 sub_mod_num[12] = {
    {0x00005555, 0x00005555, 0x00005555, 0x00005555}, // 0x00005555
    {0x46010000, 0x46010000, 0x46010000, 0x46010000}, // 0x46010000
    {0x4eac0000, 0x4eac0000, 0x4eac0000, 0x4eac0000}, // 0x4eac0000
    {0xe1540001, 0xe1540001, 0xe1540001, 0xe1540001}, // 0xe1540001 -
    {0x094f09db, 0x094f09db, 0x094f09db, 0x094f09db}, // 0x094f09db
    {0x98cf2d5f, 0x98cf2d5f, 0x98cf2d5f, 0x98cf2d5f}, // 0x98cf2d5f -
    {0x0c7aed40, 0x0c7aed40, 0x0c7aed40, 0x0c7aed40}, // 0x0c7aed40
    {0x9b88b47b, 0x9b88b47b, 0x9b88b47b, 0x9b88b47b}, // 0x9b88b47b -
    {0xbcb45328, 0xbcb45328, 0xbcb45328, 0xbcb45328}, // 0xbcb45328
    {0xb4e45849, 0xb4e45849, 0xb4e45849, 0xb4e45849}, // 0xb4e45849 -
    {0xc6801965, 0xc6801965, 0xc6801965, 0xc6801965},  // 0xc6801965
    {0xe5feee15, 0xe5feee15, 0xe5feee15, 0xe5feee15}, // 0xe5feee15
};

vec256 upMask = {0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000};
vec256 lowMask = {0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF,0x00000000FFFFFFFF};

/*
 * BLS12-381-specific Fp shortcuts to assembly.
 */
static inline void add_fp(vec384 ret, const vec384 a, const vec384 b)
{   add_mod_384(ret, a, b, BLS12_381_P);   }

void subModulo_v2 (vec256 *num_chunks, const vec384 primeNumber, int *indexes) {
    for (int j = 0; j < 4; j++) {
        if (indexes[j] == -1) {
            break;
        }
        uint64_t carry = 0;
        for (int i = 0; i < 12; i+=2) {
            uint64_t num = ((num_chunks[i+1][indexes[j]] << 32) + num_chunks[i][indexes[j]]);
            uint64_t new_value = num - primeNumber[i/2] - carry;
            if(new_value > num && i != 5) {
                carry = 1;
            } else {
                carry = 0;
            }
            num_chunks[i+1][indexes[j]] = new_value >> 32;
            num_chunks[i][indexes[j]] = new_value & 0xFFFFFFFF;
        }
    }
}

static inline void sub_ass_384 (vec256 *out, vec256 *a, vec256 *b, vec256 *lowMask, vec256 *upMask) {
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "vpxor %%ymm6, %%ymm6, %%ymm6\n"    // ymm6 for rest
        "vpxor %%ymm7, %%ymm7, %%ymm7\n"    // ymm6 for rest
        "mov $11, %%rcx\n"                  // loop of 6
    "1:\n"
        "vmovdqu (%[a]), %%ymm0\n"          // first operand in ymm0
        "vmovdqu (%[b]), %%ymm1\n"          // second operand in ymm1

        "vpsubq %%ymm1, %%ymm0, %%ymm0\n"   // sum in ymm0
        "vpsubq %%ymm6, %%ymm0, %%ymm0\n"   // sum the rest

        "vpand %%ymm0, %%ymm4, %%ymm1\n"    // and with lowerMap
        "vmovdqu %%ymm1, (%[c])\n"          // back in C

        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest

        "vpsubq %%ymm6, %%ymm7, %%ymm6\n"   // shift sign
        "vpand %%ymm6, %%ymm4, %%ymm6\n"    // and with lowerMap

        "add $32, %[a]\n"                   // new pointers
        "add $32, %[b]\n"
        "add $32, %[c]\n"

        "dec %%rcx\n"                       // decrement counter
        "jge 1b\n"                          // if not zero, loop again
        : [a]"+r" (a), [b]"+r" (b), [c] "+r" (out), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "rcx", "memory"
    );
}

void subModulo_v2_sum (vec256 *num_chunks) {
    vec256 num_chunks_nev[12] = {0};
    sub_ass_384(num_chunks_nev, num_chunks, prime, &lowMask, &upMask);
    for (int i = 0; i < 4; i++) {
        if (num_chunks_nev[11][i] < num_chunks[11][i]) {
            for (int j = 0; j < 12; j++) {
                num_chunks[j][i] = num_chunks_nev[j][i];
            }
        }
    }
}

void subModulo_v2_sub (vec256 *num_chunks) {
    vec256 num_chunks_nev[12] = {0};
    sub_ass_384(num_chunks_nev, num_chunks, sub_mod_num, &lowMask, &upMask);
    for (int i = 0; i < 4; i++) {
        if (num_chunks_nev[11][i] < num_chunks[11][i]) {
            for (int j = 0; j < 12; j++) {
                num_chunks[j][i] = num_chunks_nev[j][i];
            }
        }
    }
}

void checkFourModulo384_v2 (vec256 *num_chunks, const vec384 primeNumber, int mode) {
    int indexes[4] = {-1, -1, -1, -1};
    int index = 0;
    for (int j = 0; j < 4; j++) {
        for (int i = 11; i > -1; i-=2) {
            uint64_t num = (num_chunks[i][j] << 32) + num_chunks[i-1][j];
            if (num < primeNumber[i/2]) {
                break;
            } else if (num > primeNumber[i/2]) {
                indexes[index] = j;
                index++;
                break;
            }
        }
    }
    if (indexes[0] != -1) {
        if (mode == 0) {
            subModulo_v2(num_chunks, primeNumber, indexes);
            checkFourModulo384_v2(num_chunks, primeNumber, 0);
        } else {
            vec384 numToSum = {0x4601000000005555, 0xe15400014eac0000, 0x98cf2d5f094f09db, 0x9b88b47b0c7aed40,
                0xb4e45849bcb45328, 0xe5feee15c6801965};
            subModulo_v2(num_chunks, numToSum, indexes);
        }
    }
}

static inline void add_ass_384 (vec256 *out, vec256 *a, vec256 *b, vec256 *lowMask, vec256 *upMask) {
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "vpxor %%ymm6, %%ymm6, %%ymm6\n"    // ymm6 for rest
        "mov $11, %%rcx\n"                  // loop of 6
    "1:\n"
        "vmovdqu (%[a]), %%ymm0\n"          // first operand in ymm0
        "vmovdqu (%[b]), %%ymm1\n"          // second operand in ymm1

        "vpaddq %%ymm1, %%ymm0, %%ymm0\n"   // sum in ymm0
        "vpaddq %%ymm6, %%ymm0, %%ymm0\n"   // sum the rest

        "vpand %%ymm0, %%ymm4, %%ymm1\n"    // and with lowerMap
        "vmovdqu %%ymm1, (%[c])\n"          // back in C
        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest

        "add $32, %[a]\n"                   // new pointers
        "add $32, %[b]\n"
        "add $32, %[c]\n"

        "dec %%rcx\n"                       // decrement counter
        "jge 1b\n"                          // if not zero, loop again
        : [a]"+r" (a), [b]"+r" (b), [c]"+r" (out), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rcx", "memory"
    );
}

static inline void mul_ass_384_fixed_b (vec256 *out, vec256 *a, vec256 *b, vec256 *lowMask, vec256 *upMask) {
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "mov $2, %%rax\n"                  // loop of 2
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
        : [a]"+r" (a), [b]"+r" (b), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        , [temp] "+r" (out)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rax", "rbx", "rcx", "memory"
    );
}

static inline void mul_ass_64 (vec256 *out, vec256 *a, vec256 *b, vec256 *lowMask, vec256 *upMask) {
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "mov $2, %%rax\n"                  // loop of 12
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
        : [a]"+r" (a), [b]"+r" (b), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        , [temp] "+r" (out)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rax", "rbx", "rcx", "memory"
    );
}

static inline void mul_ass_384_fixed_b_shift (vec256 *out, vec256 *a, vec256 *b, vec256 *lowMask, vec256 *upMask) {
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "mov $2, %%rax\n"                  // loop of 2
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
        "add $384, %[temp]\n"
        "vmovdqu (%[temp]), %%ymm0\n"
        "sub $64, %[temp]\n"
        "vmovdqu (%[temp]), %%ymm1\n"
        "vpaddq %%ymm1, %%ymm0, %%ymm0\n"

        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest
        "vpand %%ymm0, %%ymm4, %%ymm0\n"    // and with lowerMap
        "vmovdqu %%ymm0, (%[temp])\n"       // back in temp

        "add $96, %[temp]\n"
        "vmovdqu (%[temp]), %%ymm0\n"
        "sub $64, %[temp]\n"
        "vmovdqu (%[temp]), %%ymm1\n"
        "vpaddq %%ymm1, %%ymm0, %%ymm0\n"
        "vpaddq %%ymm6, %%ymm0, %%ymm0\n"

        "vpand %%ymm0, %%ymm4, %%ymm0\n"    // and with lowerMap
        "vmovdqu %%ymm0, (%[temp])\n"       // back in temp
        : [a]"+r" (a), [b]"+r" (b), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        , [temp] "+r" (out)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rax", "rbx", "rcx", "rdx", "memory"
    );
}

static inline void simd_add_fp(vec256 *out, vec256 *a, vec256 *b) {
    add_ass_384(out, a, b, &lowMask, &upMask);
    subModulo_v2_sum(out);
}

static inline void simd_sub_fp(vec256 *out, vec256 *a, vec256 *b) {
    sub_ass_384(out, a, b, &lowMask, &upMask);
    subModulo_v2_sub(out);
}

static inline void print_fp(vec256 *out, char *str) {
    for(int j = 0; j < 4; j++) {
        printf("%s[%d]:\t %08lx%08lx", str, j, out[11][j], out[10][j]);
        for (int i = 9; i > -1; i-=2) {
            printf("_%08lx%08lx", out[i][j], out[i-1][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void simd_mul_fp(vec256 *out, vec256 *a, vec256 *b) {
    vec256 temp[16] = {0}, mx[2] = {0};

    mul_ass_384_fixed_b(temp, a, b, &lowMask, &upMask);
    mul_ass_64(mx, temp, n0, &lowMask, &upMask);

    for(int j = 0; ;) {
        memmove(temp[14], temp[12], 2 * sizeof(vec256));
        mul_ass_384_fixed_b_shift(temp, prime, mx, &lowMask, &upMask);
        memmove(temp, temp[2], 14 * sizeof(vec256));

        j +=2;
        if (j==12)
            break;
            
        mul_ass_384_fixed_b(temp, a, &b[j], &lowMask, &upMask);
        memset(mx, 0, sizeof(vec256) * 2);
        mul_ass_64(mx, temp, n0, &lowMask, &upMask);
    }
    memcpy(out, temp, sizeof(vec256) * 12);
    checkFourModulo384_v2(out, BLS12_381_P, 0);
}

static inline void sub_fp(vec384 ret, const vec384 a, const vec384 b)
{   sub_mod_384(ret, a, b, BLS12_381_P);   }

static inline void mul_by_3_fp(vec384 ret, const vec384 a)
{   mul_by_3_mod_384(ret, a, BLS12_381_P);   }

static inline void mul_by_8_fp(vec384 ret, const vec384 a)
{   mul_by_8_mod_384(ret, a, BLS12_381_P);   }

static inline void lshift_fp(vec384 ret, const vec384 a, size_t count)
{   lshift_mod_384(ret, a, count, BLS12_381_P);   }

static inline void rshift_fp(vec384 ret, const vec384 a, size_t count)
{   rshift_mod_384(ret, a, count, BLS12_381_P);   }

static inline void div_by_2_fp(vec384 ret, const vec384 a)
{   div_by_2_mod_384(ret, a, BLS12_381_P);   }

static inline void mul_fp(vec384 ret, const vec384 a, const vec384 b)
{   mul_mont_384(ret, a, b, BLS12_381_P, p0);   }

static inline void sqr_fp(vec384 ret, const vec384 a)
{   sqr_mont_384(ret, a, BLS12_381_P, p0);   }

static inline void cneg_fp(vec384 ret, const vec384 a, bool_t flag)
{   cneg_mod_384(ret, a, flag, BLS12_381_P);   }

static inline void from_fp(vec384 ret, const vec384 a)
{   from_mont_384(ret, a, BLS12_381_P, p0);   }

static inline void redc_fp(vec384 ret, const vec768 a)
{   redc_mont_384(ret, a, BLS12_381_P, p0);   }

/*
 * BLS12-381-specific Fp2 shortcuts to assembly.
 */

static inline void print_fp2(fourVec384x out, char *str) {
    for(int j = 0; j < 4; j++) {
        printf("%sx[0][%d]:\t %08lx%08lx", str, j, out[0][11][j], out[0][10][j]);
        for (int i = 9; i > -1; i-=2) {
            printf("_%08lx%08lx", out[0][i][j], out[0][i-1][j]);
        }
        printf("\n");
    }
    printf("\n");
    for(int j = 0; j < 4; j++) {
        printf("%sx[1][%d]:\t %08lx%08lx", str, j, out[1][11][j], out[1][10][j]);
        for (int i = 9; i > -1; i-=2) {
            printf("_%08lx%08lx", out[1][i][j], out[1][i-1][j]);
        }
        printf("\n");
    }
    printf("\n");
}

static inline void add_fp2(vec384x ret, const vec384x a, const vec384x b)
{   add_mod_384x(ret, a, b, BLS12_381_P);   }

static inline void simd_add_fp2(fourVec384x out, fourVec384x a, fourVec384x b) {
    simd_add_fp(out[0], a[0], b[0]);
    simd_add_fp(out[1], a[1], b[1]);
}

static inline void sub_fp2(vec384x ret, const vec384x a, const vec384x b)
{   sub_mod_384x(ret, a, b, BLS12_381_P);   }

static inline void simd_sub_fp2(fourVec384x out, fourVec384x a, fourVec384x b) {
    simd_sub_fp(out[0], a[0], b[0]);
    simd_sub_fp(out[1], a[1], b[1]);
}

static inline void simd_mul_fp2(fourVec384x out, fourVec384x a, fourVec384x b) {
    vec256 four_ac[12], four_bd[12], four_ad[12], four_bc[12];
    simd_mul_fp(four_ac, a[0], b[0]);
    simd_mul_fp(four_bd, a[1], b[1]);
    simd_mul_fp(four_ad, a[0], b[1]);
    simd_mul_fp(four_bc, a[1], b[0]);
    // outR
    simd_sub_fp(out[0], four_ac, four_bd);
    // outI
    simd_add_fp(out[1], four_ad, four_bc);
}

static inline void mul_by_3_fp2(vec384x ret, const vec384x a)
{   mul_by_3_mod_384x(ret, a, BLS12_381_P);   }

static inline void mul_by_8_fp2(vec384x ret, const vec384x a)
{   mul_by_8_mod_384x(ret, a, BLS12_381_P);   }

static inline void lshift_fp2(vec384x ret, const vec384x a, size_t count)
{
    lshift_mod_384(ret[0], a[0], count, BLS12_381_P);
    lshift_mod_384(ret[1], a[1], count, BLS12_381_P);
}

static inline void mul_fp2(vec384x ret, const vec384x a, const vec384x b)
{   mul_mont_384x(ret, a, b, BLS12_381_P, p0);   }

static inline void sqr_fp2(vec384x ret, const vec384x a)
{   sqr_mont_384x(ret, a, BLS12_381_P, p0);   }

static inline void cneg_fp2(vec384x ret, const vec384x a, bool_t flag)
{
    cneg_mod_384(ret[0], a[0], flag, BLS12_381_P);
    cneg_mod_384(ret[1], a[1], flag, BLS12_381_P);
}

#define vec_load_global vec_copy

static void reciprocal_fp(vec384 out, const vec384 inp);
static void flt_reciprocal_fp(vec384 out, const vec384 inp);
static bool_t recip_sqrt_fp(vec384 out, const vec384 inp);
static bool_t sqrt_fp(vec384 out, const vec384 inp);

static void reciprocal_fp2(vec384x out, const vec384x inp);
static void flt_reciprocal_fp2(vec384x out, const vec384x inp);
static bool_t recip_sqrt_fp2(vec384x out, const vec384x inp,
                             const vec384x recip_ZZZ, const vec384x magic_ZZZ);
static bool_t sqrt_fp2(vec384x out, const vec384x inp);
static bool_t sqrt_align_fp2(vec384x out, const vec384x ret,
                             const vec384x sqrt, const vec384x inp);

typedef vec384x   vec384fp2;
typedef vec384fp2 vec384fp6[3];
typedef vec384fp6 vec384fp12[2];

static void sqr_fp12(vec384fp12 ret, const vec384fp12 a);
static void cyclotomic_sqr_fp12(vec384fp12 ret, const vec384fp12 a);
static void mul_fp12(vec384fp12 ret, const vec384fp12 a, const vec384fp12 b);
static void mul_by_xy00z0_fp12(vec384fp12 ret, const vec384fp12 a,
                                               const vec384fp6 xy00z0);
static void conjugate_fp12(vec384fp12 a);
static void inverse_fp12(vec384fp12 ret, const vec384fp12 a);
/* caveat lector! |n| has to be non-zero and not more than 3! */
static void frobenius_map_fp12(vec384fp12 ret, const vec384fp12 a, size_t n);

#define neg_fp(r,a) cneg_fp((r),(a),1)
#define neg_fp2(r,a) cneg_fp2((r),(a),1)

#endif /* __BLS12_381_ASM_FIELDS_H__ */
