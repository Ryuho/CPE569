#include "GameServer.h"
#include "Packets.h"
#include "ServerUtil.h"
#include "Constants.h"
#include <cstdio>

using namespace pack;
using namespace server;

void GameServer::processServerPacket(pack::Packet p, int id)
{
   if(p.type == PacketType::serialPlayer) {
      Player obj(p);
      if(!om.contains(obj.getId()))
         om.add(new Player(obj));
   }
   else if(p.type == PacketType::serialItem) {
      Item obj(p);
      if(!om.contains(obj.getId()))
         om.add(new Item(obj));
   }
   else if(p.type == PacketType::serialMissile) {
      Missile obj(p);
      if(!om.contains(obj.getId()))
         om.add(new Missile(obj));
   }
   else if(p.type == PacketType::serialNPC) {
      NPC obj(p);
      if(!om.contains(obj.getId()))
         om.add(new NPC(obj));
   }
   else if(p.type == PacketType::signal) {
      Signal signal(p);
      if(signal.sig == Signal::remove) {
         if(om.contains(signal.val))
            om.remove(signal.val);
      }
      else
         printf("Error: unknown signal (sig=%d val=%d)\n", 
            signal.sig, signal.val);
   }
   else
      printf("Error: unknown server packet, size: %d\n", p.data.size());
}

void GameServer::newServerConnection(int id)
{
   printf("New server connection: %d\n", id);

   //tell new server about previous players (includes self)
   for(unsigned i = 0; i < om.playerCount(); i++) {
      Player &obj = *static_cast<Player *>(om.get(ObjectType::Player, i));
      if(obj.sid == cm.ownServerId)
         cm.serverSendPacket(obj.serialize(), id);
   }
   //tell new server about previous Items
   for(unsigned i = 0; i < om.itemCount(); i++) {
      Item &obj = *static_cast<Item *>(om.get(ObjectType::Item, i));
      if(obj.sid == cm.ownServerId)
         cm.serverSendPacket(obj.serialize(), id);
   }
   //tell new server about previous NPCs
   for(unsigned i = 0; i < om.npcCount(); i++) {
      NPC &obj = *static_cast<NPC *>(om.get(ObjectType::NPC, i));
      if(obj.sid == cm.ownServerId)
         cm.serverSendPacket(obj.serialize(), id);
   }
   //tell new servers about previous Missiles
   for(unsigned i = 0; i < om.npcCount(); i++) {
      Missile &obj = *static_cast<Missile *>(om.get(ObjectType::Missile, i));
      if(obj.sid == cm.ownServerId)
         cm.serverSendPacket(obj.serialize(), id);
   }
}

void GameServer::serverDisconnect(int id)
{
   printf("Server %d disconnected\n", id);
}

void GameServer::removeClientConnection(int id)
{
   cm.removeClientConnection(id);
}

void GameServer::serverSendPacket(Packet &p, int id)
{
   cm.serverSendPacket(p, id);
}

void GameServer::clientSendPacket(Packet &p, int id)
{
   cm.clientSendPacket(p, id);
}

void GameServer::clientBroadcast(Packet &p)
{
   cm.clientBroadcast(p);
}

void GameServer::serverBroadcast(Packet &p)
{
   cm.serverBroadcast(p);
}

void GameServer::update(int ticks)
{
   //get the current delta time (time passed since it last ran update())
   dt = (ticks - this->ticks)/1000.0f;
   this->ticks = ticks;


   if(rsid < 0) {
      //if there is a player connected, spawn NPCs, evenly distributed
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
}