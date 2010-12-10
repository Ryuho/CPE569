#include "GameServer.h"
#include "Packets.h"
#include "ServerUtil.h"
#include "Constants.h"
#include <cstdio>

using namespace pack;
using namespace server;

GameServer *serverState = 0;

GameServer &getGS()
{
   return *serverState;
}

GameServer::GameServer(ConnectionManager &cm, int remoteServerId) 
   : cm(cm), ticks(0), dt(0.0f), rsid(remoteServerId), totalUpdates(0)
{
   serverState = this;

   Item *stump = spawnStump(newId());
   clientBroadcast(stump->cserialize());
}

void GameServer::processClientPacket(pack::Packet p, int id)
{
   if(!om.contains(id, ObjectType::Player)) {
      printf("Error: Client %d sent packet and is not connected", id);
      return;
   }
   Player &player = *om.getPlayer(id);
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
         sendPlayerSpecial(player);
      }
      else if(signal.sig == Signal::setPvp) {
         player.pvp = signal.val != 0;
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
         sendPlayerArrow(player, ar.dir);
      }
   }
   else if(p.type == PacketType::click) {
      Click click(p);
      Geometry point(Point(click.pos));
      std::vector<ItemBase *> items;
      om.collidingItems(point, click.pos, items);
      for(unsigned i = 0; i < items.size(); i++) {
         Item &item = *static_cast<Item *>(items[i]);
         if(collectItem(player, item)) {
            break;
         }
      }
   }
   else
      printf("Unknown client packet type=%d size=%d\n", p.type, p.data.size());
}


///////////////////////////////////////
/////////////// Updates ///////////////
///////////////////////////////////////
void GameServer::updatePlayers(int ticks, float dt)
{
   std::vector<Player *> playersToRemove;
   for(unsigned i = 0; i < om.playerCount(); i++) {
      Player &p = *static_cast<Player *>(om.get(ObjectType::Player, i));
      Geometry g(p.getGeom());
      p.shotThisFrame = false;

      p.gainHp(playerHpPerTick);
      std::vector<ItemBase *> collidedItems;
      om.collidingItems(g, p.pos, collidedItems);
      som.collidingItems(g, p.pos, collidedItems);
      for(unsigned j = 0; j < collidedItems.size(); j++) {
         Item &item = *static_cast<Item *>(collidedItems[j]);
         if(collectItem(p, item)) {
            //collected, do nothing
         }
         else if(collideItem(p, item)) {
            clientSendPacket(Teleport(p.pos), p.getId());
         }
      }

      //if player is colliding with any missile that is not owned by it, they take dmg
      std::vector<MissileBase *> collidedMis;
      om.collidingMissiles(g, p.pos, collidedMis);
      som.collidingMissiles(g, p.pos, collidedMis);
      //bool damaged = false;
      for(unsigned j = 0; j < collidedMis.size() && p.hp > 0; j++) {
         Missile &m = *static_cast<Missile *>(collidedMis[j]);
         collideMissile(p, m);
      }
      //collidedMis.clear();
      //som.collidingMissiles(g, p.pos, collidedMis);
      //for(unsigned j = 0; j < collidedMis.size() && p.hp > 0; j++) {
      //   Missile &m = *static_cast<Missile *>(collidedMis[j]);
      //   collideMissile(p, m);
      //}
      if(p.hp <= 0)
         playersToRemove.push_back(&p);
      else {
         sendPlayerAOI(p, om);
         sendPlayerAOI(p, som);
      }
   }

   for(unsigned i = 0; i < playersToRemove.size(); i++) {
      removePlayer(*playersToRemove[i]);
   }
}

void GameServer::updateNPCs(int ticks, float dt)
{
   //NPC update - collision detection with missiles, death, exp/loot distribution
   std::vector<NPC *> npcsToRemove;
   for(unsigned i = 0; i < om.npcCount(); i++) {
      NPC &npc = *static_cast<NPC *>(om.get(ObjectType::NPC, i));
      Geometry g(npc.getGeom());

      std::vector<ItemBase *> collidedItems;
      om.collidingItems(g, npc.pos, collidedItems);
      for(unsigned j = 0; j < collidedItems.size(); j++) {
         Item &item = *static_cast<Item *>(collidedItems[j]);
         if(collideItem(npc, item)) {
            //already pushed back
         }
      }
  
      std::vector<MissileBase *> ms;
      om.collidingMissiles(g, npc.pos, ms);
      som.collidingMissiles(g, npc.pos, ms);
      for(unsigned j = 0; j < ms.size() && npc.hp > 0; j++) {
         Missile &m = *static_cast<Missile *>(ms[j]);
         if(collideMissile(npc, m)) {
            //do nothing
         }
      }
      if(npc.hp > 0)
         npc.update();
      else
         npcsToRemove.push_back(&npc);
   }

   //remove dead npcs
   for(unsigned i = 0; i < npcsToRemove.size(); i++) {
      NPC &npc = *npcsToRemove[i];
      int lootItem = npc.getLoot();
      if(lootItem >= 0) {
         Item *item = new Item(newId(), cm.ownServerId, npc.pos, lootItem);
         createObject(item);
      }
      removeObject(npc);
   }
}

void GameServer::updateMissiles(int ticks, float dt)
{
   //missles loop, checks for missles TOF, 
   //remove if above set value, else move the position
   std::vector<Missile *> missilesToRemove;
   for(unsigned i = 0; i < om.missileCount(); i++) {
      //missile out of bound
      Missile &m = *static_cast<Missile *>(om.get(ObjectType::Missile, i));
      if(ticks - m.spawnTime >= maxProjectileTicks){
         missilesToRemove.push_back(&m);
      }
      else {
         m.update();
         std::vector<ItemBase *> collidedItems;
         om.collidingItems(m.getGeom(), m.pos, collidedItems);
         for(unsigned j = 0; j < collidedItems.size(); j++) {
            Item &item = *static_cast<Item *>(collidedItems[j]);
            if(item.isCollidable()) {
               missilesToRemove.push_back(&m);
               break;
            }
         }
      }
   }
   for(unsigned i = 0; i < som.missileCount(); i++) {
      Missile &m = *static_cast<Missile *>(som.get(ObjectType::Missile, i));
      m.update();
   }

   for(unsigned i = 0; i < missilesToRemove.size(); i++) {
      removeObject(*missilesToRemove[i]);
   }
}

void GameServer::clientSendPacket(Packet &p, int id)
{
   cm.clientSendPacket(p, id);
}

void GameServer::clientBroadcast(Packet &p)
{
   cm.clientBroadcast(p);
}

void GameServer::removeClientConnection(int id)
{
   cm.removeClientConnection(id);
}

/////////////////////////////////////////
/////////////// Utilities ///////////////
/////////////////////////////////////////
NPC *GameServer::spawnNPC(int regionX, int regionY)
{
   util::clamp(regionX, 0, (int)regionXSize-1);
   util::clamp(regionY, 0,  (int)regionYSize-1);

   vec2 botLeft(om.worldBotLeft.x + regionSize*regionX,
      om.worldBotLeft.y + regionSize*regionY);
   vec2 pos(util::frand(botLeft.x, botLeft.x + regionSize),
      util::frand(botLeft.y, botLeft.y + regionSize));

   NPC *n = new server::NPC(newId(), cm.ownServerId, npcMaxHp, pos, 
      vec2(0,1), npcType(regionX, regionY));

   createObject(n);
   return n;
}

Item *GameServer::spawnItem(int id)
{
   vec2 pos = randPos2(200, 350);
   Item *item = new Item(id, cm.ownServerId, pos, rand() % (ItemType::Explosion+1));
   //printf("Spawn Item id=%d type=%d\n", item->getId(), item->type);
   createObject(item);
   return item;
}

Item *GameServer::spawnStump(int id)
{
   vec2 pos = randPos2(200, 350);
   Item *stump = new Item(id, cm.ownServerId, pos, ItemType::Stump);
   createObject(stump);
   //printf("Spawn Stump id=%d type=%d\n", stump->getId(), stump->type);
   return stump;
}

bool GameServer::collectItem(Player &pl, Item &item)
{
   if(item.isCollectable()) {
      int rupees = item.type == ItemType::GreenRupee ? greenRupeeValue :
         item.type == ItemType::BlueRupee ? blueRupeeValue :
         item.type == ItemType::RedRupee ? redRupeeValue :
         0;
      if(rupees > 0) {
         pl.gainRupees(rupees);
         clientSendPacket(Signal(Signal::changeRupee, pl.rupees), pl.getId());
      }
      else if(item.type == ItemType::Heart) {
         pl.gainHp(heartValue);
      }
      else {
         printf("Collected unknown item type %d type=%d\n", item.getId(), 
            item.type);
      }
      removeObject(item);
      //om.remove(item.getId()); //only remove one item per click max
      return true;
   }
   return false;
}

bool GameServer::collideMissile(Player &p, Missile &m)
{
   //if not in pvp or missle is owned by player, don't hurt
   if(!p.pvp && m.owned != p.getId()) {
      if(om.contains(m.owned, ObjectType::Player) && !static_cast<Player *>(om.getPlayer(m.owned))->pvp){
         return false;
      }
      else if(som.contains(m.owned, ObjectType::Player) && !static_cast<Player *>(som.getPlayer(m.owned))->pvp){
         return false;
      }
      p.takeDamage(m.getDamage());
      removeObject(m);
      return true;
   }
   return false;
}

bool GameServer::collideMissile(NPC &npc, Missile &m)
{
   if(m.owned != npc.getId() && (om.contains(m.owned, ObjectType::Player) || som.contains(m.owned, ObjectType::Player))) {
      npc.takeDamage(m.getDamage());
      if(npc.hp <= 0) {
         if(om.contains(m.owned, ObjectType::Player)) {
            Player &p = *static_cast<Player *>(om.getPlayer(m.owned));
            p.gainExp(npc.getExp());
            clientSendPacket(Signal(Signal::changeExp, p.exp), p.getId());
         }
      }
      removeObject(m);
      return true;
   }
   return false;
}

bool GameServer::collideItem(ObjectBase &obj, Item &item)
{
   if(item.isCollidable()) {
      vec2 pushDir = mat::to(item.pos, obj.pos);
      if(pushDir.length() > 0.1f) //no divide by zero!
         pushDir.normalize();
      else
         pushDir = vec2(1, 0);
      vec2 newPos(item.pos + pushDir * (item.getRadius() + obj.getRadius()));
      om.move(&obj, newPos);
      return true;
   }
   return false;
}

void GameServer::sendPlayerInitData(int playerId, ObjectHolder &oh)
{
   //tell new player about previous players (includes self)
   for(unsigned i = 0; i < oh.playerCount(); i++) {
      Player &p = *static_cast<Player *>(oh.get(ObjectType::Player, i));
      clientSendPacket(p.cserialize(), playerId);
      if(p.pvp) //already in init packet atm?
         clientSendPacket(Pvp(p.getId(), p.pvp), playerId);
   }
   //tell new player about previous Items
   for(unsigned i = 0; i < oh.itemCount(); i++) {
      Item &item = *static_cast<Item *>(oh.get(ObjectType::Item, i));
      clientSendPacket(item.cserialize(), playerId);
   }
   //tell new player about previous NPCs
   for(unsigned i = 0; i < oh.npcCount(); i++) {
      NPC &npc = *static_cast<NPC *>(oh.get(ObjectType::NPC, i));
      clientSendPacket(npc.cserialize(), playerId);
   }
   //tell new player about previous Missiles
   for(unsigned i = 0; i < oh.missileCount(); i++) {
      Missile &m = *static_cast<Missile *>(oh.get(ObjectType::Missile, i));
      clientSendPacket(m.cserialize(), playerId);
   }
}

void GameServer::sendPlayerSpecial(Player &player)
{
   for(int i = 0; i < constants::numArrows; i++) {
	   float t = i/(float)constants::numArrows;
      vec2 dir((float)cos(t*2*PI), (float)sin(t*2*PI));
	   sendPlayerArrow(player, dir);
   }
}
