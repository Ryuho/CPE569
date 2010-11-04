#include "GameServer.h"
#include "Constants.h"
#include <cstdio>

using namespace pack;

GameServer *serverState;

void spawnNPC(int id)
{
   /*NPC n(id);

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

   n.init(vec2(), (NPC::Type) (rand() % ((int)NPC::MaxNPC)));
   n.moving = true;
   n.dir = vec2(0,1);
   n.alive = true;
   serverState->objs.addNPC(n);
   //printf("server npc id=%d type=%d (1)\n", n.id, n.type);*/
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
   
   vec2 pos(rand()%200, rand()%200);
   
   Player newPlayer(id, pos, vec2(0,1));
   om.addPlayer(newPlayer);

   cm.sendPacket(Connect(id), id);
   cm.broadcast(Initialize(newPlayer.id, ObjectType::Player, 0, newPlayer.pos, newPlayer.dir));

   for(unsigned i = 0; i < om.players.size(); i++) {
      Player *p = om.players[i];
      Initialize init(p->id, ObjectType::Player, 0, p->pos, p->dir);
      cm.sendPacket(init, id);
   }
   
   /**spawnNPC(npcid);
   NPC *npc = objs.getNPC(npcid);
   cm.broadcast(UnitSpawn(npcid, npc->type).makePacket());
   cm.broadcast(pack::Pos(npc->pos, npcid));
   for(unsigned i = 0; i < objs.npcs.size(); i++) {
      cm.sendPacket(UnitSpawn(objs.npcs[i].id, objs.npcs[i].type).makePacket(), id);
      cm.sendPacket(pack::Pos(objs.npcs[i].pos, objs.npcs[i].id), id);
   }
   //Tell the new user about all other users on the server (and their position)
   for(unsigned i = 0; i < objs.players.size(); i++) {
      cm.sendPacket(Signal(Signal::playerconnect, objs.players[i].id), id);
      cm.sendPacket(pack::Pos(objs.players[i].pos, objs.players[i].id), id);
   }*/
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

   /*std::vector<vec2> playerPoses;
   for(unsigned i = 0; i < om.players.size(); i++) {
      playerPoses.push_back(om.players[i].pos);
   }
   for(unsigned i = 0; i < om.npcs.size(); i++) {
      if(om.npcs[i].alive) {
         om.npcs[i].updateServer(playerPoses);
         if(om.npcs[i].moving)
            cm.broadcast(pack::Pos(om.npcs[i].pos, om.npcs[i].id));
         else
            cm.broadcast(Signal(Signal::stopped, om.npcs[i].id));
      }
      else { //!alive
         cm.broadcast(pack::Signal(Signal::death,om.npcs[i].id).makePacket());
         om.removeObject(om.npcs[i].id);
      }
   }*/
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
