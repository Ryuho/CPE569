
all: client server

server: obj/server.o obj/socket.o
	g++ -o server $^

client: obj/Camera.o obj/Characters.o obj/GLUtil.o obj/InputManager.o obj/client.o obj/SDLApp.o obj/Shader.o obj/Sprite.o obj/Texture.o obj/VBO.o obj/World.o obj/socket.o
	g++ -o client $^ libGLEW.a -lglut -lSDL

obj/server.o: src/server.cpp src/packet.h
	g++ -c -o obj/server.o src/server.cpp

obj/socket.o: src/socket.cpp
	g++ -c -o obj/socket.o src/socket.cpp

obj/Camera.o: src/Camera.cpp
	g++ -c -o obj/Camera.o src/Camera.cpp -I/home/whess/glew-1.5.6/include

obj/Characters.o: src/Characters.cpp
	g++ -c -o obj/Characters.o src/Characters.cpp -I/home/whess/glew-1.5.6/include

obj/GLUtil.o: src/GLUtil.cpp
	g++ -c -o obj/GLUtil.o src/GLUtil.cpp -I/home/whess/glew-1.5.6/include

obj/InputManager.o: src/InputManager.cpp
	g++ -c -o obj/InputManager.o src/InputManager.cpp -I/home/whess/glew-1.5.6/include

obj/client.o: src/client.cpp
	g++ -c -o obj/client.o src/client.cpp -I/home/whess/glew-1.5.6/include

obj/SDLApp.o: src/SDLApp.cpp
	g++ -c -o obj/SDLApp.o src/SDLApp.cpp -I/home/whess/glew-1.5.6/include

obj/Shader.o: src/Shader.cpp
	g++ -c -o obj/Shader.o src/Shader.cpp -I/home/whess/glew-1.5.6/include

obj/Sprite.o: src/Sprite.cpp
	g++ -c -o obj/Sprite.o src/Sprite.cpp -I/home/whess/glew-1.5.6/include

obj/Texture.o: src/Texture.cpp
	g++ -c -o obj/Texture.o src/Texture.cpp -I/home/whess/glew-1.5.6/include

obj/VBO.o: src/VBO.cpp
	g++ -c -o obj/VBO.o src/VBO.cpp -I/home/whess/glew-1.5.6/include

obj/World.o: src/World.cpp src/packet.h src/socket.h
	g++ -c -o obj/World.o src/World.cpp -I/home/whess/glew-1.5.6/include

clean:
	rm obj/*.o client server
