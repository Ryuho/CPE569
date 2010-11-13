#ifndef _BOTCHARACTERS_H_
#define _BOTCHARACTERS_H_

#include <vector>
#include <map>
#include "matrix.h"
#include "Geometry.h"

using mat::vec2;

struct Player {
   Player() : id(0), moving(false), alive(true) {}
   Player(int id, vec2 pos, vec2 dir, int hp);
   void move(vec2 pos, vec2 dir);

   void update();

   vec2 dir, pos;
   float radius;
   bool moving, alive;
   int id;
   int hp;
};


struct Missile {
   Missile(int id, int type, vec2 pos, vec2 dir);
   void update();

   int type;
   bool alive;
   vec2 pos, dir;
   int id;
};

struct Item {
   Item(int id, int type, vec2 pos);
   void update();
   
   bool alive;
   int type;
   vec2 pos;
   int id;
};

struct NPC {
   NPC(int id, int type, vec2 pos, vec2 dir, bool moving);
   void update();
   
   bool moving, alive;
   int type;
   vec2 pos, dir;
   int id;
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
};


namespace game {
   int getTicks();
   float getDt();
   Player &getPlayer(); // available only on client
};



#endif //_BOTCHARACTERS_H_
