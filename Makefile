CC=gcc
CFLAGS=-Wall -Wextra -std=c99
SRC=src/*.c

build:
	$(CC) $(CFLAGS) $(SRC) -o image_editor -lm

clean:
	rm image_editor

pack:
	zip -FSr 314CA_Nedelcu_Andrei_David_Tema_3_PC.zip src Makefile README
