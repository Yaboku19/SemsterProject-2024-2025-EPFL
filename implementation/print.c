#include "../header/print.h"
#include "../header/struct.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

void printFunction128(char *functionName, double time, uint128_t num) {
    int totalWidth = 15;
    int nameLength = strlen(functionName);
    int padding = (totalWidth - nameLength) / 2;
    printf("-------------------------------- %*s%*s --------------------------------\n", padding + nameLength, functionName, padding, "");
    printf("- Time (abs): \t%.1f\n", time);
    printf("- Time (sec): \t%.4f\n", time / CLOCKS_PER_SEC);
    printf("- Result: \t0x%016lx_%016lx", num.high, num.low);
    printf("\n");
}

void printFunction384(char *functionName, double time, uint384_t *result) {
    int totalWidth = 15;
    int nameLength = strlen(functionName);
    int padding = (totalWidth - nameLength) / 2;
    printf("-------------------------------- %*s%*s --------------------------------\n", padding + nameLength, functionName, padding, "");
    printf("- Time (abs): \t%.1f\n", time);
    printf("- Time (sec): \t%.4f\n", time / CLOCKS_PER_SEC);
    for(int i = 0; i < 4; i ++) {
        printf("- Result[%d]: \t0x%016lx", i, result[i].chunk[5]);
        for (int j = 4; j >= 0; j--) {
            printf("_%016lx", result[i].chunk[j]);
        }
        printf("\n");
    }

}

void printFunction384_v2(char *functionName, double time, uint384_t_v2 *upC, uint384_t_v2 *lowC) {
    int totalWidth = 15;
    int nameLength = strlen(functionName);
    int padding = (totalWidth - nameLength) / 2;
    printf("-------------------------------- %*s%*s --------------------------------\n", padding + nameLength, functionName, padding, "");
    printf("- Time (abs): \t%.1f\n", time);
    printf("- Time (sec): \t%.4f\n", time / CLOCKS_PER_SEC);
    for(int i = 0; i < 12; i ++) {
        printf("Result[%d] = \t0x%08lx%08lx", i, upC[0].chunk[i], lowC[0].chunk[i]);
        for(int j = 1; j < 6; j++) {
            printf("_%08lx%08lx", upC[j].chunk[i], lowC[j].chunk[i]);
        }
        printf("\n");
    }
    
}
