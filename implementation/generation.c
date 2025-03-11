#include "../header/generation.h"
#include "../header/struct.h"
#include <stdio.h>

void generate_large_number64(uint64_t *num, uint64_t *low, uint64_t *high, int length, 
        uint64_t numValue, uint64_t lowValue, uint64_t highValue) {
    for (int i = 0; i < length; i++) {
        num[i] = numValue;
        low[i] = lowValue;
        high[i] = highValue;
    }
}

void generate_large_number384(uint384_t *num, int length, uint64_t chunkValue) {
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < 6; j++) {
            num[i].chunk[j] = chunkValue;
        }
    }
}

void generate_number_384_v2(uint384_t_v2 *num, int length, uint64_t value) {
    for(int i = 0; i < 6; i++) {
        for (int j = 0; j < length; j++) {
            num[i].chunk[j] = value;
        }
    }
}
