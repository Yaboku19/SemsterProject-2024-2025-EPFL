#include "../header/sum.h"
#include "../header/struct.h"
#include <stdio.h>

void traditional_sum384(uint384_t *a, uint384_t *b, uint384_t *c, int length) {
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < 6; j++) {
            uint64_t new_value = a[i].chunk[j] + b[i].chunk[j];
            if(new_value < a[i].chunk[j] && j != 5) {
                c[i].chunk[j+1] += 1;
            }
            c[i].chunk[j] += new_value;
        }
    }
}


void sequential_sum_ass(uint384_t *a, uint384_t *b, uint384_t *c, int length) {
    asm volatile(
        "1:\n"
        "   test %[len], %[len]\n"
        "   jle 2f\n"

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

        "   addq $48, %[a]\n"
        "   addq $48, %[b]\n"
        "   addq $48, %[c]\n"
        "   dec %[len]\n"
        "   jnz 1b\n"

        "2:\n"
        : [a]"+r"(a), [b]"+r"(b), [c]"+r"(c), [len]"+r"(length)
        :
        : "rax", "rdx", "memory"
    );
}

void simd_sum_32_ass_v2(int length, uint384_t_v2 *upA, uint384_t_v2 *lowA, uint384_t_v2 *upB, uint384_t_v2 *lowB, uint384_t_v2 *upC, uint384_t_v2 *lowC,
        uint256_t *upMask, uint256_t *lowMask) {
    uint64_t *PupA = upA[5].chunk;
    uint64_t *PlowA = lowA[5].chunk;
    uint64_t *PupB = upB[5].chunk;
    uint64_t *PlowB = lowB[5].chunk;
    uint64_t *PupC = upC[5].chunk;
    uint64_t *PlowC = lowC[5].chunk;
    size_t addLen = length * sizeof(uint64_t);
    size_t restore = (addLen * 7) + 32;
    asm volatile (
        "vmovdqu (%[lowMask]), %%ymm4\n"    // ymm4 for lowMask
        "vmovdqu (%[upMask]), %%ymm5\n"     // ymm5 for upMask
        "mov %[addLen], %%rax\n"
        "mov %[restore], %%rbx\n"
    "1:\n"
        "vpxor %%ymm6, %%ymm6, %%ymm6\n"    // ymm6 for rest
        "mov $6, %%rcx\n"                   // loop of 6
    "2:\n"
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

        "sub %%rax, %[lowA]\n"              // new pointers
        "sub %%rax, %[lowB]\n"
        "sub %%rax, %[upA]\n"
        "sub %%rax, %[upB]\n"
        "sub %%rax, %[lowC]\n"
        "sub %%rax, %[upC]\n"

        "dec %%rcx\n"                       // Decrement counter
        "jge 2b\n"                          //if not zero, loop again

        "add %%rbx, %[lowA]\n"              // new pointers
        "add %%rbx, %[lowB]\n"
        "add %%rbx, %[upA]\n"
        "add %%rbx, %[upB]\n"
        "add %%rbx, %[lowC]\n"
        "add %%rbx, %[upC]\n"

        "sub $4, %[len]\n"
        "jg 1b\n"
        : [len] "+r" (length), [addLen] "+r" (addLen), [restore] "+r" (restore), [upA]"+r" (PupA), [lowA]"+r" (PlowA), [upB]"+r" (PupB), [lowB]"+r" (PlowB), 
        [upC]"+r" (PupC), [lowC]"+r" (PlowC), [upMask]"+r" (upMask), [lowMask]"+r" (lowMask)
        :
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "rcx", "rax", "rbx", "memory"
    );
}
/*
certification:
- OCP / oldCP? (expensive 2k)
- CPTF (400) hack the box
- do more CTF
*/
