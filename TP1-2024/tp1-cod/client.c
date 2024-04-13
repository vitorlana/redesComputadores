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

int setupTCPClientSocket(const char* serverIP, const char* port);

int main(int argc, char *argv[])
{
	int sockfd, portno, n;

	int typeConnection;

	char buffer[500];

	fprintf(stderr, "argv[1]: %s , argv[2]: %s\n", argv[1], argv[2]);

	if (argc < 3)
	{
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}

	if (strchr(argv[1], '.') != NULL){
		typeConnection = 4;
	}else if (strchr(argv[1], ':') != NULL){
		typeConnection = 6;
	}else {
		fprintf(stderr, "ERROR, invalid argument use only ipv4 or ipv6\n");
		exit(1);
	}


	sockfd = setupTCPClientSocket(argv[1], argv[2]);

	printf("Please enter the message: ");

	memset(buffer, 0, 500);
	fgets(buffer, 499, stdin);
	n = send(sockfd, buffer, strlen(buffer), 0);
	if (n < 0)
		error("ERROR writing to socket");

	memset(buffer, 0, 500);
	n = recv(sockfd, buffer, 499, 0);
	if (n < 0)
		error("ERROR reading from socket");

	printf("%s\n", buffer);

	return EXIT_SUCCESS;
}

int setupTCPClientSocket(const char *host, const char *service) {
	// Informe ao sistema que tipo(s) de informações de endereço desejamos
	struct addrinfo addrCriteria;                   // Critérios para correspondência de endereço
	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zera a estrutura
	addrCriteria.ai_family = AF_UNSPEC;             // v4 ou v6 está OK
	addrCriteria.ai_socktype = SOCK_STREAM;         // Somente sockets de fluxo
	addrCriteria.ai_protocol = IPPROTO_TCP;         // Somente protocolo TCP

	// Obtenha o(s) endereço(s)
	struct addrinfo *servAddr; // Armazena a lista de endereços do servidor retornados
	int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);
	if (rtnVal != 0){
		error("getaddrinfo() failed");
	}

	int sock = -1;
	for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next) {
		// Crie um socket confiável e de fluxo usando TCP
		sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (sock < 0)
			continue;  // Falha na criação do socket; tente o próximo endereço

		// Estabeleça a conexão com o servidor
		if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0)
			break;     // Conexão do socket bem-sucedida; interrompa e retorne o socket

		close(sock); // Falha na conexão do socket; tente o próximo endereço
		sock = -1;
	}

	freeaddrinfo(servAddr); // Libere a memória alocada em getaddrinfo()
	return sock;
}
