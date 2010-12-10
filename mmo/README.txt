Team: Green Horse
Project Help file
CSC 579
Bellardo

Building:
=========
Install Visual Studio 2008
Run mmo.sln
Ensure that the required libraries are included with the VS2008.
Select Release/Debug, then build.

Required Libraries to install:
==============================
FreeType
SDL
GL/Glu/Glut
Glew
Boost

To include a library into Visual Studio 2008:
Tools > Projects and Solutions > VC++ Directories
Show directories for: Libraries/Include Files
Then add the base folder for each library's install directory.

Example Run:
============
server_multi.exe 27027 27028 (server A)
server_multi.exe 27029 27030 localhost 27028 (server B connects to server A)
client_bot.exe localhost 27027 (connects a bot client to server A)
client.exe localhost 27029 (connects a client to server B)

Usage:
======
Ensure that the server is running before attempting to connect a client.
Usage will be displayed when running with no arguments.

server <client port number> <server port number> [ <alternate server address> <alternate server port> ]
Default client port: 27027
Default server port: 27028

client <server address> <port number>
Default server address: localhost
Default port number: 27027

client_bot <server address> <port number>\n
Default server address: localhost
Default port number: 27027

G++ Compiler Incompatabilities:
===============================
There is a compatability issue with g++. Therefore we have not included
a Makefile for the project. G++ will not compile the program because of 
a non-standard C++ use of temporary objects passed by value being 
immediately passed by reference.

To make it compatible, do one of two things:
1. Convert methods being passed by reference into const references
2. Store the value into a variable before passing it as an argument.