#include <stdint.h>
#pragma once

#define MULT_VALUE 10
#define SIZE 1000
#define SIZE_SIMD SIZE / 4 + (SIZE % 4 == 0 ? 0 : 1)
#define NUM_PRINT 10

typedef struct {
    uint64_t low;
    uint64_t high;
} uint128_t;

typedef struct {
    uint64_t chunk[6];
} uint384_t;

typedef struct {
    uint64_t chunk[4];
} uint256_t;

typedef struct {
    uint256_t chunk[6];
} four_uint384_t;