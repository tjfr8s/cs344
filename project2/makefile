CC=gcc
CFLAGS=-g -Wall 

all: freitast.buildrooms 

freitast.buildrooms: freitast.buildrooms.o 
	$(CC) $^ -o $@

clean:
	-rm *.o

cleanall: clean
	-rm chatclient
