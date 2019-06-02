#define BUFFER_SIZE 1024

void error(const char *msg);
int get_char_index(char character);
void decrypt_file(FILE* key, FILE* text, FILE* encfile);
void send_to_client(int sockfd, FILE* fp);
void receive_file(int sockfd, FILE* tempfd);
void read_file(FILE* tempfd);
void check_client(int sockfd);
