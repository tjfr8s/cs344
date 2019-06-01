#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFFER_SIZE 1024

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

void send_to_server(int sockfd, char* filename) {
    char buffer[1024];
    FILE* fp;
    int charsWritten;
    int charsRead;
    int numRead;
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
    fp = fopen(filename, "rb");

    while ((numRead = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        charsWritten = send(sockfd, buffer, strlen(buffer), 0); // Write to the server
        if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
        if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

        // Get return message from server
        memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
        charsRead = recv(sockfd, buffer, sizeof(buffer), 0); // Read data from the socket, leaving \0 at end
        if (charsRead < 0) error("CLIENT: ERROR reading from socket");
        memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
    }
}

void receive_decrypted_file(int sockfd) {
	char buffer[BUFFER_SIZE];
    int charsRead;
	memset(buffer, '\0', BUFFER_SIZE);

    while ( strstr(buffer, "\n") == NULL) {
        // Get the message from the client and display it
        memset(buffer, '\0', BUFFER_SIZE);
        charsRead = recv(sockfd, buffer, BUFFER_SIZE, 0); // Read the client's message from the socket
        if (charsRead < 0) error("ERROR reading from socket");
        printf(buffer);
        fflush(stdout);
        // Send a Success message back to the client
        charsRead = send(sockfd, "ok", 3, 0); // Send success back
        if (charsRead < 0) error("ERROR writing to socket");
    }
}

void check_server(int socketFD) {
	char buffer[BUFFER_SIZE];
    int charsRead;
    memset(buffer, '\0', BUFFER_SIZE);
    charsRead = recv(socketFD, buffer, BUFFER_SIZE, 0); // Read the client's message from the socket
    fflush(stdout);
    if (charsRead < 0) error("ERROR reading from socket");
    // Terminate if the server attempts to connect to a server other than
    // opt_enc_d.
    if (strcmp(buffer, "dec") != 0) {
        fprintf(stderr, "Attempting to connect to invalid server\n");
        exit(2);
        charsRead = send(socketFD, "error", 6, 0); // Send success back
        close(socketFD); // Close the socket
    }

    charsRead = send(socketFD, "ok", 3, 0); // Send success back

}

int main(int argc, char *argv[])
{
	int socketFD, portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
    
	if (argc < 3) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[2]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

    check_server(socketFD);
    send_to_server(socketFD, "./myciphertext");
    send_to_server(socketFD, "./keyfile");
    receive_decrypted_file(socketFD);

	close(socketFD); // Close the socket
	return 0;
}
