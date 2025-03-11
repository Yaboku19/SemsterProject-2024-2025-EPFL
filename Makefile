CC = gcc
CFLAGS = -g -mavx2 -O2 -mavx512f -mavx512dq
TARGET = main.o
SRC = main.c implementation/print.c implementation/generation.c implementation/sum.c implementation/moltiplication.c
HEADERS = header/struct.h header/print.h header/generation.h header/sum.h header/moltiplication.h

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
