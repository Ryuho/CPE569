#ifndef _GAME_SERVER_H_
#define _GAME_SERVER_H_

#include "ServerUtil.h"
#include "ServerData.h"

using namespace server;

struct GameServer {
   GameServer(ConnectionManager &cm);
   void newConnection(int id);
   void disconnect(int id);
   void processPacket(pack::Packet p, int fromid);
   void update(int ticks);

   ConnectionManager &cm;
   ObjectManager om;

   int ticks;
   float dt;
};

int getTicks();
float getDt();

#endif