#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "common.h"

int main(int argc, char* argv[]) {
    int sockfd, newsockfd, n;
    char buffer[500];

    Classroom classrooms[MAX_ROOMS];

    initializeClassrooms(classrooms);

    // Check if the required command line arguments are provided
    if (argc < 3) {
        fprintf(stderr, "ERROR, missing arguments\n");
        exit(1);
    }

    // Check if the first argument is either "v4" or "v6"
    if (strcmp(argv[1], "v4") == 0 || strcmp(argv[1], "v6") == 0) {
        // Valid IP address
    } else {
        fprintf(stderr, "ERROR, invalid argument use only v4 or v6\n");
        exit(1);
    }

    // Set up a TCP server socket using the provided IP address and port number
    sockfd = setupTCPServerSocket(argv[2]);

    // Accept a new TCP connection
    newsockfd = acceptTCPConnection(sockfd);

    // Start an infinite loop to handle client requests
    while (1) {
        // Clear the buffer
        memset(buffer, 0, 500);

        // Receive data from the client
        n = recv(newsockfd, buffer, 499, 0);
        if (n < 0) {
            errorWithoutKill("ERROR reading from socket");
        } else if (n == 0) {
            fprintf(stderr, "Client closed the connection\n");
            break;
        }

        // Check if the received command is "kill"
        if (strcmp(buffer, "kill\n") == 0) {
            fprintf(stderr, "Kill was received - %s\n", buffer);
            break;
        } else {
            fprintf(stderr, "Request received - %s\n", buffer);

            // Execute the received command and get the result
            char* resultCommand = executeCommand(buffer, classrooms);

            fprintf(stderr, "Response send - %s\n", resultCommand);

            // Send the result back to the client
            n = send(newsockfd, resultCommand, strlen(resultCommand), 0);
            if (n < 0)
                errorWithoutKill("ERROR writing to socket");
        }
    }

    // Close the sockets
    close(sockfd);
    close(newsockfd);

    return EXIT_SUCCESS;
}