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

NPC *spawnNPC(int regionX, int regionY)
{
   util::clamp(regionX, 0, (int)regionXSize-1);
   util::clamp(regionY, 0,  (int)regionYSize-1);

   vec2 botLeft(getOM().worldBotLeft.x + regionSize*regionX,
      getOM().worldBotLeft.y + regionSize*regionY);
   vec2 pos(util::frand(botLeft.x, botLeft.x + regionSize),
      util::frand(botLeft.y, botLeft.y + regionSize));

   NPC *n = new server::NPC(newId(), getCM().ownServerId, npcMaxHp, pos, 
      vec2(0,1), npcType(regionX, regionY));

   getOM().add(n);

   //getCM().clientBroadcast(Initialize(n->getId(), ObjectType::NPC, 
   //   n->type, n->pos, n->dir, n->hp).makePacket());
   return n;
}

Item *spawnItem(int id)
{
   vec2 pos = randPos2(200, 350);
   Item *item = new Item(id, getCM().ownServerId, pos, rand() % (ItemType::Explosion+1));
   getOM().add(item);
   printf("Spawn Item id=%d type=%d\n", item->getId(), item->type);
   //getCM().clientBroadcast(Initialize(item->getId(), ObjectType::Item, item->type,
   //      item->pos, vec2(), 0));
   return item;
}

Item *spawnStump(int id)
{
   vec2 pos = randPos2(200, 350);
   Item *stump = new Item(id, getCM().ownServerId, pos, ItemType::Stump);
   getOM().add(stump);
   printf("Spawn Stump id=%d type=%d\n", stump->getId(), stump->type);
   //getCM().clientBroadcast(Initialize(stump->getId(), ObjectType::Item, stump->type,
   //      stump->pos, vec2(), 0));
   return stump;
}

void collectItem(Player &pl, Item &item)
{
   int rupees = item.type == ItemType::GreenRupee ? greenRupeeValue :
      item.type == ItemType::BlueRupee ? blueRupeeValue :
      item.type == ItemType::RedRupee ? redRupeeValue :
      0;
   if(rupees > 0) {
      pl.gainRupees(rupees);
      getCM().clientSendPacket(Signal(Signal::changeRupee, pl.rupees), pl.getId());
   }
   else if(item.type == ItemType::Heart) {
      pl.gainHp(heartValue);
   }
   else {
      printf("Collected unknown item type %d type=%d\n",
         item.getId(), item.type);
   }
   //getCM().clientBroadcast(Signal(Signal::remove, item.getId()));
   getOM().remove(item.getId()); //only remove one item per click max
}