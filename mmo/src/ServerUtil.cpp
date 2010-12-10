#include "ServerUtil.h"
#include "Util.h"
#include "Packets.h"
#include "Constants.h"
#include <stdio.h>

using namespace mat;
using namespace server;
using namespace pack;

vec2 randPos()
{
   int val = max(constants::worldWidth/2, constants::worldHeight/2);
   return randPos(-val, val);
}

vec2 randPos(int minxy, int maxxy)
{
   return vec2(util::frand((float)minxy, (float)maxxy), 
      util::frand((float)minxy, (float)maxxy));
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
   int maxType = (int) NPCType::Ganon;
   int rows = regionXSize;
   int cols = regionYSize;
   float difficulty = getCM().ownServerId/(float)(constants::maxServerCount+1);
   float difficulty2 = (getCM().ownServerId+1)/(float)(constants::maxServerCount+1);
   int type = (int)util::irand((int)(difficulty*maxType), 
      (int)(difficulty2*maxType));
   //int difficulty = abs(std::max(regionX - rows/2, regionY - cols/2));
   //int type = (int)(difficulty*difficultyScalar + rand() % 5);
   return util::clamp(type, 0, maxType); //ensure valid range
}