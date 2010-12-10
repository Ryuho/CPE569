#include "GameServer.h"
#include "Packets.h"
#include "ServerUtil.h"
#include "Constants.h"
#include <cstdio>

using namespace pack;
using namespace server;

const static int positionPackets = 20;

//TODO: update this!!!
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
   //clientBroadcast(newPlayer->cserialize());

   sendPlayerAOI(*newPlayer, om);
   sendPlayerAOI(*newPlayer, som);
   //sendPlayerInitData(id, om);
   //sendPlayerInitData(id, som);
   //TODO: make a targeted send to other servers
   serverBroadcast(newPlayer->serialize());
}

//TODO: update for multi-server as needed
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
   //TODO: update to remove server-server broadcast
   serverBroadcast(removePacket);
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
}

void updateServers()
{
   //tell new server about previous players (includes self)
   for(unsigned i = 0; i < getOM().playerCount(); i++) {
      Player &obj = *static_cast<Player *>(getOM().get(ObjectType::Player, i));
      getGS().serverBroadcast(obj.serialize());
   }
   //tell new server about previous Items
   for(unsigned i = 0; i < getOM().itemCount(); i++) {
      Item &obj = *static_cast<Item *>(getOM().get(ObjectType::Item, i));
      getGS().serverBroadcast(obj.serialize());
   }
   //tell new server about previous NPCs
   for(unsigned i = 0; i < getOM().npcCount(); i++) {
      NPC &obj = *static_cast<NPC *>(getOM().get(ObjectType::NPC, i));
      getGS().serverBroadcast(obj.serialize());
   }
   //tell new servers about previous Missiles
   for(unsigned i = 0; i < getOM().missileCount(); i++) {
      Missile &obj = *static_cast<Missile *>(getOM().get(ObjectType::Missile, i));
      getGS().serverBroadcast(obj.serialize());
   }
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
   //TODO: update to remove server->server broadcast
   serverBroadcast(Signal(Signal::remove, p.getId()));
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
   //TODO: update to remove server->server broadcast
   serverBroadcast(Signal(Signal::remove, obj.getId()));
   om.remove(obj.getId());
}

void GameServer::update(int ticks)
{
   //get the current delta time (time passed since it last ran update())
   dt = (ticks - this->ticks)/1000.0f;
   this->ticks = ticks;

   updatePlayers(ticks, dt);
   updateNPCs(ticks, dt);
   updateMissiles(ticks, dt); //updated last to ensure near monsters are hit
   if(rsid < 0) {
      //if there is a player connected, spawn NPCs, evenly distributed
      if(om.npcCount() < constants::npcQuantity) {
         for(unsigned i = 0; i < regionXSize; i++) {
            for(unsigned j = 0; j < regionYSize; j++) {
               NPC *npc = spawnNPC(i, j);
               //clientBroadcast(npc->cserialize());
            }
         }
      }
   }
   updateServers();
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
   //TODO: remove server->server broadcast
   serverBroadcast(om.getSerialized(obj->getId()));
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
   printf("New server connection: %d\n", id);
}

void GameServer::serverDisconnect(int id)
{
   printf("Server %d disconnected\n", id);
}

void GameServer::serverSendPacket(Packet &p, int id)
{
   cm.serverSendPacket(p, id);
}

void GameServer::serverBroadcast(Packet &p)
{
   cm.serverBroadcast(p);
}

void GameServer::processServerPacket(pack::Packet p, int id)
{
   if(p.type == PacketType::serialPlayer) {
      Player obj(p);
      if(!som.contains(obj.getId())) {
         som.add(new Player(obj));
         clientBroadcast(obj.cserialize());
      }
      else {
         Player *obj2 = som.getPlayer(obj.getId());
         *obj2 = obj;
      }
   }
   else if(p.type == PacketType::serialItem) {
      Item obj(p);
      if(!som.contains(obj.getId())) {
         som.add(new Item(obj));
         //clientBroadcast(obj.cserialize());
      }
      else {
         Item *obj2 = som.getItem(obj.getId());
         *obj2 = obj;
      }
   }
   else if(p.type == PacketType::serialMissile) {
      Missile obj(p);
      if(!som.contains(obj.getId())) {
         som.add(new Missile(obj));
         Geometry aoi(Circle(obj.pos, missileInfluenceRadius));
         std::vector<PlayerBase *> aoiplayers;
         om.collidingPlayers(aoi, obj.pos, aoiplayers);
         for(unsigned i = 0; i < aoiplayers.size(); i++) {
            clientSendPacket(obj.cserialize(), aoiplayers[i]->getId());
         }
         //clientBroadcast(obj.cserialize());
      }
      //else {
         //Missile *obj2 = som.getMissile(obj.getId());
         //*obj2 = obj;
      //}
   }
   else if(p.type == PacketType::serialNPC) {
      NPC obj(p);
      if(!som.contains(obj.getId())) {
         som.add(new NPC(obj));
         //clientBroadcast(obj.cserialize());
      }
      else {
         NPC *obj2 = som.getNPC(obj.getId());
         *obj2 = obj;
      }
   }
   else if(p.type == PacketType::signal) {
      Signal signal(p);
      if(signal.sig == Signal::remove) {
         if(som.contains(signal.val)) {
            som.remove(signal.val);
            clientBroadcast(signal);
            //may still remove other server's objects... and no easy way to check
         }
         else if(om.contains(signal.val)) {
            om.remove(signal.val);
            clientBroadcast(signal);
            printf("Server %d requesting remote removal of %d\n", 
               id, signal.val);
         }
      }
      else
         printf("Error: unknown signal (sig=%d val=%d)\n", 
            signal.sig, signal.val);
      /*Signal signal(p);
      if(signal.sig == Signal::remove) {
         if(som.contains(signal.val)) {
            Item *obj = som.getItem(signal.val);
            Geometry aoi(Circle(obj->pos, areaOfInfluenceRadius));
            std::vector<PlayerBase *> aoiplayers;
            som.collidingPlayers(aoi, obj->pos, aoiplayers);
            for(unsigned i = 0; i < aoiplayers.size(); i++) {
               Player &player = *static_cast<Player *>(aoiplayers[i]);
	            clientSendPacket(Signal(Signal::remove, obj->getId()), player.getId());
            }
            som.remove(signal.val);
            //clientBroadcast(signal);
            //may still remove other server's objects... and no easy way to check
         }
         else if(om.contains(signal.val)) {
            removeObject(*om.getItem(signal.val));
            //om.remove(signal.val);
            //clientBroadcast(signal);
            printf("Server %d requesting remote removal of %d\n", 
               id, signal.val);
         }
      }
      else
         printf("Error: unknown signal (sig=%d val=%d)\n", 
            signal.sig, signal.val);*/
   }
   else
      printf("Error: unknown server packet, size: %d\n", p.data.size());
}