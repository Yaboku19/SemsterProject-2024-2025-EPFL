## Compilation
gcc -g -o sumOfBigNumber.o sumOfBigNumber.c -mavx2 -O2 -O3 -mavx512f -mavx512dq
gcc -O3 -mavx512f -mavx512dq sumOfBigNumber.c -o sumOfBigNumber

