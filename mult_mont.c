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
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rax", "rbx", "rcx", "rdx", "memory"
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
        if (++j==n)
            break;
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
    printf("carry: %lx\n\n", carry);

    for (borrow=0, i=0; i<n; i++) {
        limbx = tmp[i] - (p[i] + (__uint128_t )borrow);
        ret[i] = (__uint64_t)limbx;
        borrow = (__uint64_t)(limbx >> 64) & 1;
    }
    printf("ret:\t%16lx", ret[5]);
        for(int i = 4; i > -1; i --) {
            printf("_%16lx", ret[i]);
        }
    printf("\n\n");

    mask = carry - borrow;

    for(i=0; i<n; i++)
        ret[i] = (ret[i] & ~mask) | (tmp[i] & mask);
}

void mul_mont_n_2(vec256 *out, vec256 *a, vec256 *b) {
    vec256 temp[16] = {0}, mx[2] = {0};
    __uint64_t carry;
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
    sub_ass_384(out, temp, prime, &lowMask, &upMask);
    printf("ret:\t%08lx%08lx", out[11][0], out[10][0]);
        for(int i = 9; i > -1; i -= 2) {
            printf("_%08lx%08lx", out[i][0], out[i-1][0]);
        }
    printf("\n\n");
    
    memcpy(out, temp, sizeof(vec256) * 12);
}

int main() {
    /*
        a[0]:    13feb3927ca422fd_49d66ce7036e2ffb_034c68e0d3b8ed55_64a10eb23853b29c_55045b46a393b867_44c0632a86f49ff2
        b[0]:    13feb3927ca422fd_49d66ce7036e2ffb_034c68e0d3b8ed55_64a10eb23853b29c_55045b46a393b867_44c0632a86f49ff2
        out[0]:  1a66d7a7647e9ffd_51cfbbab712f66d4_943b54f1acc5a8f9_76225f7379597509_84fa8e61bc40e4c1_a05a5d6ee59714b5
                 1a66d7a7647e9ffd_51cfbbab712f66d4_943b54f1acc5a8f9_76225f7379597509_84fa8e61bc40e4c1_a05a5d6ee59714b5
    outAfter[0]: 0065c5bd2afeb963_06b413f52de3b9fd_2fc4096cb940963a_0ef18cd282a87ee5_664e8e630aece4c1_e65b5d6ee5976a0a
                 0065c5bd2afeb963_06b413f52de3b9fd_2fc4096cb940963a_0ef18cd282a87ee5_664e8e630aece4c1_e65b5d6ee5976a0a
    */            
    __uint64_t a[6] = {0x44c0632a86f49ff2, 0x55045b46a393b867, 0x64a10eb23853b29c, 0x034c68e0d3b8ed55, 0x49d66ce7036e2ffb, 0x13feb3927ca422fd};
    __uint64_t b[6] = {0x44c0632a86f49ff2, 0x55045b46a393b867, 0x64a10eb23853b29c, 0x034c68e0d3b8ed55, 0x49d66ce7036e2ffb, 0x13feb3927ca422fd};
    __uint64_t p[6] = {0xb9feffffffffaaab, 0x1eabfffeb153ffff, 0x6730d2a0f6b0f624, 0x64774b84f38512bf, 0x4b1ba7b6434bacd7, 0x1a0111ea397fe69a};
    __uint64_t ris[6];
    __uint64_t n0 = 0x89f3fffcfffcfffd;
    mul_mont_n(ris, a, b, p, n0, 6);

    vec256 four_a[13] = {
        {0x86f49ff2, 0x86f49ff2, 0x86f49ff2, 0x86f49ff2},
        {0x44c0632a, 0x44c0632a, 0x44c0632a, 0x44c0632a},
        {0xa393b867, 0xa393b867, 0xa393b867, 0xa393b867},
        {0x55045b46, 0x55045b46, 0x55045b46, 0x55045b46},
        {0x3853b29c, 0x3853b29c, 0x3853b29c, 0x3853b29c},
        {0x64a10eb2, 0x64a10eb2, 0x64a10eb2, 0x64a10eb2},
        {0xd3b8ed55, 0xd3b8ed55, 0xd3b8ed55, 0xd3b8ed55},
        {0x34c68e0, 0x34c68e0, 0x34c68e0, 0x34c68e0},
        {0x36e2ffb, 0x36e2ffb, 0x36e2ffb, 0x36e2ffb},
        {0x49d66ce7, 0x49d66ce7, 0x49d66ce7, 0x49d66ce7},
        {0x7ca422fd, 0x7ca422fd, 0x7ca422fd, 0x7ca422fd},
        {0x13feb392, 0x13feb392, 0x13feb392, 0x13feb392},
        {0x0, 0x0, 0x0, 0x0}
    };

    vec256 four_b[13] = {
        {0x86f49ff2, 0x86f49ff2, 0x86f49ff2, 0x86f49ff2},
        {0x44c0632a, 0x44c0632a, 0x44c0632a, 0x44c0632a},
        {0xa393b867, 0xa393b867, 0xa393b867, 0xa393b867},
        {0x55045b46, 0x55045b46, 0x55045b46, 0x55045b46},
        {0x3853b29c, 0x3853b29c, 0x3853b29c, 0x3853b29c},
        {0x64a10eb2, 0x64a10eb2, 0x64a10eb2, 0x64a10eb2},
        {0xd3b8ed55, 0xd3b8ed55, 0xd3b8ed55, 0xd3b8ed55},
        {0x34c68e0, 0x34c68e0, 0x34c68e0, 0x34c68e0},
        {0x36e2ffb, 0x36e2ffb, 0x36e2ffb, 0x36e2ffb},
        {0x49d66ce7, 0x49d66ce7, 0x49d66ce7, 0x49d66ce7},
        {0x7ca422fd, 0x7ca422fd, 0x7ca422fd, 0x7ca422fd},
        {0x13feb392, 0x13feb392, 0x13feb392, 0x13feb392},
        {0x0, 0x0, 0x0, 0x0}
    };
    vec256 four_ris[13];
    mul_mont_n_2(four_ris, four_a, four_b);
    for(int j = 0; j < 4; j ++) {
        printf("ris[%d]:\t%08lx%08lx", j, four_ris[11][j], four_ris[10][j]);
        for(int i = 9; i > -1; i-=2) {
            printf("_%08lx%08lx", four_ris[i][j], four_ris[i-1][j]);
        }
        printf("\n");
    }
    printf("\n");
    printf("ris:\t%lx", ris[5]);
    for(int i = 4; i > -1; i--) {
        printf("_%lx", ris[i]);
    }
    printf("\n");
}