#define _POSIX_C_SOURCE 1
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <netinet/in.h>
#define BUFFER_SIZE 1024

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int get_char_index(char character) {
    char charOptions[27] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
        'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
        'Z', ' '};
    int i;
    for (i = 0; i < 27; i++) {
        if (charOptions[i] == character) {
            return i;
        }    
    }
    return -1;
}
void decrypt_file(FILE* key, FILE* text, FILE* encfile) {
    rewind(encfile);
    rewind(key);
    int charIndex;
    char textBuffer[BUFFER_SIZE];
    char keyBuffer[BUFFER_SIZE];
    char encBuffer[BUFFER_SIZE];
    int reachedEnd = 0;
    int encIndex;
    int keyIndex;
    char charOptions[27] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
        'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
        'Z', ' '};
    int i;
	memset(textBuffer, '\0', BUFFER_SIZE);
	memset(keyBuffer, '\0', BUFFER_SIZE);
	memset(encBuffer, '\0', BUFFER_SIZE);

    while (!reachedEnd) {
        if(fread(encBuffer, 1, BUFFER_SIZE, encfile) < 1) {
            printf("textBuffer");
            break;
        }
        if(fread(keyBuffer, 1, BUFFER_SIZE, key) < 1) {
            printf("keyBuffer");
            break;
        }

        for (i = 0; i < BUFFER_SIZE && (encBuffer[i] != '\n'); i++) {
            if (encBuffer[i] > 64 && encBuffer[i] < 91) {
                encIndex = encBuffer[i] - 65; 
            } else if (encBuffer[i] == 32) {
                encIndex = 26;
            }
            //encIndex = get_char_index(encBuffer[i]);
            if (keyBuffer[i] > 64 && keyBuffer[i] < 91) {
                keyIndex = keyBuffer[i] - 65; 
            } else if (keyBuffer[i] == 32) {
                keyIndex = 26;
            }
            //keyIndex = get_char_index(keyBuffer[i]);
            charIndex = (encIndex - keyIndex);
            if(charIndex < 0) {
                charIndex = charIndex + 27;
            }
            textBuffer[i] = charOptions[charIndex];
        }
        if (i < BUFFER_SIZE) {
            textBuffer[i] = '\n';
        }

        fflush(stdout);
        fwrite(textBuffer, BUFFER_SIZE, 1, text);

        if(strstr(encBuffer, "\n") != NULL) {
            reachedEnd = 1;
        }

        memset(textBuffer, '\0', BUFFER_SIZE);
        memset(keyBuffer, '\0', BUFFER_SIZE);
        memset(encBuffer, '\0', BUFFER_SIZE);
    }
}

void send_to_client(int sockfd, FILE* fp) {
    char buffer[BUFFER_SIZE];
    int charsWritten;
    int charsRead;
    int numRead;
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
    rewind(fp);

    while ((numRead = fread(buffer, 1, sizeof(buffer) - 1, fp)) > 0) {
        fflush(stdout);
        charsWritten = send(sockfd, buffer, strlen(buffer), 0); // Write to the server
        if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
        if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

        // Get return message from server
        memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
        charsRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
        if (charsRead < 0) error("CLIENT: ERROR reading from socket");
        memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
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
        fwrite(buffer, BUFFER_SIZE - 1, 1, tempfd);
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

void check_client(int sockfd) {
    int charsWritten;
    int charsRead;
    char* decString = "dec";
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);
    charsWritten = send(sockfd, decString, strlen(decString), 0); // Write to the server
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    charsRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0) error("ERROR reading from socket");
    if (strcmp(buffer, "ok") != 0 ) {
        fprintf(stderr, "Connection for invalid client\n");
        exit(2);
    }
}

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber;
    FILE* textfile;
    FILE* keyfile;
    FILE* encfile;
    pid_t pid;
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

    int exitPid = -5;
    int status = -5;
    while (1) {
        while ((exitPid = waitpid(-1, &status, WNOHANG)) > 0) {
            printf("exited: %d\n", exitPid);
        }
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
        pid = fork();
        switch (pid) {
            case -1:
                printf("error\n");
                break;
            case 0:
                if (establishedConnectionFD < 0) error("ERROR on accept");
                check_client(establishedConnectionFD);
                encfile = tmpfile();
                receive_file(establishedConnectionFD, encfile);
                keyfile = tmpfile();
                receive_file(establishedConnectionFD, keyfile);
                textfile = tmpfile();

                decrypt_file(keyfile, textfile, encfile);
                send_to_client(establishedConnectionFD, textfile);

                close(establishedConnectionFD); // Close the existing socket which is connected to the client
                exit(0);
                break;
           default:
                close(establishedConnectionFD); // Close the existing socket which is connected to the client
        }
    }
	close(listenSocketFD); // Close the listening socket
	return 0; 
}
