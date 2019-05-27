CC=gcc
CFLAGS=-g -Wall -std=c99

all: keygen 

keygen: keygen.o 
	$(CC) $^ -o $@ 


clean:
	-rm *.o

cleanall: clean
	-rm keygen
