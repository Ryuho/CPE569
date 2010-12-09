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
			Geometry aoi(Circle(m->pos, missileInfluenceRadius));
            om.add(m);
            std::vector<MissileBase *> aoiplayers;
			om.collidingMissiles(aoi, m->pos, aoiplayers);
			for(unsigned i = 0; i < aoiplayers.size(); i++) {
				clientSendPacket(m->cserialize(), aoiplayers[i]->getId());
			}
			/*clientBroadcast(Initialize(m->getId(), ObjectType::Missile, 
               m->type, m->pos, m->dir, 0));*/
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
		 Geometry aoi(Circle(m->pos, missileInfluenceRadius));
         std::vector<MissileBase *> aoiplayers;
		 om.collidingMissiles(aoi, m->pos, aoiplayers);
		 for(unsigned i = 0; i < aoiplayers.size(); i++) {
			clientSendPacket(m->cserialize(), aoiplayers[i]->getId());
		 }
         /*clientBroadcast(Initialize(m->getId(), ObjectType::Missile, 
            m->type, m->pos, m->dir, 0));*/
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
/*void GameServer::processClientPacket(pack::Packet p, int id)
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
}*/

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

void GameServer::sendPlayerAOI(Player &p, ObjectHolder &oh)
{
   Geometry aoi(Circle(p.pos, areaOfInfluenceRadius));
   std::vector<NPCBase *> aoinpcs;
   oh.collidingNPCs(aoi, p.pos, aoinpcs);
   for(unsigned i = 0; i < aoinpcs.size(); i++) {
      NPC &npc = *static_cast<NPC *>(aoinpcs[i]);
	  clientSendPacket(npc.cserialize(), p.getId());
   }
   std::vector<PlayerBase *> aoiplayers;
   oh.collidingPlayers(aoi, p.pos, aoiplayers);
   for(unsigned i = 0; i < aoiplayers.size(); i++) {
      Player &player = *static_cast<Player *>(aoiplayers[i]);
      clientSendPacket(HealthChange(player.getId(), player.hp), p.getId());
      if(player.getId() != p.getId()) {
		  clientSendPacket(Initialize(player.getId(), player.getType(), 0, player.pos, player.dir, player.hp), p.getId());
      }
   }

   std::vector<ItemBase *> aoiitems;
   oh.collidingItems(aoi, p.pos, aoiitems);
   for(unsigned i = 0; i < aoiitems.size(); i++) {
	   Item &item = *static_cast<Item *>(aoiitems[i]);
	   clientSendPacket(Initialize(item.getId(), item.getType(), item.type, item.pos, vec2(), 0), p.getId());
   }

   /*std::vector<MissileBase *> aoimissile;
   oh.collidingMissiles(aoi, p.pos, aoimissile);
   for(unsigned i = 0; i < aoimissile.size(); i++) {
	   Missile &missile = *static_cast<Missile *>(aoimissile[i]);
	   clientSendPacket(missile.cserialize(), p.getId());
   }*/
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
   if(om.playerCount() > 0) {
      if(om.npcCount() < 100){
         for(unsigned i = 0; i < regionXSize; i++) {
            for(unsigned j = 0; j < regionYSize; j++) {
               NPC *npc = spawnNPC(i, j);
               //clientBroadcast(Initialize(npc->getId(), ObjectType::NPC, 
               //   npc->type, npc->pos, npc->dir, npc->hp));
            }
         }
      }
   }

   updateNPCs(ticks, dt);
   updatePlayers(ticks, dt);
   updateMissiles(ticks, dt); //updated last to ensure near monsters are hit
}