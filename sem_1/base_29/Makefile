.DEFAULT_GOALS := debug
CC=gcc
main=main.c
debug_out=bin/debug_base29

debug:
	$(CC) $(main) -Wall -o $(debug_out) -lgmp

build:
	$(CC) $(main) -O3 -o bin/base29 -lgmp

run:
	$(debug_out)
