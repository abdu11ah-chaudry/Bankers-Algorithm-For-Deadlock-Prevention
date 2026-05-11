CC=gcc

CFLAGS=-Wall

SRC=src/main.c src/banker.c src/logger.c src/utils.c

OUT=banker

all:
	$(CC) $(SRC) $(CFLAGS) -o $(OUT)
