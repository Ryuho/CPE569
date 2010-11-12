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

   struct Player {
      Player(int id, vec2 pos, vec2 dir, int hp);
      void move(vec2 pos, vec2 dir, bool moving = true);
      void takeDamage(int damage);
      void gainExp(int exp);
      void gainRupees(int rupees);
      Geometry getGeom();
      
      vec2 pos, dir;
      bool moving, alive;
      int id, hp, exp, rupees;
   };

   struct Missile {
      Missile(int id, int owned, vec2 pos, vec2 dir, int type = MissileType::Arrow);
      void update();
      Geometry getGeom();

      vec2 pos, dir;
      int id, owned, type, spawnTime;
   };

   struct NPC {
      NPC(int id, int hp, vec2 pos, vec2 dir, int type = NPCType::Skeleton);
      void update();
      void takeDamage(int damage);
      Geometry getGeom();
      int getLoot();
      int getExp();
      
      vec2 pos, dir, initPos;
      int aiTicks, aiType, attackId;
      int id, hp, type;
   };

   struct Item {
      Item(int id, vec2 pos, int type = ItemType::GreenRupee);
      Geometry getGeom();

      vec2 pos;
      int id, type;
   };


struct Region {
     vector<Player *> players;
     vector<Missile *> missiles;
     vector<NPC *> npcs;
     vector<Item *> items;
  };
 

struct ObjectManager {
     struct Index {
        Index() {}
        Index(int index, int type) : index(index), type(type) {}
        int index, type;
     };
     
     void init(float width, float length, float regionWidth);

     void addPlayer(Player p);
     void addMissile(Missile m);
     void addNPC(NPC n);
     void addItem(Item i);

     Player *getPlayer(int id);
     Missile *getMissile(int id);
     NPC *getNpc(int id);
     Item *getItem(int id);
     
     vector<Player *> collidingPlayers(Geometry g, vec3 center);
     vector<Missile *> collidingMissiles(Geometry g, vec3 center);
     vector<NPC *> collidingNPCs(Geometry g, vec3 center);
     vector<Item *> collidingItems(Geometry g, vec3 center);
     void updateRegions();

     void remove(int id);
     bool check(int id, int type);

     map<int, Index> idToIndex;
     vector<Player *> players;
     vector<Missile *> missiles;
     vector<NPC *> npcs;
     vector<Item *> items;
     
     vector<vector<Region> > regions;
  };


} // end server namespace

#endif
