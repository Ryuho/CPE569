#ifndef _GAME_SERVER_H_
#define _GAME_SERVER_H_

#include "ServerConnManager.h"
#include "ServerData.h"
#include "Objects.h"

using namespace server;

struct GameServer {
   GameServer(ConnectionManager &cm, int remoteServerId);
   void newClientConnection(int id);
   void newServerConnection(int id);

   void clientDisconnect(int id);
   void serverDisconnect(int id);

   void processClientPacket(pack::Packet p, int fromid);
   void processServerPacket(pack::Packet p, int fromid);

   void updateNPCs(int ticks, float dt);
   void updateMissiles(int ticks, float dt);
   void updatePlayers(int ticks, float dt);

   void removeClientConnection(int id);

   void clientSendPacket(Packet &p, int id);
   void clientBroadcast(Packet &p);
   void serverSendPacket(Packet &p, int id);
   void serverBroadcast(Packet &p);
      template<typename T>
   void clientSendPacket(T &t, int id) {
      clientSendPacket(t.makePacket(), id);
   }
   template<typename T>
   void clientBroadcast(T &t) {
      clientBroadcast(t.makePacket());
   }

   template<typename T>
   void serverSendPacket(T &t, int id) {
      serverSendPacket(t.makePacket(), id);
   }
   template<typename T>
   void serverBroadcast(T &t) {
      serverBroadcast(t.makePacket());
   }

   void update(int ticks);

   ConnectionManager &cm;
   ObjectHolder om;

   int ticks;
   float dt;
};

#endif // _GAME_SERVER_H_