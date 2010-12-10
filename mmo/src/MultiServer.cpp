#include "GameServer.h"
#include "Packets.h"
#include "ServerUtil.h"
#include "Constants.h"
#include <cstdio>

using namespace pack;
using namespace server;

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
   clientBroadcast(newPlayer->cserialize());

   sendPlayerInitData(id, om);
   sendPlayerInitData(id, som);
   serverBroadcast(newPlayer->serialize());
}

//TODO: update for multi-server as needed
void GameServer::clientDisconnect(int id)
{
   printf("Client %d disconnected\n", id);
   Packet removePacket(Signal(Signal::remove, id).makePacket());
   clientBroadcast(removePacket);
   serverBroadcast(removePacket);
   om.remove(id);
}

//TODO: update as needed for multi-server
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
   //for(unsigned i = 0; i < getOM().missileCount(); i++) {
   //   Missile &obj = *static_cast<Missile *>(getOM().get(ObjectType::Missile, i));
   //   getGS().serverBroadcast(obj.serialize());
   //}
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
   clientBroadcast(om.getCSerialized(obj->getId()));
}

void GameServer::createMissile(Missile *m)
{
   createObject(m);
   serverBroadcast(om.getSerialized(m->getId()));
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
         clientBroadcast(obj.cserialize());
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
         clientBroadcast(obj.cserialize());
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
         clientBroadcast(obj.cserialize());
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
   }
   else
      printf("Error: unknown server packet, size: %d\n", p.data.size());
}