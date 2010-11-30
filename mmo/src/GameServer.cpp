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

int npcType(int regionX, int regionY)
{
   //assumes regionX and regionY are valid
   int maxType = (int) NPCType::Goblin;
   int rows = getOM().regions.size();
   int cols = getOM().regions[0].size();
   int difficulty = abs(std::max(regionX - rows/2, regionY - cols/2));
   //float difficultyScalar = ((float)rows) / (2*maxType);
   float difficultyScalar = ((float)maxType*2) / rows;
   int type = (int)(difficulty*difficultyScalar + rand() % 5);
   return util::clamp(type, 0, maxType); //ensure valid range
}

void spawnNPC(int regionX, int regionY)
{
   util::clamp(regionX, 0, (int)getOM().regions.size()-1);
   util::clamp(regionY, 0,  (int)getOM().regions[0].size()-1);

   vec2 botLeft(getOM().worldBotLeft.x + regionSize*regionX,
      getOM().worldBotLeft.y + regionSize*regionY);
   vec2 pos(util::frand(botLeft.x, botLeft.x + regionSize),
      util::frand(botLeft.y, botLeft.y + regionSize));

   NPC *n = new NPC(newId(), getCM().ownServerId, npcMaxHp, pos, 
      vec2(0,1), npcType(regionX, regionY));

   getOM().add(n);
   printf("Spawned NPC id=%d type=%d\n", n->id, n->type);
   serverState->cm.clientBroadcast(Initialize(n->id, ObjectType::NPC, 
      n->type, n->pos, n->dir, n->hp).makePacket());
}

void spawnItem(int id)
{
   vec2 pos = randPos2(200, 350);
   Item *item = new Item(id, getCM().ownServerId, pos, rand() % (ItemType::Explosion+1));
   getOM().add(item);
   printf("Spawn Item id=%d type=%d\n", item->id, item->type);
   serverState->cm.clientBroadcast(Initialize(item->id, ObjectType::Item, item->type,
         item->pos, vec2(), 0));
}

void spawnStump(int id)
{
   vec2 pos = randPos2(200, 350);
   Item *stump = new Item(id, getCM().ownServerId, pos, ItemType::Stump);
   getOM().add(stump);
   printf("Spawn Stump id=%d type=%d\n", stump->id, stump->type);
   serverState->cm.clientBroadcast(Initialize(stump->id, ObjectType::Item, stump->type,
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
   
   Player *newPlayer = new Player(id, getCM().ownServerId, pos, vec2(0,1), playerMaxHp);
   om.add(newPlayer);

	cm.clientSendPacket(Connect(id, constants::worldHeight, constants::worldWidth), id);
	cm.clientBroadcast(Initialize(newPlayer->id, 
      ObjectType::Player, 0, newPlayer->pos, newPlayer->dir, newPlayer->hp));

   //tell new player about old players (includes self)
   for(unsigned i = 0; i < om.players.size(); i++) {
      Player &p = *om.players[i];
      cm.clientSendPacket(Initialize(p.id, ObjectType::Player, 
         0, p.pos, p.dir, p.hp), id);
      if(p.pvp)
         cm.clientSendPacket(Pvp(p.id, p.pvp), id);
   }
   //tell new player about previous Items
   for(unsigned i = 0; i < om.items.size(); i++) {
      Item &item = *om.items[i];
      cm.clientSendPacket(Initialize(item.id, ObjectType::Item, item.type,
         item.pos, vec2(), 0), id);
   }
   //tell new player about previous NPCs
   for(unsigned i = 0; i < om.npcs.size(); i++) {
      NPC &npc = *om.npcs[i];
      cm.clientSendPacket(Initialize(npc.id, ObjectType::NPC, 
         npc.type, npc.pos, npc.dir, npc.hp).makePacket(), id);
   }
   //make a new NPC and tell everybody about it
   //int npcid = newId();
   //spawnNPC(npcid);
}

void GameServer::newServerConnection(int id)
{
   printf("New server connection: %d\n", id);
}

void GameServer::clientDisconnect(int id)
{
   printf("Client %d disconnected\n", id);
   cm.clientBroadcast(Signal(Signal::remove, id).makePacket());
   om.remove(id);
}

void GameServer::serverDisconnect(int id)
{
   printf("Server %d disconnected\n", id);
   //cm.clientBroadcast(Signal(Signal::remove, id).makePacket());
   //om.remove(id);
}

void collectItem(Player &pl, Item &item)
{
   int rupees = item.type == ItemType::GreenRupee ? greenRupeeValue :
      item.type == ItemType::BlueRupee ? blueRupeeValue :
      item.type == ItemType::RedRupee ? redRupeeValue :
      0;
   if(rupees > 0) {
      pl.gainRupees(rupees);
      getCM().clientSendPacket(Signal(Signal::changeRupee, pl.rupees), pl.id);
   }
   else if(item.type == ItemType::Heart) {
      pl.gainHp(heartValue);
   }
   else {
      printf("Collected unknown item type %d type=%d\n",
         item.id, item.type);
   }
   getCM().clientBroadcast(Signal(Signal::remove, item.id));
   getOM().remove(item.id); //only remove one item per click max
}


void GameServer::processClientPacket(pack::Packet p, int id)
{
   if (p.type == pack::position) {
      Position pos(p);
      if(om.check(id, ObjectType::Player)) {
         Player &pl = *om.getPlayer(id);
         pl.move(pos.pos, pos.dir, pos.moving != 0);
         //player went out of bounds or invalid positon?
         if(pos.pos.x != pl.pos.x || pos.pos.y != pl.pos.y)
            cm.clientSendPacket(Position(pl.pos, pl.dir, pl.moving, pl.id), pl.id);
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
               Missile *m = new Missile(newId(), cm.ownServerId, id, play.pos, 
                  vec2((float)cos(t*2*PI), (float)sin(t*2*PI)));
               om.add(m);
               Initialize init(m->id, ObjectType::Missile, 
                  m->type, m->pos, m->dir, 0);
               cm.clientBroadcast(init);
            }
         }
         else
            printf("Error: Packet Unknown Player id %d\n", id);
      }
      else if (signal.sig == Signal::hurtme) {
         if(om.check(id, ObjectType::Player)) {
            Player *p = om.getPlayer(id);
            p->takeDamage(1);
            cm.clientBroadcast(HealthChange(id, p->hp));
         }
         else
            printf("Error: Packet Unknown Player id %d\n", id);
      }
      else
         printf("Error: Unknown Signal packet type=%d val=%d\n", 
         signal.sig, signal.val);
   }
   else if (p.type == pack::arrow) {
      Arrow ar(p);
      Missile *m = new Missile(newId(),getCM().ownServerId,id, om.getPlayer(id)->pos, ar.direction);
      om.add(m);
      Initialize init(m->id, ObjectType::Missile, m->type, m->pos, m->dir, 0);
      cm.clientBroadcast(init);
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
            if(item.isCollectable()) {
               collectItem(pl, item);
            }
         }
      }
      else
         printf("Error invalid click Player id %d\n", click.id);
   }
   else if(p.type == pack::changePvp) {
      Pvp pvpPacket(p);
      if(om.check(pvpPacket.id, ObjectType::Player)) {
         Player &play = *om.getPlayer(pvpPacket.id);
         play.pvp = pvpPacket.isPvpMode != 0;
         cm.clientBroadcast(pvpPacket.makePacket());
      }
      else
         printf("Error: Unknown player %d for pvp packet\n", pvpPacket.id);
   }
   else
      printf("Unknown client packet type=%d size=%d\n", p.type, p.data.size());
}

void GameServer::processServerPacket(pack::Packet p, int id)
{
   /*if (p.type == pack::serverList) {
      ServerList servList(p);
      printf("Got a server list packet! uLongList size is %d\n",servList.uLongList.size());
      for(unsigned i = 0; i < servList.uLongList.size(); i++){
         printf("%d: %lu\n",i,servList.uLongList[i]);
      }
   }
   else{
      printf("Unknown server packet type=%d size=%d\n", p.type, p.data.size());
   }*/

   printf("Got a server packet, size: %d\n", p.data.size());
}

void GameServer::update(int ticks)
{
   //get the current delta time (time passed since it last ran update())
   dt = (float)((ticks - this->ticks)/1000.0);
   this->ticks = ticks;

   //if there is a player connected, spawn up to 500 NPCs, distributed
   if(om.players.size() > 0) {
      if(om.npcs.size() < 150){
         for(unsigned i = 0; i < om.regions.size(); i++) {
            for(unsigned j = 0; j < om.regions[0].size(); j++) {
               spawnNPC(i, j);
            }
         }
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
      //bool npcHit = false;

      std::vector<Item *> collidedItems = om.collidingItems(npc.getGeom(), npc.pos);
      if(collidedItems.size() > 0) {
         Item &item = *collidedItems[0];
         if(item.isCollidable()) {
            vec2 pushDir = mat::to(item.pos, npc.pos);
            if(pushDir.length() > 0.01) //no divide by zero!
               pushDir.normalize();
            else
               pushDir = vec2(1, 0);
            npc.move(item.pos + pushDir * (item.getRadius() + npc.getRadius()), 
               npc.dir, npc.moving);
            //not needed since self is within Area of Influence?
            //cm.sendPacket(Position(p.pos, p.dir, p.moving, p.id), p.id);
         }
      }

      std::vector<Missile *> ms = om.collidingMissiles(npc.getGeom(), npc.pos);
      for(unsigned j = 0; j < ms.size() && !removeNPC; j++) {
         Missile &m = *ms[j];
         if(m.owned != npc.id && om.check(m.owned, ObjectType::Player)) {
            npc.takeDamage(m.getDamage());
            if(npc.hp == 0) {
               if(om.check(m.owned, ObjectType::Player)) {
                  Player &p = *om.getPlayer(m.owned);
                  p.gainExp(npc.getExp());
                  cm.clientSendPacket(Signal(Signal::changeExp, p.exp).makePacket(), p.id);
               }
               removeNPC = true;
            }
            cm.clientBroadcast(Signal(Signal::remove, m.id).makePacket());
            om.remove(m.id);
         }
      }
      if(removeNPC) {
         int lootItem = npc.getLoot();
         if(lootItem >= 0) {
            Item *item = new Item(newId(),getCM().ownServerId, npc.pos, lootItem);
            om.add(item);
            cm.clientBroadcast(Initialize(item->id, ObjectType::Item, item->type,
               item->pos, vec2(0,1), 0));
         }
         cm.clientBroadcast(Signal(Signal::remove, npc.id).makePacket());
         om.remove(npc.id);
         i--;
      } else {
         npc.update();
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
         cm.clientBroadcast(Signal(Signal::remove, m.id).makePacket());
         om.remove(m.id);
         i--;
      }
      else {
         m.update();
         std::vector<Item *> collidedItems = om.collidingItems(m.getGeom(), m.pos);
         for(unsigned j = 0; j < collidedItems.size(); j++) {
            if(collidedItems[j]->isCollidable()) {
               cm.clientBroadcast(Signal(Signal::remove, m.id).makePacket());
               om.remove(m.id);
               break;
            }
         }
      }
   }
}

void GameServer::updatePlayers(int ticks, float dt)
{
   for(unsigned pdx = 0; pdx < om.players.size(); pdx++) {
      Player &p = *om.players[pdx];

      p.gainHp(playerHpPerTick);
      std::vector<Item *> collidedItems = om.collidingItems(p.getGeom(), p.pos);
      if(collidedItems.size() > 0) {
         Item &item = *collidedItems[0];
         if(item.isCollidable()) {
            vec2 pushDir = mat::to(item.pos, p.pos);
            if(pushDir.length() > 0.0f) //no divide by zero!
               pushDir.normalize();
            else
               pushDir = vec2(1, 0);
            p.move(item.pos + pushDir * (item.getRadius() + playerRadius), p.dir);
            cm.clientSendPacket(Position(p.pos, p.dir, p.moving, p.id), p.id);
         }
         else if(item.isCollectable()) {
            collectItem(p, item);
         }
      }

      //if player is colliding with any missile that is not owned by it, they take dmg
      std::vector<Missile *> collidedMis = om.collidingMissiles(p.getGeom(), p.pos);
      //bool damaged = false;
      for(unsigned mdx = 0; mdx < collidedMis.size(); mdx++) {
         Missile &m = *collidedMis[mdx];
         if(m.owned != p.id) {
            if(p.pvp && (om.check(m.owned, ObjectType::Player)
                  && !om.getPlayer(m.owned)->pvp)) {
               continue;
            }
            else if(!p.pvp && (om.check(m.owned, ObjectType::Player)))
               continue;
            p.takeDamage(m.getDamage());
            cm.clientBroadcast(Signal(Signal::remove, m.id).makePacket());
            om.remove(m.id);
         }
      }
      if(p.hp == 0) {
         //death
         cm.clientBroadcast(Signal(Signal::remove, p.id).makePacket());
         cm.removeClientConnection(p.id);
         om.remove(p.id);
         pdx--;
         continue;
      } else {
         Geometry areaOfInfluence(Circle(p.pos, areaOfInfluenceRadius));
         std::vector<NPC *> aoinpcs = om.collidingNPCs(areaOfInfluence, p.pos);
         for(unsigned i = 0; i < aoinpcs.size(); i++) {
            NPC &npc = *aoinpcs[i];
            cm.clientSendPacket(HealthChange(npc.id, npc.hp), p.id);
            cm.clientSendPacket(Position(npc.pos, npc.dir, npc.moving, npc.id), p.id);
         }
         std::vector<Player *> aoiplayers = om.collidingPlayers(areaOfInfluence, p.pos);
         for(unsigned i = 0; i < aoiplayers.size(); i++) {
            Player &player = *aoiplayers[i];
            cm.clientSendPacket(HealthChange(player.id, player.hp), p.id);
            if(player.id != p.id) {
               //Sending pos packet to player can cause jittering
               cm.clientSendPacket(Position(player.pos, player.dir, player.moving, player.id), 
                  p.id);
            }
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

ConnectionManager &getCM()
{
   return serverState->cm;
}


// empty stubs of character graphics-related functions
