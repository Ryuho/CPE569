
all: client server

client: socket.o client.cpp
	g++ -o client client.cpp socket.o

server: socket.o server.cpp
	g++ -o server server.cpp socket.o

socket.o: socket.cpp socket.h
	g++ -c socket.cpp
