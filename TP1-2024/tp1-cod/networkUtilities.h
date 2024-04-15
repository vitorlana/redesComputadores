#ifndef NETWORK_UTILITIES_H 
#define NETWORK_UTILITIES_H

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


void error(char* msg);
void errorWithoutKill(char* msg);
void printSocketPortAndAddress(const struct sockaddr* address, char* message);
int setupTCPServerSocket(const char* service);
int acceptTCPConnection(int servSock);
int setupTCPClientSocket(const char* host, const char* service);

#endif  // NETWORK_UTILITIES_H