#ifndef _BOTWORLD_H_
#define _BOTWORLD_H_

#include "packet.h"
#include "BotCharacters.h"
#include <cstdio>

struct BotWorldData {
   BotWorldData() {};

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
   void doSpecial();
   void rightClick(vec2 mousePos);

   bool tostart;
   bool fighting;
   bool shooting;
   bool homing;
   bool dodging;
   bool looting;
   int nextLoot;
   int nextDirChange;
   int fightingId;
   int delay;
   NPC *fightNpc;

   static const int printDelay = 50;
   static const int botLootTickDelay = 1300;
   static const int dirChangeDelay = 2000;

   static const float botHomingRange;
   static const float botAggroRange;
   static const float botFightRange;
   static const float maxBotItemGrab;
   static const float maxBotWalkDistance;
};

struct BotWorld {
   BotWorld() {}

   void init(const char *host, int port);
   void update(int ticks, float dt);

   BotWorldData data;
};

#endif //_BOTWORLD_H_
