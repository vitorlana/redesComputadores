CC = gcc
CFLAGS = -Wall -Wextra

all: client server

client: client.c
	$(CC) $(CFLAGS) -o client client.c networkUtilities.c roomsSystemUtilities.c

server: server.c
	$(CC) $(CFLAGS) -o server server.c networkUtilities.c roomsSystemUtilities.c

clean:
	rm -f client server