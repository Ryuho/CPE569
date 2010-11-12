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
      Geometry getGeom();
      
      vec2 pos, dir;
      bool moving, alive;
      int id, hp, exp;
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


   struct ObjectManager {
      struct Index {
         Index() {}
         Index(int index, int type) : index(index), type(type) {}
         int index, type;
      };

      void addPlayer(Player p);
      void addMissile(Missile m);
      void addNPC(NPC n);
      void addItem(Item i);

      Player *getPlayer(int id);
      Missile *getMissile(int id);
      NPC *getNpc(int id);
      Item *getItem(int id);

      void remove(int id);
      bool check(int id, int type);

      map<int, Index> idToIndex;
      vector<Player *> players;
      vector<Missile *> missiles;
      vector<NPC *> npcs;
      vector<Item *> items;
   };

} // end server namespace

#endif
