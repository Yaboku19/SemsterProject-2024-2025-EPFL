#include "../header/print.h"
#include "../header/struct.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

#define DASHES 39

void printFunction128(char *functionName, double time, uint128_t num) {
    char dash[DASHES+1] = "";
    for (int i = 0; i < DASHES; i++) {
        dash[i] = '-';
    }
    printf("%s \t[ %s ]\t %s\n", dash, functionName, dash);
    printf("- Time (abs): \t%.1f\n", time);
    printf("- Time (sec): \t%.4f\n", time / CLOCKS_PER_SEC);
    printf("- Result: \t0x%016lx_%016lx", num.high, num.low);
    printf("\n");
}

void printFunction384(char *functionName, double time, uint384_t *result) {
    char dash[DASHES+1] = "";
    for (int i = 0; i < DASHES; i++) {
        dash[i] = '-';
    }
    printf("%s ", dash);
    printf(strlen(functionName) > 10 ? "\t" : "\t\t");
    printf("[ %s ]", functionName);
    printf(strlen(functionName) > 20 ? "\t" : "\t\t");
    printf("%s\n", dash);
    printf("- Time (abs): \t%.1f\n", time);
    printf("- Time (sec): \t%.4f\n", time / CLOCKS_PER_SEC);
    for(int i = 0; i < NUM_PRINT; i ++) {
        printf("- Result[%d]: \t0x%016lx", i, result[i].chunk[5]);
        for (int j = 4; j >= 0; j--) {
            printf("_%016lx", result[i].chunk[j]);
        }
        printf("\n");
    }

}

void printFunction384_v2(char *functionName, double time, four_uint384_t *upC, four_uint384_t *lowC) {
    char dash[DASHES+1] = "";
    for (int i = 0; i < DASHES; i++) {
        dash[i] = '-';
    }
    printf("%s ", dash);
    printf(strlen(functionName) > 10 ? "\t" : "\t\t");
    printf("[ %s ]", functionName);
    printf(strlen(functionName) > 20 ? "\t" : "\t\t");
    printf("%s\n", dash);
    printf("- Time (abs): \t%.1f\n", time);
    printf("- Time (sec): \t%.4f\n", time / CLOCKS_PER_SEC);
    for(int k = 0; k < (NUM_PRINT / 4) + (NUM_PRINT % 4 == 0 ? 0 : 1); k ++) {
        for(int i = 0; i < 4; i ++) {
            if (i+k*4 >= NUM_PRINT) {
                break;
            }
            printf("- Result[%d] = \t0x%08lx%08lx", i+k*4, upC[k].chunk[5].chunk[i], lowC[k].chunk[5].chunk[i]);
            for(int j = 4; j > -1; j--) {
                printf("_%08lx%08lx", upC[k].chunk[j].chunk[i], lowC[k].chunk[j].chunk[i]);
            }
            printf("\n");
        }
    }
}
