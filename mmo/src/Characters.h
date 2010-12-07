#ifndef _CHARACTERS_H_
#define _CHARACTERS_H_

#include "Sprite.h"
#include "Objects.h"
#include "matrix.h"
#include "Geometry.h"
#include <vector>
#include <map>

namespace client {

using mat::vec2;
using namespace objectManager;

//struct Animation;

void initCharacterResources();

struct Player : PlayerBase {
   Player() : PlayerBase(0, vec2()), alive(false) {}
   Player(int id, vec2 pos, vec2 dir, int hp);

   void move(vec2 pos, vec2 dir, bool moving);
   void update();
   void draw();

   vec2 dir;
   float radius;
   bool moving, alive;
   int animStart, lastUpdate;
   int hp, rupees, exp;
   bool pvp;
};

struct Missile : MissileBase {
   Missile(int id, int type, vec2 pos, vec2 dir);

   void move(vec2 pos, vec2 dir);
   void update();
   void draw();

   bool alive;
   vec2 dir;
   int lastUpdate;
};

struct Item : ItemBase {
   Item(int id, int type, vec2 pos);

   void move(vec2 pos);
   void update();
   void draw();
   
   bool alive;
   Animation *anim;
   int lastUpdate;
private:
   void initGraphics();
};

struct NPC : NPCBase {
   NPC(int id, int type, int hp, vec2 pos, vec2 dir, bool moving);

   void move(vec2 pos, vec2 dir, bool moving);
   void resetAnimation();
   void update();
   void draw();
   
   vec2 dir;
   bool alive, moving;
   Animation *anim;
   int hp, lastUpdate;
private:
   void initGraphics();
};

struct ObjectHolder : ObjectManager {
   ObjectHolder() : ObjectManager() {}

   bool addPlayer(Player *obj);
   bool addNPC(NPC *obj);
   bool addItem(Item *obj);
   bool addMissile(Missile *obj);

   Player *getPlayer(int id);
   NPC *getNPC(int id);
   Item *getItem(int id);
   Missile *getMissile(int id);

   void updateAll();
   void drawAll();

   std::vector<Item> border;
};

} //end namespace

//namespace game {
//   int getTicks();
//   float getDt();
//   client::Player &getPlayer(); // available only on client
//};

#endif
