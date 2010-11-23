#include "GameServer.h"
#include "Constants.h"
#include "Numbers.h"
#include <cstdio>
#include <time.h>

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
   NPC *n = new NPC(id, npcMaxHp, randPos(-400, 400), 
      vec2(0,1), (rand() % ((int)NPCType::Goblin+1)));
   getOM().add(n);
   printf("Spawned NPC id=%d type=%d\n", n->id, n->type);
   serverState->cm.broadcast(Initialize(n->id, ObjectType::NPC, 
      n->type, n->pos, n->dir, n->hp).makePacket());
}

void spawnItem(int id)
{
   vec2 pos = randPos2(200, 350);
   Item *item = new Item(id, pos, rand() % (ItemType::Explosion+1));
   getOM().add(item);
   printf("Spawn Item id=%d type=%d\n", item->id, item->type);
   serverState->cm.broadcast(Initialize(item->id, ObjectType::Item, item->type,
         item->pos, vec2(), 0));
}

void spawnStump(int id)
{
   vec2 pos = randPos2(200, 350);
   Item *stump = new Item(id, pos, ItemType::Stump);
   getOM().add(stump);
   printf("Spawn Stump id=%d type=%d\n", stump->id, stump->type);
   serverState->cm.broadcast(Initialize(stump->id, ObjectType::Item, stump->type,
         stump->pos, vec2(), 0));
}

GameServer::GameServer(ConnectionManager &cm) : cm(cm)
{
   serverState = this;
   ticks = 0;
   dt = 0;

   om.init((float)worldWidth, (float)worldHeight, (float)regionSize);

   //spawnItem(newId());
   spawnStump(newId());
}

void GameServer::newClientConnection(int id)
{
   printf("New client connection: %d\n", id);
   
   vec2 pos((float)(rand()%200), (float)(rand()%200));
   
   Player *newPlayer = new Player(id, pos, vec2(0,1), playerMaxHp);
   om.add(newPlayer);

	cm.sendPacket(Connect(id, constants::worldHeight, constants::worldWidth), id);
	cm.broadcast(Initialize(newPlayer->id, 
      ObjectType::Player, 0, newPlayer->pos, newPlayer->dir, newPlayer->hp));

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
      cm.sendPacket(Initialize(npc.id, ObjectType::NPC, 
         npc.type, npc.pos, npc.dir, npc.hp).makePacket(), id);
   }
   //make a new NPC and tell everybody about it
   //int npcid = newId();
   //spawnNPC(npcid);
}

void GameServer::newServerConnection(int id)
{
   printf("New server connection: %d\n", id);

   //std::vector<unsigned long> ulong();
}

void GameServer::clientDisconnect(int id)
{
   printf("Client %d disconnected\n", id);
   cm.broadcast(Signal(Signal::remove, id).makePacket());
   om.remove(id);
}

void GameServer::serverDisconnect(int id)
{
   printf("Server %d disconnected\n", id);
   //cm.broadcast(Signal(Signal::remove, id).makePacket());
   //om.remove(id);
}

void GameServer::processClientPacket(pack::Packet p, int id)
{
   if (p.type == pack::position) {
      Position pos(p);
      if(om.check(id, ObjectType::Player)) {
         om.getPlayer(id)->move(pos.pos, pos.dir, pos.moving != 0);
////////////TODO: Replace with Area of Influence ////////////
/////////////////////////////////////////////////////////////
//cm.broadcast(pos);
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
               Missile *m = new Missile(newId(),id, play.pos, 
                  vec2((float)cos(t*2*PI), (float)sin(t*2*PI)));
               om.add(m);
               Initialize init(m->id, ObjectType::Missile, 
                  m->type, m->pos, m->dir, 0);
               cm.broadcast(init);
            }
         }
         else
            printf("Error: Packet Unknown Player id %d\n", id);
      }
      else if (signal.sig == Signal::hurtme) {
         if(om.check(id, ObjectType::Player)) {
            Player *p = om.getPlayer(id);
            p->takeDamage(1);
            cm.broadcast(HealthChange(id, p->hp));
         }
         else
            printf("Error: Packet Unknown Player id %d\n", id);
      }
   }
   else if (p.type == pack::arrow) {
      Arrow ar(p);
      Missile *m = new Missile(newId(),id, om.getPlayer(id)->pos, ar.direction);
      om.add(m);
      Initialize init(m->id, ObjectType::Missile, m->type, m->pos, m->dir, 0);
      cm.broadcast(init);
   }
   else if (p.type == pack::click) {
      Click click(p);
      if(om.check(id, ObjectType::Player)) {
         Geometry point = Point(click.pos);
         printf("Player %d clicked <%0.1f, %0.1f>\n", 
            click.id, click.pos.x, click.pos.y);
         std::vector<Item *> items = om.collidingItems(point, click.pos);
		 if(items.size() > 0) {
            Player &pl = *om.getPlayer(click.id);
            Item &item = *items[0];
            int rupees = item.type == ItemType::GreenRupee ? greenRupeeValue :
               item.type == ItemType::BlueRupee ? blueRupeeValue :
               item.type == ItemType::RedRupee ? redRupeeValue :
               0;
            if(rupees > 0) {
               pl.gainRupees(rupees);
               cm.sendPacket(Signal(Signal::changeRupee, pl.rupees), click.id);
            }
			else if(item.type == ItemType::Stump)
			{
				return;
			}
            cm.broadcast(Signal(Signal::remove, item.id));
            om.remove(item.id); //only remove one item per click max
         }
      }
      else
         printf("Error invalid click Player id %d\n", click.id);
   }
   else
      printf("Unknown client packet type=%d size=%d\n", p.type, p.data.size());
}

void GameServer::processServerPacket(pack::Packet p, int id)
{
   if (p.type == pack::serverList) {
      ServerList servList(p);
      printf("Got a server list packet!\n");
   }
   else{
      printf("Unknown server packet type=%d size=%d\n", p.type, p.data.size());
   }
}

void GameServer::update(int ticks)
{
   //get the current delta time (time passed since it last ran update())
   dt = (float)((ticks - this->ticks)/1000.0);
   this->ticks = ticks;

   //if there is a player connected, spawn up to 6 NPCs

   if(om.players.size() > 0) {
      if(rint(5) == 0 && om.npcs.size() < 10){
         spawnNPC(newId());
      }
   }

   updateMissiles(ticks, dt);
   updateNPCs(ticks, dt);
   updatePlayers(ticks, dt);
}

void GameServer::updateNPCs(int ticks, float dt)
{
   //NPC update - collision detection with missiles, death, exp/loot distribution
   for(unsigned i = 0; i < om.npcs.size(); i++) {
      NPC &npc = *om.npcs[i];
      bool removeNPC = false;
      bool npcHit = false;
      std::vector<Missile *> ms = om.collidingMissiles(npc.getGeom(), npc.pos);
      for(unsigned j = 0; j < ms.size() && !removeNPC; j++) {
         Missile &m = *ms[j];
         npcHit = true;
         npc.takeDamage(m.getDamage());
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
      }
      if(removeNPC) {
         int lootItem = npc.getLoot();
         if(lootItem >= 0) {
            Item *item = new Item(newId(), npc.pos, lootItem);
            om.add(item);
            cm.broadcast(Initialize(item->id, ObjectType::Item, item->type,
               item->pos, vec2(0,1), 0));
         }
         cm.broadcast(Signal(Signal::remove, npc.id).makePacket());
         om.remove(npc.id);
         i--;
      } else {
         npc.update();
////////////TODO: Replace with Area of Influence ////////////
/////////////////////////////////////////////////////////////
//cm.broadcast(pack::Position(npc.pos, npc.dir, npc.moving, npc.id));
         if(npcHit)
            cm.broadcast(HealthChange(npc.id, npc.hp));
      }
   }
}

void GameServer::updateMissiles(int ticks, float dt)
{
   //missles loop, checks for missles TOF, 
   //remove if above set value, else move the position
   for(unsigned i = 0; i < om.missiles.size(); i++) {
      //missile out of bound
      Missile &m = *om.missiles[i];
      if(ticks - m.spawnTime >= maxProjectileTicks){
         cm.broadcast(Signal(Signal::remove, m.id).makePacket());
         om.remove(m.id);
         i--;
      }
      else
         m.update();
   }
}

void GameServer::updatePlayers(int ticks, float dt)
{
   for(unsigned pdx = 0; pdx < om.players.size(); pdx++) {
      Player &p = *om.players[pdx];
      /*
      std::vector<Item *> collidedItems = om.collidingItems(p.getGeom(), p.pos);
      if(collidedItems.size() > 0) {
         printf(".");
         Item &item = *collidedItems[0];
         if(item.isCollidable()) {
            vec2 pushDir = mat::to(item.pos, p.pos);
            if(pushDir.length() > 0.1)
               pushDir.normalize();
            p.pos = pushDir * item.getRadius();
         }
      }
      */
      //if player is colliding with any missle that is not owned by it, they take dmg
      std::vector<Missile *> collidedMis = om.collidingMissiles(p.getGeom(), p.pos);
      bool damaged = false;
      for(unsigned mdx = 0; mdx < collidedMis.size(); mdx++) {
         Missile &m = *collidedMis[mdx];
         if(m.owned != p.id) {
            p.takeDamage(m.getDamage());
            cm.broadcast(Signal(Signal::remove, m.id).makePacket());
            om.remove(m.id);
            damaged = true;
         }
      }
      if(p.hp == 0) {
         //death
         cm.broadcast(Signal(Signal::remove, p.id).makePacket());
         cm.removeClientConnection(p.id);
         om.remove(p.id);
         pdx--;
         continue;
      } else {
         if(damaged)
            cm.broadcast(HealthChange(p.id, p.hp));

         Geometry areaOfInfluence(Circle(p.pos, areaOfInfluenceRadius));
         std::vector<NPC *> aoinpcs = om.collidingNPCs(areaOfInfluence, p.pos);
         for(unsigned i = 0; i < aoinpcs.size(); i++) {
            NPC &npc = *aoinpcs[i];
            cm.sendPacket(Position(npc.pos, npc.dir, npc.moving, npc.id), p.id);
         }
         std::vector<Player *> aoiplayers = om.collidingPlayers(areaOfInfluence, p.pos);
         for(unsigned i = 0; i < aoiplayers.size(); i++) {
            Player &player = *aoiplayers[i];
            cm.sendPacket(Position(player.pos, player.dir, player.moving, player.id), 
               p.id);
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
