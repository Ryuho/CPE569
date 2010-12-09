#include "GameServer.h"
#include "Packets.h"
#include "ServerUtil.h"
#include "Constants.h"
#include <cstdio>

using namespace pack;
using namespace server;

void GameServer::newClientConnection(int id)
{
   printf("New client connection: %d\n", id);
   
   vec2 pos((float)(rand()%200), (float)(rand()%200));
   
   Player *newPlayer 
      = new Player(id, cm.ownServerId, pos, vec2(0,1), playerMaxHp);
   om.add(newPlayer);

	clientSendPacket(Connect(id, constants::worldHeight, 
      constants::worldWidth), id);
   sendPlayerInitData(id);
   clientBroadcast(newPlayer->cserialize());
   serverBroadcast(newPlayer->serialize());
}

void GameServer::clientDisconnect(int id)
{
   printf("Client %d disconnected\n", id);
   Packet removePacket(Signal(Signal::remove, id).makePacket());
   clientBroadcast(removePacket);
   serverBroadcast(removePacket);
   om.remove(id);
}

void GameServer::sendPlayerAOI(Player &p, ObjectHolder &oh)
{
   Geometry aoi(Circle(p.pos, areaOfInfluenceRadius));
   std::vector<NPCBase *> aoinpcs;
   oh.collidingNPCs(aoi, p.pos, aoinpcs);
   for(unsigned i = 0; i < aoinpcs.size(); i++) {
      NPC &npc = *static_cast<NPC *>(aoinpcs[i]);
      clientSendPacket(HealthChange(npc.getId(), npc.hp), p.getId());
      clientSendPacket(Position(npc.pos, npc.dir, npc.moving, npc.getId()), 
         p.getId());
   }
   std::vector<PlayerBase *> aoiplayers;
   oh.collidingPlayers(aoi, p.pos, aoiplayers);
   for(unsigned i = 0; i < aoiplayers.size(); i++) {
      Player &player = *static_cast<Player *>(aoiplayers[i]);
      clientSendPacket(HealthChange(player.getId(), player.hp), p.getId());
      if(player.getId() != p.getId()) {
         clientSendPacket(Position(player.pos, player.dir, player.moving, 
            player.getId()),  p.getId());
      }
   }
}

void GameServer::removePlayer(Player &p)
{
   clientBroadcast(Signal(Signal::remove, p.getId()));
   serverBroadcast(Signal(Signal::remove, p.getId()));
   removeClientConnection(p.getId());
   om.remove(p.getId());
}

void GameServer::removeObject(ObjectBase &obj)
{
   clientBroadcast(Signal(Signal::remove, obj.getId()));
   serverBroadcast(Signal(Signal::remove, obj.getId()));
   om.remove(obj.getId());
}

void GameServer::update(int ticks)
{
   //get the current delta time (time passed since it last ran update())
   dt = (ticks - this->ticks)/1000.0f;
   this->ticks = ticks;


   //if there is a player connected, spawn up to 50 NPCs, evenly distributed
   if(om.npcCount() < constants::npcQuantity){
      for(unsigned i = 0; i < regionXSize; i++) {
         for(unsigned j = 0; j < regionYSize; j++) {
            NPC *npc = spawnNPC(i, j);
            clientBroadcast(npc->cserialize());
         }
      }
   }

   updateNPCs(ticks, dt);
   updatePlayers(ticks, dt);
   updateMissiles(ticks, dt); //updated last to ensure near monsters are hit
   totalUpdates++;
}

void GameServer::sendPlayerArrow(Player &player, vec2 dir)
{
   player.shotThisFrame = true;
   Missile *m = new Missile(newId(), cm.ownServerId, player.getId(), player.pos, 
      dir);
   om.add(m);
   clientBroadcast(m->cserialize());
}

////////////////////////////////////////////
/////////// Multi Server Methods ///////////
////////////////////////////////////////////
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

void GameServer::processServerPacket(pack::Packet p, int fromid) 
{
   printf("Single Game Server Implementation!!!\n");
   printf("Illegal action: processServerPacket.\n");
   exit(EXIT_FAILURE); 
}

void GameServer::serverDisconnect(int id) 
{
   printf("Single Game Server Implementation!!!\n");
   printf("Illegal action: serverDisconnect.\n");
   exit(EXIT_FAILURE); 
}

void GameServer::serverSendPacket(Packet &p, int id)
{
   return; //do nothing
}

void GameServer::serverBroadcast(Packet &p)
{
   return;  //do nothing
}