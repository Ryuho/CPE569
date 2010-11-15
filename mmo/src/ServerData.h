#ifndef _SERVER_DATA_H_
#define _SERVER_DATA_H_

#include "Constants.h"
#include "matrix.h"
#include "packet.h"
#include "Geometry.h"
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
      Player(int id, vec2 pos, vec2 dir, int hp);
      Player(Packet &serialized);
      void move(vec2 pos, vec2 dir, bool moving = true);
      void takeDamage(int damage);
      void gainExp(int exp);
      void gainRupees(int rupees);
      Geometry getGeom() const;
      int getObjectType() const;
      Packet serialize() const;
      void deserialize(Packet &serialized);
      
      vec2 pos, dir;
      bool moving, alive;
      int id, hp, exp, rupees;
   };

   struct Missile : Object {
      Missile(int id, int owned, vec2 pos, vec2 dir, int type = MissileType::Arrow);
      Missile(Packet &serialized);
      void update();
      Geometry getGeom() const;
      int getObjectType() const;
      Packet serialize() const;
      void deserialize(Packet &serialized);

      vec2 pos, dir;
      int id, owned, type, spawnTime;
   };

   struct NPC : Object {
      NPC(int id, int hp, vec2 pos, vec2 dir, int type = NPCType::Skeleton);
      NPC(Packet &serialized);
      void update();
      void takeDamage(int damage);
      Geometry getGeom() const;
      int getLoot();
      int getExp();
      int getObjectType() const;
      Packet serialize() const;
      void deserialize(Packet &serialized);
      
      vec2 pos, dir, initPos;
      int aiTicks, aiType, attackId;
      int id, hp, type;
   };

   struct Item : Object {
      Item(int id, vec2 pos, int type = ItemType::GreenRupee);
      Item(Packet &serialized);
      Geometry getGeom() const;
      int getObjectType() const;
      Packet serialize() const;
      void deserialize(Packet &serialized);

      vec2 pos;
      int id, type;
   };


struct Region {
   vector<Player *> players;
   vector<Missile *> missiles;
   vector<NPC *> npcs;
   vector<Item *> items;

   void remove(Player *p);
   void remove(Missile *m);
   void remove(NPC *n);
   void remove(Item *i);
};

struct ObjectManager {
     struct Index {
        Index() {}
        Index(int index, int type) : index(index), type(type) {}
        int index, type;
     };
     
     void init(float width, float height, float regionWidth);

     void addPlayer(Player p);
     void addMissile(Missile m);
     void addNPC(NPC n);
     void addItem(Item i);

     void getRegion(vec2 pos, int &x, int &y);
     void getRegions(vec2 pos, Geometry g, std::vector<Region *> &regs);
     void move(Player *p, vec2 newPos);
     void move(Item *i, vec2 newPos);
     void move(Missile *m, vec2 newPos);
     void move(NPC *n, vec2 newPos);
     Geometry getRegionGeom(int x, int y);

     Player *getPlayer(int id);
     Missile *getMissile(int id);
     NPC *getNpc(int id);
     Item *getItem(int id);
     
     vector<Player *> collidingPlayers(Geometry g, vec3 center);
     vector<Missile *> collidingMissiles(Geometry g, vec3 center);
     vector<NPC *> collidingNPCs(Geometry g, vec3 center);
     vector<Item *> collidingItems(Geometry g, vec3 center);

     void remove(int id);
     bool check(int id, int type);

     map<int, Index> idToIndex;
     vector<Player *> players;
     vector<Missile *> missiles;
     vector<NPC *> npcs;
     vector<Item *> items;
     
     vector<vector<Region> > regions; //starts botLeft and increases in <X,Y> 
   private:
     vec2 worldBotLeft;
     float regionSize;
  };


} // end server namespace

#endif
