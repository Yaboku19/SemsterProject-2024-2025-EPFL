#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

typedef __uint64_t vec256[4];

vec256 upMask = {0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000};
vec256 lowMask = {0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF,0x00000000FFFFFFFF};
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

vec256 n0[2] = {
    {0xfffcfffd, 0xfffcfffd, 0xfffcfffd, 0xfffcfffd},
    {0x89f3fffc, 0x89f3fffc, 0x89f3fffc, 0x89f3fffc}
};

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

static inline void mul_ass_384_fixed_b_shift (vec256 *out, vec256 *a, vec256 *b, vec256 *lowMask, vec256 *upMask, vec256 *carry) {
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

        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest
        "vpand %%ymm0, %%ymm4, %%ymm0\n"    // and with lowerMap
        "vmovdqu %%ymm0, (%[temp])\n"       // back in temp
        "vmovdqu %%ymm6, (%[carry])\n"
        : [a]"+r" (a), [b]"+r" (b), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        , [temp] "+r" (out), [carry] "+r" (carry)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rax", "rbx", "rcx", "rdx", "memory"
    );
}

void mul_mont_n(__uint64_t ret[], const __uint64_t a[], const __uint64_t b[], const __uint64_t p[], __uint64_t n0, size_t n) {
    __uint128_t  limbx;
    __uint64_t mask, borrow, mx, hi, tmp[n+1], carry;
    size_t i, j;

    int condtion = 1;
    for(int i = 0; i < 6; i++) {
        if (a[i] != 0 ) {
            condtion = 0;
            break;
        }
    }
    if (condtion) {
        memcpy(ret, b, sizeof(__uint64_t) * 6);
        return;
    }

    for (mx=b[0], hi=0, i=0; i<n; i++) {
        limbx = (mx * (__uint128_t )a[i]) + hi;
        tmp[i] = (__uint64_t)limbx;
        hi = (__uint64_t)(limbx >> 64);
    }
    mx = n0*tmp[0];
    tmp[i] = hi;
    for (carry=0, j=0; ; ) {
        if (j >= 3) {
            printf("ris[%lx]:\t%lx",j, tmp[5]);
            for(int i = 4; i > -1; i--) {
                printf("_%lx", tmp[i]);
            }
            printf("\n\n");
        }
        limbx = (mx * (__uint128_t )p[0]) + tmp[0];
        hi = (__uint64_t)(limbx >> 64);
        for (i=1; i<n; i++) {
            limbx = (mx * (__uint128_t )p[i] + hi) + tmp[i];
            tmp[i-1] = (__uint64_t)limbx;
            hi = (__uint64_t)(limbx >> 64);
        }
        limbx = tmp[i] + (hi + (__uint128_t )carry);
        tmp[i-1] = (__uint64_t)limbx;
        carry = (__uint64_t)(limbx >> 64);
        if (j >= 3) {
            printf("ris2[%lx]:\t%lx",j, tmp[5]);
            for(int i = 4; i > -1; i--) {
                printf("_%lx", tmp[i]);
            }
            printf("\n\n");
        }
        if (++j==n)
            break;
        printf("b[%lx] = %lx\n",j, b[j]);
        for (mx=b[j], hi=0, i=0; i<n; i++) {
            limbx = (mx * (__uint128_t )a[i] + hi) + tmp[i];
            tmp[i] = (__uint64_t)limbx;
            hi = (__uint64_t)(limbx >> 64);
        }
        
        mx = n0*tmp[0];
        limbx = hi + (__uint128_t )carry;
        tmp[i] = (__uint64_t)limbx;
        carry = (__uint64_t)(limbx >> 64);

    }

    for (borrow=0, i=0; i<n; i++) {
        limbx = tmp[i] - (p[i] + (__uint128_t )borrow);
        ret[i] = (__uint64_t)limbx;
        borrow = (__uint64_t)(limbx >> 64) & 1;
    }

    mask = carry - borrow;

    for(i=0; i<n; i++)
        ret[i] = (ret[i] & ~mask) | (tmp[i] & mask);
}

static inline int isZero(vec256 *a) {
    int condtion = 1;
    for(int j = 0; j < 4; j++) {
        for(int i = 0; i < 6; i++) {
            if (a[i] != 0 ) {
                condtion = 0;
                break;
            }
        }
    }
    return condtion;
}

void mul_mont_n_2(vec256 *out, vec256 *a, vec256 *b) {
    size_t n = 12;
    vec256 temp[16] = {0}, carry = {0}, mx[2] = {0};

    mul_ass_384_fixed_b(temp, a, b, &lowMask, &upMask);
    mul_ass_64(mx, temp, n0, &lowMask, &upMask);

    for(int j = 0; ;) {
        memmove(temp[14], temp[12], 2 * sizeof(vec256));
        mul_ass_384_fixed_b_shift(temp, prime, mx, &lowMask, &upMask, &carry);
        memmove(temp, temp[2], 14 * sizeof(vec256));

        j +=2;
        if (j==n)
            break;

        mul_ass_384_fixed_b(temp, a, &b[j], &lowMask, &upMask);
        memset(mx, 0, sizeof(vec256) * 2);
        mul_ass_64(mx, temp, n0, &lowMask, &upMask);
    }
    memcpy(out, temp, sizeof(vec256) * 12);
}

int main() {
    //  12411094956edf70_e6da76eb26ef60dd_98bebf354dced295_5f6530e63fa0875a_fef39853a08661b2_e5e07fe36d6b0a9e    
    //  12411094956edf70_e6da76eb26ef60dd_98bebf354dced295_5f6530e63fa0875a_fef39853a08661b2_e5e07fe36d6b0a9e              
    __uint64_t a[6] = {0x5d7160ffc1915ece, 0x3d1e9cf5b265b226, 0x1cfdd1c2489dfd3e, 0x450caf0ac57b2c3, 0x7936d83e186ca72c, 0x1972b7d81caacb3c};
    __uint64_t b[6] = {0x461620bd9e4841bb, 0xea93fe24d4641e9e, 0xcbe9de6cbb7d4e83, 0xf55c161b20f05f91, 0xd32185e0b7f86ac, 0x73ba60324611aaa};
    __uint64_t p[6] = {0xb9feffffffffaaab, 0x1eabfffeb153ffff, 0x6730d2a0f6b0f624, 0x64774b84f38512bf, 0x4b1ba7b6434bacd7, 0x1a0111ea397fe69a};
    __uint64_t ris[6];
    __uint64_t n0 = 0x89f3fffcfffcfffd;
    mul_mont_n(ris, a, b, p, n0, 6);

    vec256 four_a[13] = {
        {0xc1915ece, 0xc1915ece, 0xc1915ece, 0xc1915ece},
        {0x5d7160ff, 0x5d7160ff, 0x5d7160ff, 0x5d7160ff},
        {0xb265b226, 0xb265b226, 0xb265b226, 0xb265b226},
        {0x3d1e9cf5, 0x3d1e9cf5, 0x3d1e9cf5, 0x3d1e9cf5},
        {0x489dfd3e, 0x489dfd3e, 0x489dfd3e, 0x489dfd3e},
        {0x1cfdd1c2, 0x1cfdd1c2, 0x1cfdd1c2, 0x1cfdd1c2},
        {0xac57b2c3, 0xac57b2c3, 0xac57b2c3, 0xac57b2c3},
        {0x0450caf0, 0x0450caf0, 0x0450caf0, 0x0450caf0},
        {0x186ca72c, 0x186ca72c, 0x186ca72c, 0x186ca72c},
        {0x7936d83e, 0x7936d83e, 0x7936d83e, 0x7936d83e},
        {0x1caacb3c, 0x1caacb3c, 0x1caacb3c, 0x1caacb3c},
        {0x1972b7d8, 0x1972b7d8, 0x1972b7d8, 0x1972b7d8},
        {0x0, 0x0, 0x0, 0x0}
    };

    vec256 four_b[13] = {
        {0x9e4841bb, 0x9e4841bb, 0x9e4841bb, 0x9e4841bb},
        {0x461620bd, 0x461620bd, 0x461620bd, 0x461620bd},
        {0xd4641e9e, 0xd4641e9e, 0xd4641e9e, 0xd4641e9e},
        {0xea93fe24, 0xea93fe24, 0xea93fe24, 0xea93fe24},
        {0xbb7d4e83, 0xbb7d4e83, 0xbb7d4e83, 0xbb7d4e83},
        {0xcbe9de6c, 0xcbe9de6c, 0xcbe9de6c, 0xcbe9de6c},
        {0x20f05f91, 0x20f05f91, 0x20f05f91, 0x20f05f91},
        {0xf55c161b, 0xf55c161b, 0xf55c161b, 0xf55c161b},
        {0x0b7f86ac, 0x0b7f86ac, 0x0b7f86ac, 0x0b7f86ac},
        {0xd32185e0, 0xd32185e0, 0xd32185e0, 0xd32185e0},
        {0x24611aaa, 0x24611aaa, 0x24611aaa, 0x24611aaa},
        {0x73ba603, 0x73ba603, 0x73ba603, 0x73ba603},
        {0x0, 0x0, 0x0, 0x0}
    };
    vec256 four_ris[13];
    mul_mont_n_2(four_ris, four_a, four_b);
    // for(int j = 0; j < 4; j ++) {
    //     printf("ris[%d]:\t%08lx%08lx", j, four_ris[11][j], four_ris[10][j]);
    //     for(int i = 9; i > -1; i-=2) {
    //         printf("_%08lx%08lx", four_ris[i][j], four_ris[i-1][j]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");
    // printf("ris:\t%lx", ris[5]);
    // for(int i = 4; i > -1; i--) {
    //     printf("_%lx", ris[i]);
    // }
    // printf("\n");
}