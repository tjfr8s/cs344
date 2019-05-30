CC=gcc
CFLAGS=-g -Wall 
all: keygen opt_enc_d opt_enc opt_dec_d opt_dec

keygen: keygen.o 
	$(CC) $^ -o $@ 

opt_enc_d: opt_enc_d.o 
	$(CC) $^ -o $@ 
	
opt_enc: opt_enc.o 
	$(CC) $^ -o $@ 

opt_dec_d: opt_dec_d.o 
	$(CC) $^ -o $@ 
	
opt_dec: opt_dec.o 
	$(CC) $^ -o $@ 

clean:
	-rm *.o

cleanall: clean
	-rm keygen opt_enc_d opt_enc

