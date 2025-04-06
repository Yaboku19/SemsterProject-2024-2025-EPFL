#pragma once
#include "struct.h"

void mul384(uint384_t *a, uint384_t *b, uint384_t *c, int length);

void mul384Fast(uint384_t *a, uint384_t *b, uint384_t *c, int length);

void mul384Simd_ass(four_uint384_t *lowA, four_uint384_t *upA, four_uint384_t *lowB, four_uint384_t *upB,
    four_uint384_t *lowC, four_uint384_t *upC, uint256_t *upMask, uint256_t *lowMask, int length);

void mul384SimdMon_ass(four_uint384_t *lowA, four_uint384_t *upA, four_uint384_t *lowB, four_uint384_t *upB,
    four_uint384_t *lowC, four_uint384_t *upC, uint256_t *upMask, uint256_t *lowMask, int length);