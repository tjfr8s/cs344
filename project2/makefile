CC=gcc
CFLAGS=-g -Wall
CLIB=-lpthread

all: freitast.buildrooms freitast.adventure

freitast.buildrooms: freitast.buildrooms.o 
	$(CC) $^ -o $@ 

freitast.adventure: freitast.adventure.o 
	$(CC) $^ -o $@ $(CLIB)

clean:
	-rm *.o

cleanall: clean
	-rm chatclient
