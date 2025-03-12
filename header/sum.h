#pragma once
#include "struct.h"

void normalSumArray384(uint384_t *a, uint384_t *b, uint384_t *c, int length);

void sequentialSumArray384_ass(uint384_t *a, uint384_t *b, uint384_t *c, int length);

void simdSumArray384_ass(int length, four_uint384_t *upA, four_uint384_t *lowA, four_uint384_t *upB, four_uint384_t *lowB, 
    four_uint384_t *upC, four_uint384_t *lowC, uint256_t *upMask, uint256_t *lowMask);