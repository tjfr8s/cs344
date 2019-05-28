#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#define BUFFER_SIZE 1024

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

void encrypt_file(FILE* key, FILE* text) {
    rewind(text);
    rewind(key);
    int charIndex;
    char textBuffer[BUFFER_SIZE];
    char keyBuffer[BUFFER_SIZE];
    char encBuffer[BUFFER_SIZE];
    char charOptions[27] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
        'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
        'Z', ' '};
    int i;
	memset(textBuffer, '\0', BUFFER_SIZE);
	memset(keyBuffer, '\0', BUFFER_SIZE);
	memset(encBuffer, '\0', BUFFER_SIZE);

    while (strstr(textBuffer, "\n") == NULL) {
        fread(textBuffer, 1, BUFFER_SIZE, text);
        fread(keyBuffer, 1, BUFFER_SIZE, key);

        for (i = 0; i < BUFFER_SIZE && (textBuffer[i] != 0); i++) {
            charIndex = (textBuffer[i] + encBuffer[i]) % 27;
            encBuffer[i] = charOptions[charIndex];
        }

        printf(encBuffer);


        memset(textBuffer, '\0', BUFFER_SIZE);
        memset(keyBuffer, '\0', BUFFER_SIZE);
        memset(encBuffer, '\0', BUFFER_SIZE);
    }
}
void receive_file(int sockfd, FILE* tempfd) {
	char buffer[BUFFER_SIZE];
    int charsRead;
	memset(buffer, '\0', BUFFER_SIZE);

    while ( strstr(buffer, "\n") == NULL) {
        // Get the message from the client and display it
        memset(buffer, '\0', BUFFER_SIZE);
        charsRead = recv(sockfd, buffer, BUFFER_SIZE - 1, 0); // Read the client's message from the socket
        if (charsRead < 0) error("ERROR reading from socket");
        fwrite(buffer, BUFFER_SIZE, 1, tempfd);
        // Send a Success message back to the client
        charsRead = send(sockfd, "ok", 3, 0); // Send success back
        if (charsRead < 0) error("ERROR writing to socket");
    }
}

void read_file(FILE* tempfd) {
    char buffer[1024];
    int numRead;
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array

    rewind(tempfd);

    while ((numRead = fread(buffer, 1, sizeof(buffer), tempfd)) > 0) {
        printf(buffer);
        memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
    }
    
}

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber;
    FILE* textfile;
    FILE* keyfile;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect

	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
	if (establishedConnectionFD < 0) error("ERROR on accept");
    textfile = tmpfile();
    receive_file(establishedConnectionFD, textfile);
    keyfile = tmpfile();
    receive_file(establishedConnectionFD, keyfile);

    encrypt_file(keyfile, textfile);

	close(establishedConnectionFD); // Close the existing socket which is connected to the client

	close(listenSocketFD); // Close the listening socket
	return 0; 
}
