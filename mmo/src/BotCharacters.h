#ifndef _BOTCHARACTERS_H_
#define _BOTCHARACTERS_H_

#include <vector>
#include <map>
#include "matrix.h"
#include "Geometry.h"
#include "Objects.h"

using mat::vec2;

namespace botclient {

struct Player : objmanager::Object {
   Player() : Object(0), moving(false), alive(true) {}
   Player(int id, vec2 pos, vec2 dir, int hp);
   void move(vec2 pos, vec2 dir, bool moving);

   void update();

   int lastUpdate;
   vec2 dir, pos;
   float radius;
   bool moving, alive;
   int hp;
};


struct Missile : objmanager::Object {
   Missile(int id, int type, vec2 pos, vec2 dir);
   void update();

   int type;
   bool alive;
   vec2 pos, dir;
};

struct Item : objmanager::Object {
   Item(int id, int type, vec2 pos);
   void update();
   bool isCollectable() const;

   int lastUpdate;
   bool alive;
   int type;
   vec2 pos;
};

struct NPC : objmanager::Object {
   NPC(int id, int type, int hp, vec2 pos, vec2 dir, bool moving);
   void update();
   
   int lastUpdate;
   bool moving, alive;
   int type;
   vec2 pos, dir;
   int hp;
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

/*
struct ObjectHolder {
   ObjectHolder() 
      : rm(constants::totalRegions, constants::ObjectType::ObjectTypeCount) {}
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

private:
   objmanager::RegionManager rm;
};
*/

}



#endif //_BOTCHARACTERS_H_
