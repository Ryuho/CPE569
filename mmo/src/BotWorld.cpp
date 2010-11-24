#include "BotWorld.h"
#include "packet.h"
#include "Constants.h"
#include "Util.h"

using namespace constants;

const float BotWorldData::botHomingRange = 1200.0f;
const float BotWorldData::botAggroRange = 650.0f;
const float BotWorldData::botFightRange = 300.0f;
const float BotWorldData::maxBotItemGrab = botAggroRange;
const float BotWorldData::maxBotWalkDistance 
      = (constants::worldWidth + constants::worldHeight - 1)/4;
const float BotWorldData::returnWalkDistance 
      = BotWorldData::maxBotWalkDistance* 0.7f;

//Near zero means not really dodging
//Large numbers will mean pretty much only dodging instead of heading towards the target
const float BotWorldData::botDodgeRatio = 2.0f; 

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
   if (p.type != pack::connect) {
      printf("Expected connect packet for handshake, got: %d\n", p.type);
      exit(-1);
   }
   
   pack::Connect c(p);
	
   if (!conn.select(1000)) {
      printf("Timed out waiting for initalize packet\n");
      exit(-1);
   }
   
   p = pack::readPacket(conn);
   if (p.type != pack::initialize) {
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
   returnsAllWayToStart = false;
   nextLoot = 0;
   nextDirChange = 0;
   nextDodgeChange = 0;
   fightingId = 0;
   delay = 30;

   printf("Connected to server successfully\nYour id is %d\n", player.id);
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
   //handle packets
   while (conn.select()) {
      if (conn) {
         processPacket(pack::readPacket(conn));
      } else {
         printf("server disconnected!\n");
         exit(-1);
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
   pack::Position(player.pos, player.dir, player.moving, player.id).makePacket().sendTo(conn);
   if(shooting) {
      shootArrow(mat::to(player.pos, fightNpc->pos).normalize());
   }
   //sleep delay (so it doesn't eat the CPU resources)
   sleepms(delay);
}

void BotWorldData::updateLooting(int ticks, float dt)
{
   for(unsigned i = 0; i < objs.items.size(); i++) {
      if(objs.items[i].isCollectable() 
            && mat::dist(player.pos, objs.items[i].pos) < maxBotItemGrab
            && nextLoot < ticks) {
         player.moving = false;
         pack::Position(player.pos, player.dir, false, player.id).makePacket().sendTo(conn);
         nextLoot = ticks + botLootTickDelay;
         rightClick(objs.items[i].pos);
         printf("looting item %d\n", objs.items[i].id);
      }
   }
}

vec2 tangentVec(vec2 v, bool right) {
   v.normalize();
   float angle = asin(v.y);
   angle += 90.0f * ((float)PI / 180.0f);
   if(right)
      angle = -angle;
   return vec2(cos(angle), sin(angle));
}

void BotWorldData::updateFighting(int ticks, float dt)
{
   //finish NPC
   if(objs.checkObject(fightingId, ObjectType::NPC)) {
      fightNpc = objs.getNPC(fightingId);
      if(mat::dist(player.pos, fightNpc->pos) > botAggroRange + 5.0f
            || fightNpc->lastUpdate > noDrawTicks)
         fighting = false;
      else {
         if(homing) {
            if(mat::dist(player.pos, fightNpc->pos) < botFightRange)
               player.moving = false;
            player.dir = mat::to(player.pos, fightNpc->pos);
         }
         else if (dodging) {
            vec2 towardNpc = mat::to(player.pos, fightNpc->pos);
            towardNpc.normalize();
            if(ticks > nextDodgeChange) {
               dodgeDir = tangentVec(towardNpc, randomizeLeftRightDodge
                  && util::irand(0,1) == 0);
               nextDodgeChange = ticks + dodgeChangeDelay;
               //printf("%0.2f %0.2f tangent(%0.2f %0.2f)\n", towardNpc.x, towardNpc.y, dodgeDir.x, dodgeDir.y);
            }
            //if(mat::dist(player.pos, fightNpc->pos) < botFightRange) {
            //   towardNpc = vec2(0,0) - towardNpc * 2*botDodgeRatio;
            //}
            player.dir = towardNpc + dodgeDir * botDodgeRatio;
            //will be normalized later
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
}

void BotWorldData::updateNotFighting(int ticks, float dt)
{
   //check if NPC in range (turn on fighting)
   for(unsigned i = 0; i < objs.npcs.size() && !fighting; i++) {
      if(mat::dist(player.pos, objs.npcs[i].pos) < botAggroRange
            && objs.npcs[i].lastUpdate < noDrawTicks) {
         fightingId = objs.npcs[i].id;
         fighting = true;
         printf("fighting %d\n", fightingId);
      }
   }
   if(!fighting) {
      //Walking around
      if(ticks > nextDirChange) {
         nextDirChange = ticks + dirChangeDelay;
         if(homing) {
            for(unsigned i = 0; i < objs.npcs.size(); i++) {
               if(mat::dist(player.pos, objs.npcs[i].pos) < botHomingRange) {
                  player.dir = mat::to(player.pos, objs.npcs[i].pos);
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
   
   if (p.type == position) {
      Position pos(p);
      if(pos.id == player.id) {
      } else if(objs.checkObject(pos.id, ObjectType::Player)) {
         objs.getPlayer(pos.id)->move(pos.pos, pos.dir, pos.moving != 0);
      } else if (objs.checkObject(pos.id, ObjectType::NPC)) {
         NPC *npc = objs.getNPC(pos.id);
         npc->pos = pos.pos;
         npc->dir = pos.dir;
         npc->moving = pos.moving != 0;
      } else if(objs.checkObject(pos.id, ObjectType::Item)) {
         Item *item = objs.getItem(pos.id);
         item->pos = pos.pos;
      }
      else
         printf("client %d: unable to process Pos packet id=%d\n", player.id, pos.id);
   }
   else if (p.type == initialize) {
      Initialize i(p);
      if (i.type == ObjectType::Player && i.id != player.id) {
         objs.addPlayer(Player(i.id, i.pos, i.dir, i.hp));
         //printf("Added player pos: %.1f %.1f\n", objs.getPlayer(i.id)->pos.x, objs.getPlayer(i.id)->pos.y);
      }
      else if (i.type == ObjectType::Missile) {
         objs.addMissile(Missile(i.id, i.subType, i.pos, i.dir));
      } 
      else if (i.type == ObjectType::NPC) {
         objs.addNPC(NPC(i.id, i.subType, i.hp, i.pos, i.dir, false));
         //printf("Added NPC %d \n", i.id);
      }
      else if (i.type == ObjectType::Item) {
         objs.addItem(Item(i.id, i.subType, i.pos));
         //printf("Added Item %d \n", i.id);
      }
   }
   else if (p.type == signal) {
      Signal sig(p);
      if (sig.sig == Signal::remove) {
         if(sig.val == this->player.id)
            printf("\n\n!!! Disconnected from server !!!\n\n");
         else {
            objs.removeObject(sig.val);
            //printf("Object %d disconnected\n", sig.val);
         }
      } else if (sig.sig == Signal::changeRupee) {
      } else if (sig.sig == Signal::changeExp) {
      } else
         printf("Unknown signal (%d %d)\n", sig.sig, sig.val);
   } 
   else if (p.type == arrow) {
	   Arrow ar(p);
		objs.addMissile(Missile(ar.id, MissileType::Arrow, ar.orig, ar.direction));
	}
   else if (p.type == healthChange) {
      HealthChange hc(p);
      if (hc.id == player.id) {
         player.hp = hc.hp;
      } 
      else if (objs.checkObject(hc.id, ObjectType::Player)) {
         objs.getPlayer(hc.id)->hp = hc.hp;
      } 
      else if(objs.checkObject(hc.id, ObjectType::NPC)) {
         objs.getNPC(hc.id)->hp = hc.hp;
      }
      else
         printf("Error: Health change on id %d\n", hc.id);
   }
   else if(p.type == pack::changePvp) {
   }
   else
      printf("Unknown packet type=%d size=%d\n", p.type, p.data.size());
}

void BotWorldData::shootArrow(mat::vec2 dir)
{
	pack::Arrow ar(dir, player.id);	
	ar.makePacket().sendTo(conn);
}

void BotWorldData::doSpecial()
{
	pack::Signal sig(pack::Signal::special, player.id);
   sig.makePacket().sendTo(conn);	
}

void BotWorldData::rightClick(vec2 mousePos)
{
   pack::Click(mousePos, player.id).makePacket().sendTo(conn);
}
