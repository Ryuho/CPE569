#include "GameServer.h"
#include "Constants.h"
#include <cstdio>
#include <time.h>
#include "Numbers.h"

using namespace pack;

GameServer *serverState;

vec2 randPos(int minxy, int maxxy)
{
   float x = (float)minxy + (rand() % (maxxy - minxy));
   float y = (float)minxy + (rand() % (maxxy - minxy));
   return vec2(x, y);
}

vec2 randPos2(int minRadius, int maxRadius)
{
   float angle = (float) (rand() % 359);
   float radius = (float)(minRadius + (rand() % (maxRadius - minRadius)));
   return vec2(sin(angle)*radius, cos(angle)*radius);
}

void spawnNPC(int id)
{
   NPC n(id, randPos(400, 1200), vec2(0,1), (rand() % ((int)NPCType::Goblin)));
   serverState->om.addNPC(n);
   printf("Spawn NPC id=%d type=%d\n", n.id, n.type);
   serverState->cm.broadcast(Initialize(n.id, ObjectType::NPC, 
      n.type, n.pos, n.dir, 0).makePacket());
}

void spawnItem(int id)
{
   vec2 pos = randPos2(200, 350);
   //vec2 pos = vec2(500, 500);
   Item item(id, pos, rand() % (ItemType::Explosion));
   serverState->om.addItem(item);
   printf("Spawn Item id=%d type=%d\n", item.id, item.type);
   serverState->cm.broadcast(Initialize(item.id, ObjectType::Item, item.type,
         item.pos, vec2(), 0));
}

void spawnStump(int id)
{
   vec2 pos = randPos2(200, 350);
   //vec2 pos = vec2(500, 500);
   Item item(id, pos, ItemType::Stump);
   serverState->om.addItem(item);
   printf("Spawn Stump id=%d type=%d\n", item.id, item.type);
   serverState->cm.broadcast(Initialize(item.id, ObjectType::Item, item.type,
         item.pos, vec2(), 0));
}

GameServer::GameServer(ConnectionManager &cm) : cm(cm)
{
   serverState = this;
   ticks = 0;
   dt = 0;

   //spawnItem(newId());
   spawnStump(newId());
}

void GameServer::newConnection(int id)
{
   printf("New connection: %d\n", id);
   
   vec2 pos((float)(rand()%200), (float)(rand()%200));
   
   Player newPlayer(id, pos, vec2(0,1), playerMaxHp);
   om.addPlayer(newPlayer);

	cm.sendPacket(Connect(id, constants::worldHeight, constants::worldWidth), id);
	cm.broadcast(Initialize(newPlayer.id, ObjectType::Player, 0, newPlayer.pos, newPlayer.dir, newPlayer.hp));

   //tell old players about new player
   for(unsigned i = 0; i < om.players.size(); i++) {
      Player &p = *om.players[i];
      Initialize init(p.id, ObjectType::Player, 0, p.pos, p.dir, p.hp);
      cm.sendPacket(init, id);
   }
   //tell new player about previous Items
   for(unsigned i = 0; i < om.items.size(); i++) {
      Item &item = *om.items[i];
      cm.sendPacket(Initialize(item.id, ObjectType::Item, item.type,
         item.pos, vec2(), 0), id);
   }
   //tell new player about previous NPCs
   for(unsigned i = 0; i < om.npcs.size(); i++) {
      NPC &npc = *om.npcs[i];
      cm.sendPacket(Initialize(npc.id, ObjectType::NPC, npc.type, npc.pos, npc.dir, 0).makePacket(), id);
   }
   //make a new NPC and tell everybody about it
   int npcid = newId();
   spawnNPC(npcid);
   //NPC *npc = om.getNpc(npcid);
   //cm.broadcast(Initialize(npcid, ObjectType::NPC, npc->type, npc->pos, npc->dir, 0).makePacket());
}

void GameServer::disconnect(int id)
{
   printf("Client %d disconnected\n", id);
   cm.broadcast(Signal(Signal::remove, id).makePacket());
   om.remove(id);
}

void GameServer::processPacket(pack::Packet p, int id)
{
   if (p.type == pack::position) {
      Position pos(p);
      if(om.check(id, ObjectType::Player)) {
         om.getPlayer(id)->move(pos.pos, pos.dir, pos.moving != 0);
         pos.id = id;
         cm.broadcast(pos);
      } else
         printf("Accessing unknown Player %d\n", pos.id);
   }
   else if (p.type == pack::signal) {
      Signal signal(p);

      if (signal.sig == Signal::special) {
         Player* play = om.getPlayer(id);
         for (int i = 0; i < constants::numArrows; i++) {
            float t = i/(float)constants::numArrows;
            int arrowID = newId();
            Missile m(newId(),id, play->pos, vec2(cos(t*2*constants::PI), sin(t*2*constants::PI)));
            om.addMissile(m);
            Initialize init(m.id, ObjectType::Missile, m.type, m.pos, m.dir, 0);
            cm.broadcast(init);
         }
      }
      else if (signal.sig == Signal::hurtme) {
         Player *p = om.getPlayer(id);
         p->takeDamage(rand()%6 + 5);
         cm.broadcast(HealthChange(id, p->hp));
      }
   }
   else if (p.type == pack::arrow) {
      Arrow ar(p);
      Missile m(newId(),id, om.getPlayer(id)->pos, ar.direction);
      om.addMissile(m);
      Initialize init(m.id, ObjectType::Missile, m.type, m.pos, m.dir, 0);
      cm.broadcast(init);
   }
   else if (p.type == pack::click) {
      Click click(p);
      for(unsigned i = 0; i < om.items.size(); i++) {
         Item &item = *om.items[i];
         if(item.getGeom().collision(new geom::Point(click.pos))) {
            om.remove(item.id);
            cm.broadcast(Signal(Signal::remove, item.id));
            spawnItem(newId());
            break;
         } 
      }
   }
   else
      printf("Unknown packet type=%d size=%d\n", p.type, p.size);
}

void GameServer::update(int ticks)
{
   //get the current delta time (time passed since it last ran update())
   dt = (float)((ticks - this->ticks)/1000.0);
   this->ticks = ticks;

   //if there is a player connected, spawn up to 6 NPCs
   if(om.players.size() > 0) {
      if(rint(5) == 0 && om.npcs.size() <= 6){
         int npcid = newId();
         spawnNPC(npcid);
      }
   }
   
   //players loop, checks for hp == 0, or if it is hit with an arrow
   for(size_t pidx = 0; pidx < om.players.size(); pidx++) {
      //if player is colliding with any missle that is not owned by it, take dmg
      for(size_t midx = 0; midx < om.missiles.size(); midx++){
         if( om.players[pidx]->id != om.missiles[midx]->owned &&
               om.players[pidx]->getGeom().collision(om.missiles[midx]->getGeom()) ){
            om.players[pidx]->takeDamage(rand()%6 + 5);
            cm.broadcast(HealthChange(om.players[pidx]->id, om.players[pidx]->hp));
            cm.broadcast(Signal(Signal::remove, om.missiles[midx]->id).makePacket());
            om.remove(om.missiles[midx]->id);
         }
      }

      //if hp is 0, remove the player
      if(om.players[pidx]->hp == 0){
         cm.broadcast(Signal(Signal::remove, om.players[pidx]->id).makePacket());
         cm.removeConnection(om.players[pidx]->id);
         om.remove(om.players[pidx]->id);
         pidx--;
         continue;
      }
   }
   

   //missles loop, checks for missles TOF, remove if above set value, else move the position
   for(size_t i = 0; i < om.missiles.size(); i++) {
      //missle out of bound
      if(ticks - om.missiles[i]->spawnTime  >= 500){
         cm.broadcast(Signal(Signal::remove, om.missiles[i]->id).makePacket());
         om.remove(om.missiles[i]->id);
         i--;
      }
      else{
         Missile &missle = *om.missiles[i];
         missle.update();
      }
   }

   //NPC
   bool removeNPC = false;
   for(size_t i = 0; i < om.npcs.size(); i++) {
      if(om.players.size() > 0) {
         removeNPC = false;
         for(size_t j = 0; j < om.missiles.size(); j++) {
            if( om.npcs[i]->getGeom().collision(om.missiles[j]->getGeom()) ){
               cm.broadcast(Signal(Signal::remove, om.missiles[j]->id).makePacket());
               om.remove(om.missiles[j]->id);
               removeNPC = true;
               break;
            }
         }
         if(removeNPC){
            cm.broadcast(Signal(Signal::remove, om.npcs[i]->id).makePacket());
            om.remove(om.npcs[i]->id);
            i--;
         }
         else{
            NPC &npc = *om.npcs[i];
            npc.update();
            cm.broadcast(pack::Position(npc.pos, npc.dir, false, npc.id));
         }
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

ObjectManager &getOM()
{
   return serverState->om;
}


// empty stubs of character graphics-related functions
