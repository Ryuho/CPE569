#include "Objects.h"
#include "Util.h"
#include <vector>


namespace objectManager {


inline int omToRm(int x, int y, int regionXSize)
{
   return x + y*regionXSize;
}

///////////// getType ////////////
//////////////////////////////////
int PlayerBase::getType() const
{
   return ObjectType::Player;
}

int NPCBase::getType() const
{
   return ObjectType::NPC;
}

int ItemBase::getType() const
{
   return ObjectType::Item;
}

int MissileBase::getType() const
{
   return ObjectType::Missile;
}

//////////// getRadius ///////////
//////////////////////////////////
float PlayerBase::getRadius() const
{
   return playerRadius;
}

float NPCBase::getRadius() const
{
   switch(type) {
      case NPCType::Fairy: //16x16
      case NPCType::Bat:
      case NPCType::Bird:
         return 16*1.5f;
         break;
      case NPCType::Thief: //16x32
      case NPCType::Squirrel: 
      case NPCType::Princess:
      case NPCType::Skeleton:
      case NPCType::Cactus:
      case NPCType::Wizard:
      case NPCType::Goblin:
         return 22*1.5f;
         break;
      case NPCType::Cyclops: //32x32
      case NPCType::Chicken:
      case NPCType::Vulture:
      case NPCType::Bush:
      case NPCType::BigFairy:
      case NPCType::Ganon:
         return 23.5f*1.5f;
         break;
      default:
         printf("Error NPC::getRadius() - unknown NPC type %d\n", type);
   }
   return 0.0f;
}

float ItemBase::getRadius() const
{
   switch (type) {
      case ItemType::GreenRupee:
      case ItemType::RedRupee:
      case ItemType::BlueRupee:
         return 12*1.5f;
         break; //unreachable
      case ItemType::Explosion:
         return 55*1.5f;
         break;
      case ItemType::Stump:
         return 32*1.5f;
         break;
      case ItemType::Heart:
         return 13*1.5f;
         break;
      default:
         printf("Error Item::getGeom - Unknown item type %d\n", type);
   }
   return 0.0f;
}

float MissileBase::getRadius() const
{
   return arrowRadius;
}

///////////// Getters ////////////
//////////////////////////////////
ObjectBase *ObjectManager::get(int id) const
{
   return _get(id);
}

ObjectBase *ObjectManager::get(int type, int index_Not_The_Id) const
{
   return static_cast<ObjectBase *>(rm[type][index_Not_The_Id]);
}

ObjectBase *ObjectManager::_get(int id, int type) const
{
   ObjectBase *obj = static_cast<ObjectBase *>(rm.get(id));
   if(obj && obj->getType() == type)
      return obj;
   return 0;
}

ObjectBase *ObjectManager::_get(int id) const
{
   return static_cast<ObjectBase *>(rm.get(id));
}

PlayerBase *ObjectManager::getPlayer(int id) const
{
   return static_cast<PlayerBase *>(_get(id, ObjectType::Player));
}

MissileBase *ObjectManager::getMissile(int id) const
{
   return static_cast<MissileBase *>(_get(id, ObjectType::Missile));
}

NPCBase *ObjectManager::getNPC(int id) const
{
   return static_cast<NPCBase *>(_get(id, ObjectType::NPC));
}

ItemBase *ObjectManager::getItem(int id) const
{
   return static_cast<ItemBase *>(_get(id, ObjectType::Item));
}

////////////// Count /////////////
//////////////////////////////////
unsigned ObjectManager::itemCount() const
{
   return rm.count(ObjectType::Item);
}

unsigned ObjectManager::playerCount() const
{
   return rm.count(ObjectType::Player);
}

unsigned ObjectManager::npcCount() const
{
   return rm.count(ObjectType::NPC);
}

unsigned ObjectManager::missileCount() const
{
   return rm.count(ObjectType::Missile);
}

unsigned ObjectManager::objectCount() const 
{
   return rm.objectCount();
}

/////////////// Add //////////////
//////////////////////////////////
bool ObjectManager::_add(ObjectBase *obj, vec2 pos, Geometry g) 
{
   std::vector<int> regionIds;
   getRegions(pos, g, regionIds);
   return rm.add(obj, obj->getType(), regionIds);
}

bool ObjectManager::add(PlayerBase *obj)
{
   return _add(obj, obj->pos, obj->getGeom());
}

bool ObjectManager::add(MissileBase *obj)
{
   return _add(obj, obj->pos, obj->getGeom());
}

bool ObjectManager::add(NPCBase *obj)
{
   return _add(obj, obj->pos, obj->getGeom());
}

bool ObjectManager::add(ItemBase *obj)
{
   return _add(obj, obj->pos, obj->getGeom());
}

////////////// Move //////////////
//////////////////////////////////
void ObjectManager::move(ObjectBase *obj, const vec2 &newPos)
{
   assert(rm.get(obj->getId()));
   std::vector<int> regsNew;
   const std::vector<int> &regsOld = rm.getData(obj->getId())->regionIds;
   obj->pos = toWorldPos(newPos);
   getRegions(newPos, obj->getGeom(), regsNew);
   if(!util::isequal(regsOld, regsNew)) 
   {
      rm.remove(obj->getId(), obj->getType());
      rm.add(obj, obj->getType(), regsNew);

/*
//debug version
      regionManager::RMObject * rmobj;
      assert(regsNew.size() > 0 && regsOld.size() > 0);
      int count = rm.objectCount();
      std::vector<int> regionIds(rm.getData(obj->getId())->regionIds);
      assert(regionIds.size() > 0);
      assert(count);
      assert(rm.getData(obj->getId()));
      assert(rm.getData(obj->getId())->obj == obj);
      assert(rm.getData(obj->getId())->regionIds.size() == regsOld.size());
      for(unsigned i = 0; i < regionIds.size(); i++) {
         Region &region = *rm.getRegion(regionIds[i]);
         assert(region.contains(obj->getId()));
         int type = obj->getType();
         rmobj = region.get(obj->getId(), type);
         if(!rmobj || rmobj != obj) {
            assert(0);
         }
         assert(!region.add(obj, obj->getType()));
         assert(region.count(obj->getType()) > 0);
      }

      //remove
      assert(rm.removeObject(obj->getId(), obj->getType()));

      for(unsigned i = 0; i < regionIds.size(); i++) {
         assert(!rm.getRegion(regionIds[i])->contains(obj->getId()));
         assert(!rm.getRegion(regionIds[i])->get(obj->getId(), obj->getType()));
      }

      //check RM
      assert(!rm.removeObject(obj->getId(), obj->getType()));
      assert(count - 1 == rm.objectCount());
      assert(!rm.getData(obj->getId()));
      for(unsigned i = 0; i < rm[ObjectType::NPC].size(); i++) {
         assert(rm[ObjectType::NPC][i]->getId() != obj->getId());
      }
      //check Regions
      for(unsigned i = 0; i < rm.regionCount(); i++) {
         Region &region = *rm.getRegion(i);
         assert(!region.contains(obj->getId()));
         assert(!region.get(obj->getId(), ObjectType::NPC));
         std::vector<regionManager::RMObject *> &vec 
            = region.getObjects(ObjectType::NPC);
         std::vector<regionManager::RMObject *>::iterator iter;
         for(iter = vec.begin(); iter != vec.end(); iter++) {
            assert((*iter)->getId() != obj->getId());
         }
      }

      //add
      assert(rm.addObject(obj, obj->getType(), regsNew));

      assert(count == rm.objectCount());
      assert(rm.getData(obj->getId()));
      assert(rm.getData(obj->getId())->obj == obj);
      assert(rm.getData(obj->getId())->regionIds.size() == regsNew.size());
      regionIds = rm.getData(obj->getId())->regionIds;
      for(unsigned i = 0; i < regionIds.size(); i++) {
         Region &region = *rm.getRegion(regionIds[i]);
         assert(region.contains(obj->getId()));
         assert(region.get(obj->getId(), obj->getType()));
         assert(!region.add(obj, obj->getType()));
      }
//#undef assert
*/


   }
}

/*
void ObjectManager::move(ObjectBase *obj, vec2 newPos)
{
   _move(obj, obj->pos, newPos);
}

void ObjectManager::move(ItemBase *obj, vec2 newPos)
{
   _move(obj, obj->pos, newPos);
}

void ObjectManager::move(MissileBase *obj, vec2 newPos)
{
   _move(obj, obj->pos, newPos);
}

void ObjectManager::move(NPCBase *obj, vec2 newPos)
{
   _move(obj, obj->pos, newPos);
}
*/

/////////// Colliding ////////////
//////////////////////////////////
void ObjectManager::collidingPlayers(Geometry g, vec2 center, 
   std::vector<PlayerBase *> &collided) const
{
   _colliding<PlayerBase *, ObjectType::Player>(g, center, collided);
}

void ObjectManager::collidingMissiles(Geometry g, vec2 center,
   std::vector<MissileBase *> &collided) const
{
   _colliding<MissileBase *, ObjectType::Missile>(g, center, collided);
}

void ObjectManager::collidingNPCs(Geometry g, vec2 center,
   std::vector<NPCBase *> &collided) const
{
   _colliding<NPCBase *, ObjectType::NPC>(g, center, collided);
}

void ObjectManager::collidingItems(Geometry g, vec2 center,
   std::vector<ItemBase *> &collided) const
{
   _colliding<ItemBase *, ObjectType::Item>(g, center, collided);
}

///////////// Others /////////////
//////////////////////////////////
ObjectManager::ObjectManager()
   : worldBotLeft(vec2(-worldWidth/2.0f, -worldHeight/2.0f)),
   rm(totalRegions, ObjectType::ObjectTypeCount)
{
   printf("Initialized ObjectManager <%d by %d> total=%d\n", regionXSize, 
      regionYSize, totalRegions);
}

bool ObjectManager::inBounds(vec2 pos) const
{
   return pos.x >= worldBotLeft.x && pos.x <= worldBotLeft.x+worldWidth
      && pos.y >= worldBotLeft.y && pos.y <= worldBotLeft.y+worldHeight;
}

bool ObjectManager::remove(int id)
{
   ObjectBase *obj = _get(id);
   if(!obj || !rm.remove(id, obj->getType())) {
      printf("Error: Failed to remove Object id=%d", id);
      if(obj)
         printf(" type=%d\n", obj->getType());
      printf("\n");
      return false;
   }
   delete obj;
   return true;
}

bool ObjectManager::contains(int id, int type) const
{
   return rm.contains(id, type);
}

bool ObjectManager::contains(int id) const
{
   return rm.contains(id);
}

/////////// Protected ////////////
//////////////////////////////////
void ObjectManager::getRegion(vec2 pos, int &x, int &y) const
{
   x = (int) ((pos.x - worldBotLeft.x) / regionSize);
   y = (int) ((pos.y - worldBotLeft.y) / regionSize);
   util::clamp(x, 0, (int) regionXSize-1);
   util::clamp(y, 0, (int) regionYSize-1);
}

Geometry ObjectManager::getRegionGeom(int x, int y) const
{
   return Rectangle(
      vec2(worldBotLeft.x + regionSize*x, worldBotLeft.y + regionSize*y),
      (float)regionSize, (float)regionSize);
}

void ObjectManager::getRegions(vec2 pos, Geometry g, 
   std::vector<int> &regionIds) const
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
   if(regionIds.size() == 0) {
      if(inBounds(pos)) {
         //Geometry g2 = getRegionGeom(x,y); 
         //Geometry g3 = Circle(pos, 0.1f);
         //printf("Error getRegions: pos<%0.1f %0.1f> reg<%d %d> %d %d %d %d\n", 
         //   pos.x, pos.y, x, y, g2.collision(g), g.collision(g2), g3.collision(g2), g2.collision(g3));
         printf("Error getRegions\n");
      }
      regionIds.push_back(omToRm(x, y, regionXSize));
   }
}

vec2 ObjectManager::toWorldPos(vec2 pos) const
{
   if(!inBounds(pos)) {
      int x, y;
      getRegion(pos, x, y);
      return vec2(worldBotLeft.x + regionXSize*(x+0.5f), 
         worldBotLeft.y + regionYSize*(y+0.5f));
   }
   return pos;
}

} //end namespace