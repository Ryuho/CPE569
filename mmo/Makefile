
all: client server client_bot

server: obj/server.o obj/socket.o obj/GameServer.o obj/ServerData.o obj/Geometry.o obj/ServerObjManager.o
	g++ -o server $^ -lrt

client: obj/Characters.o obj/GLUtil.o obj/InputManager.o obj/client.o obj/SDLApp.o obj/Shader.o obj/Sprite.o obj/Texture.o obj/World.o obj/socket.o obj/CharacterGraphics.o
	g++ -o client $^ libGLEW.a -lglut -lSDL -lGLU

client_bot: obj/botmain.o obj/BotApp.o obj/BotWorld.o obj/BotCharacters.o obj/socket.o
	g++ -o client_bot $^ libGLEW.a -lglut -lSDL -lGLU

obj/ServerData.o: src/ServerData.h src/ServerData.cpp
	g++ -c -o obj/ServerData.o src/ServerData.cpp

obj/CharacterGraphics.o: src/CharacterGraphics.cpp src/Characters.h
	g++ -c -o obj/CharacterGraphics.o src/CharacterGraphics.cpp -I/home/whess/glew-1.5.6/include

obj/GameServer.o: src/GameServer.cpp src/GameServer.h src/packet.h src/ServerUtil.h
	g++ -c -o obj/GameServer.o src/GameServer.cpp

obj/ServerObjManager.o: src/ServerObjManager.h src/ServerObjManager.cpp
	g++ -c -o obj/ServerObjManager.o src/ServerObjManager.cpp

obj/server.o: src/server.cpp src/packet.h
	g++ -c -o obj/server.o src/server.cpp

obj/socket.o: src/socket.cpp
	g++ -c -o obj/socket.o src/socket.cpp

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

obj/World.o: src/World.cpp src/packet.h src/socket.h
	g++ -c -o obj/World.o src/World.cpp -I/home/whess/glew-1.5.6/include

obj/botmain.o: src/botmain.cpp
	g++ -c -o obj/botmain.o src/botmain.cpp -I/home/whess/glew-1.5.6/include

obj/BotApp.o: src/BotApp.cpp src/BotApp.h src/BotWorld.h src/BotWorld.cpp src/BotCharacters.cpp src/BotCharacters.h
	g++ -c -o obj/BotApp.o src/BotApp.cpp

obj/BotCharacters.o: src/BotCharacters.cpp src/BotCharacters.h
	g++ -c -o obj/BotCharacters.o src/BotCharacters.cpp -I/home/whess/glew-1.5.6/include

obj/BotWorld.o: src/BotWorld.cpp src/BotWorld.h src/BotCharacters.cpp src/BotCharacters.h
	g++ -c -o obj/BotWorld.o src/BotWorld.cpp -I/home/whess/glew-1.5.6/include

obj/Geometry.o: src/Geometry.cpp src/Geometry.h
	g++ -c -o obj/Geometry.o src/Geometry.cpp

clean:
	rm obj/*.o client client_bot server
