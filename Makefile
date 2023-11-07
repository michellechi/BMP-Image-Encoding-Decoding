CC = gcc
CFLAGS = -Wall -ansi -pedantic
DEBUG = -g
RM = rm -f

main: main.c bmp.c header.h
	$(CC) $(CFLAGS) $(DEBUG) main.c bmp.c header.h compress.c decompress.c -lm -o bmp
all: main
clean:
	$(RM) *.o
clear: clean
	$(RM) main

