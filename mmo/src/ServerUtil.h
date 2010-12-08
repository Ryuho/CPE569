#ifndef __SERVER_UTIL_H__
#define __SERVER_UTIL_H__

#include "matrix.h"
#include "ServerData.h"
#include "ServerConnManager.h"
#include "GameServer.h"

mat::vec2 randPos(int minxy, int maxxy);
mat::vec2 randPos2(int minRadius, int maxRadius);

int npcType(int regionX, int regionY);
server::NPC *spawnNPC(int regionX, int regionY);
server::Item *spawnItem(int id);
server::Item *spawnStump(int id);

////////////////////////////////////////
// THESE ARE IMPLEMENTED PER SERVER   //
// DO NOT IMPLEMENT IN SERVERUTIL.CPP //
////////////////////////////////////////

GameServer &getGS();

inline int getTicks() {
   return getGS().ticks;
}
inline float getDt() {
   return getGS().dt;
}
inline objectManager::ObjectManager &getOM() {
   return getGS().om;
}
inline ConnectionManager &getCM() {
   return getGS().cm;
}

#endif //__SERVER_UTIL_H__