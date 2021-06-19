.PHONY := build

CC = gcc
CFLAGS = -pthread -lncurses -Wall -Werror -O3
FILE = game.c
OUT = warship

build:
	$(CC) $(FILE) $(CFLAGS) -o $(OUT)

clean:
	rm -rf $(OUT)
