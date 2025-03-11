#include "../header/moltiplication.h"
#include "../header/struct.h"
#include <stdio.h>

void multiply_two_variables_optimized(const uint384_t *a, const uint384_t *b, uint384_t *c) {
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
}

void fast_384_multiplication(uint384_t *a, uint384_t *b, uint384_t *c, int length) {
    for (int i = 0; i < length; i++) {
        multiply_two_variables_optimized(&a[i], &b[i], &c[i]);
    }
}

void multiply_two_variables_kar(const uint384_t *a, const uint384_t *b, uint384_t *c) {
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
}

void multiplication384_kar(uint384_t *a, uint384_t *b, uint384_t *c, int length) {
    for (int i = 0; i < length; i++) {
        multiply_two_variables_optimized(&a[i], &b[i], &c[i]);
    }
}
