#include "ServerUtil.h"
#include "Util.h"
#include "Packets.h"
#include <stdio.h>

using namespace mat;
using namespace server;
using namespace pack;

vec2 randPos(int minxy, int maxxy)
{
   float x = (float)minxy + (rand() % (maxxy - minxy));
   float y = (float)minxy + (rand() % (maxxy - minxy));
   return vec2(x, y);
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
   int maxType = (int) NPCType::Goblin;
   int rows = regionXSize;
   int cols = regionYSize;
   int difficulty = abs(std::max(regionX - rows/2, regionY - cols/2));
   //float difficultyScalar = ((float)rows) / (2*maxType);
   float difficultyScalar = ((float)maxType*2) / rows;
   int type = (int)(difficulty*difficultyScalar + rand() % 5);
   return util::clamp(type, 0, maxType); //ensure valid range
}