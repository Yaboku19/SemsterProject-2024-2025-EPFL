#include "../header/generation.h"
#include "../header/struct.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../header/modulo.h"

uint64_t generate_random_64bit2() {
    uint64_t high = (uint64_t)rand() << 32;
    uint64_t low = (uint64_t)rand();
    return high | low;
}

void generate_large_number384(uint384_t *num, int length, unsigned int seed) {
    srand(seed);
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < 6; j++) {
            num[i].chunk[j] = generate_random_64bit2();
        }
        checkModulo384(&num[i]);
    }
}

void generate_number_384_v2(four_uint384_t *num, int length, uint384_t *num2, int mode) {
    if (mode == 0) {
        for(int i = 0; i < length; i++) {
            for (int j = 0; j < 6; j++) {
                for (int k = 0; k < 4; k++) {
                    num[i].chunk[j].chunk[k] = num2[k + (i*4)].chunk[j] >> 32;
                }
            }
        }
    } else if (mode == 1){
        for(int i = 0; i < length; i++) {
            for (int j = 0; j < 6; j++) {
                for (int k = 0; k < 4; k++) {
                    num[i].chunk[j].chunk[k] = num2[k + (i*4)].chunk[j] & 0xFFFFFFFF;
                }
            }
        }
    } else {
        for(int i = 0; i < length; i++) {
            for (int j = 0; j < 6; j++) {
                for (int k = 0; k < 4; k++) {
                    num[i].chunk[j].chunk[k] = 0x0;
                }
            }
        }
    }
    
}
