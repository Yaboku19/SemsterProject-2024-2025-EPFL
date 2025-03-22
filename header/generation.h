#pragma once
#include "struct.h"

void generate_large_number384(uint384_t *num, int length, unsigned int seed);

void generate_number_384_v2(four_uint384_t *num, int length, uint384_t *num2, int mode);