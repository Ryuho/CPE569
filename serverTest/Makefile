
all: server client

client: socket.o client.cpp packet.h
	g++ -g -o client client.cpp socket.o

server: socket.o server.cpp packet.h
	g++ -g -o server server.cpp socket.o

socket.o: socket.cpp socket.h
	g++ -c -g socket.cpp

clean:
	rm socket.o client server
