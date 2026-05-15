CC=gcc

CFLAGS=-Wall

SRC=solution.c banker.c logger.c utils.c

OUT=banker

all:
	$(CC) $(SRC) $(CFLAGS) -o $(OUT)
