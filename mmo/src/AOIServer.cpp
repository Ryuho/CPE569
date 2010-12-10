#include "GameServer.h"
#include "Packets.h"
#include "ServerUtil.h"
#include "Constants.h"
#include <cstdio>

using namespace pack;
using namespace server;

//How often you should send init packets vs positions
const static int positionPackets = 20;

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
   sendPlayerAOI(*newPlayer, om);
}

void GameServer::clientDisconnect(int id)
{
   printf("Client %d disconnected\n", id);
   Player &p = *om.getPlayer(id);
   Geometry g(p.getGeom());
   std::vector<PlayerBase *> collidedPlayers;
   om.collidingPlayers(g, p.pos, collidedPlayers);
   Packet removePacket(Signal(Signal::remove, id).makePacket());
   for(unsigned i = 0; i < collidedPlayers.size(); i++) {
	   clientSendPacket(removePacket, collidedPlayers[i]->getId());
   }
   //Packet removePacket(Signal(Signal::remove, id).makePacket());
   //clientBroadcast(removePacket);
   //serverBroadcast(removePacket);
   om.remove(id);
}

void GameServer::sendPlayerAOI(Player &p, ObjectHolder &oh)
{
   bool initRun = positionPackets%positionPackets == 0;
   Geometry aoi(Circle(p.pos, areaOfInfluenceRadius));
   std::vector<NPCBase *> aoinpcs;
   oh.collidingNPCs(aoi, p.pos, aoinpcs);
   for(unsigned i = 0; i < aoinpcs.size(); i++) {
      NPC &npc = *static_cast<NPC *>(aoinpcs[i]);
      if(initRun) {
	      clientSendPacket(npc.cserialize(), p.getId());
      }
      else {
         clientSendPacket(pack::Position(npc.pos, npc.dir, 
            npc.moving, npc.getId()), p.getId());
      }
   }
   std::vector<PlayerBase *> aoiplayers;
   oh.collidingPlayers(aoi, p.pos, aoiplayers);
   for(unsigned i = 0; i < aoiplayers.size(); i++) {
      Player &player = *static_cast<Player *>(aoiplayers[i]);
      clientSendPacket(HealthChange(player.getId(), player.hp), p.getId());
      if(player.getId() != p.getId()) {
         if(initRun) {
		      clientSendPacket(p.cserialize(), p.getId());
         }
         else {
            clientSendPacket(pack::Position(player.pos, player.dir,
               player.moving, player.getId()), p.getId());
         }
      }
   }

   std::vector<ItemBase *> aoiitems;
   oh.collidingItems(aoi, p.pos, aoiitems);
   for(unsigned i = 0; i < aoiitems.size(); i++) {
	   Item &item = *static_cast<Item *>(aoiitems[i]);
      if(initRun)
	      clientSendPacket(item.cserialize(), p.getId());
   }

   //std::vector<MissileBase *> aoimissile;
   //oh.collidingMissiles(aoi, p.pos, aoimissile);
   //for(unsigned i = 0; i < aoimissile.size(); i++) {
	//   Missile &missile = *static_cast<Missile *>(aoimissile[i]);
	//   clientSendPacket(missile.cserialize(), p.getId());
   //}
}

void GameServer::removePlayer(Player &p)
{
   Geometry aoi(Circle(p.pos, areaOfInfluenceRadius));
   std::vector<PlayerBase *> aoiplayers;
   om.collidingPlayers(aoi, p.pos, aoiplayers);
   for(unsigned i = 0; i < aoiplayers.size(); i++) {
      Player &player = *static_cast<Player *>(aoiplayers[i]);
	   clientSendPacket(Signal(Signal::remove, p.getId()), player.getId());
   }
	//clientBroadcast(Signal(Signal::remove, p.getId()));
   //serverBroadcast(Signal(Signal::remove, p.getId()));
   removeClientConnection(p.getId());
   om.remove(p.getId());
}

void GameServer::removeObject(ObjectBase &obj)
{
   Geometry aoi(Circle(obj.pos, areaOfInfluenceRadius));
   std::vector<PlayerBase *> aoiplayers;
   om.collidingPlayers(aoi, obj.pos, aoiplayers);
   for(unsigned i = 0; i < aoiplayers.size(); i++) {
      Player &player = *static_cast<Player *>(aoiplayers[i]);
	   clientSendPacket(Signal(Signal::remove, obj.getId()), player.getId());
   }
   om.remove(obj.getId());
}

void GameServer::update(int ticks)
{
   //get the current delta time (time passed since it last ran update())
   dt = (ticks - this->ticks)/1000.0f;
   this->ticks = ticks;


   //if there is a player connected, spawn up to 50 NPCs, evenly distributed
   if(om.npcCount() < constants::npcQuantity) {
      for(unsigned i = 0; i < regionXSize; i++) {
         for(unsigned j = 0; j < regionYSize; j++) {
            NPC *npc = spawnNPC(i, j);
            //clientBroadcast(npc->cserialize()
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
   createMissile(m);
}

void GameServer::createObject(ObjectBase *obj)
{
   om.add(obj);
}

void GameServer::createMissile(Missile *m)
{
   createObject(m);
   Geometry aoi(Circle(m->pos, missileInfluenceRadius));
   std::vector<PlayerBase *> aoiplayers;
   om.collidingPlayers(aoi, m->pos, aoiplayers);
   for(unsigned i = 0; i < aoiplayers.size(); i++) {
      clientSendPacket(m->cserialize(), aoiplayers[i]->getId());
   }
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