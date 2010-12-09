#ifndef _CHARACTERS_H_
#define _CHARACTERS_H_

#include "Sprite.h"
#include "Objects.h"
#include "matrix.h"
#include "Geometry.h"
#include "Packets.h"
#include <vector>
#include <map>

namespace client {

using mat::vec2;
using namespace objectManager;

void initCharacterResources();

struct Player : PlayerBase {
   Player() : PlayerBase(0, vec2()), alive(false), moving(false) {}
   Player(int id, vec2 pos, vec2 dir, int hp);
   Player(pack::Packet &p);
   Player(pack::Initialize &ini);
   void deserialize(pack::Packet &p);
   void deserialize(pack::Initialize &ini);

   void move(vec2 pos, vec2 dir, bool moving);
   void update();
   void draw();

   vec2 dir;
   bool moving, alive;
   int animStart, lastUpdate;
   int hp, rupees, exp;
   bool pvp;
};

struct Missile : MissileBase {
   Missile(int id, int type, vec2 pos, vec2 dir);
   Missile(pack::Packet &p);
   Missile(pack::Initialize &ini);
   void deserialize(pack::Packet &p);
   void deserialize(pack::Initialize &ini);

   void move(vec2 pos, vec2 dir);
   void update();
   void draw();

   bool alive;
   vec2 dir;
   int lastUpdate;
};

struct Item : ItemBase {
   Item(int id, int type, vec2 pos);
   Item(pack::Packet &p);
   Item(pack::Initialize &ini);
   void deserialize(pack::Packet &p);
   void deserialize(pack::Initialize &ini);

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
   NPC(pack::Packet &p);
   NPC(pack::Initialize &ini);
   void deserialize(pack::Packet &p);
   void deserialize(pack::Initialize &ini);

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
   ObjectHolder();

   bool add(Player *obj);
   bool add(NPC *obj);
   bool add(Item *obj);
   bool add(Missile *obj);

   Player *getPlayer(int id);
   NPC *getNPC(int id);
   Item *getItem(int id);
   Missile *getMissile(int id);

   void updateAll();
   void drawAll(vec2 pos, bool checkNoDraw);
   void drawAll(bool checkNoDraw);

   std::vector<Item> border[4];
   vec2 corners[4];
};

} //end namespace

//namespace game {
//   int getTicks();
//   float getDt();
//   client::Player &getPlayer(); // available only on client
//};

#endif
