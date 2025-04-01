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

/*
 * BLS12-381-specific Fp shortcuts to assembly.
 */
static inline void add_fp(vec384 ret, const vec384 a, const vec384 b)
{   add_mod_384(ret, a, b, BLS12_381_P);   }

void subModulo_l (vec256 *up, vec256 *low, const vec384 primeNumber, int *indexes) {
    for (int j = 0; j < 4; j++) {
        if (indexes[j] == -1) {
            break;
        }
        uint64_t carry = 0;
        for (int i = 0; i < 6; i++) {
            uint64_t num = ((up[i][indexes[j]] << 32) + low[i][indexes[j]]);
            uint64_t new_value = num - primeNumber[i] - carry;
            if(new_value > num && i != 5) {
                carry = 1;
            } else {
                carry = 0;
            }
            up[i][indexes[j]] = new_value >> 32;
            low[i][indexes[j]] = new_value & 0xFFFFFFFF;
        }
    }
}

void add_Modulo_l (vec256 *up, vec256 *low, const vec384 primeNumber, int *indexes) {
    for (int j = 0; j < 4; j++) {
        if (indexes[j] == -1) {
            break;
        }
        uint64_t carry = 0;
        for (int i = 0; i < 6; i++) {
            uint64_t num = ((up[i][indexes[j]] << 32) + low[i][indexes[j]]);
            uint64_t new_value = num + primeNumber[i] + carry;
            if(new_value < num - carry && i != 5) {
                carry = 1;
            } else {
                carry = 0;
            }
            up[i][indexes[j]] = new_value >> 32;
            low[i][indexes[j]] = new_value & 0xFFFFFFFF;
        }
    }
}

void checkFourModulo384_l (vec256 *up, vec256 *low, const vec384 primeNumber, int mode) {
    int indexes[4] = {-1, -1, -1, -1};
    int index = 0;
    for (int j = 0; j < 4; j++) {
        for (int i = 5; i > -1; i--) {
            uint64_t num = (up[i][j] << 32) + low[i][j];
            if (num < primeNumber[i]) {
                break;
            } else if (num > primeNumber[i]) {
                indexes[index] = j;
                index++;
                break;
            }
        }
    }
    if (indexes[0] != -1) {
        if (mode == 0) {
            subModulo_l(up, low, primeNumber, indexes);
        } else {
            add_Modulo_l(up, low, primeNumber, indexes);
        }
        checkFourModulo384_l(up, low, primeNumber, mode);
    }
}

static inline void simd_add_fp(vec384 *out, vec384 *a, vec384 *b) {
    vec256 four_a_up[6], four_a_low[6], four_b_up[6], four_b_low[6], four_out_low[6] = {0}, four_out_up[6] = {0};
    vec256 upMask = {0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000};
    vec256 lowMask = {0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF,0x00000000FFFFFFFF};
    vec256 *lowA = &four_a_low[0], *lowB = &four_b_low[0], *lowC = &four_out_low[0];
    vec256 *upA = &four_a_up[0], *upB = &four_b_up[0], *upC = &four_out_up[0];
    vec256 *pupMask = &upMask, *plowMask = &lowMask;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; j++) {
            four_a_up[i][j] = a[j][i] >> 32;
            four_a_low[i][j] = a[j][i] & 0xFFFFFFFF;
            four_b_up[i][j] = b[j][i] >> 32;
            four_b_low[i][j] = b[j][i] & 0xFFFFFFFF;
        }
    }
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "vpxor %%ymm6, %%ymm6, %%ymm6\n"    // ymm6 for rest
        "mov $5, %%rcx\n"                   // loop of 6
    "1:\n"
        "vmovdqu (%[lowA]), %%ymm0\n"       // first operand in ymm0
        "vmovdqu (%[lowB]), %%ymm1\n"       // second operand in ymm1

        "vpaddq %%ymm1, %%ymm0, %%ymm0\n"   // sum in ymm0
        "vpaddq %%ymm6, %%ymm0, %%ymm0\n"   // sum the rest

        "vpand %%ymm0, %%ymm4, %%ymm1\n"    // and with lowerMap
        "vmovdqu %%ymm1, (%[lowC])\n"       // back in lowC
        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest

        "vmovdqu (%[upA]), %%ymm1\n"        // first operand in ymm1
        "vmovdqu (%[upB]), %%ymm2\n"        // second operand in ymm2

        "vpaddq %%ymm1, %%ymm2, %%ymm1\n"   // sum in ymm1
        "vpaddq %%ymm1, %%ymm6, %%ymm1\n"   // sum the rest

        "vpand %%ymm1, %%ymm4, %%ymm2\n"    // and with lowerMap
        "vmovdqu %%ymm2, (%[upC])\n"        // back in upC
        "vpand %%ymm1, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest

        "add $32, %[lowA]\n"                     // new pointers
        "add $32, %[lowB]\n"
        "add $32, %[upA]\n"
        "add $32, %[upB]\n"
        "add $32, %[lowC]\n"
        "add $32, %[upC]\n"

        "dec %%rcx\n"                       // decrement counter
        "jge 1b\n"                          // if not zero, loop again
        : [upA]"+r" (upA), [lowA]"+r" (lowA), [upB]"+r" (upB), [lowB]"+r" (lowB), 
        [upC]"+r" (upC), [lowC]"+r" (lowC), [upMask]"+r" (pupMask), [lowMask]"+r" (plowMask)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rcx", "memory"
    );
    checkFourModulo384_l(four_out_up, four_out_low, BLS12_381_P, 0);
    for (int i = 0; i < 4; i++) {
        out[i][0] = four_out_low[0][i] | (four_out_up[0][i] << 32);
        out[i][1] = four_out_low[1][i] | (four_out_up[1][i] << 32);
        out[i][2] = four_out_low[2][i] | (four_out_up[2][i] << 32);
        out[i][3] = four_out_low[3][i] | (four_out_up[3][i] << 32);
        out[i][4] = four_out_low[4][i] | (four_out_up[4][i] << 32);
        out[i][5] = four_out_low[5][i] | (four_out_up[5][i] << 32);
    }
}

static inline void simd_mul_fp(vec384 *out, vec384 *a, vec384 *b) {
    vec256 four_a[12], four_b[12], four_out_low[6] = {0}, four_out_up[6] = {0}, tempArray[12] = {0};
    vec256 upMask = {0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000};
    vec256 lowMask = {0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF,0x00000000FFFFFFFF};
    vec256 *pA = &four_a[0], *pB = &four_b[0], *lowC = &four_out_low[0], *upC = &four_out_up[0], *temp = &tempArray[0];
    vec256 *pupMask = &upMask, *plowMask = &lowMask;
    for (int i = 0; i < 6; i++) {
        four_a[i * 2][0] = a[0][i] & 0xFFFFFFFF;
        four_a[i * 2][1] = a[1][i] & 0xFFFFFFFF;
        four_a[i * 2][2] = a[2][i] & 0xFFFFFFFF;
        four_a[i * 2][3] = a[3][i] & 0xFFFFFFFF;
        four_a[i * 2 + 1][0] = a[0][i] >> 32;
        four_a[i * 2 + 1][1] = a[1][i] >> 32;
        four_a[i * 2 + 1][2] = a[2][i] >> 32;
        four_a[i * 2 + 1][3] = a[3][i] >> 32;
        four_b[i * 2][0] = b[0][i] & 0xFFFFFFFF;
        four_b[i * 2][1] = b[1][i] & 0xFFFFFFFF;
        four_b[i * 2][2] = b[2][i] & 0xFFFFFFFF;
        four_b[i * 2][3] = b[3][i] & 0xFFFFFFFF;
        four_b[i * 2 + 1][0] = b[0][i] >> 32;
        four_b[i * 2 + 1][1] = b[1][i] >> 32;
        four_b[i * 2 + 1][2] = b[2][i] >> 32;
        four_b[i * 2 + 1][3] = b[3][i] >> 32;
    }
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
        : [a]"+r" (pA), [b]"+r" (pB), [upC]"+r" (upC), [lowC]"+r" (lowC), [upMask]"+r" (pupMask), [lowMask]"+r" (plowMask)
        , [temp] "+r" (temp)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rax", "rbx", "rcx", "memory"
    );
    for (int i = 0; i < 12; i ++) {
        if (i % 2 == 0) {
            four_out_low[i / 2][0] = tempArray[i][0];
            four_out_low[i / 2][1] = tempArray[i][1];
            four_out_low[i / 2][2] = tempArray[i][2];
            four_out_low[i / 2][3] = tempArray[i][3];
        } else {
            four_out_up[i / 2][0] = tempArray[i][0];
            four_out_up[i / 2][1] = tempArray[i][1];
            four_out_up[i / 2][2] = tempArray[i][2];
            four_out_up[i / 2][3] = tempArray[i][3];
        }
    }
    checkFourModulo384_l(four_out_up, four_out_low, BLS12_381_P, 0);
    for (int i = 0; i < 4; i++) {
        out[i][0] = four_out_low[0][i] | (four_out_up[0][i] << 32);
        out[i][1] = four_out_low[1][i] | (four_out_up[1][i] << 32);
        out[i][2] = four_out_low[2][i] | (four_out_up[2][i] << 32);
        out[i][3] = four_out_low[3][i] | (four_out_up[3][i] << 32);
        out[i][4] = four_out_low[4][i] | (four_out_up[4][i] << 32);
        out[i][5] = four_out_low[5][i] | (four_out_up[5][i] << 32);
    }
}

static inline void print_fp(vec384 *out) {
    for(int j = 0; j < 4; j++) {
        printf("out[%d]:\t %016lx", j, out[j][5]);
        for (int i = 4; i > -1; i--) {
            printf("_%016lx", out[j][i]);
        }
        printf("\n");
    }
    printf("\n");
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
static inline void add_fp2(vec384x ret, const vec384x a, const vec384x b)
{   add_mod_384x(ret, a, b, BLS12_381_P);   }

static inline void simd_add_fp2(vec384x *out, vec384x *a, vec384x *b) {
    for(int j = 0; j < 4; j++) {
        printf("ax[%d]:\t %llx", j, a[j][0][0]);
        for (int i = 1; i < 6; i++) {
            printf("_%llx", a[j][0][i]);
        }
        printf("\n");
    }
    printf("\n");
    for(int j = 0; j < 4; j++) {
        printf("bx[%d]:\t %llx", j, b[j][0][0]);
        for (int i = 1; i < 6; i++) {
            printf("_%llx", b[j][0][i]);
        }
        printf("\n");
    }
    printf("\n");
    for(int j = 0; j < 4; j++) {
        printf("outx[%d]:\t %llx", j, out[j][0][0]);
        for (int i = 1; i < 6; i++) {
            printf("_%llx", out[j][0][i]);
        }
        printf("\n");
    }
    printf("\n"); 
}

static inline void print_fp2(vec384x *out) {
    for(int j = 0; j < 4; j++) {
        printf("outx[%d]:\t %llx", j, out[j][0][0]);
        for (int i = 1; i < 6; i++) {
            printf("_%llx", out[j][0][i]);
        }
        printf("\n");
    }
}

static inline void sub_fp2(vec384x ret, const vec384x a, const vec384x b)
{   sub_mod_384x(ret, a, b, BLS12_381_P);   }

static inline void simd_mul_fp2(vec384x *out, vec384x *a, vec384x *b) {
    out = a;
    a = b;
    b = out;
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
