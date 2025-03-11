#pragma once
#include "struct.h"

void generate_large_number64(uint64_t *num, uint64_t *low, uint64_t *high, int length, 
    uint64_t numValue, uint64_t lowValue, uint64_t highValue);

void generate_large_number384(uint384_t *num, int length, uint64_t chunkValue);

void generate_number_384_v2(uint384_t_v2 *num, int length, uint64_t value);