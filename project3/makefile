CC=gcc
CFLAGS=-g -Wall 

all: smallsh 

smallsh: smallsh.o 
	$(CC) $^ -o $@ 


clean:
	-rm *.o

cleanall: clean
	-rm smallsh 
