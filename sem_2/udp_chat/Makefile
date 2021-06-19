CC = gcc
CFLAGS = -lpthread -O2 -g -Wall
DEBUG = TEST_FLAG

all: release

debug:
	@echo "Debug version..."
	$(CC) chatd.c $(CFLAGS) -D $(DEBUG) -o server

release:
	@echo "Release version..."
	$(CC) chatd.c $(CFLAGS) -o server

