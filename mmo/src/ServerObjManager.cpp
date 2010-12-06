#include "ServerObjManager.h"
#include "Util.h"
#include <vector>

namespace server {

inline unsigned omToRm(int x, int y, int regionXSize)
{
   return x + y*regionXSize;
}

///////////// Vectors ////////////
//////////////////////////////////
Object *ObjectManager::get(unsigned type, int index_Not_The_Id)
{
   return static_cast<Object *>((*rm)[type][index_Not_The_Id]);
}

///////////// Getters ////////////
//////////////////////////////////
server::Object *ObjectManager::_get(int id, unsigned type) const
{
   server::Object *obj = static_cast<server::Object *>(rm->getObject(id));
   if(obj && obj->getType() == type)
      return obj;
   return 0;
}

server::Object *ObjectManager::_get(int id) const
{
   return static_cast<server::Object *>(rm->getObject(id));
}

Player *ObjectManager::getPlayer(int id)
{
   return static_cast<Player *>(_get(id, ObjectType::Player));
}

Missile *ObjectManager::getMissile(int id)
{
   return static_cast<Missile *>(_get(id, ObjectType::Missile));
}

NPC *ObjectManager::getNpc(int id)
{
   return static_cast<NPC *>(_get(id, ObjectType::NPC));
}

Item *ObjectManager::getItem(int id)
{
   return static_cast<Item *>(_get(id, ObjectType::Item));
}

////////////// Count /////////////
//////////////////////////////////
unsigned ObjectManager::itemCount() const
{
   return rm->count(ObjectType::Item);
}

unsigned ObjectManager::playerCount() const
{
   return rm->count(ObjectType::Player);
}

unsigned ObjectManager::npcCount() const
{
   return rm->count(ObjectType::NPC);
}

unsigned ObjectManager::missileCount() const
{
   return rm->count(ObjectType::Missile);
}

/////////////// Add //////////////
//////////////////////////////////
bool ObjectManager::_add(Object *obj, vec2 pos, Geometry g) 
{
   std::vector<unsigned> regionIds;
   getRegions(pos, g, regionIds);
   return regionIds.size() != 0 
      && rm->addObject(obj, obj->getType(), regionIds);
}

bool ObjectManager::add(Player *obj)
{
   return _add(obj, obj->pos, obj->getGeom());
}

bool ObjectManager::add(Missile *obj)
{
   return _add(obj, obj->pos, obj->getGeom());
}

bool ObjectManager::add(NPC *obj)
{
   return _add(obj, obj->pos, obj->getGeom());
}

bool ObjectManager::add(Item *obj)
{
   return _add(obj, obj->pos, obj->getGeom());
}

////////////// Move //////////////
//////////////////////////////////
void ObjectManager::_move(Object *obj, vec2 &pos, vec2 &newPos)
{
   assert(rm->getObject(obj->getId()));
   std::vector<unsigned> regsNew;
   const std::vector<unsigned> &regsOld = rm->getData(obj->getId())->regionIds;
   pos = toWorldPos(newPos);
   getRegions(newPos, obj->getGeom(), regsNew);
   if(regsNew.size() != regsOld.size()
      || !std::equal(regsOld.begin(), regsOld.end(), regsNew.begin())) 
   {
      rm->removeObject(obj->getId(), obj->getType());
      rm->addObject(obj, obj->getType(), regsNew);

//debug version
/*
      int count = rm->objectCount();
      std::vector<unsigned> regionIds(rm->getData(obj->getId())->regionIds);
      assert(rm->getData(obj->getId()));
      assert(rm->getData(obj->getId())->obj == obj);
      assert(rm->getData(obj->getId())->regionIds.size() > 0);
      for(unsigned i = 0; i < regionIds.size(); i++) {
         assert(rm->getRegion(regionIds[i])->contains(obj->getId()));
         assert(rm->getRegion(regionIds[i])->get(obj->getId(), obj->getType()));
      }

      assert(rm->removeObject(obj->getId(), obj->getType()));
      for(unsigned i = 0; i < regionIds.size(); i++) {
         assert(!rm->getRegion(regionIds[i])->contains(obj->getId()));
         assert(!rm->getRegion(regionIds[i])->get(obj->getId(), obj->getType()));
      }

      //check RM
      assert(!rm->removeObject(obj->getId(), obj->getType()));
      assert(count - 1 == rm->objectCount());
      assert(!rm->getData(obj->getId()));
      RegionManager::iterator iter = rm->begin(ObjectType::NPC);
      for(;iter != rm->end(ObjectType::NPC); iter++) {
         assert((*iter)->getId() != obj->getId());
      }
      //check Regions
      for(unsigned i = 0; i < rm->regionCount(); i++) {
         Region &region = *rm->getRegion(i);
         assert(!region.contains(obj->getId()));
         assert(!region.get(obj->getId(), ObjectType::NPC));
         std::vector<objmanager::Object *> &vec 
            = region.getObjects(ObjectType::NPC);
         std::vector<objmanager::Object *>::iterator iter;
         for(iter = vec.begin(); iter != vec.end(); iter++) {
            assert((*iter)->getId() != obj->getId());
         }
      }
      rm->addObject(obj, obj->getType(), regsNew);

      assert(count == rm->objectCount());
      regionIds = rm->getData(obj->getId())->regionIds;
      for(unsigned i = 0; i < regionIds.size(); i++) {
         assert(rm->getRegion(regionIds[i])->contains(obj->getId()));
         assert(rm->getRegion(regionIds[i])->get(obj->getId(), obj->getType()));
      }
*/
   }
}

void ObjectManager::move(Player *obj, vec2 newPos)
{
   _move(obj, obj->pos, newPos);
}

void ObjectManager::move(Item *obj, vec2 newPos)
{
   _move(obj, obj->pos, newPos);
}

void ObjectManager::move(Missile *obj, vec2 newPos)
{
   _move(obj, obj->pos, newPos);
}

void ObjectManager::move(NPC *obj, vec2 newPos)
{
   _move(obj, obj->pos, newPos);
}

/////////// Colliding ////////////
//////////////////////////////////
void ObjectManager::collidingPlayers(Geometry g, vec2 center, 
   std::vector<Player *> &collided)
{
   _colliding<Player *, ObjectType::Player>(g, center, collided);
}

void ObjectManager::collidingMissiles(Geometry g, vec2 center,
   std::vector<Missile *> &collided)
{
   _colliding<Missile *, ObjectType::Missile>(g, center, collided);
}

void ObjectManager::collidingNPCs(Geometry g, vec2 center,
   std::vector<NPC *> &collided)
{
   _colliding<NPC *, ObjectType::NPC>(g, center, collided);
}

void ObjectManager::collidingItems(Geometry g, vec2 center,
   std::vector<Item *> &collided)
{
   _colliding<Item *, ObjectType::Item>(g, center, collided);
}

///////////// Others /////////////
//////////////////////////////////
void ObjectManager::init(float width, float height, float regionWidth)
{
   assert(!rm); //only initialize once

   width = width;
   height = height;
   this->worldBotLeft = vec2(-width/2.0f, -height/2.0f);
   this->regionSize = constants::regionSize;
   regionXSize = ((int)(width / regionSize));
   regionYSize = ((int)(height / regionSize));
   if(regionXSize * regionSize != width)
      regionXSize++;
   if(regionYSize * regionSize != height)
      regionYSize++;

   rm = new RegionManager(regionXSize*regionYSize, ObjectType::ObjectTypeCount);
   printf("Initialized ObjectManager <%d by %d> total=%d\n", regionXSize, 
      regionYSize, regionXSize*regionYSize);
}

bool ObjectManager::inBounds(vec2 pos) const
{
   return pos.x >= worldBotLeft.x && pos.x <= worldBotLeft.x+worldWidth
      && pos.y >= worldBotLeft.y && pos.y <= worldBotLeft.y+worldHeight;
}

bool ObjectManager::remove(int id)
{
   Object *obj = _get(id);
   if(!obj || !rm->removeObject(id, obj->getType())) {
      printf("Error: Failed to remove Object id=%d", id);
      if(obj)
         printf(" type=%d\n", obj->getType());
      printf("\n");
      return false;
   }
   delete obj;
   return true;
}

bool ObjectManager::check(int id, unsigned type)
{
   Object *obj = _get(id);
   return obj && obj->getType() == type;
}

/////////// Protected ////////////
//////////////////////////////////
void ObjectManager::getRegion(vec2 pos, int &x, int &y)
{
   x = (int) ((pos.x - worldBotLeft.x) / regionSize);
   y = (int) ((pos.y - worldBotLeft.y) / regionSize);
   util::clamp(x, 0, (int) regionXSize-1);
   util::clamp(y, 0, (int) regionYSize-1);
}

Geometry ObjectManager::getRegionGeom(int x, int y)
{
   return Rectangle(
      vec2(worldBotLeft.x + regionSize*x, worldBotLeft.y + regionSize*y),
      regionSize, regionSize);
}

void ObjectManager::getRegions(vec2 pos, Geometry g, 
   std::vector<unsigned> &regionIds)
{
   regionIds.clear();
   int x, y;
   getRegion(pos, x, y);
   int minX = std::max<int>(0, x-deltaRegion);
   int maxX = std::min<int>(regionXSize-1, x+deltaRegion);
   int minY = std::max<int>(0, y-deltaRegion);
   int maxY = std::min<int>(regionYSize-1, y+deltaRegion);
   for(int i = minX; i <= maxX; i++) {
      for(int j = minY; j <= maxY; j++) {
         if(getRegionGeom(i,j).collision(g)) {
            regionIds.push_back(omToRm(i, j, regionXSize));
         }
      }
   }
   if(regionIds.size() == 0)
      regionIds.push_back(omToRm(x, y, regionXSize));
}

vec2 ObjectManager::toWorldPos(vec2 pos)
{
   if(!inBounds(pos)) {
      int x, y;
      getRegion(pos, x, y);
      return vec2(worldBotLeft.x + regionXSize*(x+0.5f), 
         worldBotLeft.y + regionYSize*(y+0.5f));
   }
   return pos;
}

} //end namespace server