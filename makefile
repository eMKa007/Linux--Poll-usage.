CFLAGS = -g -Wall 
CC = gcc

all: RoundBuffer.o TimeFunctions.o producent.o konsument.o
	$(CC) $(CFLAGS) RoundBuffer.o TimeFunctions.o producent.o -o Serwer
	$(CC) $(CFLAGS) RoundBuffer.o TimeFunctions.o konsument.o -lcrypto -lssl -o Klient

RoundBuffer.o: RoundBuffer.c RoundBuffer.h
	$(CC) $(CFLAGS) RoundBuffer.c -c -o RoundBuffer.o

TimeFunctions.o: TimeFunctions.c TimeFunctions.h
	$(CC) $(CFLAGS) TimeFunctions.c -c -o TimeFunctions.o

konsument.o: konsument.c
	$(CC) $(CFLAGS) konsument.c -c -o konsument.o

producent.o: producent.c
	$(CC) $(CFLAGS) producent.c -c -o producent.o 

clean:
	rm -f *.o test




