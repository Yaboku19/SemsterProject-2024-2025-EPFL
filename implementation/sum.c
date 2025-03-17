#include "../header/sum.h"
#include "../header/struct.h"
#include <stdio.h>

uint384_t primeNumber = {0xFFFFFFFFFFFFFec3, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0FFFFFFFFFFFFFFF};

void normalSumTwoVariables384(uint384_t a, uint384_t b, uint384_t *c, int mode);

void checkModulo (uint384_t *num) {
    for (int i = 5; i > -1; i--) {
        if (num->chunk[i] < primeNumber.chunk[i]) {
            break;
        } else if (num->chunk[i] > primeNumber.chunk[i]) {
            normalSumTwoVariables384(*num, primeNumber, num, 1);
            break;
        }
    }
}

void normalSumTwoVariables384(uint384_t a, uint384_t b, uint384_t *c, int mode) {
    if (mode == 0) {
        for (int i = 0; i < 6; i++) {
            uint64_t new_value = a.chunk[i] + b.chunk[i];
            if(new_value < a.chunk[i] && i != 5) {
                c->chunk[i+1] += 1;
            }
            c->chunk[i] += new_value;
        }
        checkModulo(c);
    } else {
        uint64_t carry = 0;
        for (int i = 0; i < 6; i++) {
            uint64_t new_value = a.chunk[i] - b.chunk[i] - carry;
            if(new_value > a.chunk[i] && i != 5) {
                carry = 1;
            } else {
                carry = 0;
            }
            c->chunk[i] = new_value;
        }
        checkModulo(c);
    }

}

void normalSumArray384(uint384_t *a, uint384_t *b, uint384_t *c, int length) {
    for (int i = 0; i < length; i++) {
        normalSumTwoVariables384(a[i], b[i], &c[i], 0);
    }
}

void sequentialSumTwoVariables384_ass(uint384_t *a, uint384_t *b, uint384_t *c) {
    asm volatile(
        "   movq %[c], %%rdx\n" 

        "   movq (%[a]), %%rax\n"
        "   addq (%[b]), %%rax\n"
        "   movq %%rax, (%%rdx)\n"

        "   movq 8(%[a]), %%rax\n"
        "   adcq 8(%[b]), %%rax\n"
        "   movq %%rax, 8(%%rdx)\n"

        "   movq 16(%[a]), %%rax\n"
        "   adcq 16(%[b]), %%rax\n"
        "   movq %%rax, 16(%%rdx)\n"

        "   movq 24(%[a]), %%rax\n"
        "   adcq 24(%[b]), %%rax\n"
        "   movq %%rax, 24(%%rdx)\n"

        "   movq 32(%[a]), %%rax\n"
        "   adcq 32(%[b]), %%rax\n"
        "   movq %%rax, 32(%%rdx)\n"

        "   movq 40(%[a]), %%rax\n"
        "   adcq 40(%[b]), %%rax\n"
        "   movq %%rax, 40(%%rdx)\n"
        : [a]"+r"(a), [b]"+r"(b), [c]"+r"(c)
        :
        : "rax", "rdx", "memory"
    );
    checkModulo(c);
}

void sequentialSumArray384_ass(uint384_t *a, uint384_t *b, uint384_t *c, int length) {
    for (int i = 0; i < length; i++) {
        sequentialSumTwoVariables384_ass(&a[i], &b[i], &c[i]);
    }
}

void checkFourModulo (uint256_t *up, uint256_t *low) ;

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
    checkFourModulo(up, low);
}

void checkFourModulo (uint256_t *up, uint256_t *low) {
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
    }
}

void simdTwoVariables384_ass(uint256_t *upA, uint256_t *lowA, uint256_t *upB, uint256_t *lowB, 
        uint256_t *upC, uint256_t *lowC, uint256_t *upMask, uint256_t *lowMask) {
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "vpxor %%ymm6, %%ymm6, %%ymm6\n"    // ymm6 for rest
        "mov $6, %%rcx\n"                   // loop of 6
    "1:\n"
        "vmovdqu (%[lowA]), %%ymm0\n"       // first operand in ymm0
        "vmovdqu (%[lowB]), %%ymm1\n"       // second operand in ymm1

        "vpaddq %%ymm1, %%ymm0, %%ymm0\n"   // sum in ymm0
        "vpaddq %%ymm6, %%ymm0, %%ymm0\n"   // sum the rest

        "vpand %%ymm0, %%ymm4, %%ymm1\n"    // and with lowerMap
        "vmovdqu %%ymm1, (%[lowC])\n"       // back in lowC
        "vpand %%ymm0, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest

        "vmovdqu (%[upA]), %%ymm1\n"        // first operand in ymm1
        "vmovdqu (%[upB]), %%ymm2\n"        // second operand in ymm2

        "vpaddq %%ymm1, %%ymm2, %%ymm1\n"   // sum in ymm1
        "vpaddq %%ymm1, %%ymm6, %%ymm1\n"   // sum the rest

        "vpand %%ymm1, %%ymm4, %%ymm2\n"    // and with lowerMap
        "vmovdqu %%ymm2, (%[upC])\n"        // back in upC
        "vpand %%ymm1, %%ymm5, %%ymm6\n"    // and with upMask
        "vpsrlq $32, %%ymm6, %%ymm6\n"      // new rest

        "add $32, %[lowA]\n"                     // new pointers
        "add $32, %[lowB]\n"
        "add $32, %[upA]\n"
        "add $32, %[upB]\n"
        "add $32, %[lowC]\n"
        "add $32, %[upC]\n"

        "dec %%rcx\n"                       // decrement counter
        "jge 1b\n"                          // if not zero, loop again
        "sub $224, %[lowC]\n"
        "sub $224, %[upC]\n"
        : [upA]"+r" (upA), [lowA]"+r" (lowA), [upB]"+r" (upB), [lowB]"+r" (lowB), 
        [upC]"+r" (upC), [lowC]"+r" (lowC), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rcx", "memory"
    );
    checkFourModulo(upC, lowC);
}

void simdSumArray384_ass(int length, four_uint384_t *upA, four_uint384_t *lowA, four_uint384_t *upB, four_uint384_t *lowB, 
        four_uint384_t *upC, four_uint384_t *lowC, uint256_t *upMask, uint256_t *lowMask) {
    for (int i = 0; i < length; i++) {
        simdTwoVariables384_ass(upA[i].chunk, lowA[i].chunk, upB[i].chunk, lowB[i].chunk,
            upC[i].chunk, lowC[i].chunk, upMask, lowMask);
    }
}
/*
certification:
- OCP / oldCP? (expensive 2k)
- CPTF (400) hack the box
- do more CTF
*/
/*
1111111 -
0100100 =
--------
1011011 -
0100100 =
--------
0010011
*/