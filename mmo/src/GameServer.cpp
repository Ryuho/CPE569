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
   NPC n(id, npcMaxHp, randPos(400, 1200), vec2(0,1), (rand() % ((int)NPCType::Goblin)));
   serverState->om.addNPC(n);
   printf("Spawn NPC id=%d type=%d\n", n.id, n.type);
   serverState->cm.broadcast(Initialize(n.id, ObjectType::NPC, 
      n.type, n.pos, n.dir, n.hp).makePacket());
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
      cm.sendPacket(Initialize(npc.id, ObjectType::NPC, npc.type, npc.pos, npc.dir, npc.hp).makePacket(), id);
   }
   //make a new NPC and tell everybody about it
   int npcid = newId();
   spawnNPC(npcid);
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
         if(om.check(id, ObjectType::Player)) {
            Player &play = *om.getPlayer(id);
            for (int i = 0; i < constants::numArrows; i++) {
               float t = i/(float)constants::numArrows;
               int arrowID = newId();
               Missile m(newId(),id, play.pos, vec2(cos(t*2*PI), sin(t*2*PI)));
               om.addMissile(m);
               Initialize init(m.id, ObjectType::Missile, m.type, m.pos, m.dir, 0);
               cm.broadcast(init);
            }
         }
         else
            printf("Error: Packet Unknown Player id %d\n", id);
      }
      else if (signal.sig == Signal::hurtme) {
         if(om.check(id, ObjectType::Player)) {
            Player *p = om.getPlayer(id);
            p->takeDamage(rand()%6 + 5);
            cm.broadcast(HealthChange(id, p->hp));
         }
         else
            printf("Error: Packet Unknown Player id %d\n", id);
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
      if(om.check(id, ObjectType::Player)) {
         Player &p = *om.getPlayer(click.id);
         for(unsigned i = 0; i < om.items.size(); i++) {
            Item &item = *om.items[i];
            if(item.getGeom().collision(new geom::Point(click.pos))) {
               cm.broadcast(Signal(Signal::remove, item.id));
               int rupees = item.type == ItemType::GreenRupee ? greenRupeeValue :
                  item.type == ItemType::BlueRupee ? blueRupeeValue :
                  item.type == ItemType::RedRupee ? redRupeeValue :
                  0;
               if(rupees > 0) {
                  p.gainRupees(rupees);
                  cm.sendPacket(Signal(Signal::changeRupee, p.rupees), click.id);
               }
               om.remove(item.id); //only remove one item per click max
               break;
            } 
         }
      } 
      else
         printf("Error invalid click Player id %d\n", click.id);
   }
   else
      printf("Unknown packet type=%d size=%d\n", p.type, p.data.size());
}

void GameServer::update(int ticks)
{
   //get the current delta time (time passed since it last ran update())
   dt = (float)((ticks - this->ticks)/1000.0);
   this->ticks = ticks;

   //if there is a player connected, spawn up to 6 NPCs
   if(om.players.size() > 0) {
      if(rint(5) == 0 && om.npcs.size() <= 6){
         spawnNPC(newId());
      }
   }
   
   //players loop, checks for hp == 0, or if it is hit with an arrow
   for(unsigned pidx = 0; pidx < om.players.size(); pidx++) {
      Player &p = *om.players[pidx];
      bool playerHit = false;
      //if player is colliding with any missle that is not owned by it, take dmg
      for(unsigned midx = 0; midx < om.missiles.size(); midx++) {
         Missile &m = *om.missiles[midx];
         if(p.id != m.owned && p.getGeom().collision(m.getGeom())) {
            playerHit = true;
            p.takeDamage(rand()%6 + 5);
            cm.broadcast(Signal(Signal::remove, m.id).makePacket());
            om.remove(m.id);
         }
      }
      //if hp is 0, remove the player
      if(p.hp == 0) {
         cm.broadcast(Signal(Signal::remove, p.id).makePacket());
         cm.removeConnection(p.id);
         om.remove(p.id);
         pidx--;
         continue;
      }
      else if(playerHit) {
         cm.broadcast(HealthChange(p.id, p.hp));
      }
   }

   //missles loop, checks for missles TOF, remove if above set value, else move the position
   for(unsigned i = 0; i < om.missiles.size(); i++) {
      //missle out of bound
      Missile &m = *om.missiles[i];
      if(ticks - m.spawnTime >= 500){
         cm.broadcast(Signal(Signal::remove, m.id).makePacket());
         om.remove(m.id);
         i--;
      }
      else
         m.update();
   }

   //NPC update - collision detection with missiles, death, exp/loot distribution
   for(unsigned i = 0; i < om.npcs.size(); i++) {
      NPC &npc = *om.npcs[i];
      bool removeNPC = false;
      bool npcHit = false;
      for(unsigned j = 0; j < om.missiles.size() && !removeNPC; j++) {
         Missile &m = *om.missiles[j];
         if(npc.getGeom().collision(m.getGeom())) {
            npcHit = true;
            npc.takeDamage(rand()%6);
            if(npc.hp == 0) {
               if(om.check(m.owned, ObjectType::Player)) {
                  Player &p = *om.getPlayer(m.owned);
                  p.gainExp(npc.getExp());
                  cm.sendPacket(Signal(Signal::changeExp, p.exp).makePacket(), p.id);
               }
               removeNPC = true;
            }
            cm.broadcast(Signal(Signal::remove, m.id).makePacket());
            om.remove(m.id);
            j--;
         }
      }
      if(removeNPC) {
         int lootItem = npc.getLoot();
         if(lootItem >= 0) {
            Item item(newId(), npc.pos, lootItem);
            om.addItem(item);
            cm.broadcast(Initialize(item.id, ObjectType::Item, item.type,
               item.pos, vec2(0,1), 0));
         }
         cm.broadcast(Signal(Signal::remove, npc.id).makePacket());
         om.remove(npc.id);
         i--;
      } 
      else if(npcHit) {
         cm.broadcast(HealthChange(npc.id, npc.hp));
      }
      else {
         npc.update();
         cm.broadcast(pack::Position(npc.pos, npc.dir, npc.aiType != AIType::Stopped, npc.id));
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
