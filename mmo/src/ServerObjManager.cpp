#include "ServerObjManager.h"
#include "Util.h"

namespace server {

template <typename T>
void omAddTemplate(T *val, std::vector<T *> &vec, int type, ObjectManager &om)
{
   om.oldTypes[val->id] = type;

   //Add to Index map
   om.idToIndex[val->id] = ObjectManager::Index(vec.size(), type);
   //Add to OM's vector
   vec.push_back(val);
   //Add to regions
   std::vector<Region *> regs;
   om.getRegions(val->pos, val->getGeom(), regs);
   for(unsigned i = 0; i < regs.size(); i++) {
      regs[i]->add(val);
   }
}

void ObjectManager::add(Player *p)
{
   omAddTemplate(p, players, ObjectType::Player, *this);
}

void ObjectManager::add(Missile *m)
{
   omAddTemplate(m, missiles, ObjectType::Missile, *this);
}

void ObjectManager::add(NPC *n)
{
   omAddTemplate(n, npcs, ObjectType::NPC, *this);
}

void ObjectManager::add(Item *i)
{
   omAddTemplate(i, items, ObjectType::Item, *this);
}

Player *ObjectManager::getPlayer(int id)
{
   return players[idToIndex[id].index];
}

Missile *ObjectManager::getMissile(int id)
{
   return missiles[idToIndex[id].index];
}

NPC *ObjectManager::getNpc(int id)
{
   return npcs[idToIndex[id].index];
}

Item *ObjectManager::getItem(int id)
{
   return items[idToIndex[id].index];
}

template<typename T>
void omRemoveTempl(map<int, ObjectManager::Index> &idToIndex, 
   vector<T*> &objs, int id, ObjectManager &om)
{
   int i = idToIndex[id].index;

   std::vector<Region *> regs;
   om.getRegions(objs[i]->pos, objs[i]->getGeom(), regs);
   for(unsigned j = 0; j < regs.size(); j++) {
      regs[j]->remove(objs[i]);
   }


   //HUGE AMOUNT OF DEBUG CODE!!
/*
   int c = 0;
   for(unsigned x = 0; x < om.regions.size(); x++) {
      for(unsigned y = 0; y < om.regions[x].size(); y++) {
         int isize = om.regions[x][y].items.size();
         int msize = om.regions[x][y].missiles.size();
         int psize = om.regions[x][y].players.size();
         int nsize = om.regions[x][y].npcs.size();
         if(om.regions[x][y].contains(objs[i])) {
            om.regions[x][y].remove(objs[i]);
            if(om.regions[x][y].items.size() 
               + om.regions[x][y].missiles.size() 
               + om.regions[x][y].players.size()
               + om.regions[x][y].npcs.size() != isize + msize + psize + nsize - 1)
               printf("Error 1\n");
            om.regions[x][y].remove(objs[i]);
            if(om.regions[x][y].items.size() 
               + om.regions[x][y].missiles.size() 
               + om.regions[x][y].players.size()
               + om.regions[x][y].npcs.size() != isize + msize + psize + nsize - 1)
               printf("Error 2\n");
            c++;
            //printf("%d=<%d,%d> ", c, x, y);
         }
         om.regions[x][y].remove(objs[i]);
         if(om.regions[x][y].items.size() 
            + om.regions[x][y].missiles.size() 
            + om.regions[x][y].players.size()
            + om.regions[x][y].npcs.size() != isize + msize + psize + nsize)
            printf("Error 3\n");
         om.regions[x][y].remove(objs[i]);
         if(om.regions[x][y].items.size() 
            + om.regions[x][y].missiles.size() 
            + om.regions[x][y].players.size()
            + om.regions[x][y].npcs.size() != isize + msize + psize + nsize)
            printf("Error 4\n");
      }
   }
   if(c != 0) {
      printf("Error 5: remove %d failed %d times\n", objs[i]->id, c);
      exit(1);
   }
*/

   delete objs[i];
   idToIndex.erase(id);
   if (objs.size() > 1) {
      objs[i] = objs.back();
      idToIndex[objs[i]->id].index = i;
   }
   objs.pop_back();
}

void ObjectManager::remove(int id)
{   
   if (idToIndex.find(id) == idToIndex.end()) {
      printf("Tried to remove an object %d with no idToIndex value type %d\n", id, oldTypes[id] == constants::ObjectType::Missile);
      return;
   }

   int i = idToIndex[id].index;

   if (idToIndex[id].type == ObjectType::Player)
      omRemoveTempl(idToIndex, players, id, *this);
   else if (idToIndex[id].type == ObjectType::Missile)
      omRemoveTempl(idToIndex, missiles, id, *this);
   else if (idToIndex[id].type == ObjectType::NPC)
      omRemoveTempl(idToIndex, npcs, id, *this);
   else if (idToIndex[id].type == ObjectType::Item)
      omRemoveTempl(idToIndex, items, id, *this);
   else
      printf("object %d had unknown type: %d\n", id, idToIndex[id].type);
}

bool ObjectManager::check(int id, int type)
{
   map<int, Index>::iterator itr = idToIndex.find(id);
   return itr != idToIndex.end() && itr->second.type == type;
}

void ObjectManager::init(float width, float height, float regionSize)
{
   width = 2*width;
   height = 2*height;
   this->worldBotLeft = vec2(-width/2.0f, -height/2.0f);
   this->regionSize = regionSize;
   unsigned xBuckets = ((int)(width / regionSize));
   unsigned yBuckets = ((int)(height / regionSize));
   if(xBuckets * regionSize != width)
      xBuckets++;
   if(yBuckets * regionSize != height)
      yBuckets++;

   if(regions.size() > 0) {
      printf("Error: Reinitializing Object Manager\n");
      regions.clear();
   }
   for(unsigned x = 0; x < xBuckets; x++) {
      regions.push_back(vector<Region>());
      for(unsigned y = 0; y < yBuckets; y++) {
         regions[x].push_back(Region());
      }
   }
   printf("Initialized ObjectManager <%d by %d> total=%d\n", xBuckets, yBuckets, xBuckets*yBuckets);
}

void ObjectManager::getRegion(vec2 pos, int &x, int &y)
{
   x = (int) ((pos.x - worldBotLeft.x) / regionSize);
   y = (int) ((pos.y - worldBotLeft.y) / regionSize);
   //clamp
   int tempx = x;
   int tempy = y;
   util::clamp(x, 0, (int)regions.size()-1);
   util::clamp(y, 0, (int)regions[0].size()-1);
   if(tempx != x || tempy != y) {
      printf("Error getRegion() <%d, %d> -> <%d, %d>\n", tempx, tempy, x, y);
   }
}

Geometry ObjectManager::getRegionGeom(int x, int y)
{
   return Rectangle(
      vec2(worldBotLeft.x + regionSize*x, worldBotLeft.y + regionSize*y),
      regionSize, regionSize);
}

void ObjectManager::getRegions(vec2 pos, Geometry g, std::vector<Region *> &regs)
{
   int x, y;
   getRegion(pos, x, y);
   unsigned minX, maxX, minY, maxY;
   minX = max(0, x-1);
   maxX = min((int)regions.size()-1, x+1);
   minY = max(0, y-1);
   maxY = min((int)regions[x].size()-1, y+1);
   regs.clear();
   for(unsigned i = minX; i <= maxX; i++) {
      for(unsigned j = minY; j <= maxY; j++) {
         if(getRegionGeom(i,j).collision(g))
            regs.push_back(&regions[i][j]);
      }
   }
   if(regs.size() == 0) {
      //if((x == 0 || x == (int)regions.size()-1) 
      //      && (y == 0 || y == (int)regions[0].size()-1)) {
      //   printf("Error3: getRegions() xy<%d %d> pos<%0.1f %0.1f>\n", x, y, pos.x, pos.y);
      //}
      regs.push_back(&regions[x][y]);
   } 
   //else if (regs.size() > 4) {
   //   printf("Error: regs.size = %d (4 should be max?)\n", regs.size());
   //}
}

template <typename T>
void omMoveTemplate(T *p, const vec2 &newPos, ObjectManager &om)
{
   std::vector<Region *> regsOld, regsNew;
   om.getRegions(p->pos, p->getGeom(), regsOld);
   p->pos = newPos;
   om.getRegions(p->pos, p->getGeom(), regsNew);
   bool same = regsOld.size() == regsNew.size() &&
      std::equal(regsOld.begin(), regsOld.end(), regsNew.begin());
   if(!same) {
      //remove old regions
      for(unsigned i = 0; i < regsOld.size(); i++) {
         regsOld[i]->remove(p);
      }
      //add new regions
      for(unsigned i = 0; i < regsNew.size(); i++) {
         regsNew[i]->add(p);
      }
      //int x, y;
      //om.getRegion(newPos, x, y);
      //printf("Moved %d to region <%d %d> (removed %d)(added %d)\n", 
      //   p->id, x, y, regsOld.size(), regsNew.size());
   }
}

void ObjectManager::move(Player *p, const vec2 &newPos)
{
   omMoveTemplate(p, newPos, *this);
}

void ObjectManager::move(Item *i, const vec2 &newPos)
{
   omMoveTemplate(i, newPos, *this);
}

void ObjectManager::move(Missile *m, const vec2 &newPos)
{
   omMoveTemplate(m, newPos, *this);
}

void ObjectManager::move(NPC *n, const vec2 &newPos)
{
   omMoveTemplate(n, newPos, *this);
}

vector<Player *> ObjectManager::collidingPlayers(Geometry g, const vec2 &center)
{
   std::vector<Player *> ret;
   std::vector<Region *> regs;
   getRegions(center, g, regs);
   for(unsigned i = 0; i < regs.size(); i++) {
      std::vector<Player *> &regPlayers = regs[i]->players;
      for(unsigned j = 0; j < regPlayers.size(); j++) {
         if(regPlayers[j]->getGeom().collision(g)) {
            ret.push_back(regPlayers[j]);
         }
      }
   }
   util::removeDuplicates(ret);
   return ret;
}

vector<Missile *> ObjectManager::collidingMissiles(Geometry g, const vec2 &center)
{
   std::vector<Missile *> ret;
   std::vector<Region *> regs;
   getRegions(center, g, regs);
   for(unsigned i = 0; i < regs.size(); i++) {
      std::vector<Missile *> &regMissiles = regs[i]->missiles;
      for(unsigned j = 0; j < regMissiles.size(); j++) {
         if(regMissiles[j]->getGeom().collision(g)) {
            ret.push_back(regMissiles[j]);
         }
      }
   }
   util::removeDuplicates(ret);
   return ret;
}

vector<NPC *> ObjectManager::collidingNPCs(Geometry g, const vec2 &center)
{
   std::vector<NPC *> ret;
   std::vector<Region *> regs;
   getRegions(center, g, regs);

   for(unsigned i = 0; i < regs.size(); i++) {
      std::vector<NPC *> &regNPCs = regs[i]->npcs;
      for(unsigned j = 0; j < regNPCs.size(); j++) {
         if(regNPCs[j]->getGeom().collision(g)) {
            ret.push_back(regNPCs[j]);
         }
      }
   }
   util::removeDuplicates(ret);
   return ret;
}

vector<Item *> ObjectManager::collidingItems(Geometry g, const vec2 &center)
{
   std::vector<Item *> ret;
   std::vector<Region *> regs;
   getRegions(center, g, regs);

   for(unsigned i = 0; i < regs.size(); i++) {
      std::vector<Item *> &regItems = regs[i]->items;
      for(unsigned j = 0; j < regItems.size(); j++) {
         if(regItems[j]->getGeom().collision(g)) {
            ret.push_back(regItems[j]);
         }
      }
   }
   util::removeDuplicates(ret);
   return ret;
}

//add
void Region::add(Player *t) {
   players.push_back(t);
}

void Region::add(NPC *t) {
   npcs.push_back(t);
}

void Region::add(Item *t) {
   items.push_back(t);
}

void Region::add(Missile *t) {
   missiles.push_back(t);
}

//remove
void Region::remove(Player *p) {
   util::remove(p, players);
}

void Region::remove(Missile *m) {
   util::remove(m, missiles);
}

void Region::remove(NPC *n) {
   util::remove(n, npcs);
}

void Region::remove(Item *i) {
   util::remove(i, items);
}

bool Region::contains(Player *p) {
   return util::contains(p, players);
}

bool Region::contains(NPC *n) {
   return util::contains(n, npcs);
}

bool Region::contains(Missile *m) {
   return util::contains(m, missiles);
}

bool Region::contains(Item *i) {
   return util::contains(i, items);
}

} //end namespace server