#pragma once
#include "struct.h"

void normalSumArray384(uint384_t *a, uint384_t *b, uint384_t *c, int length);

void sequential_sum_ass(uint384_t *a, uint384_t *b, uint384_t *c, int length);

void simd_sum_32_ass_v2(int length, uint384_t_v2 *upA, uint384_t_v2 *lowA, uint384_t_v2 *upB, uint384_t_v2 *lowB, uint384_t_v2 *upC, uint384_t_v2 *lowC,
    uint256_t *upMask, uint256_t *lowMask);