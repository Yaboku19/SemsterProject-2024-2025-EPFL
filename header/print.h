#pragma once
#include "struct.h"

void printFunction128(char *functionName, double time, uint128_t num);

void printFunction384(char *functionName, double time, uint384_t *result);

void printFunction384_v2(char *functionName, double time, four_uint384_t *upC, four_uint384_t *lowC);