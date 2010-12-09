#ifndef _BOTWORLD_H_
#define _BOTWORLD_H_

#include "packet.h"
#include "BotCharacters.h"
#include <cstdio>

using namespace botclient;

struct BotWorldData {
   BotWorldData() : ticks(0), dt(0) {}

   int ticks;
   float dt;

   ObjectHolder objs;
   Player player;
   vec2 initPos;
   
   sock::Connection conn;

   void init(const char *host, int port);
   void update(int ticks, float dt);
   void updatePlayerPos(int ticks, float dt);
   void updateFighting(int ticks, float dt);
   void updateNotFighting(int ticks, float dt);
   void updateLooting(int ticks, float dt);
   void processPacket(pack::Packet p);
   void shootArrow(mat::vec2 dir);
   void hurtMe();
   void togglePvp();
   void doSpecial();
   void rightClick(vec2 mousePos);

   //These three should be set before running
   bool homing; //goes directly for its target (probably can't use dodging with this)
   bool dodging; // goes left/right from target (probably can't use homing and this)
   //if neither dodging/homing then it randomly changes direction every dirChangeDelay
   bool looting; //able to loot objects
   bool backup; //able to back up if too close to enemy

   bool fighting; //in aggro range
   bool shooting; //within firing range
   bool tostart; //means bot is going to initial position since it travelled to far
   bool returnsAllWayToStart; //means the bot will return completely to start
   bool alive;

   int nextLoot;
   int nextDirChange;
   int fightingId;
   int delay;
   int nextDodgeChange;
   NPC *fightNpc;
   vec2 dodgeDir;

   static const int printDelay = 50;
   static const int botLootTickDelay = 1300;
   static const int dirChangeDelay = 2000;
   static const int dodgeChangeDelay = 800;
   static const bool randomizeLeftRightDodge = true;

   static const float botDodgeRatio;
   static const float botBackupRange;
   static const float botDodgeRange;
   static const float botHomingRange;
   static const float botAggroRange;
   static const float botFightRange;
   static const float maxBotItemGrab;
   static const float maxBotWalkDistance;
   static const float returnWalkDistance;
};

struct BotWorld {
   BotWorld() {}

   void init(const char *host, int port);
   void update(int ticks, float dt);

   BotWorldData data;
};

void sleepms(int ms);
int currentTicks();

int getTicks();
float getDt();
Player &getPlayer();

#endif //_BOTWORLD_H_
