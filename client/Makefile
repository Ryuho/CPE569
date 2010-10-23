
all: Camera.o Characters.o GLUtil.o InputManager.o main.o SDLApp.o Shader.o Sprite.o Texture.o VBO.o World.o
	g++ -o client_linux $^ libGLEW.a -lglut -lSDL

normal:
	g++ -o client_linux src/*.cpp -lSDL -lGLEW -lglut

Camera.o: src/Camera.cpp
	g++ -c src/Camera.cpp -I/home/whess/glew-1.5.6/include

Characters.o: src/Characters.cpp
	g++ -c src/Characters.cpp -I/home/whess/glew-1.5.6/include

GLUtil.o: src/GLUtil.cpp
	g++ -c src/GLUtil.cpp -I/home/whess/glew-1.5.6/include

InputManager.o: src/InputManager.cpp
	g++ -c src/InputManager.cpp -I/home/whess/glew-1.5.6/include

main.o: src/main.cpp
	g++ -c src/main.cpp -I/home/whess/glew-1.5.6/include

SDLApp.o: src/SDLApp.cpp
	g++ -c src/SDLApp.cpp -I/home/whess/glew-1.5.6/include

Shader.o: src/Shader.cpp
	g++ -c src/Shader.cpp -I/home/whess/glew-1.5.6/include

Sprite.o: src/Sprite.cpp
	g++ -c src/Sprite.cpp -I/home/whess/glew-1.5.6/include

Texture.o: src/Texture.cpp
	g++ -c src/Texture.cpp -I/home/whess/glew-1.5.6/include

VBO.o: src/VBO.cpp
	g++ -c src/VBO.cpp -I/home/whess/glew-1.5.6/include

World.o: src/World.cpp
	g++ -c src/World.cpp -I/home/whess/glew-1.5.6/include

clean:
	rm *.o
