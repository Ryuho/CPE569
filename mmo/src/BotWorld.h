#ifndef _BOTWORLD_H_
#define _BOTWORLD_H_

#include "packet.h"
#include "BotCharacters.h"

struct BotWorldData {
   BotWorldData() {};

   ObjectHolder objs;
   Player player;
   vec2 initPos;
   
   sock::Connection conn;

   void init(const char *host, int port);
   void update(int ticks, float dt);
   void processPacket(pack::Packet p);
   void shootArrow(mat::vec2 dir);
   void doSpecial();
   void rightClick(vec2 mousePos);
};

struct BotWorld {
   BotWorld() {}

   void init(const char *host, int port);
   void update(int ticks, float dt);

   BotWorldData data;
};

#endif //_BOTWORLD_H_