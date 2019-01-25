CFLAGS = -g -Wall
CC = gcc

all: RoundBuffer.o producent.o konsument.o
	$(CC) $(CFLAGS) RoundBuffer.o producent.o -o test
	$(CC) $(CFLAGS) RoundBuffer.o konsument.o -o testKons

RoundBuffer.o: RoundBuffer.c RoundBuffer.h
	$(CC) $(CFLAGS) RoundBuffer.c -c -o RoundBuffer.o

konsument.o: konsument.c
	$(CC) $(CFLAGS) konsument.c -c -o konsument.o

producent.o: producent.c
	$(CC) $(CFLAGS) producent.c -c -o producent.o
clean:
	rm -f *.o test




