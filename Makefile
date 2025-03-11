CC = gcc
CFLAGS = -g -mavx2 -O2 -mavx512f -mavx512dq
TARGET = sumOfBigNumber.o
SRC = sumOfBigNumber.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
