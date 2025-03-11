CC = gcc
CFLAGS = -g -mavx2 -O2 -mavx512f -mavx512dq
TARGET = sumOfBigNumber.o
SRC = sumOfBigNumber.c implementation/print.c implementation/generation.c
HEADERS = header/struct.h header/print.h

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
