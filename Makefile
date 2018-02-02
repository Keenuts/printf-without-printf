CC=clang
CFLAGS=-g -O0

all: main

main: main.o
	$(CC) $(CFLAGS) $< -o $@
