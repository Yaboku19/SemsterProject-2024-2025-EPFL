#include "../header/modulo.h"
#include "../header/struct.h"
#include <stdio.h>

uint384_t primeNumber = {0xFFFFFFFFFFFFFec3, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0FFFFFFFFFFFFFFF};

void subModulo384(uint384_t *num) {
    uint64_t carry = 0;
    for (int i = 0; i < 6; i++) {
        uint64_t new_value = num->chunk[i] - primeNumber.chunk[i] - carry;
        if(new_value > num->chunk[i] && i != 5) {
            carry = 1;
        } else {
            carry = 0;
        }
        num->chunk[i] = new_value;
    }
}

void checkModulo384 (uint384_t *num) {
    for (int i = 5; i > -1; i--) {
        if (num->chunk[i] < primeNumber.chunk[i]) {
            break;
        } else if (num->chunk[i] > primeNumber.chunk[i]) {
            subModulo384(num);
            checkModulo384(num);
        }
    }
}

void subModulo (uint256_t *up, uint256_t *low, int *indexes) {
    for (int j = 0; j < 4; j++) {
        if (indexes[j] == -1) {
            break;
        }
        uint64_t carry = 0;
        for (int i = 0; i < 6; i++) {
            uint64_t num = ((up[i].chunk[indexes[j]] << 32) + low[i].chunk[indexes[j]]);
            uint64_t new_value = num - primeNumber.chunk[i] - carry;
            if(new_value > num && i != 5) {
                carry = 1;
            } else {
                carry = 0;
            }
            up[i].chunk[indexes[j]] = new_value >> 32;
            low[i].chunk[indexes[j]] = new_value & 0xFFFFFFFF;
        }
    }
}

void checkFourModulo384 (uint256_t *up, uint256_t *low) {
    int indexes[4] = {-1, -1, -1, -1};
    int index = 0;
    for (int j = 0; j < 4; j++) {
        for (int i = 5; i > -1; i--) {
            uint64_t num = (up[i].chunk[j] << 32) + low[i].chunk[j];
            if (num < primeNumber.chunk[i]) {
                break;
            } else if (num > primeNumber.chunk[i]) {
                indexes[index] = j;
                index++;
                break;
            }
        }
    }
    if (indexes[0] != -1) {
        subModulo(up, low, indexes);
        checkFourModulo384(up, low);
    }
}