CC=gcc
BFLAGS=-Wall -Wextra -Werror -ffast-math -march=native
DEBUG_FLAGS=$(BFLAGS) -g
RFLAGS=$(BFLAGS) -O3 -s
LIBS=-lpthread -lncurses
BUILD=../bin
SRC=./src
ADD_FLAG=VPN_NETWORK
FNAME=game
DEPENDIES = game.c server.c logic.c

all: release

debug: $(DEPENDIES)
	$(CC) $^ $(DEBUG_FLAGS) -D $(ADD_FLAG) $(LIBS) -o $(BUILD)/$(FNAME)

local: $(DEPENDIES)
	$(CC) $^ $(RFLAGS) -D $(ADD_FLAG) $(LIBS) -o $(BUILD)/$(FNAME)

release: $(DEPENDIES)
	$(CC) $^ $(RFLAGS) $(LIBS) -o $(BUILD)/$(FNAME)

