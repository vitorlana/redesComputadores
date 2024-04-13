#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, portno, n;

	struct sockaddr_in serv_addrv4, cli_addrv4;
	struct sockaddr_in6 serv_addrv6, cli_addrv6;
	struct hostent *server;
	int typeConnection;

	char buffer[256];

	fprintf(stderr, "argv[1]: %s , argv[2]: %s\n", argv[1], argv[2]);

	if (argc < 3)
	{
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}

	if (strchr(argv[1], '.') != NULL)
	{
		typeConnection = 4;
	}
	else if (strchr(argv[1], ':') != NULL)
	{
		typeConnection = 6;
	}
	else
	{
		fprintf(stderr, "ERROR, invalid argument use only ipv4 or ipv6\n");
		exit(1);
	}
	portno = atoi(argv[2]);

	switch (typeConnection)
	{
	case 4:
		sockfd = socket(AF_INET, SOCK_STREAM, 0);

		if (sockfd < 0)
			error("v4: ERROR opening socket");
		server = gethostbyname(argv[1]);
		if (server == NULL)
		{
			fprintf(stderr, "v4: ERROR, no such host\n");
			exit(0);
		}

		memset(&serv_addrv4, 0, sizeof(serv_addrv4));
		serv_addrv4.sin_family = AF_INET;
		bcopy((char *)server->h_addr,
			  (char *)&serv_addrv4.sin_addr.s_addr,
			  server->h_length);
		serv_addrv4.sin_port = htons(portno);

		if (connect(sockfd, (struct sockaddr *)&serv_addrv4, sizeof(serv_addrv4)) < 0)
			error("v4: ERROR connecting");

		break;

	case 6:
		sockfd = socket(AF_INET6, SOCK_STREAM, 0);

		if (sockfd < 0)
			error("v6: ERROR opening socket");

		server = getaddrinfo(argv[1]);
		if (server == NULL)
		{
			fprintf(stderr, "v6: ERROR, no such host v6\n");
			exit(0);
		}

		memset(&serv_addrv6, 0, sizeof(serv_addrv6));
		serv_addrv6.sin6_family = AF_INET6;
		bcopy((char *)server->h_addr,
			  (char *)&serv_addrv6.sin6_addr.s6_addr,
			  server->h_length);
		serv_addrv6.sin6_port = htons(portno);

		if (connect(sockfd, (struct sockaddr *)&serv_addrv6, sizeof(serv_addrv6)) < 0)
			error("v6: ERROR connecting");

		break;
	}

	printf("Please enter the message: ");

	memset(buffer, 0, 256);
	fgets(buffer, 255, stdin);
	n = send(sockfd, buffer, strlen(buffer), 0);
	if (n < 0)
		error("ERROR writing to socket");

	memset(buffer, 0, 256);
	n = recv(sockfd, buffer, 255, 0);
	if (n < 0)
		error("ERROR reading from socket");

	printf("%s\n", buffer);

	return EXIT_SUCCESS;
}
