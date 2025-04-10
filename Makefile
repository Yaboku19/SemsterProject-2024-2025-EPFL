CC = gcc
CFLAGS = -pg -g -mavx2 -O2 -mavx512f -mavx512dq -I libraries/blst-master
LDFLAGS = -L libraries/blst-master -lblst

SUMMUL = runSumMul.o
SUMMUL_SRC = runSumMul.c
SIGNVERIFY = signVerify.o
SIGNVERIFY_SRC = signVerify.c
TEST = test.o
SRC = implementation/print.c implementation/generation.c implementation/sum.c implementation/multiplication.c implementation/modulo.c
HEADERS = header/struct.h header/print.h header/generation.h header/sum.h header/multiplication.h header/modulo.h

all: runSumMul signVerify

runSumMul: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(SUMMUL) $(SUMMUL_SRC) $(SRC) $(LDFLAGS)

signVerify: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(SIGNVERIFY) $(SIGNVERIFY_SRC) $(SRC) $(LDFLAGS)

clean:
	rm -f $(SUMMUL) $(SIGNVERIFY)
