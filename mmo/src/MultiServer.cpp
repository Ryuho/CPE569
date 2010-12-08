#include "GameServer.h"
#include "Packets.h"
#include "ServerUtil.h"
#include "Constants.h"
#include <cstdio>

using namespace pack;
using namespace server;

GameServer *serverState = 0;

GameServer::GameServer(ConnectionManager &cm, int remoteServerId) 
   : cm(cm), ticks(0), dt(0.0f)
{
   serverState = this;

   Item *stump = spawnStump(newId());
   getCM().clientBroadcast(Initialize(stump->getId(), ObjectType::Item, 
      stump->type, stump->pos, vec2(), 0));
}

void GameServer::newClientConnection(int id)
{
   printf("New client connection: %d\n", id);
   
   vec2 pos((float)(rand()%200), (float)(rand()%200));
   
   Player *newPlayer 
      = new Player(id, getCM().ownServerId, pos, vec2(0,1), playerMaxHp);
   om.add(newPlayer);

	cm.clientSendPacket(Connect(id, constants::worldHeight, 
      constants::worldWidth), id);
   cm.clientBroadcast(Initialize(newPlayer->getId(), ObjectType::Player, 
      0, newPlayer->pos, newPlayer->dir, newPlayer->hp));

   //tell new player about previous players (includes self)
   for(unsigned i = 0; i < om.playerCount(); i++) {
      Player &p = *static_cast<Player *>(om.get(ObjectType::Player, i));
      cm.clientSendPacket(Initialize(p.getId(), ObjectType::Player, 
         0, p.pos, p.dir, p.hp), id);
      if(p.pvp)
         cm.clientSendPacket(Pvp(p.getId(), p.pvp), id);
   }
   //tell new player about previous Items
   for(unsigned i = 0; i < om.itemCount(); i++) {
      Item &item = *static_cast<Item *>(om.get(ObjectType::Item, i));
      cm.clientSendPacket(Initialize(item.getId(), ObjectType::Item, item.type,
         item.pos, vec2(), 0), id);
   }
   //tell new player about previous NPCs
   for(unsigned i = 0; i < om.npcCount(); i++) {
      NPC &npc = *static_cast<NPC *>(om.get(ObjectType::NPC, i));
      cm.clientSendPacket(Initialize(npc.getId(), ObjectType::NPC, 
         npc.type, npc.pos, npc.dir, npc.hp).makePacket(), id);
   }

   cm.serverBroadcast(newPlayer->serialize());
}

void GameServer::newServerConnection(int id)
{
   printf("New server connection: %d\n", id);

   //tell new server about previous players (includes self)
   for(unsigned i = 0; i < om.playerCount(); i++) {
      Player &obj = *static_cast<Player *>(om.get(ObjectType::Player, i));
      if(obj.sid == cm.ownServerId)
         cm.serverSendPacket(obj.serialize(), id);
   }
   //tell new server about previous Items
   for(unsigned i = 0; i < om.itemCount(); i++) {
      Item &obj = *static_cast<Item *>(om.get(ObjectType::Item, i));
      if(obj.sid == cm.ownServerId)
         cm.serverSendPacket(obj.serialize(), id);
   }
   //tell new server about previous NPCs
   for(unsigned i = 0; i < om.npcCount(); i++) {
      NPC &obj = *static_cast<NPC *>(om.get(ObjectType::NPC, i));
      if(obj.sid == cm.ownServerId)
         cm.serverSendPacket(obj.serialize(), id);
   }
}

void GameServer::clientDisconnect(int id)
{
   printf("Client %d disconnected\n", id);
   Packet removePacket(Signal(Signal::remove, id).makePacket());
   cm.clientBroadcast(removePacket);
   cm.serverBroadcast(removePacket);
   om.remove(id);
}

void GameServer::serverDisconnect(int id)
{
   printf("Server %d disconnected\n", id);
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
      getOM().move(&player, pos.pos);
      player.move(pos.pos, pos.dir, pos.moving != 0);
      //player went out of bounds or invalid positon?
      if(pos.pos.x != player.pos.x || pos.pos.y != player.pos.y) {
         cm.clientSendPacket(Teleport(pos.pos), id);
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
            cm.clientBroadcast(Initialize(m->getId(), ObjectType::Missile, 
               m->type, m->pos, m->dir, 0));
         }
      }
      else if(signal.sig == Signal::setPvp) {
         player.pvp = signal.val != 0;
         cm.clientBroadcast(Pvp(id, signal.val).makePacket());
      }
      else if(signal.sig == Signal::hurtme) {
         player.takeDamage(1);
         cm.clientBroadcast(HealthChange(id, player.hp));
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
         cm.clientBroadcast(Initialize(m->getId(), ObjectType::Missile, 
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
               getCM().clientBroadcast(Signal(Signal::remove, item.getId()));
               collectItem(player, item);
            }
         }
      }
   }
   else
      printf("Unknown client packet type=%d size=%d\n", p.type, p.data.size());
}

void GameServer::processServerPacket(pack::Packet p, int id)
{
   /*
   if(p.type == PacketType::serialPlayer) {
      Player *obj = new Player(p);
      if(getOM().contains(obj->getId())) {
         delete obj;
      }
      else
         getOM().add(obj);
   } 
   else if(p.type == PacketType::serialItem) {
      Item *obj = new Item(p);
      if(getOM().contains(obj->getId())) {
         delete obj;
      }
      else
         getOM().add(obj);
   }
   else if(p.type == PacketType::serialMissile) {
      Missile *obj = new Missile(p);
      if(getOM().contains(obj->getId())) {
         delete obj;
      }
      else
         getOM().add(obj);
   }
   else if(p.type == PacketType::serialNPC) {
      NPC *obj = new NPC(p);
      if(getOM().contains(obj->getId())) {
         delete obj;
      }
      else
         getOM().add(obj);
   }
   else if(p.type == PacketType::signal) {
      Signal signal(p);
      if(signal.sig == Signal::remove) {
         int total = getOM().itemCount() + getOM().playerCount() 
            + getOM().npcCount() + getOM().missileCount();
         getOM().remove(signal.val);
         total -= getOM().itemCount() + getOM().playerCount() 
            + getOM().npcCount() + getOM().missileCount();
         if(total)
            printf("Error: Failed to remove %d\n", signal.val);
      }
      else
         printf("Error: unknown signal (sig=%d val=%d)\n", 
            signal.sig, signal.val);
   }
   else
   */
      printf("Error: unknown server packet, size: %d\n", p.data.size());
}

void GameServer::update(int ticks)
{
   //get the current delta time (time passed since it last ran update())
   dt = (ticks - this->ticks)/1000.0f;
   this->ticks = ticks;


   //if there is a player connected, spawn up to 50 NPCs, evenly distributed
   if(om.playerCount() > 0) {
      if(om.npcCount() < 300){
         for(unsigned i = 0; i < regionXSize; i++) {
            for(unsigned j = 0; j < regionYSize; j++) {
               NPC *npc = spawnNPC(i, j);
               getCM().clientBroadcast(Initialize(npc->getId(), ObjectType::NPC, 
                  npc->type, npc->pos, npc->dir, npc->hp).makePacket());
            }
         }
      }
   }

   /*
   while(om.npcCount() < 4) {
      int i = regionXSize/2;
      int j = regionYSize/2;
      NPC *npc = spawnNPC(i, j);
      getCM().clientBroadcast(Initialize(npc->getId(), ObjectType::NPC, 
         npc->type, npc->pos, npc->dir, npc->hp).makePacket());
   }
   */

   updateNPCs(ticks, dt);
   updatePlayers(ticks, dt);
   updateMissiles(ticks, dt); //updated last to ensure near monsters are hit
}

void GameServer::updateNPCs(int ticks, float dt)
{
   //NPC update - collision detection with missiles, death, exp/loot distribution
   std::vector<NPC *> npcsToRemove;
   for(unsigned i = 0; i < om.npcCount(); i++) {
      NPC &npc = *static_cast<NPC *>(om.get(ObjectType::NPC, i));
      bool removeNPC = false;

      std::vector<ItemBase *> collidedItems;
      om.collidingItems(npc.getGeom(), npc.pos, collidedItems);
      for(unsigned j = 0; j < collidedItems.size(); j++) {
         Item &item = *static_cast<Item *>(collidedItems[j]);
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
  
      std::vector<MissileBase *> ms;
      om.collidingMissiles(npc.getGeom(), npc.pos, ms);
      for(unsigned j = 0; j < ms.size() && !removeNPC; j++) {
         Missile &m = *static_cast<Missile *>(ms[j]);
         if(m.owned != npc.getId() && om.contains(m.owned, ObjectType::Player)) {
            npc.takeDamage(m.getDamage());
            if(npc.hp == 0) {
               if(om.contains(m.owned, ObjectType::Player)) {
                  Player &p = *static_cast<Player *>(om.getPlayer(m.owned));
                  p.gainExp(npc.getExp());
                  cm.clientSendPacket(Signal(Signal::changeExp, p.exp).makePacket(), p.getId());
               }
               removeNPC = true;
            }
            cm.clientBroadcast(Signal(Signal::remove, m.getId()).makePacket());
            om.remove(m.getId());
         }
      }
      if(!removeNPC) {
         npc.update();
      }
      else
         npcsToRemove.push_back(&npc);
   }

   //remove dead npcs
   for(unsigned i = 0; i < npcsToRemove.size(); i++) {
      NPC &npc = *npcsToRemove[i];
      int lootItem = npc.getLoot();
      if(lootItem >= 0) {
         Item *item = new Item(newId(),getCM().ownServerId, npc.pos, lootItem);
         om.add(item);
         cm.clientBroadcast(Initialize(item->getId(), ObjectType::Item, item->type,
            item->pos, vec2(0,1), 0));
      }
      cm.clientBroadcast(Signal(Signal::remove, npc.getId()).makePacket());
      om.remove(npc.getId());
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

   for(unsigned i = 0; i < missilesToRemove.size(); i++) {
      Missile &m = *missilesToRemove[i];
      cm.clientBroadcast(Signal(Signal::remove, m.getId()).makePacket());
      om.remove(m.getId());
   }
}

void GameServer::updatePlayers(int ticks, float dt)
{
   std::vector<Player *> playersToRemove;
   for(unsigned i = 0; i < om.playerCount(); i++) {
      Player &p = *static_cast<Player *>(om.get(ObjectType::Player, i));
      p.shotThisFrame = false;

      p.gainHp(playerHpPerTick);
      std::vector<ItemBase *> collidedItems;
      om.collidingItems(p.getGeom(), p.pos, collidedItems);
      for(unsigned j = 0; j < collidedItems.size(); j++) {
         Item &item = *static_cast<Item *>(collidedItems[j]);
         if(item.isCollidable()) {
            vec2 pushDir = mat::to(item.pos, p.pos);
            if(pushDir.length() > 0.1f) //no divide by zero!
               pushDir.normalize();
            else
               pushDir = vec2(1, 0);
            vec2 newPos(item.pos + pushDir * (item.getRadius() + p.getRadius()));
            om.move(&p, newPos);
            p.move(newPos, p.dir, false);
            cm.clientSendPacket(Teleport(newPos), p.getId());
            printf("Player %d collided\n", p.getId());
         }
         else if(item.isCollectable()) {
            getCM().clientBroadcast(Signal(Signal::remove, item.getId()));
            collectItem(p, item);
         }
      }

      //if player is colliding with any missile that is not owned by it, they take dmg
      std::vector<MissileBase *> collidedMis;
      om.collidingMissiles(p.getGeom(), p.pos, collidedMis);
      //bool damaged = false;
      for(unsigned mdx = 0; mdx < collidedMis.size(); mdx++) {
         Missile &m = *static_cast<Missile *>(collidedMis[mdx]);
         if(m.owned != p.getId()) {
            if(p.pvp && (om.contains(m.owned, ObjectType::Player)
                  && !static_cast<Player *>(om.getPlayer(m.owned))->pvp)) 
            {
               continue;
            }
            else if(!p.pvp && (om.contains(m.owned, ObjectType::Player)))
               continue;
            p.takeDamage(m.getDamage());
            cm.clientBroadcast(Signal(Signal::remove, m.getId()).makePacket());
            om.remove(m.getId());
         }
      }
      if(p.hp == 0) {
         playersToRemove.push_back(&p);
         continue;
      } else {
         Circle areaOfInfluence(p.pos, areaOfInfluenceRadius);
         std::vector<NPCBase *> aoinpcs;
         om.collidingNPCs(areaOfInfluence, p.pos, aoinpcs);
         for(unsigned i = 0; i < aoinpcs.size(); i++) {
            NPC &npc = *static_cast<NPC *>(aoinpcs[i]);
            cm.clientSendPacket(HealthChange(npc.getId(), npc.hp), p.getId());
            cm.clientSendPacket(Position(npc.pos, npc.dir, npc.moving, npc.getId()), p.getId());
            //printf("id=%d pos=%f %f\n", npc.getId(), npc.pos.x, npc.pos.y);
         }
         std::vector<PlayerBase *> aoiplayers;
         om.collidingPlayers(areaOfInfluence, p.pos, aoiplayers);
         for(unsigned i = 0; i < aoiplayers.size(); i++) {
            Player &player = *static_cast<Player *>(aoiplayers[i]);
            cm.clientSendPacket(HealthChange(player.getId(), player.hp), p.getId());
            if(player.getId() != p.getId()) {
               //Sending pos packet to player can cause jittering
               cm.clientSendPacket(Position(player.pos, player.dir, player.moving, player.getId()), 
                  p.getId());
            }
         }
      }
   }

   for(unsigned i = 0; i < playersToRemove.size(); i++) {
      Player &p = *playersToRemove[i];
      cm.clientBroadcast(Signal(Signal::remove, p.getId()).makePacket());
      cm.removeClientConnection(p.getId());
      om.remove(p.getId());
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