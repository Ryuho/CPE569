#ifndef _SERVER_OM_H_
#define _SERVER_OM_H_

#include "Constants.h"
#include "matrix.h"
#include "packet.h"
#include "Geometry.h"
#include "ServerData.h"
#include <vector>
#include <map>
#include "Objects.h"
#include "Util.h"

namespace server {
   //using namespace std;
   using namespace mat;
   using namespace constants;
   using namespace geom;
   using namespace objmanager;

   struct ObjectManager {
      ObjectManager() : rm(0), regionSize(0), regionXSize(0), regionYSize(0) {}
      void init(float width, float height, float regionWidth);
      bool inBounds(vec2 pos) const;

      Player *getPlayer(int id);
      Missile *getMissile(int id);
      NPC *getNpc(int id);
      Item *getItem(int id);
      
      bool add(Player *p);
      bool add(Missile *m);
      bool add(NPC *n);
      bool add(Item *i);
      
      bool remove(int id);
      bool check(int id, unsigned type);
      
      void collidingPlayers(Geometry g, vec2 center, 
         std::vector<Player *> &collided);
      void collidingMissiles(Geometry g, vec2 center,
         std::vector<Missile *> &collided);
      void collidingNPCs(Geometry g, vec2 center,
         std::vector<NPC *> &collided);
      void collidingItems(Geometry g, vec2 center,
         std::vector<Item *> &collided);
      
      void move(Player *p, vec2 newPos);
      void move(Item *i, vec2 newPos);
      void move(Missile *m, vec2 newPos);
      void move(NPC *n, vec2 newPos);
      
      Object *get(unsigned type, int index_Not_The_Id);
      
      unsigned itemCount() const;
      unsigned playerCount() const;
      unsigned npcCount() const;
      unsigned missileCount() const;
      
      vec2 worldBotLeft;
      float regionSize;
      unsigned regionXSize, regionYSize;
      
   protected:
      void _move(Object *obj, vec2 &pos, vec2 &newPos);
      server::Object *_get(int id, unsigned type) const;
      server::Object *_get(int id) const;
      bool _add(Object *obj, vec2 pos, Geometry g);
      template<typename Ty, unsigned ObjectTy>
      vector<Ty> &_colliding(Geometry g, const vec2 &center, 
         std::vector<Ty> &collided);
      vec2 toWorldPos(vec2 pos);
      void getRegion(vec2 pos, int &x, int &y);
      void getRegions(vec2 pos, Geometry g, std::vector<unsigned> &regionIds);
      Geometry getRegionGeom(int x, int y);
      
      objmanager::RegionManager *rm;
   };

   template<typename Ty, unsigned ObjectTy>
   vector<Ty> &ObjectManager::_colliding(Geometry g, const vec2 &center, 
      std::vector<Ty> &collided)
   {
      std::vector<unsigned> regionIds;
      getRegions(center, g, regionIds);
      for(unsigned i = 0; i < regionIds.size(); i++) {
         Region &region = *rm->getRegion(regionIds[i]);
         std::vector<objmanager::Object *> &objs
            = region.getObjects(ObjectTy);
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
