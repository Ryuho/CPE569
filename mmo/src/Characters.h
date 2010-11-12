#ifndef _CHARACTERS_H_
#define _CHARACTERS_H_

#include <vector>
#include <map>
#include "matrix.h"
#include "Geometry.h"

using mat::vec2;
struct Animation;

void initCharacterResources();

struct Player {
   Player() : id(0), alive(false) {}
   Player(int id, vec2 pos, vec2 dir, int hp);
   void move(vec2 pos, vec2 dir, bool moving);
   void stop();

   void update();
   void draw();

   vec2 dir, pos;
   float radius;
   bool moving, alive;
   int animStart, lastUpdate;
   int id;
   int hp;
};

struct Missile {
   Missile(int id, int type, vec2 pos, vec2 dir);

   void update();
   void draw();

   int type;
   bool alive;
   vec2 pos, dir;
   int id;
};

struct Item {
   Item(int id, int type, vec2 pos);

   void update();
   void draw();
   
   int type;
   vec2 pos;
   bool alive;
   Animation *anim;
   int id;
private:
   void initGraphics();
};

struct NPC {
   NPC(int id, int type, vec2 pos, vec2 dir, bool moving);

   void resetAnimation();
   void updateServer(std::vector<mat::vec2> dests);
   void update();
   void draw();
   
   int type;
   vec2 pos, dir;
   bool alive, moving;
   Animation *anim;
   int id;
private:
   void initGraphics();
};

struct ObjectHolder {
   struct IdType {
      IdType() {}
      IdType(int index, int type) : index(index), type(type) {}
      int index, type;
   };

   std::vector<Player> players;
   std::vector<Missile> missiles;
   std::vector<Item> items;
   std::vector<NPC> npcs;
   std::map<int, IdType> idToIndex;

   void addPlayer(Player p);
   void addMissile(Missile m);
   void addItem(Item i);
   void addNPC(NPC n);

   Player *getPlayer(int id);
   Missile *getMissile(int id);
   Item *getItem(int id);
   NPC *getNPC(int id);

   bool checkObject(int id, int type);
   void removeObject(int id);

   void updateAll();
   void drawAll();
};

namespace game {
   int getTicks();
   float getDt();
   Player &getPlayer(); // available only on client
};

#endif
