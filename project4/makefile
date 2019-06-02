CC=gcc
CFLAGS=-g -Wall 
all: keygen otp_enc_d otp_enc otp_dec_d otp_dec

keygen: keygen.o 
	$(CC) $^ -o $@ 

otp_enc_d: otp_enc_d.o 
	$(CC) $^ -o $@ 
	
otp_enc: otp_enc.o 
	$(CC) $^ -o $@ 

otp_dec_d: otp_dec_d.o 
	$(CC) $^ -o $@ 
	
otp_dec: otp_dec.o 
	$(CC) $^ -o $@ 

clean:
	-rm *.o

cleanall: clean
	-rm keygen otp_enc_d otp_enc otp_dec otp_dec_d

