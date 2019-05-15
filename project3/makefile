CC=gcc
CFLAGS=-g -Wall -std=c99

all: smallsh 

smallsh: smallsh.o 
	$(CC) $^ -o $@ 


clean:
	-rm *.o

cleanall: clean
	-rm smallsh 
