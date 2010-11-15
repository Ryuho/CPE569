#include "ServerObjManager.h"
#include "Util.h"

namespace server {

void ObjectManager::addPlayer(Player &p)
{
   idToIndex[p.id] = Index(players.size(), ObjectType::Player);
   players.push_back(new Player(p));
   move(players.back(), players.back()->pos);
}

void ObjectManager::addMissile(Missile &m)
{
   idToIndex[m.id] = Index(missiles.size(), ObjectType::Missile);
   missiles.push_back(new Missile(m));
   move(missiles.back(), missiles.back()->pos);
}

void ObjectManager::addNPC(NPC &n)
{
   idToIndex[n.id] = Index(npcs.size(), ObjectType::NPC);
   npcs.push_back(new NPC(n));
   move(npcs.back(), npcs.back()->pos);
}

void ObjectManager::addItem(Item &i)
{
   idToIndex[i.id] = Index(items.size(), ObjectType::Item);
   items.push_back(new Item(i));
   move(items.back(), items.back()->pos);
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
void omRemoveTempl(map<int, ObjectManager::Index> &idToIndex, vector<T*> &objs, int id)
{
   int i = idToIndex[id].index;

   delete objs[i];
   idToIndex.erase(id);
   if (objs.size() > 1) {
      objs[i] = objs.back();
      idToIndex[objs[i]->id].index = i;
   }
   objs.pop_back();
}

void ObjectManager::remove(int id)
{   int i = idToIndex[id].index;

   if (idToIndex[id].type == ObjectType::Player)
      omRemoveTempl(idToIndex, players, id);
   else if (idToIndex[id].type == ObjectType::Missile)
      omRemoveTempl(idToIndex, missiles, id);
   else if (idToIndex[id].type == ObjectType::NPC)
      omRemoveTempl(idToIndex, npcs, id);
   else if (idToIndex[id].type == ObjectType::Item)
      omRemoveTempl(idToIndex, items, id);
}


bool ObjectManager::check(int id, int type)
{
   map<int, Index>::iterator itr = idToIndex.find(id);
   return itr != idToIndex.end() && itr->second.type == type;
}

void ObjectManager::init(float width, float height, float regionSize)
{
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
   x = min(max(x, 0), (int)regions.size()-1);
   y = min(max(y, 0), (int)regions[0].size()-1);
}

Geometry ObjectManager::getRegionGeom(int x, int y)
{
   return new Rectangle(
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
            regs.push_back(&regions[x][y]);
      }
   }
   if(regs.size() == 0) {
      regs.push_back(&regions[x][y]);
   }
}

template <typename T>
void moveTemplate(T *p, vec2 newPos, ObjectManager &om)
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
      int x, y;
      om.getRegion(newPos, x, y);
      //printf("Moved %d to region <%d %d> (total regions=%d)\n", p->id x, y, regsNew.size());
   }
}

void ObjectManager::move(Player *p, vec2 newPos)
{
   moveTemplate(p, newPos, *this);
}

void ObjectManager::move(Item *i, vec2 newPos)
{
   moveTemplate(i, newPos, *this);
}

void ObjectManager::move(Missile *m, vec2 newPos)
{
   moveTemplate(m, newPos, *this);
}

void ObjectManager::move(NPC *n, vec2 newPos)
{
   moveTemplate(n, newPos, *this);
}

vector<Player *> ObjectManager::collidingPlayers(Geometry g, vec2 center)
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

vector<Missile *> ObjectManager::collidingMissiles(Geometry g, vec2 center)
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

vector<NPC *> ObjectManager::collidingNPCs(Geometry g, vec2 center)
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

vector<Item *> ObjectManager::collidingItems(Geometry g, vec2 center)
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

} //end namespace server