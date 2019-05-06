CC=gcc
CFLAGS=-g -Wall 

all: freitast.buildrooms freitast.adventure

freitast.buildrooms: freitast.buildrooms.o 
	$(CC) $^ -o $@

freitast.adventure: freitast.adventure.o 
	$(CC) $^ -o $@

clean:
	-rm *.o

cleanall: clean
	-rm chatclient
