#include "BotWorld.h"
#include "Packets.h"
#include "Constants.h"
#include "Util.h"

using namespace constants;

BotWorldData *clientState;

const float BotWorldData::botHomingRange = 1200.0f;
const float BotWorldData::botBackupRange = 200.0f;
const float BotWorldData::botDodgeRange = 450.0f;
const float BotWorldData::botAggroRange = 650.0f;
const float BotWorldData::botFightRange = 300.0f;
const float BotWorldData::maxBotItemGrab = botAggroRange;
const float BotWorldData::maxBotWalkDistance 
      = (constants::worldWidth + constants::worldHeight - 1)/4;
const float BotWorldData::returnWalkDistance 
      = BotWorldData::maxBotWalkDistance* 0.7f;

//Near zero means not really dodging
//Large numbers will mean pretty much only dodging instead of heading towards the target
const float BotWorldData::botDodgeRatio = 1.5f; 

// This pointer points to the current 

// Interface stubs
void BotWorld::init(const char *host, int port)
{
   data.init(host,port);
}

void BotWorld::update(int ticks, float dt) { 
   data.update(ticks, dt);
}

void BotWorldData::init(const char *host, int port)
{
   clientState = this;
   sock::setupSockets();

   // Maybe ask for a hostname here, along with other info (name, class etc.)
   conn = sock::Connection(host, port);

   if (!conn) {
      printf("Could not connect!\n");
      exit(-1);
   }

   if (!conn.select(1000)) {
      printf("Timed out waiting for server\n");
      exit(-1);
   }

   pack::Packet p = pack::readPacket(conn);
   if (p.type != PacketType::connect) {
      printf("Expected connect packet for handshake, got: %d\n", p.type);
      exit(-1);
   }
   
   pack::Connect c(p);
	
   if (!conn.select(1000)) {
      printf("Timed out waiting for initalize packet\n");
      exit(-1);
   }
   
   p = pack::readPacket(conn);
   if (p.type != PacketType::initialize) {
      printf("Expecting initalize, got %d\n", p.type);
      exit(-1);
   }
   
   pack::Initialize i(p);
   if (i.id != c.id || i.type != ObjectType::Player) {
      printf("Bad init packet\n");
      exit(-1);
   }
	
   player = Player(i.id, i.pos, i.dir, playerMaxHp);
   initPos = i.pos;

   fightNpc = 0;
   tostart = false;
   fighting = false;
   homing = false;
   dodging = true;
   looting = true;
   shooting = false;
   backup = true;
   returnsAllWayToStart = false;
   nextLoot = 0;
   nextDirChange = 0;
   nextDodgeChange = 0;
   fightingId = 0;
   delay = 30;
   alive = true;

   printf("Connected to server successfully\nYour id is %d\n", player.getId());
}

#ifdef WIN32
#include <windows.h>
void sleepms(int ms)
{
   Sleep(ms);
}

#else

void sleepms(int ms)
{
   usleep(ms*1000);
}

#endif

void BotWorldData::update(int ticks, float dt)
{
   this->ticks = ticks;
   this->dt = dt;
   //handle packets
   while (conn.select()) {
      if (conn) {
         processPacket(pack::readPacket(conn));
      } else {
         printf("server disconnected!\n");
         alive = false;
         return;
      }
   }
   updatePlayerPos(ticks, dt);
   objs.updateAll();

   /////////////////// BOT AI ///////////////////
   //////////////////////////////////////////////
   player.moving = true;
   shooting = false;
   if(tostart) { 
      //Returning to start
      if(returnsAllWayToStart)
         tostart = mat::dist(player.pos, initPos) > 100;
      else
         tostart = mat::dist(player.pos, initPos) > returnWalkDistance;
   }
   else {
      tostart = mat::dist(player.pos, initPos) > maxBotWalkDistance;
      if(tostart) {
         player.dir = mat::to(player.pos, initPos);
         printf("returning to start\n");
      }
      else if(fighting) {
         updateFighting(ticks, dt);
      } else 
         updateNotFighting(ticks, dt);
   }
   player.dir.normalize();

   //pick up nearby items
   if(looting) {
      updateLooting(ticks, dt);
   }
   pack::Position(player.pos, player.dir, player.moving, 
      player.getId()).makePacket().sendTo(conn);
   if(shooting) {
      shootArrow(mat::to(player.pos, fightNpc->pos).normalize());
   }
   //sleep delay (so it doesn't eat the CPU resources)
   sleepms(delay);
}

void BotWorldData::updateLooting(int ticks, float dt)
{
   for(unsigned i = 0; i < objs.itemCount(); i++) {
      Item &item = *static_cast<Item *>(objs.get(ObjectType::Item, i));
      if(item.isCollectable() 
            && mat::dist(player.pos, item.pos) < maxBotItemGrab
            && nextLoot < ticks) {
         //player.moving = false;
         //pack::Position(player.pos, player.dir, false, 
         //   player.getId()).makePacket().sendTo(conn);
         nextLoot = ticks + botLootTickDelay;
         rightClick(item.pos);
         printf("looting item %d\n", item.getId());
      }
   }
}

vec2 tangentVec(vec2 v, bool right) {
   v.normalize();
   if(right)
      return vec2(-v.y, v.x);
   else
      return vec2(v.y, -v.x);
}

void BotWorldData::updateFighting(int ticks, float dt)
{
   //finish NPC
   if(objs.contains(fightingId, ObjectType::NPC)) {
      fightNpc = objs.getNPC(fightingId);
      float distance = abs(mat::dist(player.pos, fightNpc->pos));
      if(distance > botAggroRange + 5.0f 
            || ticks - fightNpc->lastUpdate >= noDrawTicks) {
         fighting = false;
      } 
      else {
         vec2 towardNpc(mat::to(player.pos, fightNpc->pos));
         if(towardNpc.length() != 0)
            towardNpc.normalize();
         if(backup && distance < botBackupRange) {
            player.dir = vec2(-towardNpc.x, -towardNpc.y);
         }
         else if(homing) {
            if(distance < botFightRange)
               player.moving = false;
            player.dir = towardNpc;
         }
         else if (dodging) {
            if(distance > botDodgeRange) {
               player.dir = towardNpc;
            }
            else {
               if(ticks > nextDodgeChange) {
                  dodgeDir = tangentVec(towardNpc, randomizeLeftRightDodge
                     && util::irand(0, 1) == 0);
                  if(dodgeDir.length() != 0)
                     dodgeDir.normalize();
                  nextDodgeChange = ticks + dodgeChangeDelay;
                  player.dir = towardNpc + botDodgeRatio*dodgeDir;
                  if(player.dir.length() != 0)
                     player.dir.normalize();
                  //printf("%0.2f %0.2f tangent(%0.2f %0.2f)\n", towardNpc.x, towardNpc.y, dodgeDir.x, dodgeDir.y);
               }
            }
         }
         else if(ticks > nextDirChange) {
            nextDirChange = ticks + dirChangeDelay;
            float randAngle = ((rand() % 359) / 180.0f) * (float) PI;
            player.dir = vec2(cos(randAngle), sin(randAngle));
         }
         shooting = true;
      }
   } else
      fighting = false;

   //player.dir.normalize();
   //will be normalized later
}

void BotWorldData::updateNotFighting(int ticks, float dt)
{
   //check if NPC in range (turn on fighting)
   for(unsigned i = 0; i < objs.npcCount() && !fighting; i++) {
      NPC &npc = *static_cast<NPC *>(objs.get(ObjectType::NPC, i));
      if(mat::dist(player.pos, npc.pos) < botAggroRange
            && ticks - npc.lastUpdate < noDrawTicks) {
         fightingId = npc.getId();
         fighting = true;
         printf("fighting %d\n", fightingId);
      }
   }
   if(!fighting) {
      //Walking around
      if(ticks > nextDirChange) {
         nextDirChange = ticks + dirChangeDelay;
         if(homing) {
            for(unsigned i = 0; i < objs.npcCount(); i++) {
               NPC &npc = *static_cast<NPC *>(objs.get(ObjectType::NPC, i));
               if(mat::dist(player.pos, npc.pos) < botHomingRange) {
                  player.dir = mat::to(player.pos, npc.pos);
               }
            }
         } else {
            float randAngle = ((rand() % 359) / 180.0f) * (float) PI;
            player.dir = vec2(cos(randAngle), sin(randAngle));
         }
      }
   }
}

void BotWorldData::updatePlayerPos(int ticks, float dt)
{
   //ensure the bot doesn't go off of map
   if(player.moving) {
      player.move(player.pos + player.dir * dt * playerSpeed, player.dir, player.moving);
		if (player.pos.x > worldWidth) {
			player.pos.x = worldWidth;
		}
		else if (player.pos.x < -worldWidth) {
			player.pos.x = -worldWidth;
		}
		if (player.pos.y > worldHeight) {
			player.pos.y = worldHeight;
		} 
      else if (player.pos.y < -worldHeight) {
			player.pos.y = -worldHeight;
		}
      //pack::Position(player.pos, player.dir, player.moving, player.id).makePacket().sendTo(conn);
   }
}

void BotWorldData::processPacket(pack::Packet p)
{
	using namespace pack;
   
   if (p.type == PacketType::position) {
      Position pos(p);
      if(pos.id == player.getId()) {
         player.pos = pos.pos;
      } else if(objs.contains(pos.id, ObjectType::Player)) {
         objs.getPlayer(pos.id)->move(pos.pos, pos.dir, pos.moving != 0);
      } else if (objs.contains(pos.id, ObjectType::NPC)) {
         NPC &npc = *objs.getNPC(pos.id);
         npc.pos = pos.pos;
         npc.dir = pos.dir;
         npc.moving = pos.moving != 0;
      } else if(objs.contains(pos.id, ObjectType::Item)) {
         Item *item = objs.getItem(pos.id);
         item->pos = pos.pos;
      } else if(objs.contains(pos.id, ObjectType::Missile)) {
         Missile *mis = objs.getMissile(pos.id);
         mis->pos = pos.pos;
      }
      else
         printf("client %d: unable to process Pos packet id=%d\n", 
            player.getId(), pos.id);
   }
   else if (p.type == PacketType::initialize) {
      Initialize i(p);
      if (i.type == ObjectType::Player && i.id != player.getId()) {
         objs.addPlayer(new Player(i.id, i.pos, i.dir, i.hp));
         //printf("Added player pos: %.1f %.1f\n", objs.getPlayer(i.id)->pos.x, objs.getPlayer(i.id)->pos.y);
      }
      else if (i.type == ObjectType::Missile) {
         objs.addMissile(new Missile(i.id, i.subType, i.pos, i.dir));
      }
      else if (i.type == ObjectType::NPC) {
         objs.addNPC(new NPC(i.id, i.subType, i.hp, i.pos, i.dir, false));
         //printf("Added NPC %d \n", i.id);
      }
      else if (i.type == ObjectType::Item) {
         objs.addItem(new Item(i.id, i.subType, i.pos));
         //printf("Added Item %d \n", i.id);
      }
   }
   else if (p.type == PacketType::signal) {
      Signal sig(p);
      if (sig.sig == Signal::remove) {
         if(sig.val == this->player.getId()) {
            printf("\n\n!!! Disconnected from server !!!\n\n");
         }
         else {
            objs.remove(sig.val);
            //printf("Object %d disconnected\n", sig.val);
         }
      } else if (sig.sig == Signal::changeRupee) {
      } else if (sig.sig == Signal::changeExp) {
      } else
         printf("Unknown signal (%d %d)\n", sig.sig, sig.val);
   } 
   else if (p.type == PacketType::arrow) {
	   //Arrow ar(p);
		//objs.addMissile(Missile(ar.id, MissileType::Arrow, ar.orig, ar.direction));
	}
   else if (p.type == PacketType::healthChange) {
      HealthChange hc(p);
      if (hc.id == player.getId()) {
         player.hp = hc.hp;
      } 
      else if (objs.contains(hc.id, ObjectType::Player)) {
         objs.getPlayer(hc.id)->hp = hc.hp;
      } 
      else if(objs.contains(hc.id, ObjectType::NPC)) {
         objs.getNPC(hc.id)->hp = hc.hp;
      }
      else
         printf("Error: Health change on id %d\n", hc.id);
   }
   else if(p.type == PacketType::changePvp) {
   }
   else
      printf("Unknown packet type=%d size=%d\n", p.type, p.data.size());
}

void BotWorldData::shootArrow(mat::vec2 dir)
{
   pack::Arrow ar(dir, player.getId());	
	ar.makePacket().sendTo(conn);
}

void BotWorldData::doSpecial()
{
   pack::Signal sig(pack::Signal::special, player.getId());
   sig.makePacket().sendTo(conn);	
}

void BotWorldData::rightClick(vec2 mousePos)
{
   pack::Click(mousePos, player.getId()).makePacket().sendTo(conn);
}


int getTicks()
{
   return clientState->ticks;
}

float getDt()
{
   return clientState->dt;
}

Player &getPlayer()
{
   return clientState->player;
}
