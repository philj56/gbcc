CC=gcc
CFLAGS=-Wall -Wextra -pedantic -O2

all: romgen bin2screen

romgen: romgen.c
	$(CC) -o $@ $< $(CFLAGS)

bin2screen: bin2screen.c
	$(CC) -o $@ $< $(CFLAGS)
