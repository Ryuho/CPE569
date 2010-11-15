#ifndef _GAME_SERVER_H_
#define _GAME_SERVER_H_

#include "ServerUtil.h"
#include "ServerData.h"

using namespace server;

struct GameServer {
   GameServer(ConnectionManager &cm);
   void newClientConnection(int id);
   void newServerConnection(int id);

   void clientDisconnect(int id);
   void serverDisconnect(int id);

   void processClientPacket(pack::Packet p, int fromid);
   void processServerPacket(pack::Packet p, int fromid);

   void update(int ticks);

   ConnectionManager &cm;
   ObjectManager om;

   int ticks;
   float dt;
};

int getTicks();
float getDt();
ObjectManager &getOM();

#endif
