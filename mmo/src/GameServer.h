#ifndef _GAME_SERVER_H_
#define _GAME_SERVER_H_

#include "ServerUtil.h"
#include "Characters.h"

struct GameServer {
   GameServer(ConnectionManager &cm);
   void newConnection(int id);
   void disconnect(int id);
   void processPacket(pack::Packet p, int fromid);
   void update(int ticks);

   ConnectionManager &cm;
   ObjectHolder objs;

   int ticks;
   float dt;
};

#endif