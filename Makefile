CC=clang
CFLAGS=-g -O0

all: main

main: utils.c elf-utils.o main.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) main *.o
