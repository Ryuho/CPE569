#ifndef _SERVER_OM_H_
#define _SERVER_OM_H_

#include "Regions.h"
#include "Geometry.h"
#include "matrix.h"
#include "packet.h"
#include "Util.h"
#include "Constants.h"
#include <vector>
#include <map>

namespace objectManager {
   //using namespace std;
   using namespace mat;
   using namespace constants;
   using namespace geom;
   using namespace regionManager;

   struct ObjectBase : RMObject {
      ObjectBase(int id, vec2 pos)
         : RMObject(id), pos(pos) {}
      virtual int getType() const = 0;
      virtual float getRadius() const = 0;
      Geometry getGeom() const {
         return Circle(pos, getRadius());
      }
      vec2 pos;
   };

   struct PlayerBase : ObjectBase {
      PlayerBase(int id, vec2 pos) 
         : ObjectBase(id, pos) {}
      int getType() const;
      float getRadius() const;
   };

   struct NPCBase : ObjectBase {
      NPCBase(int id, int type, vec2 pos) 
         : ObjectBase(id, pos), type(type) {}
      int getType() const;
      float getRadius() const;
      int type;
   };

   struct ItemBase : ObjectBase {
      ItemBase(int id, int type, vec2 pos) 
         : ObjectBase(id, pos), type(type) {}
      int getType() const;
      float getRadius() const;
      int type;
   };

   struct MissileBase : ObjectBase {
      MissileBase(int id, int type, vec2 pos) 
         : ObjectBase(id, pos), type(type) {}
      int getType() const;
      float getRadius() const;
      int type;
   };

   struct ObjectManager {
      ObjectManager();

      bool inBounds(vec2 pos) const;
      PlayerBase *getPlayer(int id) const;
      MissileBase *getMissile(int id) const;
      NPCBase *getNPC(int id) const;
      ItemBase *getItem(int id) const;
      
      bool add(PlayerBase *obj);
      bool add(MissileBase *obj);
      bool add(NPCBase *obj);
      bool add(ItemBase *obj);
      
      bool remove(int id);
      bool contains(int id) const;
      bool contains(int id, int type) const;
      ObjectBase *get(int id) const;
      ObjectBase *get(int type, int index_Not_The_Id) const;
      
      void collidingPlayers(Geometry g, vec2 center, 
         std::vector<PlayerBase *> &collided) const;
      void collidingMissiles(Geometry g, vec2 center,
         std::vector<MissileBase *> &collided) const;
      void collidingNPCs(Geometry g, vec2 center,
         std::vector<NPCBase *> &collided) const;
      void collidingItems(Geometry g, vec2 center,
         std::vector<ItemBase *> &collided) const;
      
      void move(PlayerBase *p, vec2 newPos);
      void move(ItemBase *i, vec2 newPos);
      void move(MissileBase *m, vec2 newPos);
      void move(NPCBase *n, vec2 newPos);
      
      unsigned itemCount() const;
      unsigned playerCount() const;
      unsigned npcCount() const;
      unsigned missileCount() const;
      unsigned objectCount() const;
      
      vec2 worldBotLeft;
      
   protected:
      void _move(ObjectBase *obj, vec2 &pos, vec2 &newPos);
      ObjectBase *_get(int id, int type) const;
      ObjectBase *_get(int id) const;
      bool _add(ObjectBase *obj, vec2 pos, Geometry g);
      template<typename Ty, int ObjectTy>
      vector<Ty> &_colliding(Geometry g, const vec2 &center, 
         std::vector<Ty> &collided) const;
      vec2 toWorldPos(vec2 pos) const;
      void getRegion(vec2 pos, int &x, int &y) const;
      void getRegions(vec2 pos, Geometry g, std::vector<int> &regionIds) const;
      Geometry getRegionGeom(int x, int y) const;
      
      RegionManager rm;
   };

   template<typename Ty, int ObjectTy>
   vector<Ty> &ObjectManager::_colliding(Geometry g, const vec2 &center, 
      std::vector<Ty> &collided) const
   {
      std::vector<int> regionIds;
      getRegions(center, g, regionIds);
      int counted = 0;
      for(unsigned i = 0; i < regionIds.size(); i++) {
         const Region &region = *rm.getRegion(regionIds[i]);
         const std::vector<RMObject *> &objs = region[ObjectTy];
         for(unsigned j = 0; j < objs.size(); j++) {
            Ty obj = static_cast<Ty>(objs[j]);
            if(obj->getGeom().collision(g)) {
               collided.push_back(obj);
            }
         }
      }
      util::removeDuplicates(collided);
      return collided;
   }

} // end server namespace
  

#endif
