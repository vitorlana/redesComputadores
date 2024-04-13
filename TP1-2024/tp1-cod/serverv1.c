#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

static const int MAXPENDING = 5; // Maximum outstanding connection requests

void error(char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	int sockfd, newsockfd, portno, clilen;
	char buffer[500];
	struct sockaddr_in serv_addrv4, cli_addrv4;
	struct sockaddr_in6 serv_addrv6, cli_addrv6;
	int n;
	int typeConnection;

	fprintf(stderr, "argv[1]: %s , argv[2]: %s\n", argv[1], argv[2]);

	if (argc < 3) {
		fprintf(stderr,"ERROR, missing arguments\n");
		exit(1);
	}

	if(strcmp(argv[1],"v4")==0){
		typeConnection = 4;
	}else if (strcmp(argv[1],"v6")==0){
		typeConnection =6;
	}else{
		fprintf(stderr,"ERROR, invalid argument use only v4 or v6\n");
		exit(1);
	}

	switch (typeConnection){
	case 4:
		sockfd = socket(AF_INET, SOCK_STREAM, 0);

		if (sockfd < 0)
			error("ERROR opening socket");

		memset(&serv_addrv4, 0, sizeof(serv_addrv4));
		portno = atoi(argv[2]);
		serv_addrv4.sin_family = AF_INET;
		serv_addrv4.sin_addr.s_addr = INADDR_ANY;
		serv_addrv4.sin_port = htons(portno);

		if (bind(sockfd, (struct sockaddr *) &serv_addrv4, sizeof(serv_addrv4)) < 0)
			error("ERROR on binding");

		listen(sockfd,5);

		clilen = sizeof(cli_addrv4);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addrv4, &clilen);
		if (newsockfd < 0)
			error("ERROR on accept");

		break;
	
	case 6:
		sockfd = socket(AF_INET6, SOCK_STREAM, 0);

		if (sockfd < 0)
			error("ERROR opening socket");

		memset(&serv_addrv6, 0, sizeof(serv_addrv6));
		portno = atoi(argv[2]);
		serv_addrv6.sin6_family = AF_INET6;
		serv_addrv6.sin6_addr = in6addr_any;
		serv_addrv6.sin6_port = htons(portno);

		if (bind(sockfd, (struct sockaddr *) &serv_addrv6, sizeof(serv_addrv6)) < 0)
			error("ERROR on binding");

		listen(sockfd,5);

		clilen = sizeof(cli_addrv6);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addrv6, &clilen);
		if (newsockfd < 0)
			error("ERROR on accept");

		break;
	}

	memset(buffer, 0, 500);
	n = recv(newsockfd,buffer,499,0);
	if (n < 0) error("ERROR reading from socket");

	n = send(newsockfd,"I got your message",18,0);
	if (n < 0) error("ERROR writing to socket");

	printf("Here is the message: %s\n",buffer);

	return EXIT_SUCCESS;
}



int setupTCPServerSocket(const char *service) {
  // Construct the server address structure
  struct addrinfo addrCriteria;                   // Criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
  addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
  addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

  struct addrinfo *servAddr; // List of server addresses
  int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

  int servSock = -1;
  for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next) {
    // Create a TCP socket
    servSock = socket(addr->ai_family, addr->ai_socktype,
        addr->ai_protocol);
    if (servSock < 0)
      continue;       // Socket creation failed; try next address

    // Bind to the local address and set socket to listen
    if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) &&
        (listen(servSock, MAXPENDING) == 0)) {
      // Print local address of socket
      struct sockaddr_storage localAddr;
      socklen_t addrSize = sizeof(localAddr);
      if (getsockname(servSock, (struct sockaddr *) &localAddr, &addrSize) < 0)
        DieWithSystemMessage("getsockname() failed");
      fputs("Binding to ", stdout);
      PrintSocketAddress((struct sockaddr *) &localAddr, stdout);
      fputc('\n', stdout);
      break;       // Bind and listen successful
    }

    close(servSock);  // Close and try again
    servSock = -1;
  }

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);

  return servSock;
}

int acceptTCPConnection(int servSock) {
  struct sockaddr_storage clntAddr; // Client address
  // Set length of client address structure (in-out parameter)
  socklen_t clntAddrLen = sizeof(clntAddr);

  // Wait for a client to connect
  int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
  if (clntSock < 0)
    DieWithSystemMessage("accept() failed");

  // clntSock is connected to a client!

  fputs("Handling client ", stdout);
  PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
  fputc('\n', stdout);

  return clntSock;
}

