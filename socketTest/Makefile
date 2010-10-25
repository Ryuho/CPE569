
all: client server

client: socket.o client.cpp packet.h
	g++ -o client client.cpp socket.o

server: socket.o server.cpp packet.h
	g++ -o server server.cpp socket.o

socket.o: socket.cpp socket.h
	g++ -c socket.cpp

clean:
	rm socket.o client server
