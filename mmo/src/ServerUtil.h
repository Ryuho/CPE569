#ifndef __SERVER_UTIL_H__
#define __SERVER_UTIL_H__

#include "matrix.h"
#include "ServerData.h"
#include "ServerConnManager.h"

mat::vec2 randPos(int minxy, int maxxy);
mat::vec2 randPos2(int minRadius, int maxRadius);

void collectItem(server::Player &pl, server::Item &item);

int npcType(int regionX, int regionY);
server::NPC *spawnNPC(int regionX, int regionY);
server::Item *spawnItem(int id);
server::Item *spawnStump(int id);

////////////////////////////////////////
// THESE ARE IMPLEMENTED PER SERVER   //
// DO NOT IMPLEMENT IN SERVERUTIL.CPP //
////////////////////////////////////////
int getTicks();
float getDt();
objectManager::ObjectManager &getOM();
ConnectionManager &getCM();

#endif //__SERVER_UTIL_H__