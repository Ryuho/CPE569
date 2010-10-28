#ifndef _GAME_SERVER_H_
#define _GAME_SERVER_H_

#include "ServerUtil.h"

struct GameServer {
   GameServer(ConnectionManager &cm);
   void newConnection(int id);
   void disconnect(int id);
   void processPacket(pack::Packet p, int fromid);

   ConnectionManager &cm;
};

#endif