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
    int sockfd, portno, n;

    int typeConnection;

    char buffer[500];

    // fprintf(stderr, "argv[1]: %s , argv[2]: %s\n", argv[1], argv[2]);

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    if (strchr(argv[1], '.') != NULL) {
        typeConnection = 4;
    } else if (strchr(argv[1], ':') != NULL) {
        typeConnection = 6;
    } else {
        fprintf(stderr, "ERROR, invalid argument use only ipv4 or ipv6\n");
        exit(1);
    }

    sockfd = setupTCPClientSocket(argv[1], argv[2]);

    while (1) {
        memset(buffer, 0, 500);
        fgets(buffer, 499, stdin);

        if (checkCommand(buffer) == -1) {
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            break;
        } else {
            char* payload = checkInputAndCreatePayload(buffer);
            if (payload != NULL) {
                n = send(sockfd, payload, strlen(payload), 0);
                if (n < 0)
                    error("ERROR writing to socket");

                
                memset(buffer, 0, 500);
                n = recv(sockfd, buffer, 499, 0);
                if (n < 0)
                    error("ERROR reading from socket");

                fprintf(stderr, "%s\n", convertErrosAndSuccess(buffer));
            }
        }
    }

    return EXIT_SUCCESS;
}
