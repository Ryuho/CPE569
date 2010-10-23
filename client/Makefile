
all: Camera.o Characters.o GLUtil.o InputManager.o main.o SDLApp.o Shader.o Sprite.o Texture.o VBO.o World.o
	g++ -o client_linux $^ libGLEW.a -lglut -lSDL

normal:
	g++ -o client_linux src/*.cpp -lSDL -lGLEW -lglut

Camera.o:
	g++ -c src/Camera.cpp -I/home/whess/glew-1.5.6/include

Characters.o:
	g++ -c src/Characters.cpp -I/home/whess/glew-1.5.6/include

GLUtil.o:
	g++ -c src/GLUtil.cpp -I/home/whess/glew-1.5.6/include

InputManager.o:
	g++ -c src/InputManager.cpp -I/home/whess/glew-1.5.6/include

main.o:
	g++ -c src/main.cpp -I/home/whess/glew-1.5.6/include

SDLApp.o:
	g++ -c src/SDLApp.cpp -I/home/whess/glew-1.5.6/include

Shader.o:
	g++ -c src/Shader.cpp -I/home/whess/glew-1.5.6/include

Sprite.o:
	g++ -c src/Sprite.cpp -I/home/whess/glew-1.5.6/include

Texture.o:
	g++ -c src/Texture.cpp -I/home/whess/glew-1.5.6/include

VBO.o:
	g++ -c src/VBO.cpp -I/home/whess/glew-1.5.6/include

World.o:
	g++ -c src/World.cpp -I/home/whess/glew-1.5.6/include

clean:
	rm *.o
