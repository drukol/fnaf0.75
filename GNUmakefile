# 

CC = gcc
CFLAGS = -Wall -ansi
LDFLAGS = -lSDL2main -lSDL2

all: main.o
	$(CC) -o fnaf34.out main.o $(CFLAGS) $(LDFLAGS)

fnaf34.out: main.o
	$(CC) -o fnaf34.out main.o $(CFLAGS) $(LDFLAGS)

main.o: main.c
	$(CC) -c main.c $(CFLAGS)

run: all fnaf34.out
	./fnaf34.out

clear:
	rm main.o
	rm fnaf34.out
