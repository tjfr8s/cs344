
#define BUFFER_SIZE 1024
void error(const char *msg);
void send_to_server(int sockfd, char* filename);
void receive_encrypted_file(int sockfd);
void check_server(int socketFD);
long get_file_size(char* fileName);
void check_key_size(char* textFileName, char* keyFileName);
void read_file(char* filename);
