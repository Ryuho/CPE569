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
      if(!som.contains(obj.getId())) {
         som.add(new Player(obj));
         clientBroadcast(obj.cserialize());
      }
   }
   else if(p.type == PacketType::serialItem) {
      Item obj(p);
      if(!som.contains(obj.getId())) {
         som.add(new Item(obj));
         clientBroadcast(obj.cserialize());
      }
   }
   else if(p.type == PacketType::serialMissile) {
      Missile obj(p);
      if(!som.contains(obj.getId())) {
         som.add(new Missile(obj));
         clientBroadcast(obj.cserialize());
      }
   }
   else if(p.type == PacketType::serialNPC) {
      NPC obj(p);
      if(!som.contains(obj.getId())) {
         som.add(new NPC(obj));
         clientBroadcast(obj.cserialize());
      }
   }
   else if(p.type == PacketType::signal) {
      Signal signal(p);
      if(signal.sig == Signal::remove) {
         if(som.contains(signal.val)) {
            som.remove(signal.val);
            clientBroadcast(p);
            //may still remove other server's objects... and no easy way to check
         }
         else if(om.contains(signal.val)) {
            om.remove(signal.val);
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

void GameServer::newServerConnection(int id)
{
   printf("New server connection: %d\n", id);
}

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
   clientBroadcast(Initialize(newPlayer->getId(), ObjectType::Player, 
      0, newPlayer->pos, newPlayer->dir, newPlayer->hp));

   //tell new player about previous players (includes self)
   for(unsigned i = 0; i < om.playerCount(); i++) {
      Player &p = *static_cast<Player *>(om.get(ObjectType::Player, i));
      clientSendPacket(Initialize(p.getId(), ObjectType::Player, 
         0, p.pos, p.dir, p.hp), id);
      if(p.pvp)
         clientSendPacket(Pvp(p.getId(), p.pvp), id);
   }
   //tell new player about previous Items
   for(unsigned i = 0; i < om.itemCount(); i++) {
      Item &item = *static_cast<Item *>(om.get(ObjectType::Item, i));
      clientSendPacket(Initialize(item.getId(), ObjectType::Item, item.type,
         item.pos, vec2(), 0), id);
   }
   //tell new player about previous NPCs
   for(unsigned i = 0; i < om.npcCount(); i++) {
      NPC &npc = *static_cast<NPC *>(om.get(ObjectType::NPC, i));
      clientSendPacket(Initialize(npc.getId(), ObjectType::NPC, 
         npc.type, npc.pos, npc.dir, npc.hp), id);
   }

   serverBroadcast(newPlayer->serialize());
}

void GameServer::serverDisconnect(int id)
{
   printf("Server %d disconnected\n", id);
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

//TODO: update process packet as needed
void GameServer::processClientPacket(pack::Packet p, int id)
{
   if(!om.contains(id, ObjectType::Player)) {
      printf("Error: Client %d sent packet and is not connected", id);
      return;
   }
   Player &player = *static_cast<Player *>(om.getPlayer(id));
   if (p.type == PacketType::position) {
      Position pos(p);
      player.move(pos.pos, pos.dir, pos.moving != 0);
      //player went out of bounds or invalid positon?
      if(pos.pos.x != player.pos.x || pos.pos.y != player.pos.y) {
         clientSendPacket(Teleport(player.pos), id);
         printf("Player %d went outside map bounds\n", id);
      }
   }
   else if(p.type == PacketType::signal) {
      Signal signal(p);
      if (signal.sig == Signal::special) {
         for(int i = 0; i < constants::numArrows; i++) {
            float t = i/(float)constants::numArrows;
            Missile *m = new Missile(newId(), cm.ownServerId, id, player.pos, 
               vec2((float)cos(t*2*PI), (float)sin(t*2*PI)));
            om.add(m);
            clientBroadcast(Initialize(m->getId(), ObjectType::Missile, 
               m->type, m->pos, m->dir, 0));
         }
      }
      else if(signal.sig == Signal::setPvp) {
         player.pvp = signal.val != 0;
         //printf("Player %d PVP: %s\n", player.getId(), player.pvp ? "on" : "off");
         clientBroadcast(Pvp(id, signal.val));
      }
      else if(signal.sig == Signal::hurtme) {
         player.takeDamage(1);
         clientBroadcast(HealthChange(id, player.hp));
      }
      else
         printf("Error: Unknown Signal packet type=%d val=%d\n", 
            signal.sig, signal.val);
   }
   else if (p.type == PacketType::arrow) {
      Arrow ar(p);
      if(!player.shotThisFrame) {
         player.shotThisFrame = true;
         Missile *m = new Missile(newId(), cm.ownServerId, id, player.pos, 
            ar.dir);
         om.add(m);
         clientBroadcast(Initialize(m->getId(), ObjectType::Missile, 
            m->type, m->pos, m->dir, 0));
      }
   }
   else if(p.type == PacketType::click) {
      Click click(p);
      Geometry point(Point(click.pos));
      //printf("Player %d clicked <%0.1f, %0.1f>\n", id, click.pos.x, 
      //   click.pos.y);
      std::vector<ItemBase *> items;
      om.collidingItems(point, click.pos, items);
      if(items.size() > 0) {
         for(unsigned i = 0; i < items.size(); i++) {
            Item &item = *static_cast<Item *>(items[i]);
            if(item.isCollectable()) {
               collectItem(player, item);
            }
         }
      }
   }
   else
      printf("Unknown client packet type=%d size=%d\n", p.type, p.data.size());
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

   std::vector<ItemBase *> aoiitems;
   oh.collidingItems(aoi, p.pos, aoiitems);
   for(unsigned i = 0; i < aoiitems.size(); i++) {
	   Item &item = *static_cast<Item *>(aoiitems[i]);
	   clientSendPacket(Initialize(item.getId(), item.getType(), item.type, item.pos, vec2(), 0), p.getId());
   }
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

void updateServers()
{
   //tell new server about previous players (includes self)
   for(unsigned i = 0; i < getOM().playerCount(); i++) {
      Player &obj = *static_cast<Player *>(getOM().get(ObjectType::Player, i));
      //if(obj.sid == getCM().ownServerId)
         getGS().serverBroadcast(obj.serialize());
   }
   //tell new server about previous Items
   for(unsigned i = 0; i < getOM().itemCount(); i++) {
      Item &obj = *static_cast<Item *>(getOM().get(ObjectType::Item, i));
      //if(obj.sid == getCM().ownServerId)
         getGS().serverBroadcast(obj.serialize());
   }
   //tell new server about previous NPCs
   for(unsigned i = 0; i < getOM().npcCount(); i++) {
      NPC &obj = *static_cast<NPC *>(getOM().get(ObjectType::NPC, i));
      //if(obj.sid == getCM().ownServerId)
         getGS().serverBroadcast(obj.serialize());
   }
   //tell new servers about previous Missiles
   for(unsigned i = 0; i < getOM().missileCount(); i++) {
      Missile &obj = *static_cast<Missile *>(getOM().get(ObjectType::Missile, i));
      //if(obj.sid == getCM().ownServerId)
         getGS().serverBroadcast(obj.serialize());
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

   updatePlayers(ticks, dt);
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
      updateMissiles(ticks, dt); //updated last to ensure near monsters are hit
   }
   updateServers();
}