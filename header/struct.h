#include <stdint.h>
#pragma once

#define MULT_VALUE 10
#define SIZE 20000

typedef struct {
    uint64_t low;
    uint64_t high;
} uint128_t;

typedef struct {
    uint64_t chunk[6];
} uint384_t;

typedef struct {
    uint64_t chunk[SIZE + SIZE % 4];
} uint384_t_v2;

typedef struct {
    uint64_t chunk[4];
} uint256_t;
