CC=gcc
CFLAGS=-O2 -Wall -C99

all: server client

server: server.c
	${CC} -o server ${CFLAGS} server.c          
client: client.c
	${CC} -o client ${CFLAGS} client.c

debug: server_debug client_debug

server_debug: server.c
	${CC} -o server_d -D_DEBUG -g ${CFLAGS} server.c

client_debug: client.c
	${CC} -o client_d -D_DEBUG -g ${CFLAGS} client.c

clean:
	rm -rf server client server_d client_d

