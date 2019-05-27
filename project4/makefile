CC=gcc
CFLAGS=-g -Wall 
all: keygen opt_enc_d opt_enc

keygen: keygen.o 
	$(CC) $^ -o $@ 

opt_enc_d: opt_enc_d.o 
	$(CC) $^ -o $@ 
	
opt_enc: opt_enc.o 
	$(CC) $^ -o $@ 

clean:
	-rm *.o

cleanall: clean
	-rm keygen opt_enc_d opt_enc

