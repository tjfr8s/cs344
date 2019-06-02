#define BUFFER_SIZE 1024
void error(const char *msg); 
void send_to_server(int sockfd, char* filename);
void receive_decrypted_file(int sockfd);
void check_server(int socketFD);
void check_key_size(char* textFileName, char* keyFileName);
