#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static const int MAXPENDING = 5;  // Maximum outstanding connection requests

void error(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void errorWithoutKill(char* msg) {
    perror(msg);
}

void printSocketPortAndAddress(const struct sockaddr* address, char* message) {
    // Test for address and stream
    if (address == NULL)
        return;

    void* numericAddress;  // Pointer to binary address
    // Buffer to contain result (IPv6 sufficient to hold IPv4)
    char addrBuffer[INET6_ADDRSTRLEN];
    in_port_t port;  // Port to print
    // Set pointer to address based on address family
    switch (address->sa_family) {
        case AF_INET:
            numericAddress = &((struct sockaddr_in*)address)->sin_addr;
            port = ntohs(((struct sockaddr_in*)address)->sin_port);
            break;
        case AF_INET6:
            numericAddress = &((struct sockaddr_in6*)address)->sin6_addr;
            port = ntohs(((struct sockaddr_in6*)address)->sin6_port);
            break;
        default:
            error("[unknown type]");  // Unhandled type
            return;
    }
    // Convert binary to printable address
    if (inet_ntop(address->sa_family, numericAddress, addrBuffer, sizeof(addrBuffer)) == NULL)
        error("[invalid address]");  // Unable to convert
    else {
		fprintf(stderr, "%s address: %s port: %hu\n", message, addrBuffer, port);
	}
}

int setupTCPServerSocket(const char* service) {
    // Construct the server address structure
    struct addrinfo addrCriteria;                    // Criteria for address match
    memset(&addrCriteria, 0, sizeof(addrCriteria));  // Zero out structure
    addrCriteria.ai_family = AF_UNSPEC;              // Any address family
    addrCriteria.ai_flags = AI_PASSIVE;              // Accept on any address/port
    addrCriteria.ai_socktype = SOCK_STREAM;          // Only stream sockets
    addrCriteria.ai_protocol = IPPROTO_TCP;          // Only TCP protocol

    struct addrinfo* servAddr;  // List of server addresses
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    if (rtnVal != 0)
        error("getaddrinfo() failed");

    int servSock = -1;
    for (struct addrinfo* addr = servAddr; addr != NULL; addr = addr->ai_next) {
        // Create a TCP socket
        servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (servSock < 0)
            continue;  // Socket creation failed; try next address

        // Bind to the local address and set socket to listen
        if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) && (listen(servSock, MAXPENDING) == 0)) {
            // Print local address of socket
            struct sockaddr_storage localAddr;
            socklen_t addrSize = sizeof(localAddr);
            if (getsockname(servSock, (struct sockaddr*)&localAddr, &addrSize) < 0)
                error("getsockname() failed");
			// printSocketPortAndAddress((struct sockaddr*)&localAddr, "Binding to");
            break;  // Bind and listen successful
        }

        close(servSock);  // Close and try again
        servSock = -1;
    }

    // Free address list allocated by getaddrinfo()
    freeaddrinfo(servAddr);

    return servSock;
}

int acceptTCPConnection(int servSock) {
    struct sockaddr_storage clntAddr;  // Client address
    // Set length of client address structure (in-out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Wait for a client to connect
    int clntSock = accept(servSock, (struct sockaddr*)&clntAddr, &clntAddrLen);
    if (clntSock < 0)
        error("accept() failed");

    // clntSock is connected to a client!

	// printSocketPortAndAddress((struct sockaddr*)&clntAddr, "Handling client: ");

    return clntSock;
}

int setupTCPClientSocket(const char* host, const char* service) {
    // Informe ao sistema que tipo(s) de informações de endereço desejamos
    struct addrinfo addrCriteria;                    // Critérios para correspondência de endereço
    memset(&addrCriteria, 0, sizeof(addrCriteria));  // Zera a estrutura
    addrCriteria.ai_family = AF_UNSPEC;              // v4 ou v6 está OK
    addrCriteria.ai_socktype = SOCK_STREAM;          // Somente sockets de fluxo
    addrCriteria.ai_protocol = IPPROTO_TCP;          // Somente protocolo TCP

    // Obtenha o(s) endereço(s)
    struct addrinfo* servAddr;  // Armazena a lista de endereços do servidor retornados
    int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);
    if (rtnVal != 0) {
        error("getaddrinfo() failed");
    }

    int sock = -1;
    for (struct addrinfo* addr = servAddr; addr != NULL; addr = addr->ai_next) {
        // Crie um socket confiável e de fluxo usando TCP
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock < 0)
            continue;  // Falha na criação do socket; tente o próximo endereço

        // Estabeleça a conexão com o servidor
        if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0)
            break;  // Conexão do socket bem-sucedida; interrompa e retorne o socket

        close(sock);  // Falha na conexão do socket; tente o próximo endereço
        sock = -1;
    }

    freeaddrinfo(servAddr);  // Libere a memória alocada em getaddrinfo()
    return sock;
}
