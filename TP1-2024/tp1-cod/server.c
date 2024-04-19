#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "networkUtilities.h"
#include "roomsSystemUtilities.h"

int main(int argc, char* argv[]) {
    int sockfd, newsockfd, n;
    char buffer[500];

    Classroom classrooms[MAX_ROOMS];

    initializeClassrooms(classrooms);

    // fprintf(stderr, "argv[1]: %s , argv[2]: %s\n", argv[1], argv[2]);

    if (argc < 3) {
        fprintf(stderr, "ERROR, missing arguments\n");
        exit(1);
    }

    if (strcmp(argv[1], "v4") == 0 || strcmp(argv[1], "v6") == 0) {
        // Valid IP address
    } else {
        fprintf(stderr, "ERROR, invalid argument use only v4 or v6\n");
        exit(1);
    }

    sockfd = setupTCPServerSocket(argv[2]);
    newsockfd = acceptTCPConnection(sockfd);

    while (1) {
        memset(buffer, 0, 500);
        n = recv(newsockfd, buffer, 499, 0);
        if (n < 0) {
            errorWithoutKill("ERROR reading from socket");
        } else if (n == 0) {
            fprintf(stderr, "Client closed the connection\n");
            break;
        }

        if (strcmp(buffer, "kill\n") == 0) {
            fprintf(stderr, "Kill was received - %s\n", buffer);
            break;
        } else {
            
            fprintf(stderr, "Request received - %s\n", buffer);

            char* resultCommand = executeCommand(buffer, classrooms);

            fprintf(stderr, "Response send - %s\n", resultCommand);


            n = send(newsockfd, resultCommand, strlen(resultCommand), 0);
            if (n < 0)
                errorWithoutKill("ERROR writing to socket");
        }
    }

    close(sockfd);
    close(newsockfd);

    return EXIT_SUCCESS;
}