CC=gcc
CFLAGS=-O2

all: server client

server: server.c             
	    ${CC} -o server ${CFLAGS} server.c          
client: client.c             
	    ${CC} -o client ${CFLAGS} client.c          
clean:                 
	    rm -rf server client
