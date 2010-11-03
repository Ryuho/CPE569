#include "GameServer.h"
#include "Constants.h"
#include <cstdio>

using namespace pack;

GameServer *serverState;

void spawnNPC(int id)
{
   NPC n(id);

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
   //printf("server npc id=%d type=%d (1)\n", n.id, n.type);
}

GameServer::GameServer(ConnectionManager &cm) : cm(cm)
{
   serverState = this;
   ticks = 0;
   dt = 0;
   Player p(5);
   p.update();
   p.draw();
}

void GameServer::newConnection(int id)
{
   printf("New connection: %d\n", id);
   
   vec2 pos(rand()%500, rand()%500);
   
   Player newPlayer(id, pos, vec2(0,1));
   objs.addPlayer(newPlayer);

   cm.sendPacket(Connect(id), id);
   cm.broadcast(Initialize(newPlayer.id, Initialize::player, 0, newPlayer.pos, newPlayer.dir));

   for(unsigned i = 0; i < objs.players.size(); i++) {
      Player &p = objs.players[i];
      Initialize init(p.id, Initialize::player, 0, p.pos, p.dir);
      cm.sendPacket(init, id);
   }
   
   /**spawnNPC(npcid);
   NPC *npc = objs.getNPC(npcid);
   cm.broadcast(UnitSpawn(npcid, npc->type).makePacket());
   cm.broadcast(pack::Pos(npc->pos, npcid));
   //printf("server npc id=%d type=%d (2)\n", npc->id, npc->type);
   for(unsigned i = 0; i < objs.npcs.size(); i++) {
      cm.sendPacket(UnitSpawn(objs.npcs[i].id, objs.npcs[i].type).makePacket(), id);
      cm.sendPacket(pack::Pos(objs.npcs[i].pos, objs.npcs[i].id), id);
   }**/
   /*for(unsigned i = 0; i < objs.players.size(); i++) {
      cm.sendPacket(Signal(Signal::playerconnect, objs.players[i].id), id);
      cm.sendPacket(pack::Pos(objs.players[i].pos, objs.players[i].id), id);
   }*/
}

void GameServer::disconnect(int id)
{
   printf("Client %d disconnected\n", id);
   cm.broadcast(Signal(Signal::disconnect, id).makePacket());
   objs.removeObject(id);
}

void GameServer::processPacket(pack::Packet p, int id)
{
   if (p.type == pack::pos) {
      Pos pos(p);
      pos.id = id;
      Player *player = objs.getPlayer(id);
      if(!player)
         printf("GameServer::processPacket: Player %d does not exist\n", id);
      else {
         objs.getPlayer(id)->moveTo(pos.v);
         cm.broadcast(pos);
      }
   }
	else if (p.type == pack::signal) {
		printf("recieved signal! id = %d\n", id);
		Player* play = objs.getPlayer(id);
		for (int i = 0; i < constants::numArrows; i++) {
         float t = i/(float)constants::numArrows;
         int arrowID = newId();
			Arrow ar(vec2(cos(t*2*constants::PI), sin(t*2*constants::PI)), play->pos, arrowID);
			Missile arrow(arrowID);
         arrow.init(play->pos, vec2(cos(t*2*constants::PI), sin(t*2*constants::PI)), Missile::Arrow);
			objs.addMissile(arrow);
			arrow.id = arrowID;
			cm.broadcast(ar);
      }
	}
	else if (p.type == pack::arrow) {
		//printf("recieved arrow!\n");
		int arrowID = newId();
		Arrow ar(p);
		Player* play = objs.getPlayer(id);		
		Missile m(arrowID);
		//m.init(play->pos, ar.orig, Missile::Arrow);				
		m.init(ar.orig, ar.direction, Missile::Arrow);
      objs.addMissile(m);		
		ar.id = arrowID;
		ar.orig = play->pos;
		//ar.orig = ar.direction;		
		//ar.direction = play->pos;
	
		cm.broadcast(ar);
	}
}

void GameServer::update(int ticks)
{
   dt = (float)((ticks - this->ticks)/1000.0);
   this->ticks = ticks;

   std::vector<vec2> playerPoses;
   for(unsigned i = 0; i < objs.players.size(); i++) {
      playerPoses.push_back(objs.players[i].pos);
   }
   for(unsigned i = 0; i < objs.npcs.size(); i++) {
      if(objs.npcs[i].alive) {
         objs.npcs[i].updateServer(playerPoses);
         if(objs.npcs[i].moving)
            cm.broadcast(pack::Pos(objs.npcs[i].pos, objs.npcs[i].id));
         else
            cm.broadcast(Signal(Signal::stopped, objs.npcs[i].id));
      }
      else { //!alive
         cm.broadcast(pack::Signal(Signal::death,objs.npcs[i].id).makePacket());
         objs.removeObject(objs.npcs[i].id);
      }
   }
}

namespace game {

Player &getPlayer()
{
   assert(false);
   static Player p(0);
   return p;
}

int getTicks()
{
   return serverState->ticks;
}

float getDt()
{
   return serverState->dt;
}

} // end game namespace

// empty stubs of character graphics-related functions

void initCharacterResources() {}
void Player::draw() {}
void Missile::draw() {}
void Item::initGraphics() {}
void Item::draw() {}
void NPC::initGraphics() {}
void NPC::resetAnimation() {}
void NPC::draw() {}
