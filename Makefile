CC=gcc
CFLAGS=-O2 -Wall

all: server client

server: server.c
	${CC} -o server ${CFLAGS} server.c          
client: client.c
	${CC} -o client ${CFLAGS} client.c

debug: server_debug client_debug

server_debug: server.c
	${CC} -o server_d -D_DEBUG ${CFLAGS} server.c

client_debug: client.c
	${CC} -o client_d -D_DEBUG ${CFLAGS} client.c

clean:                 
    rm -rf server client server_d client_d
