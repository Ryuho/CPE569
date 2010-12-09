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
   void updateNPCs(int ticks, float dt);
   void updateMissiles(int ticks, float dt);
   void updatePlayers(int ticks, float dt);

   //Utilities
   void sendPlayerArrow(Player &p, vec2 dir);
   void sendPlayerSpecial(Player &player);
   void sendPlayerInitData(int id);
   void sendPlayerAOI(Player &p, ObjectHolder &oh);
   void removeObject(ObjectBase &obj);
   void removePlayer(Player &p);
   bool collectItem(Player &pl, Item &item); //collect, not collide
   bool collideMissile(Player &p, Missile &m);
   bool collideMissile(NPC &npc, Missile &m);
   bool collideItem(ObjectBase &o, Item &item); //do not use for missiles
   NPC *spawnNPC(int regionX, int regionY);
   Item *spawnItem(int id);
   Item *spawnStump(int id);

   //members
   ConnectionManager &cm;
   ObjectHolder om;
   ObjectHolder som; //other servers' objects
   int rsid; //remote server id, -1 if you are the main server

   int ticks;
   float dt;

   int totalUpdates;

   void processClientPacket(pack::Packet p, int fromid);
   void processServerPacket(pack::Packet p, int fromid);
private:
   void removeClientConnection(int id);
};

GameServer &getGS(); //singleton

#endif // _GAME_SERVER_H_