#include "GameServer.h"
#include "Packets.h"
#include "ServerUtil.h"
#include "Constants.h"
#include <cstdio>

using namespace pack;
using namespace server;

void GameServer::newServerConnection(int id) 
{ 
   printf("Single Game Server Implementation!!!\n");
   printf("Forcing disconnect to remote server.\n");
   std::map<int, int>::iterator iter = cm.idToServerIndex.find(id);
   if(iter != cm.idToServerIndex.end())
      cm.removeServerAt(iter->second);
   else
      exit(EXIT_FAILURE); 
}

void GameServer::newClientConnection(int id)
{
   printf("New client connection: %d\n", id);
   
   vec2 pos((float)(rand()%200), (float)(rand()%200));
   
   Player *newPlayer 
      = new Player(id, cm.ownServerId, pos, vec2(0,1), playerMaxHp);
   om.add(newPlayer);

	clientSendPacket(Connect(id, constants::worldHeight, 
      constants::worldWidth), id);
	clientSendPacket(newPlayer->cserialize(), id);

   //tell new player about objects in AOI
	//TODO need to update to account for PVP mode
	Player &p = *static_cast<Player *>(om.getPlayer(newPlayer->getId()));
   sendPlayerAOI(p, om);
}

void GameServer::serverDisconnect(int id) 
{
   printf("Single Game Server Implementation!!!\n");
   printf("Illegal action: serverDisconnect.\n");
   exit(EXIT_FAILURE); 
}

void GameServer::clientDisconnect(int id)
{
   printf("Client %d disconnected\n", id);
   Player &p = *static_cast<Player *>(om.getPlayer(id));
   Geometry g(p.getGeom());
   std::vector<PlayerBase *> collidedPlayers;
   om.collidingPlayers(g, p.pos, collidedPlayers);
   Packet removePacket(Signal(Signal::remove, id).makePacket());
   for(unsigned i = 0; i < collidedPlayers.size(); i++)
   {
	   clientSendPacket(removePacket, collidedPlayers[i]->getId());
   }
   /*Packet removePacket(Signal(Signal::remove, id).makePacket());
   clientBroadcast(removePacket);
   serverBroadcast(removePacket);*/
   om.remove(id);
}

void GameServer::processServerPacket(pack::Packet p, int fromid) 
{
   printf("Single Game Server Implementation!!!\n");
   printf("Illegal action: processServerPacket.\n");
   exit(EXIT_FAILURE); 
}

void GameServer::clientSendPacket(Packet &p, int id)
{
   cm.clientSendPacket(p, id);
}

void GameServer::clientBroadcast(Packet &p)
{
   cm.clientBroadcast(p);
}

void GameServer::serverSendPacket(Packet &p, int id)
{
   return; //do nothing
}

void GameServer::serverBroadcast(Packet &p)
{
   return;  //do nothing
}

void GameServer::removeClientConnection(int id)
{
   cm.removeClientConnection(id);
}

void GameServer::update(int ticks)
{
   //get the current delta time (time passed since it last ran update())
   dt = (ticks - this->ticks)/1000.0f;
   this->ticks = ticks;


   //if there is a player connected, spawn up to 50 NPCs, evenly distributed
   if(om.playerCount() > 0) {
      if(om.npcCount() < 300){
         for(unsigned i = 0; i < regionXSize; i++) {
            for(unsigned j = 0; j < regionYSize; j++) {
               NPC *npc = spawnNPC(i, j);
               clientBroadcast(Initialize(npc->getId(), ObjectType::NPC, 
                  npc->type, npc->pos, npc->dir, npc->hp));
            }
         }
      }
   }

   updateNPCs(ticks, dt);
   updatePlayers(ticks, dt);
   updateMissiles(ticks, dt); //updated last to ensure near monsters are hit
}