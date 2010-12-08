#ifndef _BOTCHARACTERS_H_
#define _BOTCHARACTERS_H_

#include "Objects.h"
#include "matrix.h"
#include "Geometry.h"
#include "Constants.h"
#include <vector>
#include <map>


namespace botclient {

using mat::vec2;
using namespace constants;
using namespace objectManager;

struct Player : PlayerBase {
   Player() : PlayerBase(0, vec2()), moving(false) {}
   Player(int id, vec2 pos, vec2 dir, int hp);
   void move(vec2 pos, vec2 dir, bool moving);

   void update();

   vec2 dir;
   bool moving;
   int lastUpdate;
   int hp, rupees, exp;
   bool pvp;
};


struct Missile : MissileBase {
   Missile(int id, int type, vec2 pos, vec2 dir);
   void update();
   void move(vec2 pos, vec2 dir);

   int lastUpdate;
   bool alive;
   vec2 dir;
};

struct Item : ItemBase {
   Item(int id, int type, vec2 pos);
   void update();
   bool isCollectable() const;
   void move(vec2 pos);

   int lastUpdate;
   bool alive;
};

struct NPC : NPCBase {
   NPC(int id, int type, int hp, vec2 pos, vec2 dir, bool moving);
   void update();
   void move(vec2 pos, vec2 dir, bool moving);
   
   int lastUpdate;
   bool moving, alive;
   vec2 dir;
   int hp;
};

struct ObjectHolder : ObjectManager {
   ObjectHolder() : ObjectManager() {}

   bool add(Player *obj);
   bool add(NPC *obj);
   bool add(Item *obj);
   bool add(Missile *obj);

   Player *getPlayer(int id);
   NPC *getNPC(int id);
   Item *getItem(int id);
   Missile *getMissile(int id);
   
   void updateAll();
};

} //end namespace



#endif //_BOTCHARACTERS_H_
