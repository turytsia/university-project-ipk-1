PROJ_D=$(shell pwd)
SRC_D=src
BIN_D=bin
CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c99

ifeq ($(OS),Windows_NT)
run:
	$(CC) $(CFLAGS) $(shell find ./* -name '*.c') -o ipkcpc -lws2_32
else
run:
	$(CC) $(CFLAGS) $(shell find ./* -name '*.c') -o ipkcpc
endif

clean:
	rm ipkcpc