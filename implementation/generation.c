#include "../header/generation.h"
#include "../header/struct.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

uint64_t generate_random_64bit2() {
    uint64_t high = (uint64_t)rand() << 32;
    uint64_t low = (uint64_t)rand();
    return high | low;
}

void generate_large_number384(uint384_t *num, int length) {
    srand(time(NULL));
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < 6; j++) {
            num[i].chunk[j] = generate_random_64bit2();
        }
    }
}

void generate_number_384_v2(uint384_t_v2 *num, int length, uint384_t *num2, int mode) {
    if (mode == 0) {
        for(int i = 0; i < 6; i++) {
            for (int j = 0; j < length; j++) {
                num[i].chunk[j] = (uint64_t)(num2[j].chunk[i] >> 32);
            }
        }
    } else if (mode == 1){
        for(int i = 0; i < 6; i++) {
            for (int j = 0; j < length; j++) {
                num[i].chunk[j] = num2[j].chunk[i] & 0xFFFFFFFF;
            }
        }
    } else {
        for(int i = 0; i < 6; i++) {
            for (int j = 0; j < length; j++) {
                num[i].chunk[j] = 0x0;
            }
        }
    }
    
}
