all: server client

clean:
	rm -f *.o *~ $(PROGS)

server: server.c server.h
	gcc -o server server.c server.h

client: client.c
	gcc -lpthread -o client client.c