#include "GameServer.h"
#include "Constants.h"
#include <cstdio>

using namespace pack;

GameServer *serverState;

void spawnNPC(int id)
{
   int minv, maxv, midv;
   minv = 700;
   maxv = 1200;
   midv = 400;
   vec2 _pos = vec2((float)(rand()%maxv - minv), (float)(rand()%maxv - minv));
   if(_pos.x >= 0.0 && _pos.x < midv)
      _pos.x = (float) midv;
   else if(_pos.x < 0.0 && _pos.x > -midv)
      _pos.x = (float) -midv;
   if(_pos.y >= 0.0 && _pos.y < midv)
      _pos.y = (float) midv;
   else if(_pos.y < 0.0 && _pos.y > -midv)
      _pos.y = (float) -midv;

   NPC n(id, _pos, vec2(0,1), (rand() % ((int)NPCType::Goblin)));
   serverState->om.addNPC(n);
   printf("server npc id=%d type=%d\n", n.id, n.type);
}

GameServer::GameServer(ConnectionManager &cm) : cm(cm)
{
   serverState = this;
   ticks = 0;
   dt = 0;
}

void GameServer::newConnection(int id)
{
   printf("New connection: %d\n", id);
   
   vec2 pos((float)(rand()%200), (float)(rand()%200));
   
   Player newPlayer(id, pos, vec2(0,1));
   om.addPlayer(newPlayer);

   cm.sendPacket(Connect(id), id);
   cm.broadcast(Initialize(newPlayer.id, ObjectType::Player, 0, newPlayer.pos, newPlayer.dir));

   for(unsigned i = 0; i < om.players.size(); i++) {
      Player *p = om.players[i];
      Initialize init(p->id, ObjectType::Player, 0, p->pos, p->dir);
      cm.sendPacket(init, id);
   }
   
   //tell new player about previous NPCs
   for(unsigned i = 0; i < om.npcs.size(); i++) {
      NPC &npc = *om.npcs[i];
      cm.sendPacket(Initialize(npc.id, ObjectType::NPC, npc.type, npc.pos, npc.dir).makePacket(), id);
   }
   //make a new NPC and tell everybody about it
   int npcid = newId();
   spawnNPC(npcid);
   NPC *npc = om.getNpc(npcid);
   cm.broadcast(Initialize(npcid, ObjectType::NPC, npc->type, npc->pos, npc->dir).makePacket());
}

void GameServer::disconnect(int id)
{
   printf("Client %d disconnected\n", id);
   cm.broadcast(Signal(Signal::disconnect, id).makePacket());
   om.remove(id);
}

void GameServer::processPacket(pack::Packet p, int id)
{
   if (p.type == pack::position) {
      Position pos(p);
      om.getPlayer(id)->move(pos.pos, pos.dir, pos.moving != 0);
      pos.id = id;
      cm.broadcast(pos);
   }
	else if (p.type == pack::signal) {
		printf("recieved signal! id = %d\n", id);
		Player* play = om.getPlayer(id);
		for (int i = 0; i < constants::numArrows; i++) {
         float t = i/(float)constants::numArrows;
         int arrowID = newId();
         Missile m(newId(), play->pos, vec2(cos(t*2*constants::PI), sin(t*2*constants::PI)));
         om.addMissile(m);
         Initialize init(m.id, ObjectType::Missile, m.type, m.pos, m.dir);
         cm.broadcast(init);
      }
	}
	else if (p.type == pack::arrow) {
      Arrow ar(p);
      Missile m(newId(), om.getPlayer(id)->pos, ar.direction);
      om.addMissile(m);
      Initialize init(m.id, ObjectType::Missile, m.type, m.pos, m.dir);
      cm.broadcast(init);
	}
}

void GameServer::update(int ticks)
{
   dt = (float)((ticks - this->ticks)/1000.0);
   this->ticks = ticks;

   for(unsigned i = 0; i < om.npcs.size(); i++) {
      if(om.players.size() > 0) {
         NPC &npc = *om.npcs[i];
         npc.pos = vec2(5,5) + om.players[0]->pos;
         npc.update();
         cm.broadcast(pack::Position(npc.pos, npc.dir, false, npc.id));
      }
   }
}

int getTicks()
{
   return serverState->ticks;
}

float getDt()
{
   return serverState->dt;
}

// empty stubs of character graphics-related functions
