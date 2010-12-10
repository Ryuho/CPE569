#ifndef _SERVER_DATA_H_
#define _SERVER_DATA_H_

#include "Objects.h"
#include "matrix.h"
#include "packet.h"
#include "Geometry.h"
#include "Constants.h"
#include "Util.h"
#include <vector>
#include <map>

namespace server {
   using namespace std;
   using namespace mat;
   using namespace constants;
   using namespace geom;
   using pack::Packet;
   using namespace objectManager;

   struct Serializable {
      virtual Packet serialize() const = 0;
      virtual void deserialize(Packet &serialized) = 0;
      virtual Packet cserialize() const = 0; //client serialization (Initialize)
   };

   struct Player : PlayerBase, Serializable {
      Player(int id, int sid, vec2 pos, vec2 dir, int hp);
      Player(Packet &serialized);
      void move(vec2 pos, vec2 dir, bool moving = true);
      void takeDamage(int damage);
      void gainExp(int exp);
      void gainRupees(int rupees);
      void gainHp(int hp);
      Packet serialize() const;
      Packet cserialize() const;
      void deserialize(Packet &serialized);
      
      vec2 dir;
      bool moving, alive, pvp;
      int sid, hp, exp, rupees;
      bool shotThisFrame;
   };

   struct Missile : MissileBase, Serializable {
      Missile(int id, int sid, int owned, vec2 pos, vec2 dir, int type = MissileType::Arrow);
      Missile(Packet &serialized);
      void move(vec2 pos, vec2 dir);
      void update();
      int getDamage() const;
      Packet serialize() const;
      Packet cserialize() const;
      void deserialize(Packet &serialized);

      vec2 dir;
      int sid, owned, spawnTime;
   };

   struct NPC : NPCBase, Serializable {
      NPC(int id, int sid, int hp, vec2 pos, vec2 dir, int type = NPCType::Skeleton);
      NPC(Packet &serialized);
      void update();
      void takeDamage(int damage);
      int getLoot();
      int getExp();
      void gainHp(int hp);
      void move(vec2 pos, vec2 dir, bool moving);
      int getAttackDelay() const;
      Packet serialize() const;
      Packet cserialize() const;
      void deserialize(Packet &serialized);
      
      vec2 dir, initPos;
      bool moving;
      int aiTicks, aiType, attackId, nextMissileTicks;
      int sid, hp;
   };

   struct Item : ItemBase, Serializable {
      Item(int id, int sid, vec2 pos, int type = ItemType::GreenRupee);
      Item(Packet &serialized);
      bool isCollectable() const;
      bool isCollidable() const; //cannot be walked onto? Stump
      void move(vec2 pos);
      Packet serialize() const;
      Packet cserialize() const;
      void deserialize(Packet &serialized);

      int sid;
   };

   struct ObjectHolder : ObjectManager {
      ObjectHolder() : ObjectManager() {}

      Player *getPlayer(int id) const;
      NPC *getNPC(int id) const;
      Item *getItem(int id) const;
      Missile *getMissile(int id) const;

      Player *getPlayerByIndex(int index) const;
      NPC *getNPCByIndex(int index) const;
      Item *getItemByIndex(int index) const;
      Missile *getMissileByIndex(int index) const;

      Packet getSerialized(int id) const;
      Packet getCSerialized(int id) const;
      Serializable *getSerializable(int id) const;
   };

} // end server namespace

#endif
