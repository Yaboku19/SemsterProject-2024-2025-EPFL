#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

void mul_mont_n(__uint64_t ret[], const __uint64_t a[], const __uint64_t b[], const __uint64_t p[], __uint64_t n0, size_t n) {
    __uint128_t  limbx;
    __uint64_t mask, borrow, mx, hi, tmp[n+1], carry;
    size_t i, j;

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

    for (borrow=0, i=0; i<n; i++) {
    limbx = tmp[i] - (p[i] + (__uint128_t )borrow);
    ret[i] = (__uint64_t)limbx;
    borrow = (__uint64_t)(limbx >> 64) & 1;
    }

    mask = carry - borrow;

    for(i=0; i<n; i++)
    ret[i] = (ret[i] & ~mask) | (tmp[i] & mask);
}

int main() {
    // _____
    __uint64_t a[6] = {0x3162f2fbf0273ef8, 0x8d115583da564559, 0xe5eaae999a426cde, 0x3788d0280568834e, 0xdc3d01bfd9d0c6b2, 0x07d29785e74ed098};
    __uint64_t b[6] = {0x321300000006554f, 0xb93c0018d6c40005, 0x57605e0db0ddbb51, 0x8b256521ed1f9bcb, 0x6cf28d7901622c03, 0x11ebab9dbb81e28c};
    __uint64_t prime[6] = {0xb9feffffffffaaab, 0x1eabfffeb153ffff, 0x6730d2a0f6b0f624, 0x64774b84f38512bf, 0x4b1ba7b6434bacd7, 0x1a0111ea397fe69a};
    __uint64_t ris[6];
    __uint64_t n0 = 0x89f3fffcfffcfffd;
    mul_mont_n(ris, a, b, prime, n0, 6);
    printf("hello\n");
    printf("ris: %lx", ris[5]);
    for(int i = 4; i > -1; i--) {
        printf("_%lx", ris[i]);
    }
    printf("\n");
}