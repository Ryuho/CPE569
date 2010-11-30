#ifndef _SERVER_DATA_H_
#define _SERVER_DATA_H_

#include "Constants.h"
#include "matrix.h"
#include "packet.h"
#include "Geometry.h"
#include "Util.h"
#include <vector>
#include <map>

namespace server {
   using namespace std;
   using namespace mat;
   using namespace constants;
   using namespace geom;
   using pack::Packet;

   struct Object {
      Object() {}
      virtual int getObjectType() const = 0;
      //void lock() const;
      //void unlock() const;
      virtual Packet serialize() const = 0;
      virtual Geometry getGeom() const = 0;
      virtual void deserialize(Packet &serialized) = 0;
   };

   struct Player : Object {
      Player(int id, int sid, vec2 pos, vec2 dir, int hp);
      Player(Packet &serialized);
      void move(vec2 pos, vec2 dir, bool moving = true);
      void takeDamage(int damage);
      void gainExp(int exp);
      void gainRupees(int rupees);
      Geometry getGeom() const;
      float getRadius() const;
      int getObjectType() const;
      Packet serialize() const;
      void gainHp(int hp);
      void deserialize(Packet &serialized);
      
      vec2 pos, dir;
      bool moving, alive, pvp;
      int id, sid, hp, exp, rupees;
      bool shotThisFrame;
   };

   struct Missile : Object {
      Missile(int id, int sid, int owned, vec2 pos, vec2 dir, int type = MissileType::Arrow);
      Missile(Packet &serialized);
      void move(vec2 pos, vec2 dir);
      void update();
      int getDamage() const;
      Geometry getGeom() const;
      int getObjectType() const;
      Packet serialize() const;
      void deserialize(Packet &serialized);

      vec2 pos, dir;
      int id, sid, owned, type, spawnTime;
   };

   struct NPC : Object {
      NPC(int id, int sid, int hp, vec2 pos, vec2 dir, int type = NPCType::Skeleton);
      NPC(Packet &serialized);
      void update();
      void takeDamage(int damage);
      float getRadius() const;
      Geometry getGeom() const;
      int getLoot();
      int getExp();
      void gainHp(int hp);
      void move(vec2 pos, vec2 dir, bool moving);
      int getObjectType() const;
      Packet serialize() const;
      void deserialize(Packet &serialized);
      int getAttackDelay() const;
      
      vec2 pos, dir, initPos;
      bool moving;
      int aiTicks, aiType, attackId, nextMissileTicks;
      int id, sid, hp, type;
   };

   struct Item : Object {
      Item(int id, int sid, vec2 pos, int type = ItemType::GreenRupee);
      Item(Packet &serialized);
      float getRadius() const;
      Geometry getGeom() const;
      bool isCollectable() const;
      bool isCollidable() const; //cannot be walked onto? Stump
      void move(vec2 pos);
      int getObjectType() const;
      Packet serialize() const;
      void deserialize(Packet &serialized);

      vec2 pos;
      int id, sid, type;
   };

} // end server namespace

#endif
