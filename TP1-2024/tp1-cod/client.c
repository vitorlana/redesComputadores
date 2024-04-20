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
    int sockfd, n;

    char buffer[500];

    // Check if the correct number of command-line arguments are provided
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    // Check if the provided argument is a valid IP address
    if (strchr(argv[1], '.') != NULL || strchr(argv[1], ':') != NULL) {
        // Valid IP address
    } else {
        fprintf(stderr, "ERROR, invalid argument use only ipv4 or ipv6\n");
        exit(1);
    }

    // Set up a TCP client socket
    sockfd = setupTCPClientSocket(argv[1], argv[2]);

    // Start an infinite loop to continuously send and receive data
    while (1) {
        // Clear the buffer before reading user input
        memset(buffer, 0, 500);
        fgets(buffer, 499, stdin);

        // Check if the user input is a special command to exit the program
        if (checkCommand(buffer) == -1) {
            // Shutdown and close the socket
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            break;
        } else {
            // Check the user input and create a payload
            char* payload = checkInputAndCreatePayload(buffer);
            if (payload != NULL) {
                // Send the payload to the server
                n = send(sockfd, payload, strlen(payload), 0);
                if (n < 0)
                    error("ERROR writing to socket");

                // Clear the buffer before receiving the response
                memset(buffer, 0, 500);
                // Receive the response from the server
                n = recv(sockfd, buffer, 499, 0);
                if (n < 0)
                    error("ERROR reading from socket");

                // Translate and print the response message
                fprintf(stderr, "%s\n", translateResponseToMessage(buffer));
            }
        }
    }

    return EXIT_SUCCESS;
}