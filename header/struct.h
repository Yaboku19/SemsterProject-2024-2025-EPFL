#include <stdint.h>

#define MULT_VALUE 10
#define SIZE 10

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

typedef struct {
    uint64_t low[6];
    uint64_t high[6];
} uint384_t2;