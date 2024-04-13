#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

static const int MAXPENDING = 5; // Maximum outstanding connection requests

int setupTCPServerSocket(const char *service);
int acceptTCPConnection(int servSock);

void error(char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void DieWithUserMessage(const char *msg, const char *detail) {
  fputs(msg, stderr);
  fputs(": ", stderr);
  fputs(detail, stderr);
  fputc('\n', stderr);
  exit(1);
}

void PrintSocketAddress(const struct sockaddr *address, FILE *stream) {
  // Test for address and stream
  if (address == NULL || stream == NULL)
    return;

  void *numericAddress; // Pointer to binary address
  // Buffer to contain result (IPv6 sufficient to hold IPv4)
  char addrBuffer[INET6_ADDRSTRLEN];
  in_port_t port; // Port to print
  // Set pointer to address based on address family
  switch (address->sa_family) {
  case AF_INET:
    numericAddress = &((struct sockaddr_in *) address)->sin_addr;
    port = ntohs(((struct sockaddr_in *) address)->sin_port);
    break;
  case AF_INET6:
    numericAddress = &((struct sockaddr_in6 *) address)->sin6_addr;
    port = ntohs(((struct sockaddr_in6 *) address)->sin6_port);
    break;
  default:
    fputs("[unknown type]", stream);    // Unhandled type
    return;
  }
  // Convert binary to printable address
  if (inet_ntop(address->sa_family, numericAddress, addrBuffer,
      sizeof(addrBuffer)) == NULL)
    fputs("[invalid address]", stream); // Unable to convert
  else {
    fprintf(stream, "%s", addrBuffer);
    if (port != 0)                // Zero not valid in any socket addr
      fprintf(stream, "-%u", port);
  }
}


int main(int argc, char *argv[]) {
	int sockfd, newsockfd, portno, clilen;
	char buffer[500];

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

	sockfd = setupTCPServerSocket(argv[2]);
	newsockfd = acceptTCPConnection(sockfd);
	
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
        error("getsockname() failed");
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
    error("accept() failed");

  // clntSock is connected to a client!

  fputs("Handling client ", stdout);
  PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
  fputc('\n', stdout);

  return clntSock;
}




